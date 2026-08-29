#ifndef WIFIPAN_HPP_
#define WIFIPAN_HPP_

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <array>
#include <string>
#include <memory>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_http_server.h"

namespace WiFiPan
{
    /* Configuration constants */
    inline constexpr const char *kApSsidDefault = "ESP32_Config";
    inline constexpr const char *kApPasswordDefault = "ESP32_Config";
    inline constexpr uint8_t kApMaxStaCon = 1;
    inline constexpr size_t kMaxParams = 10;
    inline constexpr size_t kFieldLen = 128;
    inline constexpr size_t kPortalBodySize = 1024;
    inline constexpr uint32_t kPortalTimeoutMs = 5UL * 60UL * 1000UL;
    inline constexpr uint16_t kScanDefaultMax = 10;
    inline constexpr uint32_t kApStartTimeoutMs    = 30UL * 1000UL;
    inline constexpr uint32_t kStaConnectTimeoutMs = 30UL * 1000UL;
    inline constexpr size_t kAdminTokenLen = 32;

    /* Status code */
    enum class Status
    {
        Ok = 0,
        InvalidArg = -1,
        InitFailed = -2,
        WifiError = -3,
        Timeout = -4,
        NoCreds = -5,
        NetifError = -6,
        NoMem = -7,
        ConnectFailed = -8,
        WeakApPassword = -9
    };

    /* Callbacks */
    using ConnectedCb = void (*)(void);
    using DisconnectedCb = void (*)(void);

    class Page
    {
    public:
        struct Param
        {
            std::array<char, kFieldLen> id{};
            std::array<char, kFieldLen> label{};
            std::array<char, kFieldLen> placeholder{};
            std::array<char, kFieldLen> value{};
            std::array<char, 16> type{};
            bool required = false;
        };

        /** @brief Add a dynamic input field to the portal form. */
        Status AddParam(const char *id, const char *label, const char *placeholder, const char *value, const char *type, bool required);

        /** @brief Look up a field value by id. Returns nullptr if not found. */
        const char *GetParam(const char *id) const;

        /**
         * @brief Update the value of an existing field (called by the /config-save
         * POST handler). Does nothing if no field with this id exists.
         * @return true if a matching field was found and updated.
         */
        bool SetParamValue(const char *id, const char *value);

        /** Build the dynamic extra-field HTML fragment for /config. */
        std::string BuildFieldsHtml() const;

        /** Build the "<script>window.wifiparam_schema=...</script>" injection. */
        std::string BuildConfigInject() const;

        size_t count() const { return used_; }
        const Param &operator[](size_t i) const { return params_[i]; }

    private:
        std::array<Param, kMaxParams> params_{};
        size_t used_ = 0;
    };

    class Manager
    {
    public:
        Manager();
        ~Manager();

        Manager(const Manager &) = delete;
        Manager &operator=(const Manager &) = delete;
        Manager(Manager &&) = delete;
        Manager &operator=(Manager &&) = delete;

        /* ---------------- WiFi lifecycle ---------------- */

        /** Initialize NVS, event loop, and the WiFi driver. */
        Status Init();
        /** Start WiFi in STA mode. */
        Status StartSta();
        /** Start WiFi in AP mode. */
        Status StartAp();
        /**
         * Open a captive portal AP, collect credentials, then switch to STA.
         * Blocks until the form is submitted or kPortalTimeoutMs elapses.
         */
        Status ConfigViaAp();
        /** Connect using saved NVS credentials; falls back to ConfigViaAp() on failure. */
        Status AutoConnect();
        /** Stop WiFi and unregister all event handlers. */
        Status Stop();
        bool IsConnected() const;
        wifi_mode_t GetMode() const;

        /* ---------------- Configuration ---------------- */

        void SetApConfig(const wifi_ap_config_t &cfg) { ap_config_ = cfg; }
        void SetStaConfig(const wifi_sta_config_t &cfg) { sta_config_ = cfg; }
        void SetStaRetryNum(int n) { sta_retry_num_ = n; }
        void SetScanMaxCount(uint16_t n) { scan_max_count_ = n; }

        const wifi_ap_config_t &ApConfig() const { return ap_config_; }
        const wifi_sta_config_t &StaConfig() const { return sta_config_; }
        uint16_t ScanMaxCount() const { return scan_max_count_; }

        void SetConnectedCb(ConnectedCb cb) { connected_cb_ = cb; }
        void SetDisconnectedCb(DisconnectedCb cb) { disconnected_cb_ = cb; }

        /** Optional token required (as header "X-Admin-Token" or query "?token=")
         *  to call /ota and /reset. Empty (default) = no auth, NOT recommended
         *  for anything beyond the initial closed AP setup window. */
        void SetAdminToken(const char *token)
        {
            std::strncpy(admin_token_.data(), token ? token : "", admin_token_.size() - 1);
            admin_token_[admin_token_.size() - 1] = '\0';
        }
        const char *AdminToken() const { return admin_token_.data(); }

        Page &page() { return page_; }
        const Page &page() const { return page_; }

        /* ---------------- Internal (used by WiFiPan_Portal.cpp handlers) ----------------
         * Public only so free-function httpd handlers (which hold a Manager* via
         * req->user_ctx, not a friend relationship) can call them. Application
         * code has no reason to call these directly. */

        /** Called by the POST / and /configsave handlers once the form is submitted. */
        void NotifyWifiSubmitted();

        /** Build the "/" dashboard page (placeholder substitution over HOME_HTML). */
        std::string BuildHomePage();

        /** Current STA/AP IPv4 as a string, or "192.168.4.1" if netif isn't up yet. */
        std::string CurrentIpString() const;

        Status StartWebServer();
        void StopWebServer();
        Status SetCaptivePortalUri();
        void *StartDns(esp_ip4_addr_t ip);
        void StopDns(void *dns_handle);

    private:
        struct Impl; // event group, netif, httpd handle, task handle... (see WiFiPan_Internal.hpp)

        /* ---- ESP-IDF event trampolines (require free/static function + void*) ---- */
        static void StaEventTrampoline(void *arg, esp_event_base_t base, int32_t id, void *data);
        static void ApEventTrampoline(void *arg, esp_event_base_t base, int32_t id, void *data);
        void HandleStaEvent(esp_event_base_t base, int32_t id, void *data);
        void HandleApEvent(esp_event_base_t base, int32_t id, void *data);

        bool LoadSavedCredsFromNvs();
        static wifi_ap_config_t DefaultApConfig();

        wifi_ap_config_t ap_config_{};
        wifi_sta_config_t sta_config_{};
        int sta_retry_num_ = -1;
        uint16_t scan_max_count_ = 0;

        Page page_;

        ConnectedCb connected_cb_ = nullptr;
        DisconnectedCb disconnected_cb_ = nullptr;

        std::unique_ptr<Impl> priv_;
        std::array<char, kAdminTokenLen> admin_token_{};
    };

} /* Namespace WiFiPan */

#endif /* WIFIPAN_HPP_ */