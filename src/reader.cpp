// ============================================================================
// Reader Management Implementation
// ============================================================================

#include "reader.h"

// ============================================================================
// Text Display Configuration
// ============================================================================
// 注意：屏幕使用 setRotation(1) 旋转了90度
// 物理尺寸：112*212，旋转后实际显示尺寸：212x112
const int SCREEN_WIDTH = EPD_HEIGHT; // 旋转后的实际宽度 = 212
const int SCREEN_HEIGHT = EPD_WIDTH; // 旋转后的实际高度 = 112

const int CHINESE_CHAR_WIDTH = 12; // 中文字符宽度（像素）- u8g2_font_wqy12_t_gb2312
const int ASCII_CHAR_WIDTH = 6;    // ASCII字符宽度（像素）
const int LINE_HEIGHT = 13;        // 行高（像素）
const int MARGIN_LEFT = 2;         // 左边距
const int MARGIN_TOP = 2;          // 上边距（U8g2会自动从基线向上绘制字体）
const int MARGIN_RIGHT = 2;        // 右边距（增加以避免右侧文字被裁切）
const int MARGIN_BOTTOM = 10;      // 下边距（留给页码）
const int PAGE_NUM_HEIGHT = 10;    // 页码区域高度

const int DISPLAY_WIDTH = SCREEN_WIDTH - MARGIN_LEFT - MARGIN_RIGHT;   // 212-4-8=200像素
const int DISPLAY_HEIGHT = SCREEN_HEIGHT - MARGIN_TOP - MARGIN_BOTTOM; // 112-10-10=92像素
const int CHARS_PER_LINE = DISPLAY_WIDTH / CHINESE_CHAR_WIDTH;         // 200/12=16个中文字符
const int LINES_PER_PAGE = DISPLAY_HEIGHT / LINE_HEIGHT;               // 92/13=7行
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
  if (c < 0x80)
    return 1; // ASCII
  if ((c & 0xE0) == 0xC0)
    return 2; // 2字节字符
  if ((c & 0xF0) == 0xE0)
    return 3; // 3字节字符（中文主要是这个）
  if ((c & 0xF8) == 0xF0)
    return 4; // 4字节字符
  return 1;   // 默认
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
      width += ASCII_CHAR_WIDTH; // ASCII字符宽度
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

int displayText(const String& text, int pageNum, int totalPages) {
  if (!displayPtr)
    return 0;

  int totalCharsDisplayed = 0; // 记录实际显示的字节数

  displayPtr->setFullWindow();
  displayPtr->firstPage();

  int charIndex = 0;
  int lineCount = 0;
  String displayedText = "";
  do {
    displayPtr->fillScreen(GxEPD_WHITE);
    displayPtr->setTextColor(GxEPD_BLACK);

    // 使用U8g2中文字体 - 12号字体
    u8g2.setFontMode(1);      // 透明模式
    u8g2.setFontDirection(0); // 从左到右
    u8g2.setForegroundColor(GxEPD_BLACK);
    u8g2.setBackgroundColor(GxEPD_WHITE);
    u8g2.setFont(u8g2_font_wqy12_t_gb2312); // WenQuanYi 12px 中文字体

    // 计算字体指标与分页：使用真实的行进高度以充分利用屏幕
    int fontAscent = u8g2.getFontAscent();     // 通常为正值
    int fontDescent = u8g2.getFontDescent();   // 通常为负值
    int lineAdvance = fontAscent - fontDescent; // 实际行高（基线到下一行基线）
    int availableHeight = SCREEN_HEIGHT - MARGIN_TOP - PAGE_NUM_HEIGHT; // 预留页码区域
    int linesPerPage = availableHeight / lineAdvance;

    // 显示文本内容
    int x = MARGIN_LEFT;
    // 基线 = 顶边 + ascent，使首行完整可见
    int y = MARGIN_TOP + fontAscent + 1; // 微调1px，避免面板顶部可能的裁切


    while (charIndex < text.length()) {
      // 超出可用显示区域则停止（确保最后一行完整显示）
      if (y - fontDescent > (SCREEN_HEIGHT - PAGE_NUM_HEIGHT)) {
        break;
      }
      String line = "";
      int linePixelWidth = 0; // 使用像素宽度而不是字符单位

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
        displayedText += line; // 记录显示的文本
      }

      y += lineAdvance;
      lineCount++;
    }
    // 显示页码（使用小字体，紧贴底部）
    u8g2.setFont(u8g2_font_6x10_tf);
    String pageInfo = String(pageNum + 1) + " / " + String(totalPages);
    int pageInfoWidth = pageInfo.length() * 6;
    // 将页码顶边放在底部保留区顶部：基线 = 顶边 + ascent
    u8g2.setCursor((SCREEN_WIDTH - pageInfoWidth) / 2, SCREEN_HEIGHT - PAGE_NUM_HEIGHT + u8g2.getFontAscent());
    u8g2.print(pageInfo);

  } while (displayPtr->nextPage());

  displayPtr->hibernate();

  totalCharsDisplayed = charIndex; // 更新为实际处理的字节数

  //  打印所有参数,打印显示在屏幕的文字
  Serial.printf("Displayed %d bytes, %d lines\n", totalCharsDisplayed, lineCount);
  Serial.println("Displayed Text: " + displayedText);
  return totalCharsDisplayed; // 返回实际显示的字节数
}

