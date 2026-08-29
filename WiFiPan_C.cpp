#include "WiFiPan.h"
#include "WiFiPan.hpp"

#include <cstring>
#include <new>

/* Real definition of the opaque handle - just holds a Manager. */
struct WiFiPan_Manager
{
    WiFiPan::Manager impl;
};

namespace
{
    inline WiFiPan_Status ToC(WiFiPan::Status st)
    {
        return static_cast<WiFiPan_Status>(st);
    }
} // namespace

WiFiPan_t *WiFiPan_Create(void)
{
    return new (std::nothrow) WiFiPan_t{};
}

void WiFiPan_Destroy(WiFiPan_t *m)
{
    delete m;
}

WiFiPan_Status WiFiPan_Init(WiFiPan_t *m)
{
    return ToC(m->impl.Init());
}

WiFiPan_Status WiFiPan_StartSta(WiFiPan_t *m)
{
    return ToC(m->impl.StartSta());
}

WiFiPan_Status WiFiPan_StartAp(WiFiPan_t *m)
{
    return ToC(m->impl.StartAp());
}

WiFiPan_Status WiFiPan_ConfigViaAp(WiFiPan_t *m)
{
    return ToC(m->impl.ConfigViaAp());
}

WiFiPan_Status WiFiPan_AutoConnect(WiFiPan_t *m)
{
    return ToC(m->impl.AutoConnect());
}

WiFiPan_Status WiFiPan_Stop(WiFiPan_t *m)
{
    return ToC(m->impl.Stop());
}

bool WiFiPan_IsConnected(const WiFiPan_t *m)
{
    return m->impl.IsConnected();
}

wifi_mode_t WiFiPan_GetMode(const WiFiPan_t *m)
{
    return m->impl.GetMode();
}

WiFiPan_Status WiFiPan_StartWebServer(WiFiPan_t *m)
{
    return ToC(m->impl.StartWebServer());
}

void WiFiPan_StopWebServer(WiFiPan_t *m)
{
    m->impl.StopWebServer();
}

size_t WiFiPan_CurrentIpString(const WiFiPan_t *m, char *out, size_t out_size)
{
    if (!out || out_size == 0) {
        return 0;
    }
    std::string ip = m->impl.CurrentIpString();
    std::strncpy(out, ip.c_str(), out_size - 1);
    out[out_size - 1] = '\0';
    return std::strlen(out);
}

void WiFiPan_SetApConfig(WiFiPan_t *m, const wifi_ap_config_t *cfg)
{
    if (cfg) {
        m->impl.SetApConfig(*cfg);
    }
}

void WiFiPan_SetStaConfig(WiFiPan_t *m, const wifi_sta_config_t *cfg)
{
    if (cfg) {
        m->impl.SetStaConfig(*cfg);
    }
}

void WiFiPan_SetStaRetryNum(WiFiPan_t *m, int n)
{
    m->impl.SetStaRetryNum(n);
}

void WiFiPan_SetScanMaxCount(WiFiPan_t *m, uint16_t n)
{
    m->impl.SetScanMaxCount(n);
}

void WiFiPan_SetAdminToken(WiFiPan_t *m, const char *token)
{
    m->impl.SetAdminToken(token);
}

void WiFiPan_SetConnectedCb(WiFiPan_t *m, WiFiPan_ConnectedCb cb)
{
    /* WiFiPan::ConnectedCb is already `void(*)(void)` - identical
     * signature, no adapter needed. */
    m->impl.SetConnectedCb(cb);
}

void WiFiPan_SetDisconnectedCb(WiFiPan_t *m, WiFiPan_DisconnectedCb cb)
{
    m->impl.SetDisconnectedCb(cb);
}

WiFiPan_Status WiFiPan_Page_AddParam(WiFiPan_t *m, const char *id, const char *label,
    const char *placeholder, const char *value, const char *type, bool required)
{
    return ToC(m->impl.page().AddParam(id, label, placeholder, value, type, required));
}

const char *WiFiPan_Page_GetParam(const WiFiPan_t *m, const char *id)
{
    return m->impl.page().GetParam(id);
}

bool WiFiPan_Page_SetParamValue(WiFiPan_t *m, const char *id, const char *value)
{
    return m->impl.page().SetParamValue(id, value);
}

size_t WiFiPan_Page_Count(const WiFiPan_t *m)
{
    return m->impl.page().count();
}