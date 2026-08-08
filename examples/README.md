# WiFiPan - ESP32-S3 Example

`examples/example_esp32s3.cpp` is a test app that exercises the full `WiFiPan::Manager` API: `Init`, `AutoConnect`, `ConfigViaAp`, `Stop`, `StartWebServer`/`StopWebServer`, `IsConnected`, `GetMode`, `CurrentIpString`, connect/disconnect callbacks, retry/scan settings, admin token, and dynamic `Page` params.

## Hardware

- BOOT button: GPIO0
- RGB LED: GPIO48 (WS2812, most ESP32-S3-DevKitC-1 boards - check your board revision, some use GPIO38)

## Behavior

**On boot**, the app waits up to 3s to see if BOOT is held. If saved WiFi credentials exist in NVS, it tries `AutoConnect()`; otherwise (or if BOOT is held) it opens the config portal via `ConfigViaAp()`.

**Config portal**: connect to the `ESP32_Config` AP (password `ESP32_Config`), a captive-portal prompt should pop up. Use `/scan` to pick a network and submit credentials.

**After connecting**: the app restarts the HTTP server on the STA network so `/config`, `/ota`, and `/reset` stay reachable for testing. `/config` has 3 demo fields (`dev_name`, `report_interval`, `api_key`)
to exercise the dynamic-param flow. `/ota` and `/reset` require an `X-Admin-Token` header or `?token=` query param (set in code, default `change-me-1234` - change before real use).

**Re-provisioning**: hold BOOT for 3s at any time while running to `Stop()` the current connection and reopen the config portal, without reflashing.

## LED status

| Color            | Meaning                           |
|------------------|-----------------------------------|
| White            | Initializing / waiting on BOOT    |
| Blue             | AP / captive portal mode          |
| Yellow           | Connecting to STA                 |
| Green            | Connected                         |
| Red              | Disconnected / setup failed       |