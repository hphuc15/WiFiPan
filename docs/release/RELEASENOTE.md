## WiFiPan v2.1.0
WiFiPan v2.1.0 adds NVS persistence helpers for portal parameters.

### What's Changed
- Added `SaveToNvs()` / `LoadFromNvs()` (`WiFiPan_Page_SaveToNvs()` / `WiFiPan_Page_LoadFromNvs()`) to save and restore registered portal parameters in NVS across reboots.

### Installation
Add as a managed component via `idf.py`:
```bash
idf.py add-dependency "hphuc15/wifipan^2.1.0"
```

Or manually in your `main/idf_component.yml`:
```yaml
dependencies:
  hphuc15/wifipan: "^2.1.0"
```

Usage:
- Pure C projects: `#include "WiFiPan.h"`
- C++ projects: `#include "WiFiPan.hpp"`
