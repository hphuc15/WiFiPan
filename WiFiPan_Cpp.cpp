#include "WiFiPan_Internal.h"

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
#include <cstring>

namespace WiFiPan
{
    namespace
    {
        constexpr const char *kTagInit  = "[WP][INIT]";
        constexpr const char *kTagSta   = "[WP][STA]";
        constexpr const char *kTagAp    = "[WP][AP]";
        constexpr const char *kTagStop  = "[WP][STOP]";
        constexpr const char *kTagAuto  = "[WP][AUTO]";
        constexpr const char *kTagPortal = "[WP][PORTAL]";
    } // namespace

    wifi_ap_config_t Manager::DefaultApConfig()
    {
        wifi_ap_config_t cfg{};
        std::memcpy(cfg.ssid, kApSsidDefault, std::strlen(kApSsidDefault));
        cfg.ssid_len = std::strlen(kApSsidDefault);
        std::memcpy(cfg.password, kApPasswordDefault, std::strlen(kApPasswordDefault));
        cfg.max_connection = kApMaxStaCon;
        cfg.authmode = std::strlen(kApPasswordDefault) ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;
        return cfg;
    }

    Manager::Manager() = default;

    /* ===================== Event trampolines / handlers ===================== */


    void Manager::StaEventTrampoline(void *arg, esp_event_base_t base, int32_t id, void *data)
    {
        static_cast<Manager *>(arg)->HandleStaEvent(base, id, data);
    }

    void Manager::ApEventTrampoline(void *arg, esp_event_base_t base, int32_t id, void *data)
    {
        static_cast<Manager *>(arg)->HandleApEvent(base, id, data);
    }

    void Manager::HandleStaEvent(esp_event_base_t base, int32_t id, void *data)
    {
        if (base == WIFI_EVENT) {
            switch (id) {
            case WIFI_EVENT_STA_START: {
                xEventGroupSetBits(priv_->group, kEventBitStaStart);
                priv_->sta_retry_remaining = sta_retry_num_;
                esp_err_t err = esp_wifi_connect();
                if (err != ESP_OK) {
                    ESP_LOGE(kTagSta, "Failed to connect: %s", esp_err_to_name(err));
                }
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED: {
                if (!(xEventGroupGetBits(priv_->group) & kEventBitStaStart)) {
                    break;
                }
                auto *event = static_cast<wifi_event_sta_disconnected_t *>(data);
                ESP_LOGE(kTagSta, "Disconnected, reason: %d", event->reason);

                if (!(xEventGroupGetBits(priv_->group) & kEventBitStaDisconnected)) {
                    if (disconnected_cb_) {
                        disconnected_cb_();
                    }
                }
                xEventGroupClearBits(priv_->group, kEventBitStaConnected);
                xEventGroupSetBits(priv_->group, kEventBitStaDisconnected);

                if (priv_->sta_retry_remaining != 0) {
                    if (priv_->sta_retry_remaining > 0) {
                        priv_->sta_retry_remaining--;
                    }
                    ESP_LOGW(kTagSta, "Retrying... (%d left)", priv_->sta_retry_remaining);
                    esp_wifi_connect();
                } else {
                    priv_->sta_retry_remaining = sta_retry_num_;
                }
                break;
            }
            default:
                break;
            }
        } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
            priv_->sta_retry_remaining = sta_retry_num_;
            xEventGroupClearBits(priv_->group, kEventBitStaDisconnected);
            xEventGroupSetBits(priv_->group, kEventBitStaConnected);
            auto *event = static_cast<ip_event_got_ip_t *>(data);
            ESP_LOGI(kTagSta, "Connected. IP: " IPSTR, IP2STR(&event->ip_info.ip));
            if (connected_cb_) {
                connected_cb_();
            }
        }
    }

