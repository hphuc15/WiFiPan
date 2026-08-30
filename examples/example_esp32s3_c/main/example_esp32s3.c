/**
 * @file example_esp32s3.c
 * @brief Pure-C test app for the WiFiPan_C wrapper on ESP32-S3.
 *
 * Exercises: WiFiPan_Create/destroy, WiFiPan_Init, WiFiPan_AutoConnect,
 * WiFiPan_ConfigViaAp, WiFiPan_Stop, WiFiPan_StartWebServer /
 * WiFiPan_StopWebServer, WiFiPan_IsConnected, WiFiPan_GetMode,
 * WiFiPan_CurrentIpString, WiFiPan_SetConnectedCb /
 * WiFiPan_SetDisconnectedCb, WiFiPan_SetStaRetryNum,
 * WiFiPan_SetScanMaxCount, WiFiPan_SetAdminToken, and
 * WiFiPan_Page_AddParam / WiFiPan_Page_GetParam.
 *
 * BOOT button (GPIO0):
 *   - Held 3s at startup -> WiFiPan_ConfigViaAp() instead of auto_connect()
 *   - Held 3s any time after -> re-provision (stop + config_via_ap again)
 *
 * Onboard WS2812 RGB LED shows status (driven directly via RMT, same as
 * the C++ example - no external led_strip component needed):
 *   White  : initializing / waiting on BOOT
 *   Blue   : AP / captive portal mode
 *   Yellow : connecting to STA
 *   Green  : connected
 *   Red    : disconnected / error
 *
 * kRgbLedGpio = GPIO48 matches most ESP32-S3-DevKitC-1 boards (some
 * revisions use GPIO38 - check your board).
 *
 * NOTE: WiFiPan_SetAdminToken() and WiFiPan_StartWebServer() are commented
 * out below for now - /ota and /reset run unauthenticated, and the portal
 * doesn't stay reachable once STA connects. Re-enable both together before
 * relying on those endpoints.
 */

#include "WiFiPan.h"

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "[EXAMPLE_C]";

/* ---- Board pin configuration ---- */
#define BOOT_BUTTON_GPIO GPIO_NUM_0 /* Active LOW */
#define RGB_LED_GPIO GPIO_NUM_48
#define HOLD_TO_CONFIG_MS 3000
#define LED_BRIGHTNESS 32

/* Change before real use - empty token leaves /ota and /reset open.
 * Currently unused: WiFiPan_SetAdminToken() call is commented out below. */
// static const char *kAdminToken = "ota-token";

/* WS2812 bit timings over RMT, 10MHz clock (1 tick = 0.1us). */
#define RMT_RESOLUTION_HZ (10 * 1000 * 1000)
#define WS2812_T0H_TICKS 3
#define WS2812_T0L_TICKS 9
#define WS2812_T1H_TICKS 9
#define WS2812_T1L_TICKS 3

typedef enum
{
    LED_OFF,
    LED_WHITE,
    LED_BLUE,
    LED_YELLOW,
    LED_GREEN,
    LED_RED
} led_color_t;

static rmt_channel_handle_t g_led_chan    = NULL;
static rmt_encoder_handle_t g_led_encoder = NULL;

static void LedSet(led_color_t color)
{
    if (!g_led_chan || !g_led_encoder)
    {
        return;
    }
    uint8_t r = 0, g = 0, b = 0;
    switch (color)
    {
        case LED_WHITE:
            r = g = b = LED_BRIGHTNESS;
            break;
        case LED_BLUE:
            b = LED_BRIGHTNESS;
            break;
        case LED_YELLOW:
            r = g = LED_BRIGHTNESS;
            break;
        case LED_GREEN:
            g = LED_BRIGHTNESS;
            break;
        case LED_RED:
            r = LED_BRIGHTNESS;
            break;
        case LED_OFF:
        default:
            break;
    }

    uint8_t               grb[3] = {g, r, b}; /* WS2812 wire order: G-R-B, MSB first */
    rmt_transmit_config_t tx_cfg = {0};

    /* Fire-and-forget; queue depth 4 easily absorbs our update rate. */
    rmt_transmit(g_led_chan, g_led_encoder, grb, sizeof(grb), &tx_cfg);
}

static void LedBlink(led_color_t color, int times, int period_ms)
{
    for (int i = 0; i < times; i++)
    {
        LedSet(color);
        vTaskDelay(pdMS_TO_TICKS(period_ms / 2));
        LedSet(LED_OFF);
        vTaskDelay(pdMS_TO_TICKS(period_ms / 2));
    }
}

