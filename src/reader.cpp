#include "reader.h"

// 屏幕使用 setRotation(1) 旋转了90度，物理尺寸：112*212，旋转后实际显示尺寸：212x112
const int SCREEN_WIDTH = EPD_HEIGHT; // 旋转后的实际宽度 = 212
const int SCREEN_HEIGHT = EPD_WIDTH; // 旋转后的实际高度 = 112

const int CHINESE_CHAR_WIDTH = 12;
const int LINE_HEIGHT = 13;
const int MARGIN_LEFT = 2;
const int MARGIN_TOP = 2;
const int MARGIN_RIGHT = 10;
const int MARGIN_BOTTOM = 10;
const int PAGE_NUM_HEIGHT = 4;

const int DISPLAY_WIDTH = SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT;
const int DISPLAY_HEIGHT = SCREEN_HEIGHT - MARGIN_TOP - MARGIN_BOTTOM;
const int CHARS_PER_LINE = DISPLAY_WIDTH / CHINESE_CHAR_WIDTH;
const int LINES_PER_PAGE = DISPLAY_HEIGHT / LINE_HEIGHT;
const int CHARS_PER_PAGE = CHARS_PER_LINE * LINES_PER_PAGE;

ReadingState reading;

static GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>* displayPtr = nullptr;
static U8G2_FOR_ADAFRUIT_GFX u8g2;

// UTF-8 字符字节长度
int getUTF8CharLength(const char* str, int index) {
  unsigned char c = str[index];
  if (c < 0x80) return 1;
  if ((c & 0xE0) == 0xC0) return 2;
  if ((c & 0xF0) == 0xE0) return 3;
  if ((c & 0xF8) == 0xF0) return 4;
  return 1;
}

// 使字符串在UTF-8边界结束
static void trimToUTF8Boundary(String& s) {
  while (s.length() > 0) {
    uint8_t b = (uint8_t)s[s.length() - 1];
    if ((b & 0xC0) == 0x80) {
      s.remove(s.length() - 1);
      continue;
    }
    int need = 1;
    if ((b & 0xE0) == 0xC0) need = 2;
    else if ((b & 0xF0) == 0xE0) need = 3;
    else if ((b & 0xF8) == 0xF0) need = 4;
    if (need > 1 && s.length() < need) {
      s.remove(s.length() - 1);
      continue;
    }
    break;
  }
}

static String readChunk(File& file, long startPos, int maxBytes) {
  if (!file.seek(startPos)) return "";
  String content = "";
  int bytesRead = 0;
  while (file.available() && bytesRead < maxBytes) {
    content += (char)file.read();
    bytesRead++;
  }
  trimToUTF8Boundary(content);
  return content;
}

// 计算一页可容纳的字节数
static int calcPageBytes(const String& text) {
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
  u8g2.setForegroundColor(GxEPD_BLACK);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);

  int fontAscent = u8g2.getFontAscent();
  int fontDescent = u8g2.getFontDescent();
  int lineAdvance = fontAscent - fontDescent;
  int y = MARGIN_TOP + fontAscent + 1;

  int renderIndex = 0;
  while (renderIndex < text.length()) {
    String line = "";
    int linePixelWidth = 0;
    while (renderIndex < text.length()) {
      if (text[renderIndex] == '\n' || text[renderIndex] == '\r') {
        renderIndex++;
        if (text[renderIndex - 1] == '\r' && renderIndex < text.length() && text[renderIndex] == '\n') {
          renderIndex++;
        }
        break;
      }
      int charLen = getUTF8CharLength(text.c_str(), renderIndex);
      String currentChar = text.substring(renderIndex, renderIndex + charLen);
      int charPixelWidth = u8g2.getUTF8Width(currentChar.c_str());
      if (linePixelWidth + charPixelWidth > DISPLAY_WIDTH) break;
      line += currentChar;
      linePixelWidth += charPixelWidth;
      renderIndex += charLen;
    }
    y += lineAdvance;
    if (y - fontDescent > (SCREEN_HEIGHT - PAGE_NUM_HEIGHT)) break;
  }
  return renderIndex;
}

void initDisplay(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>& display) {
  displayPtr = &display;
  u8g2.begin(*displayPtr);
  Serial.println("Display initialized with Chinese font support");
}

