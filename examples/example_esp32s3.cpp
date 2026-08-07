/**
 * @file example_esp32s3.cpp
 * @brief Example / test application for the WiFiPan library on ESP32-S3.
 *
 * Exercises: Manager::Init(), AutoConnect(), ConfigViaAp(), Stop(),
 * StartWebServer()/StopWebServer(), IsConnected(), GetMode(),
 * CurrentIpString(), SetConnectedCb/SetDisconnectedCb, SetStaRetryNum,
 * SetScanMaxCount, SetAdminToken, and Page::AddParam/GetParam.
 *
 * BOOT button (GPIO0) behavior:
 *   - Held for 3s right at startup  -> force Manager::ConfigViaAp()
 *     instead of Manager::AutoConnect().
 *   - Held for 3s at any time after boot -> re-provision: Stop() the
 *     current connection and open the config portal again
 *     (ConfigViaAp()), useful for testing without a full reflash.
 *
 * Onboard RGB LED (WS2812, via the "espressif/led_strip" managed
 * component) reports status:
 *   White  (blinking) : initializing / waiting to see if BOOT is held
 *   Blue               : AP / captive-portal mode, waiting for credentials
 *   Yellow             : attempting STA connection
 *   Green              : connected
 *   Red    (blinking)  : disconnected / setup failed
 *
 * Add the LED driver dependency, e.g.:
 *   idf.py add-dependency "espressif/led_strip^2.5.0"
 *
 * kRgbLedGpio below matches most ESP32-S3-DevKitC-1 boards (GPIO48).
 * Some earlier board revisions use GPIO38 - check your board's schematic
 * and adjust if the LED does not light up. The led_strip_config_t /
 * led_strip_rmt_config_t field names below match led_strip v2.x; verify
 * against the version actually resolved by your project if it differs.
 */

