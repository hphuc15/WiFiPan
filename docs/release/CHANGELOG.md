# Changelog

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-08-30
### Changed
- `Breaking Change`: Renamed `WiFiPan_C.h` to `WiFiPan.h` as the unified C API header.
- `Breaking Change`: Renamed `WiFiPan.cpp` to `WiFiPan_Cpp.cpp` for symmetric naming with `WiFiPan_C.cpp`.
- Updated header guards to prevent collision: `WiFiPan.h` uses `WIFIPAN_H_`, `WiFiPan.hpp` uses `WIFIPAN_HPP_`.
- Updated `CMakeLists.txt` source list from `WiFiPan.cpp` to `WiFiPan_Cpp.cpp`.
- Updated internal `#include` references across C/C++ implementations and example projects.
- Updated documentation and project structure diagrams in `README.md`.

### Added
- Doxygen doc-comments in `WiFiPan.h` clarifying pure C API usage and pointing C++ users to `WiFiPan.hpp`.
- GitHub Actions CI/CD workflows for multi-version ESP-IDF build matrix (v5.4, v5.5), target matrix (esp32, esp32s3), manifest linting, format linting, and automated Espressif Registry publishing.

## [1.0.0] - 2026-08-14
### Added
- Initial release of WiFiPan captive-portal WiFi provisioning library for ESP-IDF.
- C++ core implementation (`WiFiPan::Manager`, `WiFiPan::Page`, dynamic portal form fields).
- Pure C wrapper API (`WiFiPan_C.h`, `WiFiPan_C.cpp`).
- HTTP server endpoints for provisioning, DNS redirect, OTA firmware upload.
- Example application for ESP32-S3.
