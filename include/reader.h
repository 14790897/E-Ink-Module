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
#define MAX_PAGE_HISTORY 100

struct ReadingState {
  String currentBook;
  int currentPage;
  int totalPages;
  File bookFile;
  bool isReading;
  long currentFilePosition;  // 当前页在文件中的起始位置
  long pagePositions[MAX_PAGE_HISTORY];  // 页面位置历史记录
  int pageHistoryCount;  // 历史记录数量
};

extern ReadingState reading;

// ============================================================================
// Display Functions
// ============================================================================
void initDisplay(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>& display);
int displayText(const String& text, int pageNum, int totalPages);  // 返回实际显示的字符数
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
