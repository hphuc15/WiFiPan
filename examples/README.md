# WiFiPan - ESP32-S3 Examples
| Folder                | Description           |
|-----------------------|-----------------------|
| `example_esp32s3_c/`  | Test app via C wrapper|
| `example_esp32s3_cpp/`| Test app via C++      |

Both exercise the same API surface (`Init`, `AutoConnect`, `ConfigViaAp`, `Stop`, `StartWebServer`/`StopWebServer`, `IsConnected`, `GetMode`, `CurrentIpString`, connect/disconnect callbacks, retry/scan settings, admin token, dynamic `Page` params) and behave identically.

## Hardware
- BOOT button: GPIO0
- RGB LED: GPIO48 (WS2812, most ESP32-S3-DevKitC-1 boards - check your board revision, some use GPIO38)

## Behavior
- **Boot**: waits up to 3s for BOOT hold. Saved credentials in NVS -> `AutoConnect()`; otherwise (or BOOT held) -> `ConfigViaAp()`.
- **Config portal**: connect to `ESP32_Config` AP (password `ESP32_Config`), captive-portal prompt pops up. `/scan` to pick a network and submit credentials.
- **Config page**: `/config` has 3 demo fields (`dev_name`, `report_interval`, `api_key`) usable while the portal is up.
- **Admin token**: still under development.
- **Re-provisioning**: hold BOOT 3s anytime to `Stop()` and reopen the config portal without reflashing.

## LED status
| Color            | Meaning                           |
|------------------|-----------------------------------|
| White            | Initializing / waiting on BOOT    |
| Blue             | AP / captive portal mode          |
| Yellow           | Connecting to STA                 |
| Green            | Connected                         |
| Red              | Disconnected / setup failed       |