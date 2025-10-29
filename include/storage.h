// ============================================================================
// Storage Management Header
// ============================================================================
// This file handles LittleFS operations and book file management
// ============================================================================

#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>

// ============================================================================
// LittleFS Functions
// ============================================================================
bool initLittleFS();
String getStorageInfo();

// ============================================================================
// Book Management Functions
// ============================================================================
String listBooks();
bool deleteBook(const String& bookName);

// ============================================================================
// NVS Reading Record Functions
// ============================================================================
bool initNVS();
bool saveReadingRecord(const String& bookName, long filePosition, int currentPage);
bool loadReadingRecord(String& bookName, long& filePosition, int& currentPage);
bool clearReadingRecord();

#endif // STORAGE_H
