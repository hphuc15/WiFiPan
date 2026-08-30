# Changelog

Định dạng dựa trên [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), và dự án tuân thủ [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-08-30
### Thay đổi
- `Thay đổi phá vỡ tương thích`: Đổi tên file `WiFiPan_C.h` thành `WiFiPan.h` đóng vai trò làm header C API chuẩn duy nhất.
- `Thay đổi phá vỡ tương thích`: Đổi tên file `WiFiPan.cpp` thành `WiFiPan_Cpp.cpp` để tạo sự đối xứng tên gọi rõ ràng với `WiFiPan_C.cpp`.
- Cập nhật Include Guard để tránh xung đột: `WiFiPan.h` sử dụng `WIFIPAN_H_`, `WiFiPan.hpp` sử dụng `WIFIPAN_HPP_`.
- Cập nhật danh sách file nguồn trong `CMakeLists.txt` từ `WiFiPan.cpp` thành `WiFiPan_Cpp.cpp`.
- Cập nhật tất cả các đường dẫn `#include` nội bộ trong C/C++ implementation và ứng dụng mẫu.
- Cập nhật tài liệu hướng dẫn và sơ đồ thư mục dự án trong `README.md`.

### Thêm mới
- Bổ sung Doxygen doc-comments ở đầu `WiFiPan.h` ghi rõ pure C API và điều hướng người dùng C++ sang `WiFiPan.hpp`.
- Bổ sung GitHub Actions CI/CD workflows kiểm thử tự động trên ma trận phiên bản ESP-IDF (v5.4, v5.5), chip (`esp32`, `esp32s3`), linting định dạng, kiểm tra manifest và phát hành tự động lên Espressif Registry.

## [1.0.0] - 2026-08-14
### Thêm mới
- Phát hành phiên bản khởi đầu của thư viện Captive Portal WiFi Provisioning WiFiPan cho ESP-IDF.
- Triển khai C++ core (`WiFiPan::Manager`, `WiFiPan::Page`, dynamic portal form fields).
- Triển khai C wrapper API (`WiFiPan_C.h`, `WiFiPan_C.cpp`).
- Các HTTP server endpoints cho cấu hình WiFi, DNS redirect, OTA firmware upload.
- Ví dụ mẫu cho ESP32-S3.
