/**
 * @file example_esp32s3.cpp
 * @brief Test app for WiFiPan on ESP32-S3.
 *
 * BOOT button (GPIO0):
 *   - Held 3s at startup -> ConfigViaAp() instead of AutoConnect()
 *   - Held 3s any time after -> re-provision (Stop() + ConfigViaAp() again)
 *
 * Onboard WS2812 RGB LED shows status (driven directly via RMT, no
 * external led_strip component needed - only 5 solid colors):
 *   White  : initializing / waiting on BOOT
 *   Blue   : AP / captive portal mode
 *   Yellow : connecting to STA
 *   Green  : connected
 *   Red    : disconnected / error
 *
 * kRgbLedGpio = GPIO48 matches most ESP32-S3-DevKitC-1 boards (some
 * revisions use GPIO38 - check your board).
 */

#include "WiFiPan.hpp"

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace
{
    constexpr const char *kTag = "[EXAMPLE]";

    constexpr gpio_num_t kBootButtonGpio = GPIO_NUM_0;   /* Active LOW */
    constexpr gpio_num_t kRgbLedGpio     = GPIO_NUM_48;
    constexpr uint32_t   kHoldToConfigMs = 3000;
    constexpr uint8_t    kLedBrightness  = 32;

    /* Change before real use - empty token leaves /ota and /reset open. */
    constexpr const char *kAdminToken = "change-me-1234";

    /* WS2812 bit timings over RMT, 10MHz clock (1 tick = 0.1us). */
    constexpr uint32_t kRmtResolutionHz = 10 * 1000 * 1000;
    constexpr uint16_t kWs2812T0hTicks  = 3;
    constexpr uint16_t kWs2812T0lTicks  = 9;
    constexpr uint16_t kWs2812T1hTicks  = 9;
    constexpr uint16_t kWs2812T1lTicks  = 3;

    rmt_channel_handle_t g_led_chan    = nullptr;
    rmt_encoder_handle_t g_led_encoder = nullptr;

    enum class LedColor { Off, White, Blue, Yellow, Green, Red };

    void LedSet(LedColor color)
    {
        if (!g_led_chan || !g_led_encoder) {
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

        uint8_t grb[3] = { g, r, b }; /* WS2812 wire order: G-R-B, MSB first */
        rmt_transmit_config_t tx_cfg = {};
        tx_cfg.loop_count = 0;

        /* Fire-and-forget; queue depth 4 easily absorbs our update rate. */
        rmt_transmit(g_led_chan, g_led_encoder, grb, sizeof(grb), &tx_cfg);
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
        rmt_tx_channel_config_t tx_chan_cfg = {};
        tx_chan_cfg.clk_src           = RMT_CLK_SRC_DEFAULT;
        tx_chan_cfg.gpio_num          = kRgbLedGpio;
        tx_chan_cfg.mem_block_symbols = 64;
        tx_chan_cfg.resolution_hz     = kRmtResolutionHz;
        tx_chan_cfg.trans_queue_depth = 4;

        if (rmt_new_tx_channel(&tx_chan_cfg, &g_led_chan) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to create RMT TX channel, LED disabled");
            g_led_chan = nullptr;
            return;
        }

        rmt_bytes_encoder_config_t bytes_cfg = {};
        bytes_cfg.bit0.level0    = 1;
        bytes_cfg.bit0.duration0 = kWs2812T0hTicks;
        bytes_cfg.bit0.level1    = 0;
        bytes_cfg.bit0.duration1 = kWs2812T0lTicks;
        bytes_cfg.bit1.level0    = 1;
        bytes_cfg.bit1.duration0 = kWs2812T1hTicks;
        bytes_cfg.bit1.level1    = 0;
        bytes_cfg.bit1.duration1 = kWs2812T1lTicks;
        bytes_cfg.flags.msb_first = 1;

        if (rmt_new_bytes_encoder(&bytes_cfg, &g_led_encoder) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to create RMT bytes encoder, LED disabled");
            g_led_chan = nullptr;
            return;
        }

        if (rmt_enable(g_led_chan) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to enable RMT channel, LED disabled");
            g_led_chan = nullptr;
            return;
        }

        LedSet(LedColor::Off);
    }

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

    /* Blocks briefly at startup to check if BOOT is held for kHoldToConfigMs. */
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

    /* Demo params so /config has fields besides SSID/password to test. */
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

    /* Watches BOOT for a 3s hold and re-provisions WiFi when detected. */
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
                pressed = false;
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

        /* Server stops once STA is up; restart it so /config, /ota and
         * /reset stay reachable on the STA network for testing. */
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

    /* Endpoints once connected (admin token needed for the last two):
     *   /        dashboard
     *   /scan    WiFi scan + connect (AP mode)
     *   /config  define / fill custom params
     *   /ota     firmware upload
     *   /reset   erase saved WiFi + reboot to AP
     */
}