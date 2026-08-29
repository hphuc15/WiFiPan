#ifndef WIFIPAN_H_
#define WIFIPAN_H_

/**
 * @file WiFiPan.h
 * @brief Pure C API wrapper for the WiFiPan provisioning library.
 *
 * Safe to #include from pure C source files (.c) - contains no C++ syntax.
 * For the full C++ interface (WiFiPan::Manager, WiFiPan::Page), please #include "WiFiPan.hpp".
 *
 * The underlying implementation (WiFiPan_C.cpp) is compiled as C++ and forwards
 * each call to WiFiPan::Manager; no WiFi or portal logic is duplicated.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_wifi.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Opaque handle. The real definition (wrapping WiFiPan::Manager) lives
     * only in WiFiPan_C.cpp - callers never see the C++ type. */
    typedef struct WiFiPan_Manager WiFiPan_t;

    /* Mirrors WiFiPan::Status 1:1 - keep values in sync if that enum changes. */
    typedef enum
    {
        WIFIPAN_OK = 0,
        WIFIPAN_INVALID_ARG = -1,
        WIFIPAN_INIT_FAILED = -2,
        WIFIPAN_WIFI_ERROR = -3,
        WIFIPAN_TIMEOUT = -4,
        WIFIPAN_NO_CREDS = -5,
        WIFIPAN_NETIF_ERROR = -6,
        WIFIPAN_NO_MEM = -7,
        WIFIPAN_CONNECT_FAILED = -8,
        WIFIPAN_WEAK_AP_PASSWORD = -9,
    } WiFiPan_Status;

    typedef void (*WiFiPan_ConnectedCb)(void);
    typedef void (*WiFiPan_DisconnectedCb)(void);

    /* ---- Lifecycle ---- */

    WiFiPan_t *WiFiPan_Create(void);
    void WiFiPan_Destroy(WiFiPan_t *m);

    WiFiPan_Status WiFiPan_Init(WiFiPan_t *m);
    WiFiPan_Status WiFiPan_StartSta(WiFiPan_t *m);
    WiFiPan_Status WiFiPan_StartAp(WiFiPan_t *m);
    WiFiPan_Status WiFiPan_ConfigViaAp(WiFiPan_t *m);
    WiFiPan_Status WiFiPan_AutoConnect(WiFiPan_t *m);
    WiFiPan_Status WiFiPan_Stop(WiFiPan_t *m);

    bool WiFiPan_IsConnected(const WiFiPan_t *m);
    wifi_mode_t WiFiPan_GetMode(const WiFiPan_t *m);

    WiFiPan_Status WiFiPan_StartWebServer(WiFiPan_t *m);
    void WiFiPan_StopWebServer(WiFiPan_t *m);

    /* Writes the current IP string ("192.168.4.1" if netif isn't up yet)
     * into out, truncated to out_size. Returns strlen(out) after the copy. */
    size_t WiFiPan_CurrentIpString(const WiFiPan_t *m, char *out, size_t out_size);

    /* ---- Configuration ---- */

    void WiFiPan_SetApConfig(WiFiPan_t *m, const wifi_ap_config_t *cfg);
    void WiFiPan_SetStaConfig(WiFiPan_t *m, const wifi_sta_config_t *cfg);
    void WiFiPan_SetStaRetryNum(WiFiPan_t *m, int n);
    void WiFiPan_SetScanMaxCount(WiFiPan_t *m, uint16_t n);
    void WiFiPan_SetAdminToken(WiFiPan_t *m, const char *token);

    void WiFiPan_SetConnectedCb(WiFiPan_t *m, WiFiPan_ConnectedCb cb);
    void WiFiPan_SetDisconnectedCb(WiFiPan_t *m, WiFiPan_DisconnectedCb cb);

    /* ---- Dynamic form-field schema (WiFiPan::Page) ---- */

    WiFiPan_Status WiFiPan_Page_AddParam(WiFiPan_t *m, const char *id, const char *label,
        const char *placeholder, const char *value, const char *type, bool required);
    const char *WiFiPan_Page_GetParam(const WiFiPan_t *m, const char *id);
    bool WiFiPan_Page_SetParamValue(WiFiPan_t *m, const char *id, const char *value);
    size_t WiFiPan_Page_Count(const WiFiPan_t *m);

#ifdef __cplusplus
}
#endif

#endif /* WIFIPAN_H_ */