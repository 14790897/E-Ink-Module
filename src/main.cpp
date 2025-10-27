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
 * @brief Draw a complete face on the display (full refresh)
 * @param happy True for happy face, false for sad face
 */
void drawFaceFull(bool happy) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Draw title text
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(8, 24);
    display.print("HELLO SSD1680");

    // Draw face circle
    display.drawCircle(FACE_CENTER_X, FACE_CENTER_Y, FACE_RADIUS, GxEPD_BLACK);

    // Draw eyes
    int eyeOffsetX = FACE_RADIUS / 2;
    int eyeOffsetY = FACE_RADIUS / 3;
    const int EYE_RADIUS = 5;

    display.fillCircle(FACE_CENTER_X - eyeOffsetX, FACE_CENTER_Y - eyeOffsetY, EYE_RADIUS, GxEPD_BLACK);
    display.fillCircle(FACE_CENTER_X + eyeOffsetX, FACE_CENTER_Y - eyeOffsetY, EYE_RADIUS, GxEPD_BLACK);

    // Draw mouth
    if (happy) {
      // Draw smile (parabola curve)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 1) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 3 + (i * i) / (FACE_RADIUS / 2 + 1) / 3;
        display.fillCircle(FACE_CENTER_X + i, y, 2, GxEPD_BLACK);
      }
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(8, EPD_HEIGHT - 8);
      display.print("Have a nice day!");
    }
    else {
      // Draw frown (inverted parabola)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 1) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 2 - (i * i) / (FACE_RADIUS / 2 + 1) / 2;
        display.fillCircle(FACE_CENTER_X + i, y, 2, GxEPD_BLACK);
      }
      display.setFont(&FreeMonoBold9pt7b);
      display.setCursor(8, EPD_HEIGHT - 8);
      display.print("Keep going!");
    }
  } while (display.nextPage());
}

/**
 * @brief Draw just the mouth (partial refresh)
 * @param happy True for happy mouth, false for sad mouth
 */
void drawMouthPartial(bool happy) {
  // Define partial window for the mouth area
  // Must be aligned to 8 pixels for rotation 1 (y and h must be multiple of 8)
  // Expand window to ensure we capture the full mouth area
  int mouthX = FACE_CENTER_X - FACE_RADIUS / 2 - 10;
  int mouthY = (FACE_CENTER_Y - FACE_RADIUS / 4 - 10) & ~0x07;  // Align to 8
  int mouthWidth = FACE_RADIUS + 50;
  int mouthHeight = ((FACE_RADIUS / 2 + 25) + 70) & ~0x07;  // Align to 8

  Serial.printf("Partial window: x=%d, y=%d, w=%d, h=%d\n", mouthX, mouthY, mouthWidth, mouthHeight);

  display.setPartialWindow(mouthX, mouthY, mouthWidth, mouthHeight);

  display.firstPage();
  do {
    // Fill the partial window with white to clear old content
    display.fillRect(mouthX, mouthY, mouthWidth, mouthHeight, GxEPD_WHITE);

    // Redraw part of the face circle that intersects with partial window
    display.drawCircle(FACE_CENTER_X, FACE_CENTER_Y, FACE_RADIUS, GxEPD_BLACK);

    // Draw mouth with thicker lines for better visibility
    if (happy) {
      // Draw smile (parabola curve)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 1) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 3 + (i * i) / (FACE_RADIUS / 2 + 1) / 3;
        display.fillCircle(FACE_CENTER_X + i, y, 2, GxEPD_BLACK);
        // Add one more pixel for thickness
        display.fillCircle(FACE_CENTER_X + i, y + 1, 1, GxEPD_BLACK);
      }
    }
    else {
      // Draw frown (inverted parabola)
      for (int i = -FACE_RADIUS / 2; i <= FACE_RADIUS / 2; i += 1) {
        int y = FACE_CENTER_Y + FACE_RADIUS / 2 - (i * i) / (FACE_RADIUS / 2 + 1) / 2;
        display.fillCircle(FACE_CENTER_X + i, y, 2, GxEPD_BLACK);
        // Add one more pixel for thickness
        display.fillCircle(FACE_CENTER_X + i, y + 1, 1, GxEPD_BLACK);
      }
    }
        display.fillCircle(FACE_CENTER_X, FACE_CENTER_Y, FACE_RADIUS, GxEPD_BLACK);

  } while (display.nextPage());
}

static bool isHappy = false;

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

  // Draw first page with FULL REFRESH: happy face
  Serial.println("Drawing initial screen with FULL refresh...");
  drawFaceFull(false);

  // Put display to sleep to save power
  display.hibernate();

  Serial.println("Initial screen drawn. Starting partial refresh demo...");
  Serial.println("Partial refresh will update only the mouth area.");
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
  static uint8_t updateCount = 0;
  const unsigned long PARTIAL_UPDATE_INTERVAL_MS = 2000;
  const uint8_t FULL_REFRESH_EVERY_N_UPDATES = 5;        // Full refresh every 5 updates (manufacturer recommendation)

  delay(PARTIAL_UPDATE_INTERVAL_MS);

  // Toggle face expression
  isHappy = !isHappy;
  updateCount++;

  // Decide whether to use partial or full refresh
  bool useFullRefresh = (updateCount >= FULL_REFRESH_EVERY_N_UPDATES);

  if (useFullRefresh) {
    // Periodic FULL REFRESH to clear ghosting
    Serial.printf("Update #%d: FULL refresh (clearing ghosting)... ", updateCount);
    drawFaceFull(isHappy);
    updateCount = 0;  // Reset counter
  } else {
    // Fast PARTIAL REFRESH (only mouth area)
    Serial.printf("Update #%d: PARTIAL refresh (fast)... ", updateCount);
    drawMouthPartial(isHappy);
  }

  // Put display to sleep to save power
  display.hibernate();

  Serial.printf("happy=%d\n", isHappy);
}
