idf_component_register(
    SRCS
        "WiFiPan.cpp"
        "WiFiPan_C.cpp"
        "WiFiPan_Page.cpp"
        "WiFiPan_Portal.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        esp_http_server
        app_update
        lwip
        freertos
        log
)