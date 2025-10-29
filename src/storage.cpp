// ============================================================================
// Storage Management Implementation
// ============================================================================

#include "storage.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "reader.h"

// NVS命名空间
Preferences preferences;

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

// ============================================================================
// NVS Reading Record Functions
// ============================================================================

bool initNVS() {
  // Preferences会在begin时自动初始化NVS
  return true;
}

bool saveReadingRecord(const String& bookName, long filePosition, int currentPage) {
  if (!preferences.begin("reading", false)) {
    Serial.println("Failed to open NVS namespace");
    return false;
  }

  preferences.putString("bookName", bookName);
  preferences.putLong("filePos", filePosition);
  preferences.putInt("page", currentPage);

  preferences.end();

  Serial.printf("Saved reading record: %s, pos=%ld, page=%d\n",
                bookName.c_str(), filePosition, currentPage);
  return true;
}

bool loadReadingRecord(String& bookName, long& filePosition, int& currentPage) {
  if (!preferences.begin("reading", true)) {
    Serial.println("Failed to open NVS namespace");
    return false;
  }

  bookName = preferences.getString("bookName", "");
  filePosition = preferences.getLong("filePos", 0);
  currentPage = preferences.getInt("page", 0);

  preferences.end();

  if (bookName.length() == 0) {
    Serial.println("No reading record found in NVS");
    return false;
  }

  Serial.printf("Loaded reading record: %s, pos=%ld, page=%d\n",
                bookName.c_str(), filePosition, currentPage);
  return true;
}

bool clearReadingRecord() {
  if (!preferences.begin("reading", false)) {
    Serial.println("Failed to open NVS namespace");
    return false;
  }

  preferences.clear();
  preferences.end();

  Serial.println("Cleared reading record from NVS");
  return true;
}

