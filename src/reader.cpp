// ============================================================================
// Reader Management Implementation
// ============================================================================

#include "reader.h"

// ============================================================================
// Text Display Configuration
// ============================================================================
// 注意：屏幕使用 setRotation(1) 旋转了90度
// 物理尺寸：250x122，旋转后实际显示尺寸：250x122（宽x高）
const int SCREEN_WIDTH = EPD_HEIGHT;   // 旋转后的实际宽度 = 122
const int SCREEN_HEIGHT = EPD_WIDTH;   // 旋转后的实际高度 = 250

const int CHINESE_CHAR_WIDTH = 12;     // 中文字符宽度（像素）- u8g2_font_wqy12_t_gb2312
const int ASCII_CHAR_WIDTH = 6;        // ASCII字符宽度（像素）
const int LINE_HEIGHT = 13;            // 行高（像素）
const int MARGIN_LEFT = 2;             // 左边距
const int MARGIN_TOP = 2;              // 上边距
const int MARGIN_RIGHT = 2;            // 右边距
const int MARGIN_BOTTOM = 14;          // 下边距（留给页码）

const int DISPLAY_WIDTH = SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT;
const int DISPLAY_HEIGHT = SCREEN_HEIGHT - MARGIN_TOP - MARGIN_BOTTOM;
const int CHARS_PER_LINE = DISPLAY_WIDTH / CHINESE_CHAR_WIDTH;  // 按中文字符计算
const int LINES_PER_PAGE = DISPLAY_HEIGHT / LINE_HEIGHT;
const int CHARS_PER_PAGE = CHARS_PER_LINE * LINES_PER_PAGE;

// ============================================================================
// Reading State
// ============================================================================
ReadingState reading;

// ============================================================================
// Global Display Objects
// ============================================================================
static GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>* displayPtr = nullptr;
static U8G2_FOR_ADAFRUIT_GFX u8g2;

// ============================================================================
// UTF-8 Helper Functions
// ============================================================================

// 获取UTF-8字符的字节长度
int getUTF8CharLength(const char* str, int index) {
  unsigned char c = str[index];
  if (c < 0x80) return 1;           // ASCII
  if ((c & 0xE0) == 0xC0) return 2; // 2字节字符
  if ((c & 0xF0) == 0xE0) return 3; // 3字节字符（中文主要是这个）
  if ((c & 0xF8) == 0xF0) return 4; // 4字节字符
  return 1; // 默认
}

// 计算字符串的显示宽度（中文字符按12像素计算，ASCII按6像素计算）
int getStringDisplayWidth(const String& str) {
  int width = 0;
  int i = 0;
  while (i < str.length()) {
    int charLen = getUTF8CharLength(str.c_str(), i);
    if (charLen > 1) {
      width += CHINESE_CHAR_WIDTH; // 中文字符宽度
    } else {
      width += ASCII_CHAR_WIDTH;   // ASCII字符宽度
    }
    i += charLen;
  }
  return width;
}

// ============================================================================
// Display Functions
// ============================================================================

void initDisplay(GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT>& display) {
  displayPtr = &display;

  // 初始化U8g2字体
  u8g2.begin(*displayPtr);

  Serial.println("Display initialized with Chinese font support");
}

