// ============================================================================
// Button Handler Header
// ============================================================================
// This file handles button input using OneButton library
// Boot button functions:
// - Single click: Next page
// - Double click: Previous page
// - Long press: Return to home/close book
// ============================================================================

#ifndef BUTTON_H
#define BUTTON_H

#include <OneButton.h>

// ============================================================================
// Button Setup and Handling
// ============================================================================
void setupButton();
void handleButton();

#endif // BUTTON_H
