## WiFiPan v2.0.0
WiFiPan v2.0.0 introduces a major architecture cleanup and naming convention standardization for both C and C++ interfaces, along with a comprehensive GitHub Actions CI/CD pipeline.

### What's Changed
- `Unified C Header Entry Point`: Renamed `WiFiPan_C.h` to `WiFiPan.h` to serve as the clean, standard header entry point for pure C projects.
- `Symmetric C++ Core Naming`: Renamed `WiFiPan.cpp` to `WiFiPan_Cpp.cpp` to match `WiFiPan_C.cpp` implementation naming symmetry.
- `Non-conflicting Header Guards`: Updated header guards (`WIFIPAN_H_` for `WiFiPan.h` and `WIFIPAN_HPP_` for `WiFiPan.hpp`) allowing both headers to be safely included together without guard collision.
- `Automated CI/CD Workflows`: Added GitHub Actions workflows supporting ESP-IDF multi-version matrices (v5.4, v5.5), target matrices (esp32, esp32s3), manifest validation, clang-format linting, and Espressif Registry publishing.
- `Updated Documentation`: Comprehensive doc-comments in `WiFiPan.h` and updated layout guidelines in `README.md`.

### Installation
Add as a managed component via `idf.py`:
```bash
idf.py add-dependency "hphuc15/wifipan^2.0.0"
```

Or manually in your `main/idf_component.yml`:
```yaml
dependencies:
  hphuc15/wifipan: "^2.0.0"
```

Usage:
- Pure C projects: `#include "WiFiPan.h"`
- C++ projects: `#include "WiFiPan.hpp"`
