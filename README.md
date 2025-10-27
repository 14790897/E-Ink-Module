E-Ink Module (SSD1680) on ESP32-C3

Overview

- Target: ESP32-C3 (AirM2M CORE ESP32C3) + SSD1680 e-paper
- Libraries: Adafruit EPD + Adafruit GFX (declared in `platformio.ini`)
- Demo: Alternates two “fun” pages (smile/sad face + pattern) with full refresh every 15s.

Hardware Wiring

- Panel resolution (default): `250x122` (typical 2.13" SSD1680). Change in `src/main.cpp` if your panel differs.
- SPI (remapped to avoid conflicts with DC=5):
  - `SCK = GPIO4`
  - `MOSI = GPIO6`
  - `MISO = GPIO2` (remapped from default 5)
  - `CS  = GPIO7`
- EPD control:
  - `DC   = GPIO5`
  - `RST  = GPIO10`
  - `BUSY = GPIO3` (ensure BUSY is correctly wired to match `PIN_BUSY`)

If you use different pins, update them in `src/main.cpp`:

- `PIN_CS`, `PIN_DC`, `PIN_RST`, `PIN_BUSY`
- `SPI.begin(SCK, MISO, MOSI, CS)`

Build & Flash

- Build: `pio run`
- Upload: `pio run -t upload`
- Serial Monitor: `pio device monitor -b 115200`

Key Files

- `platformio.ini`: declares the board (`airm2m_core_esp32c3`) and libraries.
- `src/main.cpp`: SSD1680 demo using Adafruit EPD + GFX.

Customization

- Resolution: change `EPD_WIDTH` / `EPD_HEIGHT` to match your panel.
- Rotation: adjust `display.setRotation(0..3)` for portrait/landscape.
- Content: edit `drawFace()` / `drawPattern()` or add text/bitmaps via Adafruit GFX.

Troubleshooting

- Stuck at init or no refresh: check BUSY wiring and pin definitions (`PIN_BUSY`).
- Pin conflicts: ensure no overlap between SPI pins and EPD control pins.
- Wrong image orientation/clipping: verify `EPD_WIDTH/EPD_HEIGHT` and `setRotation`.
- Ghosting: e-paper requires full refresh; this demo uses full refresh via `display.display()`.

Notes

- The project uses existing, mature libraries (Adafruit EPD/GFX) instead of custom drivers.
- If you prefer using GxEPD2, it’s also included via `lib_deps`; we can switch on request.

