// ============================================================================
// API Handler Header
// ============================================================================
// This file contains all web server API endpoint handlers
// ============================================================================

#ifndef API_H
#define API_H

#include <ESPAsyncWebServer.h>
#include "storage.h"
#include "reader.h"

// ============================================================================
// API Setup Function
// ============================================================================
void setupAPI(AsyncWebServer& server);

#endif // API_H
