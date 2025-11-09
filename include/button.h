// ============================================================================
// Button Handler Header
// ============================================================================
// This file handles button input using OneButton library
// Boot button (GPIO 9) functions:
// - Single click: Next page (in reading mode) / Select (in menu mode)
// - Double click: Previous page (in reading mode)
// - Long press: Enter book list menu / Close book
// UP button (GPIO 12): Navigate up in menu
// DOWN button (GPIO 18): Navigate down in menu
// ============================================================================

#ifndef BUTTON_H
#define BUTTON_H

#include <OneButton.h>

// ============================================================================
// UI Mode Enum
// ============================================================================
enum UIMode {
  MODE_READING,  // Reading a book
  MODE_BOOKLIST  // Browsing book list
};

extern UIMode currentUIMode;

// ============================================================================
// Button Setup and Handling
// ============================================================================
void setupButton();
void handleButton();

// ============================================================================
// Menu Navigation Functions
// ============================================================================
void enterBookListMode();
void exitBookListMode();

#endif // BUTTON_H