void displayMessage(const String& title, const String& message) {
  if (!displayPtr)
    return;

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

String readPageContent(File& file, long startPos) {
  // 从指定位置开始读取
  if (!file.seek(startPos)) {
    return "";
  }

  String content = "";
  int bytesRead = 0;

  // 对于中文，每个字符约3字节，一页约126字符 = 378字节
  // 读取约3倍的字符数量以确保有足够内容（约1000字节）
  int maxBytes = CHARS_PER_PAGE * 10; // 增加读取量以确保足够的内容

  while (file.available() && bytesRead < maxBytes) {
    char c = file.read();
    content += c;
    bytesRead++;
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
  reading.currentFilePosition = 0; // 从文件开始位置
  reading.pageHistoryCount = 0;    // 清空历史记录
  reading.pagePositions[0] = 0;    // 第一页从0开始
  reading.pageHistoryCount = 1;
  reading.totalPages = (reading.bookFile.size() / CHARS_PER_PAGE) + 1; // 粗略估算
  reading.isReading = true;

  Serial.printf("Opened book: %s (Estimated pages: %d)\n", bookName.c_str(), reading.totalPages);

  // 显示第一页
  String content = readPageContent(reading.bookFile, reading.currentFilePosition);
  int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);
  reading.currentFilePosition += charsDisplayed; // 更新文件位置

  return true;
}

void nextPage() {
  if (!reading.isReading || !reading.bookFile) {
    return;
  }

  // 检查是否还有内容
  if (reading.currentFilePosition >= reading.bookFile.size()) {
    Serial.println("Already at the last page");
    return;
  }

  reading.currentPage++;

  // 保存当前页的起始位置到历史记录
  if (reading.pageHistoryCount < MAX_PAGE_HISTORY) {
    reading.pagePositions[reading.pageHistoryCount] = reading.currentFilePosition;
    reading.pageHistoryCount++;
  }

  // 从当前文件位置读取内容
  String content = readPageContent(reading.bookFile, reading.currentFilePosition);
  int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);
  reading.currentFilePosition += charsDisplayed; // 更新文件位置

  Serial.printf("Page: %d, File position: %ld / %ld\n", reading.currentPage + 1,
                reading.currentFilePosition, reading.bookFile.size());
}

void prevPage() {
  if (!reading.isReading || !reading.bookFile) {
    return;
  }

  if (reading.currentPage == 0) {
    Serial.println("Already at the first page");
    return;
  }

  reading.currentPage--;

  // 从历史记录中获取上一页的文件位置
  if (reading.currentPage < reading.pageHistoryCount) {
    long prevPosition = reading.pagePositions[reading.currentPage];
    reading.currentFilePosition = prevPosition;

    // 读取并显示上一页
    String content = readPageContent(reading.bookFile, reading.currentFilePosition);
    int charsDisplayed = displayText(content, reading.currentPage, reading.totalPages);

    // 更新文件位置到这一页的结束位置
    reading.currentFilePosition = prevPosition + charsDisplayed;

    Serial.printf("Page: %d, File position: %ld / %ld\n", reading.currentPage + 1,
                  reading.currentFilePosition, reading.bookFile.size());
  }
}