    void Manager::HandleApEvent(esp_event_base_t base, int32_t id, void *data)
    {
        if (base != WIFI_EVENT) {
            return;
        }
        switch (id) {
        case WIFI_EVENT_AP_START: {
            xEventGroupSetBits(priv_->group, kEventBitApStart);

            esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(ap_netif, &ip_info) != ESP_OK) {
                return;
            }
            ESP_LOGI(kTagAp, "AP START. SSID: %s, IP: " IPSTR ".",
                     reinterpret_cast<char *>(ap_config_.ssid), IP2STR(&ip_info.ip));
            break;
        }
        case WIFI_EVENT_AP_STACONNECTED: {
            auto *event = static_cast<wifi_event_ap_staconnected_t *>(data);
            ESP_LOGI(kTagAp, "Station [" MACSTR "] joined.", MAC2STR(event->mac));
            break;
        }
        case WIFI_EVENT_AP_STADISCONNECTED: {
            auto *event = static_cast<wifi_event_ap_staconnected_t *>(data);
            ESP_LOGI(kTagAp, "Station [" MACSTR "] left.", MAC2STR(event->mac));
            break;
        }
        case WIFI_EVENT_AP_STOP:
            xEventGroupClearBits(priv_->group, kEventBitApStart);
            ESP_LOGI(kTagAp, "AP STOPPED.");
            break;
        default:
            break;
        }
    }

    bool Manager::LoadSavedCredsFromNvs()
    {
        wifi_config_t saved_config;
        esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &saved_config);
        if (err != ESP_OK) {
            ESP_LOGE(kTagAuto, "Failed to get config from NVS: %s", esp_err_to_name(err));
            return false;
        }

        if (std::strlen(reinterpret_cast<char *>(saved_config.sta.ssid)) == 0) {
            ESP_LOGW(kTagAuto, "No saved SSID found in NVS");
            return false;
        }

        if (std::strcmp(reinterpret_cast<char *>(saved_config.sta.ssid), kApSsidDefault) == 0) {
            ESP_LOGW(kTagAuto, "Saved SSID matches config AP (%s), ignoring",
                      reinterpret_cast<char *>(saved_config.sta.ssid));
            return false;
        }

        sta_config_ = saved_config.sta;
        ESP_LOGI(kTagAuto, "Loaded saved SSID: %s", reinterpret_cast<char *>(sta_config_.ssid));
        return true;
    }

    /* ===================== Public: WiFi lifecycle ===================== */

    Status Manager::Init()
    {
        if (!priv_) {
            priv_ = std::make_unique<Impl>();
        }

        esp_err_t ret = esp_netif_init();
        if (ret != ESP_OK) {
            ESP_LOGE(kTagInit, "netif init failed: %s", esp_err_to_name(ret));
            return Status::NetifError;
        }

        if (!priv_->group) {
            priv_->group = xEventGroupCreate();
            if (!priv_->group) {
                ESP_LOGE(kTagInit, "Failed to create event group");
                return Status::NoMem;
            }
        }

        ret = esp_event_loop_create_default();
        if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGW(kTagInit, "Default event loop already created");
        } else if (ret != ESP_OK) {
            ESP_LOGE(kTagInit, "Event loop create failed: %s", esp_err_to_name(ret));
            return Status::InitFailed;
        }

        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ret = nvs_flash_erase();
            if (ret != ESP_OK) {
                ESP_LOGE(kTagInit, "NVS erase failed: %s", esp_err_to_name(ret));
                return Status::InitFailed;
            }
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            ESP_LOGE(kTagInit, "NVS init failed: %s", esp_err_to_name(ret));
            return Status::InitFailed;
        }

        wifi_mode_t mode;
        if (esp_wifi_get_mode(&mode) != ESP_ERR_WIFI_NOT_INIT) {
            Stop();
        }

        wifi_init_config_t wifi_drv_cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret = esp_wifi_init(&wifi_drv_cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(kTagInit, "WiFi driver init failed: %s", esp_err_to_name(ret));
            return Status::WifiError;
        }

        return Status::Ok;
    }

    Status Manager::StartSta()
    {
        esp_err_t ret;

        xEventGroupClearBits(priv_->group, kEventBitStaConnected | kEventBitStaDisconnected);

        if (priv_->netif) {
            esp_netif_destroy_default_wifi(priv_->netif);
        }
        priv_->netif = esp_netif_create_default_wifi_sta();
        if (!priv_->netif) {
            return Status::NetifError;
        }

        ret = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, &Manager::StaEventTrampoline, this, &priv_->sta_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &Manager::StaEventTrampoline, this, &priv_->sta_disc_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Manager::StaEventTrampoline, this, &priv_->ip_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_wifi_set_mode(WIFI_MODE_STA);
        if (ret != ESP_OK) return Status::WifiError;

        if (std::strlen(reinterpret_cast<char *>(sta_config_.ssid)) > 0) {
            wifi_config_t config{};
            config.sta = sta_config_;
            ret = esp_wifi_set_config(WIFI_IF_STA, &config);
            if (ret != ESP_OK) return Status::WifiError;
        }

        ret = esp_wifi_start();
        if (ret != ESP_OK) return Status::WifiError;

        vTaskDelay(pdMS_TO_TICKS(100));
        return Status::Ok;
    }

    Status Manager::StartAp()
    {
        esp_err_t ret;

        xEventGroupClearBits(priv_->group, kEventBitApStart);

        if (priv_->netif) {
            esp_netif_destroy_default_wifi(priv_->netif);
        }
        priv_->netif = esp_netif_create_default_wifi_ap();
        if (!priv_->netif) {
            return Status::NetifError;
        }

        ret = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &Manager::ApEventTrampoline, this, &priv_->ap_connected_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &Manager::ApEventTrampoline, this, &priv_->ap_disconnected_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_START, &Manager::ApEventTrampoline, this, &priv_->ap_start_handle);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_wifi_set_mode(WIFI_MODE_AP);
        if (ret != ESP_OK) return Status::WifiError;

        wifi_config_t config{};
        config.ap = ap_config_;
        ret = esp_wifi_set_config(WIFI_IF_AP, &config);
        if (ret != ESP_OK) return Status::WifiError;

        ret = esp_wifi_start();
        if (ret != ESP_OK) return Status::WifiError;

        vTaskDelay(pdMS_TO_TICKS(100));
        return Status::Ok;
    }

    Status Manager::Stop()
    {
        if (!priv_) {
            ESP_LOGE(kTagStop, "Manager not initialized");
            return Status::InvalidArg;
        }

        wifi_mode_t mode;
        esp_err_t err = esp_wifi_get_mode(&mode);
        if (err != ESP_OK) {
            ESP_LOGW(kTagStop, "Failed to get WiFi mode: %s", esp_err_to_name(err));
            mode = WIFI_MODE_NULL;
        }
        ESP_LOGI(kTagStop, "Stopping WiFi (mode: %d)", mode);

        if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
            err = esp_wifi_disconnect();
            if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED) {
                ESP_LOGW(kTagStop, "Disconnect failed: %s", esp_err_to_name(err));
            }
        }

        err = esp_wifi_stop();
        if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STARTED) {
            ESP_LOGE(kTagStop, "Failed to stop WiFi: %s", esp_err_to_name(err));
        }

        auto unreg = [](esp_event_base_t base, int32_t id, esp_event_handler_instance_t &h) {
            if (h) {
                esp_event_handler_instance_unregister(base, id, h);
                h = nullptr;
            }
        };
        unreg(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, priv_->ap_connected_handle);
        unreg(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, priv_->ap_disconnected_handle);
        unreg(WIFI_EVENT, WIFI_EVENT_AP_START, priv_->ap_start_handle);
        unreg(WIFI_EVENT, WIFI_EVENT_STA_START, priv_->sta_handle);
        unreg(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, priv_->sta_disc_handle);
        unreg(IP_EVENT, IP_EVENT_STA_GOT_IP, priv_->ip_handle);

        xEventGroupClearBits(priv_->group, kEventBitApStart | kEventBitStaStart | kEventBitStaConnected | kEventBitStaDisconnected);

        if (priv_->netif) {
            esp_netif_destroy_default_wifi(priv_->netif);
            priv_->netif = nullptr;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_LOGI(kTagStop, "WiFi stopped.");
        return Status::Ok;
    }

    Manager::~Manager()
    {
        if (!priv_) {
            return;
        }

        Stop();

        esp_err_t err = esp_wifi_deinit();
        if (err == ESP_ERR_WIFI_NOT_INIT) {
            ESP_LOGE(kTagStop, "WiFi driver was not installed");
        }

        err = nvs_flash_deinit();
        if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
            ESP_LOGE(kTagStop, "NVS storage not initialized");
        }

        err = esp_event_loop_delete_default();
        if (err != ESP_OK) {
            ESP_LOGE(kTagStop, "Event loop delete failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_LOGI(kTagStop, "WiFiPan::Manager destroyed");
    }

    Status Manager::ConfigViaAp()
    {
        if (std::strlen(reinterpret_cast<char *>(ap_config_.ssid)) == 0) {
            ap_config_ = DefaultApConfig();
        }

        size_t pass_len = std::strlen(reinterpret_cast<char *>(ap_config_.password));
        if (pass_len > 0 && pass_len < 8) {
            ESP_LOGE(kTagPortal, "AP password must be empty (open) or >= 8 chars, got %zu", pass_len);
            return Status::WeakApPassword;
        }

        Status st = StartAp();
        if (st != Status::Ok) {
            return st;
        }

        EventBits_t bits = xEventGroupWaitBits(priv_->group, kEventBitApStart, pdFALSE, pdFALSE, pdMS_TO_TICKS(kApStartTimeoutMs));
        if (!(bits & kEventBitApStart)) {
            ESP_LOGE(kTagPortal, "Timeout: AP failed to start");
            Stop();
            return Status::Timeout;
        }
        vTaskDelay(pdMS_TO_TICKS(200));

        esp_wifi_set_mode(WIFI_MODE_APSTA); /* Scan only works in STA or APSTA mode */

        SetCaptivePortalUri();
        StartWebServer();

        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(priv_->netif, &ip_info);
        void *dns = StartDns(ip_info.ip);

        /* Wait for WiFi credentials specifically - /configsave (extra params only)
        * no longer notifies this task, so we only wake up once an SSID has
        * actually been submitted. */
        priv_->portal_waiting_task = xTaskGetCurrentTaskHandle();
        BaseType_t notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(kPortalTimeoutMs));
        priv_->portal_waiting_task = nullptr;

        if (!notified) {
            ESP_LOGE(kTagPortal, "Timeout: no credentials received");
            StopDns(dns);
            StopWebServer();
            Stop();
            return Status::Timeout;
        }

        /* Defense in depth: even though the handler only notifies when ssid is
        * non-empty, double-check here before switching to STA. */
        if (std::strlen(reinterpret_cast<char *>(sta_config_.ssid)) == 0) {
            ESP_LOGE(kTagPortal, "Notified but SSID is empty - aborting");
            StopDns(dns);
            StopWebServer();
            Stop();
            return Status::NoCreds;
        }

        vTaskDelay(pdMS_TO_TICKS(300));

        StopDns(dns);
        StopWebServer();
        Stop();
        vTaskDelay(pdMS_TO_TICKS(500));

        /* Sanitize credentials before connecting. */
        sta_config_.ssid[sizeof(sta_config_.ssid) - 1] = '\0';
        sta_config_.password[sizeof(sta_config_.password) - 1] = '\0';
        sta_config_.threshold.authmode = WIFI_AUTH_OPEN;
        sta_config_.scan_method = WIFI_ALL_CHANNEL_SCAN;

        ESP_LOGI(kTagPortal, "Switching to STA...");
        Status sta_status = StartSta();
        if (sta_status != Status::Ok) {
            return sta_status;
        }

        /* NEW: actually wait for the connection to succeed instead of trusting
        * StartSta()'s ESP_OK, which only means the driver started - not that
        * association+DHCP finished. */
        EventBits_t conn_bits = xEventGroupWaitBits(
            priv_->group, kEventBitStaConnected | kEventBitStaDisconnected,
            pdFALSE, pdFALSE, pdMS_TO_TICKS(kStaConnectTimeoutMs));

        if (conn_bits & kEventBitStaConnected) {
            return Status::Ok;
        }

        ESP_LOGE(kTagPortal, "Provisioned SSID '%s' did not connect",
                reinterpret_cast<char *>(sta_config_.ssid));
        return Status::ConnectFailed;
    }

    Status Manager::AutoConnect()
    {
        if (!priv_) {
            ESP_LOGE(kTagAuto, "Manager not initialized");
            return Status::InvalidArg;
        }

        ESP_LOGI(kTagAuto, "Loading saved credentials from NVS...");

        if (!LoadSavedCredsFromNvs()) {
            ESP_LOGI(kTagAuto, "No valid saved config, starting AP configuration mode");
            return ConfigViaAp();
        }

        ESP_LOGI(kTagAuto, "Found SSID: %s, attempting to connect...", reinterpret_cast<char *>(sta_config_.ssid));

        Status st = StartSta();
        if (st != Status::Ok) {
            return st;
        }

        TickType_t timeout = (sta_retry_num_ == -1) ? portMAX_DELAY : pdMS_TO_TICKS(kStaConnectTimeoutMs);
        EventBits_t bits = xEventGroupWaitBits(priv_->group, kEventBitStaConnected | kEventBitStaDisconnected, pdFALSE, pdFALSE, timeout);

        if (bits & kEventBitStaConnected) {
            ESP_LOGI(kTagAuto, "Connected to: %s", reinterpret_cast<char *>(sta_config_.ssid));
            return Status::Ok;
        }

        ESP_LOGW(kTagAuto, "Failed to connect to: %s, falling back to portal", reinterpret_cast<char *>(sta_config_.ssid));
        Stop();
        return ConfigViaAp();
    }

    wifi_mode_t Manager::GetMode() const
    {
        wifi_mode_t mode = WIFI_MODE_NULL;
        esp_wifi_get_mode(&mode);
        return mode;
    }

    bool Manager::IsConnected() const
    {
        if (!priv_ || !priv_->group) {
            return false;
        }
        return (xEventGroupGetBits(priv_->group) & kEventBitStaConnected) != 0;
    }

    void Manager::NotifyWifiSubmitted()
    {
        if (priv_ && priv_->portal_waiting_task) {
            xTaskNotifyGive(priv_->portal_waiting_task);
        }
    }

} /* WiFiPan */