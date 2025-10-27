// ============================================================================
// Reader Management Header
// ============================================================================
// This file handles book reading, pagination, and E-Ink display
// ============================================================================

#ifndef READER_H
#define READER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

// ============================================================================
// Text Display Configuration
// ============================================================================
extern const int CHAR_WIDTH;
extern const int LINE_HEIGHT;
extern const int MARGIN_LEFT;
extern const int MARGIN_TOP;
extern const int MARGIN_RIGHT;
extern const int MARGIN_BOTTOM;
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;
extern const int CHARS_PER_LINE;
extern const int LINES_PER_PAGE;
extern const int CHARS_PER_PAGE;

// ============================================================================
// Reading State
// ============================================================================
struct ReadingState {
  String currentBook;
  int currentPage;
  int totalPages;
  File bookFile;
  bool isReading;
};

extern ReadingState reading;

// ============================================================================
// Display Functions
// ============================================================================
void initDisplay(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>& display);
void displayText(const String& text, int pageNum, int totalPages);
void displayMessage(const String& title, const String& message);

// ============================================================================
// Reading Functions
// ============================================================================
int getUTF8CharLength(const char* str, int index);
int getStringDisplayWidth(const String& str);
bool openBook(const String& bookName);
void nextPage();
void prevPage();

#endif // READER_H