static void InitLed(void)
{
    rmt_tx_channel_config_t tx_chan_cfg = {0};
    tx_chan_cfg.clk_src                 = RMT_CLK_SRC_DEFAULT;
    tx_chan_cfg.gpio_num                = RGB_LED_GPIO;
    tx_chan_cfg.mem_block_symbols       = 64;
    tx_chan_cfg.resolution_hz           = RMT_RESOLUTION_HZ;
    tx_chan_cfg.trans_queue_depth       = 4;

    if (rmt_new_tx_channel(&tx_chan_cfg, &g_led_chan) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create RMT TX channel, LED disabled");
        g_led_chan = NULL;
        return;
    }

    rmt_bytes_encoder_config_t bytes_cfg = {0};
    bytes_cfg.bit0.level0                = 1;
    bytes_cfg.bit0.duration0             = WS2812_T0H_TICKS;
    bytes_cfg.bit0.level1                = 0;
    bytes_cfg.bit0.duration1             = WS2812_T0L_TICKS;
    bytes_cfg.bit1.level0                = 1;
    bytes_cfg.bit1.duration0             = WS2812_T1H_TICKS;
    bytes_cfg.bit1.level1                = 0;
    bytes_cfg.bit1.duration1             = WS2812_T1L_TICKS;
    bytes_cfg.flags.msb_first            = 1;

    if (rmt_new_bytes_encoder(&bytes_cfg, &g_led_encoder) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create RMT bytes encoder, LED disabled");
        g_led_chan = NULL;
        return;
    }

    if (rmt_enable(g_led_chan) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable RMT channel, LED disabled");
        g_led_chan = NULL;
        return;
    }

    LedSet(LED_OFF);
}

/* WiFiPan connect/disconnect callbacks - plain C function pointers. */
static void OnWifiConnected(void)
{
    ESP_LOGI(TAG, "WiFi connected callback fired");
    LedSet(LED_GREEN);
}

static void OnWifiDisconnected(void)
{
    ESP_LOGW(TAG, "WiFi disconnected callback fired");
    LedSet(LED_RED);
}

static void InitBootButton(void)
{
    gpio_config_t io_cfg = {0};
    io_cfg.pin_bit_mask  = 1ULL << BOOT_BUTTON_GPIO;
    io_cfg.mode          = GPIO_MODE_INPUT;
    io_cfg.pull_up_en    = GPIO_PULLUP_ENABLE;
    gpio_config(&io_cfg);
}

/* Blocks briefly at startup to check if BOOT is held for HOLD_TO_CONFIG_MS. */
static bool BootButtonHeldForConfig(void)
{
    ESP_LOGI(TAG, "Hold BOOT for %d ms to force the config portal...", HOLD_TO_CONFIG_MS);

    int64_t start = esp_timer_get_time();
    while (gpio_get_level(BOOT_BUTTON_GPIO) == 0)
    {
        LedSet(LED_WHITE);
        vTaskDelay(pdMS_TO_TICKS(50));
        LedSet(LED_OFF);
        vTaskDelay(pdMS_TO_TICKS(50));

        int64_t held_ms = (esp_timer_get_time() - start) / 1000;
        if (held_ms >= HOLD_TO_CONFIG_MS)
        {
            ESP_LOGI(TAG, "BOOT held, forcing config portal");
            return true;
        }
    }
    return false;
}

/* Demo params so /config has fields besides SSID/password to test. */
static void SetupDemoParams(WiFiPan_t *wifi)
{
    WiFiPan_Page_AddParam(wifi, "dev_name", "Device Name", "e.g. sensor-01", "esp32s3-demo", "text", true);
    WiFiPan_Page_AddParam(wifi, "report_interval", "Report Interval (s)", "60", "60", "number", false);
    WiFiPan_Page_AddParam(wifi, "api_key", "API Key", "", "", "password", false);
}

