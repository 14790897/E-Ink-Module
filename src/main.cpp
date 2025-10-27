// ============================================================================
// E-Ink Novel Reader for ESP32-C3
// ============================================================================
// Features:
// - WiFi AP mode for web interface access
// - LittleFS file system for storing novels
// - Web interface for uploading and managing books
// - Text pagination and display on E-Ink screen
// - Partial refresh support for fast page turns
// - Boot button control: Single click (next), Double click (prev), Long press (close)
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <ESPmDNS.h>
#include "secrets.h"
#include "storage.h"
#include "reader.h"
#include "api.h"
#include "button.h"

// ============================================================================
// WiFi Configuration
// ============================================================================
// 从secrets.h读取WiFi配置
// 如果连接失败，会启动AP模式作为备用
const char* AP_SSID = "EInk-Reader";      // AP模式的默认SSID
const char* AP_PASSWORD = "12345678";     // AP模式的默认密码

// ============================================================================
// Global Objects
// ============================================================================
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(
  GxEPD2_213_BN(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)
);

AsyncWebServer server(80);

// ============================================================================
// WiFi Setup
// ============================================================================

void setupWiFi() {
  Serial.println("====================================");
  Serial.println("WiFi Configuration");
  Serial.println("====================================");

  // 尝试连接到WiFi（STA模式）
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // 等待连接，最多30秒
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  IPAddress IP;
  String accessInfo;

  if (WiFi.status() == WL_CONNECTED) {
    // STA模式连接成功
    IP = WiFi.localIP();
    Serial.println("WiFi Connected (STA Mode)!");
    Serial.print("IP Address: ");
    Serial.println(IP);
    Serial.printf("mDNS: http://%s.local\n", MDNS_HOSTNAME);

    // 启动mDNS
    if (MDNS.begin(MDNS_HOSTNAME)) {
      Serial.printf("mDNS responder started: %s.local\n", MDNS_HOSTNAME);
      MDNS.addService("http", "tcp", WEB_SERVER_PORT);
    } else {
      Serial.println("Error starting mDNS");
    }

    accessInfo = String(MDNS_HOSTNAME) + ".local";
    displayMessage("WiFi Connected", accessInfo);

  } else {
    // STA模式失败，启动AP模式
    Serial.println("WiFi connection failed!");
    Serial.println("Starting AP mode as fallback...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    IP = WiFi.softAPIP();
    Serial.println("AP Mode Started");
    Serial.printf("SSID: %s\n", AP_SSID);
    Serial.printf("Password: %s\n", AP_PASSWORD);
    Serial.print("AP IP: ");
    Serial.println(IP);

    accessInfo = IP.toString();
    displayMessage("WiFi AP Mode", accessInfo);
  }

  Serial.println("====================================");
}

// ============================================================================
// Arduino Setup
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("====================================");
  Serial.println("E-Ink Novel Reader Starting...");
  Serial.println("====================================");

  // 初始化SPI
  SPI.begin(HW_SCK, HW_MISO, HW_MOSI, PIN_CS);

  // 初始化显示屏
  display.init(115200, true, 2, false);
  display.setRotation(1); // 横屏模式

  // 初始化reader模块的显示
  initDisplay(display);

  // 显示启动画面
  displayMessage("Starting", "Initializing...");

  // 初始化LittleFS
  if (!initLittleFS()) {
    displayMessage("Error", "FS Init Failed");
    while (1) { delay(1000); }
  }

  // 初始化WiFi
  setupWiFi();

  // 初始化API和Web服务器
  setupAPI(server);

  // 初始化按钮控制
  setupButton();

  // 初始化阅读状态
  reading.isReading = false;
  reading.currentPage = 0;
  reading.totalPages = 0;

  Serial.println("System ready!");
  Serial.println("====================================");
  Serial.println("Access Information:");

  if (WiFi.getMode() == WIFI_STA) {
    Serial.printf("Mode: WiFi Station (STA)\n");
    Serial.printf("URL: http://%s.local\n", MDNS_HOSTNAME);
    Serial.printf("IP: http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("Mode: WiFi Access Point (AP)\n");
    Serial.printf("SSID: %s\n", AP_SSID);
    Serial.printf("Password: %s\n", AP_PASSWORD);
    Serial.printf("URL: http://192.168.4.1\n");
  }

  Serial.println("====================================");
}

// ============================================================================
// Arduino Loop
// ============================================================================

void loop() {
  // 处理按钮事件
  handleButton();

  // Web服务器由AsyncWebServer异步处理
  delay(10);  // 减小延迟以提高按钮响应速度
}
