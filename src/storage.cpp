// ============================================================================
// Storage Management Implementation
// ============================================================================

#include "storage.h"
#include <ArduinoJson.h>
#include "reader.h"

// ============================================================================
// LittleFS Functions
// ============================================================================

bool initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return false;
  }
  Serial.println("LittleFS Mounted Successfully");

  // 创建books目录
  if (!LittleFS.exists("/books")) {
    LittleFS.mkdir("/books");
    Serial.println("Created /books directory");
  }

  return true;
}

String getStorageInfo() {
  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();

  char buffer[50];
  sprintf(buffer, "%d KB / %d KB", usedBytes / 1024, totalBytes / 1024);
  return String(buffer);
}

// ============================================================================
// Book Management Functions
// ============================================================================

String listBooks() {
  JsonDocument doc;
  JsonArray books = doc.to<JsonArray>();

  File root = LittleFS.open("/books");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open /books directory");
    root.close();
    String output;
    serializeJson(doc, output);
    return output;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      JsonObject book = books.add<JsonObject>();

      // 获取文件名并确保UTF-8编码正确
      const char* fileName = file.name();
      book["name"] = fileName;
      book["size"] = String(file.size()) + " bytes";

      // 估算页数
      int pages = (file.size() / CHARS_PER_PAGE) + 1;
      book["pages"] = pages;
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();

  String output;
  serializeJson(doc, output);
  return output;
}

bool deleteBook(const String& bookName) {
  String path = "/books/" + bookName;
  if (LittleFS.exists(path)) {
    return LittleFS.remove(path);
  }
  return false;
}