static void PrintStatusTask(void *arg)
{
    WiFiPan_t *wifi = (WiFiPan_t *)arg;
    char       ip[16];
    while (true)
    {
        WiFiPan_CurrentIpString(wifi, ip, sizeof(ip));
        ESP_LOGI(TAG, "status: connected=%d mode=%d ip=%s", WiFiPan_IsConnected(wifi), (int)WiFiPan_GetMode(wifi), ip);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/* Watches BOOT for a 3s hold and re-provisions WiFi when detected. */
static void ButtonMonitorTask(void *arg)
{
    WiFiPan_t *wifi        = (WiFiPan_t *)arg;
    bool       pressed     = false;
    int64_t    press_start = 0;

    while (true)
    {
        bool low = (gpio_get_level(BOOT_BUTTON_GPIO) == 0);

        if (low && !pressed)
        {
            pressed     = true;
            press_start = esp_timer_get_time();
        }
        else if (!low && pressed)
        {
            pressed = false;
        }

        if (pressed && (esp_timer_get_time() - press_start) / 1000 >= HOLD_TO_CONFIG_MS)
        {
            pressed = false;
            ESP_LOGW(TAG, "BOOT held at runtime -> re-provisioning WiFi");

            WiFiPan_StopWebServer(wifi);
            WiFiPan_Stop(wifi);
            LedSet(LED_BLUE);

            WiFiPan_Status st = WiFiPan_ConfigViaAp(wifi);
            if (st == WIFIPAN_OK)
            {
                LedSet(LED_GREEN);
                // WiFiPan_StartWebServer(wifi);
                char ip[16];
                WiFiPan_CurrentIpString(wifi, ip, sizeof(ip));
                ESP_LOGI(TAG, "Re-provisioned, IP=%s", ip);
                ESP_LOGI(TAG, "dev_name=%s report_interval=%s", WiFiPan_Page_GetParam(wifi, "dev_name"),
                         WiFiPan_Page_GetParam(wifi, "report_interval"));
            }
            else
            {
                LedSet(LED_RED);
                ESP_LOGE(TAG, "Re-provisioning failed: %d", (int)st);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    InitLed();
    InitBootButton();
    LedSet(LED_WHITE);

    WiFiPan_t *wifi = WiFiPan_Create();
    if (!wifi)
    {
        ESP_LOGE(TAG, "WiFiPan_Create failed (OOM)");
        LedBlink(LED_RED, 10, 300);
        return;
    }

    bool force_config = BootButtonHeldForConfig();

    WiFiPan_Status st = WiFiPan_Init(wifi);
    if (st != WIFIPAN_OK)
    {
        ESP_LOGE(TAG, "Init failed: %d", (int)st);
        LedBlink(LED_RED, 10, 300);
        WiFiPan_Destroy(wifi);
        return;
    }

    WiFiPan_SetConnectedCb(wifi, OnWifiConnected);
    WiFiPan_SetDisconnectedCb(wifi, OnWifiDisconnected);
    WiFiPan_SetStaRetryNum(wifi, 5);
    WiFiPan_SetScanMaxCount(wifi, 15);
    // WiFiPan_SetAdminToken(wifi, kAdminToken);

    SetupDemoParams(wifi);

    if (force_config)
    {
        LedSet(LED_BLUE);
        st = WiFiPan_ConfigViaAp(wifi);
    }
    else
    {
        LedSet(LED_YELLOW);
        st = WiFiPan_AutoConnect(wifi);
    }

    if (st == WIFIPAN_OK)
    {
        LedSet(LED_GREEN);
        char ip[16];
        WiFiPan_CurrentIpString(wifi, ip, sizeof(ip));
        ESP_LOGI(TAG, "Connected. IP=%s", ip);

        /* Server stops once STA is up; restart it so /config, /ota and
         * /reset stay reachable on the STA network for testing. */
        // WiFiPan_StartWebServer(wifi);
    }
    else
    {
        LedBlink(LED_RED, 6, 400);
        ESP_LOGE(TAG, "WiFi setup failed: %d", (int)st);
    }

    ESP_LOGI(TAG, "dev_name=%s report_interval=%s", WiFiPan_Page_GetParam(wifi, "dev_name"),
             WiFiPan_Page_GetParam(wifi, "report_interval"));

    xTaskCreate(PrintStatusTask, "wifi_status", 3072, wifi, 3, NULL);
    xTaskCreate(ButtonMonitorTask, "wifi_button", 4096, wifi, 3, NULL);

    /* Endpoints once connected (admin token needed for the last two):
     *   /        dashboard
     *   /scan    WiFi scan + connect (AP mode)
     *   /config  define / fill custom params
     *   /ota     firmware upload
     *   /reset   erase saved WiFi + reboot to AP
     */
}