idf_component_register(
    SRCS            "example_esp32s3.cpp"
    INCLUDE_DIRS    "."
    REQUIRES        WiFiPan driver esp_timer
)