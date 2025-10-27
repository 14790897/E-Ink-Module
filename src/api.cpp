// ============================================================================
// API Handler Implementation
// ============================================================================

#include "api.h"
#include <ArduinoJson.h>

// ============================================================================
// API Setup Function
// ============================================================================

void setupAPI(AsyncWebServer& server) {
  // 提供静态文件
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // API: 获取状态
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["currentPage"] = reading.isReading ? reading.currentPage : 0;
    doc["totalPages"] = reading.isReading ? reading.totalPages : 0;
    doc["currentBook"] = reading.isReading ? reading.currentBook : "无";
    doc["storage"] = getStorageInfo();

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
  });

  // API: 获取书籍列表
  server.on("/api/books", HTTP_GET, [](AsyncWebServerRequest *request) {
    String bookList = listBooks();
    request->send(200, "application/json; charset=utf-8", bookList);
  });

  // API: 上传书籍
  server.on("/api/upload", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      // 文件上传完成后的响应
      JsonDocument doc;
      doc["success"] = true;
      doc["message"] = "Upload successful";

      String response;
      serializeJson(doc, response);
      request->send(200, "application/json; charset=utf-8", response);
    },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      static File uploadFile;

      if (index == 0) {
        Serial.printf("Upload Start: %s\n", filename.c_str());
        String path = "/books/" + filename;
        uploadFile = LittleFS.open(path, "w");

        if (!uploadFile) {
          Serial.println("Failed to open file for writing");
          return;
        }
      }

      if (uploadFile) {
        uploadFile.write(data, len);
      }

      if (final) {
        if (uploadFile) {
          uploadFile.close();
          Serial.printf("Upload Complete: %s (%d bytes)\n", filename.c_str(), index + len);
        }
      }
    }
  );

  // API: 打开书籍
  server.on("/api/read", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      deserializeJson(doc, data);

      String bookName = doc["book"].as<String>();
      bool success = openBook(bookName);

      JsonDocument response;
      response["success"] = success;
      response["message"] = success ? "Book opened" : "Failed to open book";

      String output;
      serializeJson(response, output);
      request->send(200, "application/json; charset=utf-8", output);
    }
  );

  // API: 删除书籍
  server.on("/api/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      deserializeJson(doc, data);

      String bookName = doc["book"].as<String>();
      bool success = deleteBook(bookName);

      JsonDocument response;
      response["success"] = success;
      response["message"] = success ? "Book deleted" : "Failed to delete book";

      String output;
      serializeJson(response, output);
      request->send(200, "application/json; charset=utf-8", output);
    }
  );

  // API: 下一页
  server.on("/api/next", HTTP_POST, [](AsyncWebServerRequest *request) {
    nextPage();

    JsonDocument doc;
    doc["success"] = true;
    doc["currentPage"] = reading.currentPage;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
  });

  // API: 上一页
  server.on("/api/prev", HTTP_POST, [](AsyncWebServerRequest *request) {
    prevPage();

    JsonDocument doc;
    doc["success"] = true;
    doc["currentPage"] = reading.currentPage;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
  });

  server.begin();
  Serial.println("Web server started");
}
