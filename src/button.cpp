// ============================================================================
// Button Handler Implementation
// ============================================================================

#include "button.h"
#include "secrets.h"
#include "reader.h"
#include "storage.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>

// ============================================================================
// UI Mode State
// ============================================================================
UIMode currentUIMode = MODE_READING;

// ============================================================================
// Book List State
// ============================================================================
std::vector<String> bookList;
int selectedBookIndex = 0;

// ============================================================================
// OneButton Instances
// ============================================================================
// Boot button is active LOW (pressed = LOW, released = HIGH)
OneButton buttonBoot(BOOT_BUTTON_PIN, true, true);
OneButton buttonUp(UP_BUTTON_PIN, true, true);
OneButton buttonDown(DOWN_BUTTON_PIN, true, true);

// ============================================================================
// Book List Management Functions
// ============================================================================

void loadBookList() {
  bookList.clear();

  File root = LittleFS.open("/books");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open /books directory");
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      bookList.push_back(String(file.name()));
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();

  Serial.printf("Loaded %d books\n", bookList.size());
}

// ============================================================================
// Button Callback Functions - Reading Mode
// ============================================================================

// 单击Boot按钮 (阅读模式):下一页
void onBootSingleClick() {
  if (currentUIMode == MODE_READING) {
    Serial.println("Button: Single Click - Next Page");
    nextPage();
  } else if (currentUIMode == MODE_BOOKLIST) {
    // 选择当前书籍
    Serial.println("Button: Single Click - Select Book");
    if (bookList.size() > 0 && selectedBookIndex < bookList.size()) {
      String selectedBook = bookList[selectedBookIndex];
      Serial.printf("Opening book: %s\n", selectedBook.c_str());

      if (openBook(selectedBook)) {
        currentUIMode = MODE_READING;
        Serial.println("Switched to reading mode");
      } else {
        displayMessage("Error", "Failed to open book");
      }
    }
  }
}

// 双击Boot按钮 (阅读模式):上一页
void onBootDoubleClick() {
  if (currentUIMode == MODE_READING) {
    Serial.println("Button: Double Click - Previous Page");
    prevPage();
  }
}

// 长按Boot按钮:进入书籍列表/关闭书籍
void onBootLongPress() {
  Serial.println("Button: Long Press");

  if (currentUIMode == MODE_READING) {
    if (reading.isReading) {
      // 关闭当前书籍,进入书籍列表
      Serial.println("Closing book and entering book list");
      if (reading.bookFile) {
        reading.bookFile.close();
      }
      reading.isReading = false;
      enterBookListMode();
    } else {
      // 没有打开的书籍,直接进入书籍列表
      Serial.println("Entering book list");
      enterBookListMode();
    }
  } else if (currentUIMode == MODE_BOOKLIST) {
    // 退出书籍列表
    Serial.println("Exiting book list");
    exitBookListMode();
  }
}

// ============================================================================
// Button Callback Functions - Navigation
// ============================================================================

// UP按钮:向上导航
void onUpButtonClick() {
  if (currentUIMode == MODE_BOOKLIST) {
    if (selectedBookIndex > 0) {
      selectedBookIndex--;
      Serial.printf("Selected index: %d\n", selectedBookIndex);
      displayBookList(bookList, selectedBookIndex);
    } else {
      Serial.println("Already at top of list");
    }
  }
}

// DOWN按钮:向下导航
void onDownButtonClick() {
  if (currentUIMode == MODE_BOOKLIST) {
    if (selectedBookIndex < bookList.size() - 1) {
      selectedBookIndex++;
      Serial.printf("Selected index: %d\n", selectedBookIndex);
      displayBookList(bookList, selectedBookIndex);
    } else {
      Serial.println("Already at bottom of list");
    }
  }
}

// ============================================================================
// Menu Mode Functions
// ============================================================================

void enterBookListMode() {
  currentUIMode = MODE_BOOKLIST;
  selectedBookIndex = 0;

  // 加载书籍列表
  loadBookList();

  if (bookList.size() == 0) {
    displayMessage("No Books", "Upload via web");
    return;
  }

  // 显示书籍列表
  displayBookList(bookList, selectedBookIndex);
}

void exitBookListMode() {
  currentUIMode = MODE_READING;

  if (reading.isReading) {
    // 恢复阅读界面
    String content = readPageContent(reading.bookFile, reading.currentFilePosition);
    displayText(content, reading.currentPage, reading.totalPages);
  } else {
    displayMessage("Ready", "Long press to browse books");
  }
}

// ============================================================================
// Button Setup
// ============================================================================

void setupButton() {
  Serial.println("Setting up button controls...");

  // 配置GPIO引脚为上拉输入模式
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  // pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  // 配置Boot按钮回调函数
  buttonBoot.attachClick(onBootSingleClick);
  buttonBoot.attachDoubleClick(onBootDoubleClick);
  buttonBoot.attachLongPressStart(onBootLongPress);

  // 配置Boot按钮参数
  buttonBoot.setClickMs(250);           // 单击检测时间 (ms)
  buttonBoot.setDebounceMs(50);         // 去抖动时间 (ms)
  buttonBoot.setPressMs(1000);          // 长按触发时间 (ms)

  // 配置UP按钮
  buttonUp.attachClick(onUpButtonClick);
  buttonUp.setDebounceMs(50);

  // 配置DOWN按钮
  buttonDown.attachClick(onDownButtonClick);
  buttonDown.setDebounceMs(50);

  Serial.println("Button controls ready:");
  Serial.println("  Boot Button:");
  Serial.println("    - Single Click: Next Page / Select");
  Serial.println("    - Double Click: Previous Page");
  Serial.println("    - Long Press: Enter/Exit Book List");
  Serial.println("  GPIO 12: Navigate Up");
  Serial.println("  GPIO 18: Navigate Down");
}

// ============================================================================
// Button Handler (call in loop)
// ============================================================================

void handleButton() {
  buttonBoot.tick();
  buttonUp.tick();
  buttonDown.tick();
}