int displayText(const String& text, int pageNum, int totalPages) {
  if (!displayPtr) return 0;

  displayPtr->setFullWindow();
  displayPtr->firstPage();

  int pageCharCount = 0;
  do {
    displayPtr->fillScreen(GxEPD_WHITE);
    displayPtr->setTextColor(GxEPD_BLACK);

    u8g2.setFontMode(1);
    u8g2.setFontDirection(0);
    u8g2.setForegroundColor(GxEPD_BLACK);
    u8g2.setBackgroundColor(GxEPD_WHITE);
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);

    int fontAscent = u8g2.getFontAscent();
    int fontDescent = u8g2.getFontDescent();
    int lineAdvance = fontAscent - fontDescent;

    int x = MARGIN_LEFT;
    int y = MARGIN_TOP + fontAscent + 1;
    int renderIndex = 0;

    while (renderIndex < text.length()) {
      String line = "";
      int linePixelWidth = 0;

      while (renderIndex < text.length()) {
        if (text[renderIndex] == '\n' || text[renderIndex] == '\r') {
          renderIndex++;
          if (text[renderIndex - 1] == '\r' && renderIndex < text.length() && text[renderIndex] == '\n') {
            renderIndex++;
          }
          break;
        }

        int charLen = getUTF8CharLength(text.c_str(), renderIndex);
        String currentChar = text.substring(renderIndex, renderIndex + charLen);
        int charPixelWidth = u8g2.getUTF8Width(currentChar.c_str());

        if (linePixelWidth + charPixelWidth > DISPLAY_WIDTH) break;

        line += currentChar;
        linePixelWidth += charPixelWidth;
        renderIndex += charLen;
      }

      u8g2.setCursor(x, y);
      if (line.length() > 0) {
        u8g2.print(line);
      }

      y += lineAdvance;

      if (y - fontDescent > (SCREEN_HEIGHT - PAGE_NUM_HEIGHT)) break;
    }

    pageCharCount = renderIndex;

    u8g2.setFont(u8g2_font_6x10_tf);
    String pageInfo = String(pageNum + 1) + " / " + String(totalPages);
    int pageInfoWidth = u8g2.getUTF8Width(pageInfo.c_str());
    u8g2.setCursor((SCREEN_WIDTH - pageInfoWidth) / 2, SCREEN_HEIGHT - PAGE_NUM_HEIGHT + u8g2.getFontAscent());
    u8g2.print(pageInfo);

  } while (displayPtr->nextPage());

  displayPtr->hibernate();

  Serial.printf("Displayed %d bytes\n", pageCharCount);
  return pageCharCount;
}

void displayMessage(const String& title, const String& message) {
  if (!displayPtr) return;

  displayPtr->setFullWindow();
  displayPtr->firstPage();

  do {
    displayPtr->fillScreen(GxEPD_WHITE);
    displayPtr->setTextColor(GxEPD_BLACK);

    u8g2.setFontMode(1);
    u8g2.setFontDirection(0);
    u8g2.setForegroundColor(GxEPD_BLACK);
    u8g2.setBackgroundColor(GxEPD_WHITE);
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);

    int titleWidth = title.length() * 6;
    u8g2.setCursor((SCREEN_WIDTH - titleWidth) / 2, 30);
    u8g2.print(title);

    int msgWidth = message.length() * 6;
    u8g2.setCursor((SCREEN_WIDTH - msgWidth) / 2, SCREEN_HEIGHT / 2);
    u8g2.print(message);

  } while (displayPtr->nextPage());

  displayPtr->hibernate();
}

String readPageContent(File& file, long startPos) {
  if (!file.seek(startPos)) return "";

  String content = "";
  int bytesRead = 0;
  int maxBytes = CHARS_PER_LINE * LINES_PER_PAGE * 10;

  while (file.available() && bytesRead < maxBytes) {
    content += (char)file.read();
    bytesRead++;
  }

  return content;
}

bool openBook(const String& bookName) {
  if (reading.bookFile) {
    reading.bookFile.close();
  }

  String path = "/books/" + bookName;
  if (!LittleFS.exists(path)) {
    Serial.println("Book not found: " + path);
    return false;
  }

  reading.bookFile = LittleFS.open(path, "r");
  if (!reading.bookFile) {
    Serial.println("Failed to open book: " + path);
    return false;
  }

  reading.currentBook = bookName;
  reading.currentPage = 0;
  reading.currentFilePosition = 0;
  reading.pageHistoryCount = 0;
  reading.pagePositions[0] = 0;
  reading.pageHistoryCount = 1;
  reading.isReading = true;

  const int kChunk = 2048;
  long size = reading.bookFile.size();
  long pos = 0;
  int pages = 0;
  while (pos < size) {
    String chunk = readChunk(reading.bookFile, pos, kChunk);
    if (chunk.length() == 0) break;
    int consumed = calcPageBytes(chunk);
    if (consumed <= 0) consumed = 1;
    pos += consumed;
    pages++;
  }
  reading.bookFile.seek(0);
  reading.totalPages = pages;

  Serial.printf("Opened book: %s (Pages: %d)\n", bookName.c_str(), reading.totalPages);

  String content = readPageContent(reading.bookFile, reading.currentFilePosition);
  int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);
  reading.currentFilePosition += charsDisplayed;

  return true;
}

void nextPage() {
  if (!reading.isReading || !reading.bookFile) return;

  if (reading.currentFilePosition >= reading.bookFile.size()) {
    Serial.println("Already at the last page");
    return;
  }

  reading.currentPage++;

  if (reading.pageHistoryCount < MAX_PAGE_HISTORY) {
    reading.pagePositions[reading.pageHistoryCount] = reading.currentFilePosition;
    reading.pageHistoryCount++;
  }

  String content = readPageContent(reading.bookFile, reading.currentFilePosition);
  int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);
  reading.currentFilePosition += charsDisplayed;

  Serial.printf("Page: %d, File position: %ld / %ld\n", reading.currentPage + 1,
                reading.currentFilePosition, reading.bookFile.size());
}

void prevPage() {
  if (!reading.isReading || !reading.bookFile) return;

  if (reading.currentPage == 0) {
    Serial.println("Already at the first page");
    return;
  }

  reading.currentPage--;

  if (reading.currentPage < reading.pageHistoryCount) {
    long prevPosition = reading.pagePositions[reading.currentPage];
    reading.currentFilePosition = prevPosition;

    String content = readPageContent(reading.bookFile, reading.currentFilePosition);
    int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);

    reading.currentFilePosition = prevPosition + charsDisplayed;

    Serial.printf("Page: %d, File position: %ld / %ld\n", reading.currentPage + 1,
                  reading.currentFilePosition, reading.bookFile.size());
  }
}
