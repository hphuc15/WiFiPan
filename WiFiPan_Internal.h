#ifndef WIFIPAN_INTERNAL_H_
#define WIFIPAN_INTERNAL_H_

/*
 * Internal-only header: defines Manager::Impl and the free functions that
 * operate on it (event handling, portal/DNS/HTTP). Include this from
 * WiFiPan.cpp and WiFiPan_Portal.cpp only - never from application code or
 * from WiFiPan.hpp, otherwise consumers would pull in FreeRTOS/httpd headers
 * just to use the public API.
 */

#include "WiFiPan.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_event.h"

namespace WiFiPan
{
    /* Event bits (was WP_EVENT_BIT_* macros). */
    inline constexpr EventBits_t kEventBitStaStart        = BIT0;
    inline constexpr EventBits_t kEventBitStaDisconnected = BIT1;
    inline constexpr EventBits_t kEventBitStaConnected    = BIT2;
    inline constexpr EventBits_t kEventBitApStart         = BIT3;

    struct Manager::Impl
    {
        EventGroupHandle_t group = nullptr;

        esp_event_handler_instance_t ap_connected_handle    = nullptr;
        esp_event_handler_instance_t ap_disconnected_handle = nullptr;
        esp_event_handler_instance_t ap_start_handle        = nullptr;
        esp_event_handler_instance_t sta_handle             = nullptr;
        esp_event_handler_instance_t sta_disc_handle        = nullptr;
        esp_event_handler_instance_t ip_handle              = nullptr;

        esp_netif_t   *netif               = nullptr;
        httpd_handle_t server              = nullptr;
        TaskHandle_t   portal_waiting_task = nullptr;
        int            sta_retry_remaining = 0;
    };

} /* WiFiPan */

#endif /* WIFIPAN_INTERNAL_H_ */