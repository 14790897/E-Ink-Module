// ============================================================================
// Button Handler Implementation
// ============================================================================

#include "button.h"
#include "secrets.h"
#include "reader.h"
#include <Arduino.h>

// ============================================================================
// OneButton Instance
// ============================================================================
// Boot button is active LOW (pressed = LOW, released = HIGH)
OneButton button(BOOT_BUTTON_PIN, true, true);

// ============================================================================
// Button Callback Functions
// ============================================================================

// 单击：下一页
void onSingleClick() {
  Serial.println("Button: Single Click - Next Page");
  nextPage();
}

// 双击：上一页
void onDoubleClick() {
  Serial.println("Button: Double Click - Previous Page");
  prevPage();
}

// 长按：关闭当前书籍
void onLongPress() {
  Serial.println("Button: Long Press - Close Book");

  if (reading.isReading) {
    // 关闭当前书籍
    if (reading.bookFile) {
      reading.bookFile.close();
    }

    reading.isReading = false;
    reading.currentPage = 0;
    reading.totalPages = 0;
    reading.currentBook = "";

    // 显示关闭消息
    displayMessage("Book Closed", "Ready for new book");
  } else {
    // 如果没有打开的书籍，显示状态信息
    displayMessage("No Book", "Upload via web");
  }
}

// ============================================================================
// Button Setup
// ============================================================================

void setupButton() {
  Serial.println("Setting up button controls...");

  // 配置按钮回调函数
  button.attachClick(onSingleClick);
  button.attachDoubleClick(onDoubleClick);
  button.attachLongPressStart(onLongPress);

  // 配置按钮参数
  button.setClickMs(250);           // 单击检测时间 (ms)
  button.setDebounceMs(50);         // 去抖动时间 (ms)
  button.setPressMs(1000);          // 长按触发时间 (ms)

  Serial.println("Button controls ready:");
  Serial.println("  - Single Click: Next Page");
  Serial.println("  - Double Click: Previous Page");
  Serial.println("  - Long Press: Close Book");
}

// ============================================================================
// Button Handler (call in loop)
// ============================================================================

void handleButton() {
  button.tick();
}
