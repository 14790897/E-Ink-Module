// ============================================================================
// Includes
// ============================================================================
#include <Arduino.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// ============================================================================
// Configuration
// ============================================================================
// All pin definitions and display resolution are configured in platformio.ini
// as build flags. This allows easy hardware configuration without modifying
// the source code.
//
// Build flags defined in platformio.ini:
//   - EPD_WIDTH, EPD_HEIGHT: Display resolution (default: 250x122 for 2.13")
//   - HW_SCK, HW_MOSI, HW_MISO: SPI hardware pins
//   - PIN_CS, PIN_DC, PIN_RST, PIN_BUSY: E-Paper control pins

// ============================================================================
// Global Objects
// ============================================================================
// Create GxEPD2 display instance for SSD1680 (2.13" 250x122 b/w e-paper)
// GxEPD2_213_BN is for DEPG0213BN panel with SSD1680 controller
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(
  GxEPD2_213_BN(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)
);

// ============================================================================
// Constants
// ============================================================================
const int FACE_CENTER_X = EPD_WIDTH / 2;
const int FACE_CENTER_Y = EPD_HEIGHT / 2 + 10;
const int FACE_RADIUS = min(EPD_WIDTH, EPD_HEIGHT) / 2 - 10;

// ============================================================================
// Display Drawing Functions
// ============================================================================

/**
 * @brief Draw a face on the display (happy or sad)
 * @param happy True for happy face, false for sad face
 * @param usePartialUpdate True to use fast partial refresh, false for full refresh
 */
void drawFace(bool happy, bool usePartialUpdate = false) {
  // Set update window based on refresh type
  if (usePartialUpdate) {
    // Define partial window for faster updates (just the mouth area)
    int mouthX = FACE_CENTER_X - FACE_RADIUS / 2;
    int mouthY = FACE_CENTER_Y + FACE_RADIUS / 4;
    int mouthWidth = FACE_RADIUS;
    int mouthHeight = FACE_RADIUS / 2;
    display.setPartialWindow(mouthX, mouthY, mouthWidth, mouthHeight);
  } else {
    display.setFullWindow();
  }

  display.firstPage();
  do {
    if (!usePartialUpdate) {
      display.fillScreen(GxEPD_WHITE);
    }

    // Draw title text with custom font (skip in partial update)
    if (!usePartialUpdate) {
      display.setFont(&FreeMonoBold12pt7b);
      display.setTextColor(GxEPD_BLACK);
      display.setCursor(8, 24);
      display.print("你好 SSD1680");

      // Draw face circle
      display.drawCircle(FACE_CENTER_X, FACE_CENTER_Y, FACE_RADIUS, GxEPD_BLACK);

      // Draw eyes
      int eyeOffsetX = FACE_RADIUS / 2;
      int eyeOffsetY = FACE_RADIUS / 3;
      const int EYE_RADIUS = 5;

      display.fillCircle(FACE_CENTER_X - eyeOffsetX, FACE_CENTER_Y - eyeOffsetY, EYE_RADIUS, GxEPD_BLACK);
      display.fillCircle(FACE_CENTER_X + eyeOffsetX, FACE_CENTER_Y - eyeOffsetY, EYE_RADIUS, GxEPD_BLACK);
    }

    // Draw mouth (this will be updated in partial refresh mode)
    if (happy) {
      // Draw smile (parabola curve)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 2) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 3 + (i * i) / (FACE_RADIUS / 2 + 1) / 3;
        display.drawPixel(FACE_CENTER_X + i, y, GxEPD_BLACK);
      }

      // Display happy message (skip in partial update)
      if (!usePartialUpdate) {
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(8, EPD_HEIGHT - 8);
        display.print("Have a nice day!");
      }
    }
    else {
      // Draw frown (inverted parabola)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 2) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 2 - (i * i) / (FACE_RADIUS / 2 + 1) / 2;
        display.drawPixel(FACE_CENTER_X + i, y, GxEPD_BLACK);
      }

      // Display encouraging message (skip in partial update)
      if (!usePartialUpdate) {
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(8, EPD_HEIGHT - 8);
        display.print("Keep going!");
      }
    }
  } while (display.nextPage());
}

/**
 * @brief Draw a hatch pattern on the display
 */
void drawPattern() {
  const int MARGIN = 8;
  const int BOTTOM_TEXT_HEIGHT = 56;
  const int LINE_SPACING = 6;

  int x0 = MARGIN;
  int y0 = 40;
  int width = EPD_WIDTH - (MARGIN * 2);
  int height = EPD_HEIGHT - BOTTOM_TEXT_HEIGHT;

  // Draw horizontal lines
  for (int y = y0; y < y0 + height; y += LINE_SPACING) {
    display.drawLine(x0, y, x0 + width, y, GxEPD_BLACK);
  }

  // Draw vertical lines
  for (int x = x0; x < x0 + width; x += LINE_SPACING) {
    display.drawLine(x, y0, x, y0 + height, GxEPD_BLACK);
  }
}

// ============================================================================
// Arduino Setup Function
// ============================================================================

/**
 * @brief Initialize the display and draw the first page
 */
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(100);
  Serial.println("GxEPD2 SSD1680 demo with partial refresh support");
  Serial.println("=================================================");

  // Initialize SPI with custom pins
  SPI.begin(HW_SCK, HW_MISO, HW_MOSI, PIN_CS);

  // Initialize the display
  // init(serial_diag_bitrate, init_busy_level, reset_duration, pulldown_rst)
  display.init(115200, true, 2, false);

  // Set display rotation (1 or 3 often suits 250x122 panels in landscape)
  display.setRotation(1);

  // Draw first page with FULL REFRESH: happy face with pattern
  Serial.println("Drawing initial screen with FULL refresh...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawFace(true, false);  // false = full refresh
    drawPattern();
  } while (display.nextPage());

  // Put display to sleep to save power
  display.hibernate();

  Serial.println("Initial screen drawn. Starting partial refresh demo...");
  Serial.println("Full refresh will occur every 10 cycles to prevent ghosting.");
}

// ============================================================================
// Arduino Loop Function
// ============================================================================

/**
 * @brief Alternate between happy and sad faces
 * Uses PARTIAL refresh for fast updates, with periodic FULL refresh
 * to prevent ghosting artifacts
 */
void loop() {
  static bool isHappy = false;
  static uint8_t updateCount = 0;
  const unsigned long PARTIAL_UPDATE_INTERVAL_MS = 3000;  // 3 seconds for demo
  const uint8_t FULL_REFRESH_EVERY_N_UPDATES = 10;        // Full refresh every 10 updates

  delay(PARTIAL_UPDATE_INTERVAL_MS);

  // Toggle face expression
  isHappy = !isHappy;
  updateCount++;

  // Decide whether to use partial or full refresh
  bool useFullRefresh = (updateCount >= FULL_REFRESH_EVERY_N_UPDATES);

  if (useFullRefresh) {
    // Periodic FULL REFRESH to clear ghosting
    Serial.printf("Update #%d: FULL refresh (clearing ghosting)... ", updateCount);

    display.setFullWindow();
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      drawFace(isHappy, false);  // false = full refresh
      drawPattern();
    } while (display.nextPage());

    updateCount = 0;  // Reset counter
  } else {
    // Fast PARTIAL REFRESH (only mouth area)
    Serial.printf("Update #%d: PARTIAL refresh (fast)... ", updateCount);

    drawFace(isHappy, true);  // true = partial refresh
  }

  // Put display to sleep to save power
  display.hibernate();

  Serial.printf("happy=%d\n", isHappy);
}
