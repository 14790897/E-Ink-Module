// ============================================================================
// Reader Management Header
// ============================================================================

#ifndef READER_H
#define READER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

extern const int CHINESE_CHAR_WIDTH;
extern const int LINE_HEIGHT;
extern const int MARGIN_LEFT;
extern const int MARGIN_TOP;
extern const int MARGIN_RIGHT;
extern const int MARGIN_BOTTOM;
extern const int PAGE_NUM_HEIGHT;
extern const int DISPLAY_WIDTH;
extern const int DISPLAY_HEIGHT;
extern const int CHARS_PER_LINE;
extern const int LINES_PER_PAGE;
extern const int CHARS_PER_PAGE;

#define MAX_PAGE_HISTORY 100

struct ReadingState {
  String currentBook;
  int currentPage;
  int totalPages;
  File bookFile;
  bool isReading;
  long currentFilePosition;
  long pagePositions[MAX_PAGE_HISTORY];
  int pageHistoryCount;
};

extern ReadingState reading;

void initDisplay(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>& display);
int displayText(const String& text, int pageNum, int totalPages);
void displayMessage(const String& title, const String& message);

int getUTF8CharLength(const char* str, int index);
bool openBook(const String& bookName);
bool resumeReading();
void nextPage();
void prevPage();

#endif // READER_H