void displayText(const String& text, int pageNum, int totalPages) {
  if (!displayPtr) return;

  displayPtr->setFullWindow();
  displayPtr->firstPage();

  do {
    displayPtr->fillScreen(GxEPD_WHITE);
    displayPtr->setTextColor(GxEPD_BLACK);

    // 使用U8g2中文字体 - 12号字体
    u8g2.setFontMode(1);                    // 透明模式
    u8g2.setFontDirection(0);               // 从左到右
    u8g2.setForegroundColor(GxEPD_BLACK);
    u8g2.setBackgroundColor(GxEPD_WHITE);
    u8g2.setFont(u8g2_font_wqy12_t_gb2312); // WenQuanYi 12px 中文字体

    // 显示文本内容
    int x = MARGIN_LEFT;
    int y = MARGIN_TOP + 11; // U8g2字体的基线位置

    int charIndex = 0;
    int lineCount = 0;

    while (charIndex < text.length() && lineCount < LINES_PER_PAGE) {
      String line = "";
      int linePixelWidth = 0;  // 使用像素宽度而不是字符单位

      // 逐字符构建一行
      while (charIndex < text.length() && linePixelWidth < DISPLAY_WIDTH) {
        // 检查换行符
        if (text[charIndex] == '\n' || text[charIndex] == '\r') {
          charIndex++;
          if (text[charIndex - 1] == '\r' && charIndex < text.length() && text[charIndex] == '\n') {
            charIndex++; // 跳过 \r\n 中的 \n
          }
          break;
        }

        // 获取当前字符长度
        int charLen = getUTF8CharLength(text.c_str(), charIndex);

        // 提取字符
        String currentChar = text.substring(charIndex, charIndex + charLen);

        // 计算添加此字符后的像素宽度
        int charPixelWidth = (charLen > 1) ? CHINESE_CHAR_WIDTH : ASCII_CHAR_WIDTH;

        if (linePixelWidth + charPixelWidth > DISPLAY_WIDTH) {
          break; // 超出行宽，停止添加
        }

        line += currentChar;
        linePixelWidth += charPixelWidth;
        charIndex += charLen;
      }

      // 显示这一行
      if (line.length() > 0) {
        u8g2.setCursor(x, y);
        u8g2.print(line);
      }

      y += LINE_HEIGHT;
      lineCount++;
    }

    // 显示页码（使用小字体）
    u8g2.setFont(u8g2_font_6x10_tf);
    String pageInfo = String(pageNum + 1) + " / " + String(totalPages);
    int pageInfoWidth = pageInfo.length() * 6;
    u8g2.setCursor((SCREEN_WIDTH - pageInfoWidth) / 2, SCREEN_HEIGHT - 4);
    u8g2.print(pageInfo);

  } while (displayPtr->nextPage());

  displayPtr->hibernate();
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

    // 显示标题 - 使用中号字体
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    int titleWidth = title.length() * 6; // 粗略估算
    u8g2.setCursor((SCREEN_WIDTH - titleWidth) / 2, 30);
    u8g2.print(title);

    // 显示消息
    int msgWidth = message.length() * 6;
    u8g2.setCursor((SCREEN_WIDTH - msgWidth) / 2, SCREEN_HEIGHT / 2);
    u8g2.print(message);

  } while (displayPtr->nextPage());

  displayPtr->hibernate();
}

// ============================================================================
// Reading Functions
// ============================================================================

String readPageContent(File& file, int pageNum) {
  // 计算起始位置
  long startPos = pageNum * CHARS_PER_PAGE;

  if (!file.seek(startPos)) {
    return "";
  }

  String content = "";
  int charsRead = 0;

  while (file.available() && charsRead < CHARS_PER_PAGE) {
    char c = file.read();
    content += c;
    charsRead++;
  }

  return content;
}

bool openBook(const String& bookName) {
  // 关闭当前书籍
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
  reading.totalPages = (reading.bookFile.size() / CHARS_PER_PAGE) + 1;
  reading.isReading = true;

  Serial.printf("Opened book: %s (Pages: %d)\n", bookName.c_str(), reading.totalPages);

  // 显示第一页
  String content = readPageContent(reading.bookFile, reading.currentPage);
  displayText(content, reading.currentPage, reading.totalPages);

  return true;
}

void nextPage() {
  if (!reading.isReading || !reading.bookFile) {
    return;
  }

  if (reading.currentPage < reading.totalPages - 1) {
    reading.currentPage++;
    String content = readPageContent(reading.bookFile, reading.currentPage);
    displayText(content, reading.currentPage, reading.totalPages);
    Serial.printf("Page: %d / %d\n", reading.currentPage + 1, reading.totalPages);
  }
}

void prevPage() {
  if (!reading.isReading || !reading.bookFile) {
    return;
  }

  if (reading.currentPage > 0) {
    reading.currentPage--;
    String content = readPageContent(reading.bookFile, reading.currentPage);
    displayText(content, reading.currentPage, reading.totalPages);
    Serial.printf("Page: %d / %d\n", reading.currentPage + 1, reading.totalPages);
  }
}
