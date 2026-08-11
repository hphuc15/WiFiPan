idf_component_register(
    SRCS            "example_esp32s3.c"
    INCLUDE_DIRS    "."
    REQUIRES        WiFiPan driver esp_timer
)