#include "WiFiPan.hpp"

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace
{
    constexpr const char *kTag = "[EXAMPLE]";

    /* ---- Board pin configuration - adjust to match your board ---- */
    constexpr gpio_num_t kBootButtonGpio = GPIO_NUM_0;   /* Active LOW */
    constexpr gpio_num_t kRgbLedGpio     = GPIO_NUM_48;  /* Onboard WS2812 */
    constexpr uint32_t   kHoldToConfigMs = 3000;
    constexpr uint8_t    kLedBrightness  = 32;           /* 0-255, kept low */

    /* Admin token required for /ota and /reset. Change before real use -
     * an empty token leaves those endpoints unauthenticated. */
    constexpr const char *kAdminToken = "change-me-1234";

    led_strip_handle_t g_led = nullptr;

    enum class LedColor { Off, White, Blue, Yellow, Green, Red };

    void LedSet(LedColor color)
    {
        if (!g_led) {
            return;
        }
        uint8_t r = 0, g = 0, b = 0;
        switch (color) {
        case LedColor::White:  r = g = b = kLedBrightness; break;
        case LedColor::Blue:   b = kLedBrightness; break;
        case LedColor::Yellow: r = g = kLedBrightness; break;
        case LedColor::Green:  g = kLedBrightness; break;
        case LedColor::Red:    r = kLedBrightness; break;
        case LedColor::Off:
        default:
            break;
        }
        led_strip_set_pixel(g_led, 0, r, g, b);
        led_strip_refresh(g_led);
    }

    void LedBlink(LedColor color, int times, int period_ms)
    {
        for (int i = 0; i < times; i++) {
            LedSet(color);
            vTaskDelay(pdMS_TO_TICKS(period_ms / 2));
            LedSet(LedColor::Off);
            vTaskDelay(pdMS_TO_TICKS(period_ms / 2));
        }
    }

    void InitLed()
    {
        led_strip_config_t strip_cfg = {};
        strip_cfg.strip_gpio_num          = kRgbLedGpio;
        strip_cfg.max_leds                = 1;
        strip_cfg.led_model               = LED_MODEL_WS2812;
        strip_cfg.color_component_format  = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
        strip_cfg.flags.invert_out        = false;

        led_strip_rmt_config_t rmt_cfg = {};
        rmt_cfg.resolution_hz  = 10 * 1000 * 1000;
        rmt_cfg.flags.with_dma = false;

        if (led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &g_led) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to init RGB LED, status reporting disabled");
            g_led = nullptr;
            return;
        }
        led_strip_clear(g_led);
    }

    /* ---- WiFi connection callbacks - plain function pointers, no user ctx ---- */
    void OnWifiConnected()
    {
        ESP_LOGI(kTag, "WiFi connected callback fired");
        LedSet(LedColor::Green);
    }

    void OnWifiDisconnected()
    {
        ESP_LOGW(kTag, "WiFi disconnected callback fired");
        LedSet(LedColor::Red);
    }

    void InitBootButton()
    {
        gpio_config_t io_cfg = {};
        io_cfg.pin_bit_mask = 1ULL << kBootButtonGpio;
        io_cfg.mode         = GPIO_MODE_INPUT;
        io_cfg.pull_up_en   = GPIO_PULLUP_ENABLE;
        gpio_config(&io_cfg);
    }

    /* Blocks briefly at startup to see whether BOOT is held for
     * kHoldToConfigMs, in which case the caller should use ConfigViaAp()
     * instead of AutoConnect(). */
    bool BootButtonHeldForConfig()
    {
        ESP_LOGI(kTag, "Hold BOOT for %lu ms to force the config portal...",
                 static_cast<unsigned long>(kHoldToConfigMs));

        int64_t start = esp_timer_get_time();
        while (gpio_get_level(kBootButtonGpio) == 0) {
            LedSet(LedColor::White);
            vTaskDelay(pdMS_TO_TICKS(50));
            LedSet(LedColor::Off);
            vTaskDelay(pdMS_TO_TICKS(50));

            int64_t held_ms = (esp_timer_get_time() - start) / 1000;
            if (held_ms >= kHoldToConfigMs) {
                ESP_LOGI(kTag, "BOOT held, forcing config portal");
                return true;
            }
        }
        return false;
    }

    /* Registers a couple of dynamic parameters so /config has something to
     * exercise besides the SSID/password fields. */
    void SetupDemoParams(WiFiPan::Manager &wifi)
    {
        wifi.page().AddParam("dev_name", "Device Name", "e.g. sensor-01", "esp32s3-demo", "text", true);
        wifi.page().AddParam("report_interval", "Report Interval (s)", "60", "60", "number", false);
        wifi.page().AddParam("api_key", "API Key", "", "", "password", false);
    }

    void PrintStatusTask(void *arg)
    {
        auto *wifi = static_cast<WiFiPan::Manager *>(arg);
        while (true) {
            ESP_LOGI(kTag, "status: connected=%d mode=%d ip=%s",
                     wifi->IsConnected(),
                     static_cast<int>(wifi->GetMode()),
                     wifi->CurrentIpString().c_str());
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    /* Watches BOOT for a 3s hold at any point after boot and, when
     * detected, re-provisions WiFi by stopping the current connection and
     * opening the captive portal again. Exercises Stop(), StopWebServer(),
     * a second ConfigViaAp() call, and StartWebServer(). */
    void ButtonMonitorTask(void *arg)
    {
        auto *wifi = static_cast<WiFiPan::Manager *>(arg);
        bool pressed = false;
        int64_t press_start = 0;

        while (true) {
            bool low = (gpio_get_level(kBootButtonGpio) == 0);

            if (low && !pressed) {
                pressed = true;
                press_start = esp_timer_get_time();
            } else if (!low && pressed) {
                pressed = false;
            }

            if (pressed && (esp_timer_get_time() - press_start) / 1000 >= kHoldToConfigMs) {
                pressed = false; /* consume this press so it doesn't retrigger */
                ESP_LOGW(kTag, "BOOT held at runtime -> re-provisioning WiFi");

                wifi->StopWebServer();
                wifi->Stop();
                LedSet(LedColor::Blue);

                WiFiPan::Status st = wifi->ConfigViaAp();
                if (st == WiFiPan::Status::Ok) {
                    LedSet(LedColor::Green);
                    wifi->StartWebServer();
                    ESP_LOGI(kTag, "Re-provisioned, IP=%s", wifi->CurrentIpString().c_str());
                } else {
                    LedSet(LedColor::Red);
                    ESP_LOGE(kTag, "Re-provisioning failed: %d", static_cast<int>(st));
                }
            }

            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }

} // namespace

extern "C" void app_main()
{
    InitLed();
    InitBootButton();
    LedSet(LedColor::White);

    static WiFiPan::Manager wifi;

    bool force_config = BootButtonHeldForConfig();

    WiFiPan::Status st = wifi.Init();
    if (st != WiFiPan::Status::Ok) {
        ESP_LOGE(kTag, "Init failed: %d", static_cast<int>(st));
        LedBlink(LedColor::Red, 10, 300);
        return;
    }

    wifi.SetConnectedCb(OnWifiConnected);
    wifi.SetDisconnectedCb(OnWifiDisconnected);
    wifi.SetStaRetryNum(5);
    wifi.SetScanMaxCount(15);
    wifi.SetAdminToken(kAdminToken);

    SetupDemoParams(wifi);

    if (force_config) {
        LedSet(LedColor::Blue);
        st = wifi.ConfigViaAp();
    } else {
        LedSet(LedColor::Yellow);
        st = wifi.AutoConnect();
    }

    if (st == WiFiPan::Status::Ok) {
        LedSet(LedColor::Green);
        ESP_LOGI(kTag, "Connected. IP=%s", wifi.CurrentIpString().c_str());

        /* AutoConnect()/ConfigViaAp() stop the HTTP server once STA is up.
         * Start it again so /config, /ota and /reset stay reachable on the
         * STA network for continued testing. */
        wifi.StartWebServer();
    } else {
        LedBlink(LedColor::Red, 6, 400);
        ESP_LOGE(kTag, "WiFi setup failed: %d", static_cast<int>(st));
    }

    ESP_LOGI(kTag, "dev_name=%s report_interval=%s",
             wifi.page().GetParam("dev_name"),
             wifi.page().GetParam("report_interval"));

    xTaskCreate(PrintStatusTask,   "wifi_status", 3072, &wifi, 3, nullptr);
    xTaskCreate(ButtonMonitorTask, "wifi_button", 4096, &wifi, 3, nullptr);

    /* Endpoints available once connected (admin token required for the
     * last two):
     *   http://<device-ip>/            dashboard
     *   http://<device-ip>/scan        WiFi scan + connect (AP mode)
     *   http://<device-ip>/config      define / fill custom params
     *   http://<device-ip>/ota         firmware upload
     *   http://<device-ip>/reset       erase saved WiFi + reboot to AP
     */
}