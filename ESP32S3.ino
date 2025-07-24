#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoWebsockets.h>
#include <ESP32QRCodeReader.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ESP32S3 WROOM FREENOVE Pin Configuration
// Manual pin definition for ESP32-S3 FREENOVE board
#define LED_BUILTIN        2  // ESP32-S3 built-in LED is on GPIO 2

// ESP32-S3 FREENOVE Camera Pin Configuration
CameraPins ESP32S3_FREENOVE_PINS = {
  .PWDN_GPIO_NUM = -1,
  .RESET_GPIO_NUM = -1,
  .XCLK_GPIO_NUM = 15,
  .SIOD_GPIO_NUM = 4,
  .SIOC_GPIO_NUM = 5,
  .Y9_GPIO_NUM = 16,
  .Y8_GPIO_NUM = 17,
  .Y7_GPIO_NUM = 18,
  .Y6_GPIO_NUM = 12,
  .Y5_GPIO_NUM = 10,
  .Y4_GPIO_NUM = 8,
  .Y3_GPIO_NUM = 9,
  .Y2_GPIO_NUM = 11,
  .VSYNC_GPIO_NUM = 6,
  .HREF_GPIO_NUM = 7,
  .PCLK_GPIO_NUM = 13,
};

// WiFi & Server Configuration - IMPORTANT: CHANGE THIS TO YOUR SERVER'S IP ADDRESS
#define WEBSOCKET_SERVER_HOST "192.168.1.232"
#define WEBSOCKET_SERVER_PORT 3000

// Globals
using namespace websockets;
WebsocketsClient client;
ESP32QRCodeReader reader(ESP32S3_FREENOVE_PINS, FRAMESIZE_QVGA); // Start with smaller frame size

void onMessageCallback(WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
}

void flashOnceParsed() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(10);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
}

esp_err_t init_camera_for_streaming() {
  // Use manually defined ESP32S3 FREENOVE pin configuration
  CameraPins pins = ESP32S3_FREENOVE_PINS;
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = pins.Y2_GPIO_NUM;    // GPIO 11
  config.pin_d1 = pins.Y3_GPIO_NUM;    // GPIO 9
  config.pin_d2 = pins.Y4_GPIO_NUM;    // GPIO 8
  config.pin_d3 = pins.Y5_GPIO_NUM;    // GPIO 10
  config.pin_d4 = pins.Y6_GPIO_NUM;    // GPIO 12
  config.pin_d5 = pins.Y7_GPIO_NUM;    // GPIO 18
  config.pin_d6 = pins.Y8_GPIO_NUM;    // GPIO 17
  config.pin_d7 = pins.Y9_GPIO_NUM;    // GPIO 16
  config.pin_xclk = pins.XCLK_GPIO_NUM;   // GPIO 15
  config.pin_pclk = pins.PCLK_GPIO_NUM;   // GPIO 13
  config.pin_vsync = pins.VSYNC_GPIO_NUM; // GPIO 6
  config.pin_href = pins.HREF_GPIO_NUM;   // GPIO 7
  config.pin_sscb_sda = pins.SIOD_GPIO_NUM; // GPIO 4
  config.pin_sscb_scl = pins.SIOC_GPIO_NUM; // GPIO 5
  config.pin_pwdn = pins.PWDN_GPIO_NUM;     // -1 (not used)
  config.pin_reset = pins.RESET_GPIO_NUM;   // -1 (not used)
  config.xclk_freq_hz = 10000000;  // Reduced frequency for stability
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // Reduced from SVGA for stability
  config.jpeg_quality = 20;  // Higher quality number = lower quality, less memory
  config.fb_count = 1;  // Single buffer to save memory
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Streaming camera init FAIL: 0x%x\n", err);
    return err;
  }
  return ESP_OK;
}

void onQrCodeTask(void *pvParameters) {
  Serial.println("=== QR Code Task Started ===");
  Serial.printf("Task running on core: %d\n", xPortGetCoreID());
  Serial.printf("Free heap in task: %u bytes\n", ESP.getFreeHeap());
  
  struct QRCodeData qrCodeData;
  bool qrCodeScanned = false;
  String ssid, pass, cameraId;

  Serial.println("Point camera at a QR code with format: S:<SSID>;P:<PASSWORD>");
  Serial.println("Waiting for QR code...");


  // 1. Loop until a valid QR code is scanned
  while (!qrCodeScanned) {
    if (reader.receiveQrCode(&qrCodeData, 100)) {
      if (qrCodeData.valid) {
        String payload = String((const char *)qrCodeData.payload);
        Serial.printf("QR Code Payload: %s\n", payload.c_str());

        // Simplified parsing logic for S:<SSID>;P:<Password>
        int ssid_start = payload.indexOf("S:") + 2;
        int ssid_end = payload.indexOf(";", ssid_start);
        ssid = payload.substring(ssid_start, ssid_end);

        int pass_start = payload.indexOf("P:") + 2;
        // Handle case where password is at the end (no semicolon after)
        int pass_end = payload.indexOf(";", pass_start);
        if (pass_end == -1) {
          pass = payload.substring(pass_start);
        } else {
          pass = payload.substring(pass_start, pass_end);
        }

        if (ssid.length() > 0 && pass.length() > 0) {
          qrCodeScanned = true;
          Serial.println("QR code parsed successfully.");
          flashOnceParsed();
          Serial.println("SSID: " + ssid);
          Serial.println("Password: [HIDDEN]");
        } else {
          Serial.println("QR code format is invalid. Expecting 'S:<SSID>;P:<Password>'");
        }
      }
    }
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }

  // 2. Stop QR reader
  Serial.println("QR Scan successful. Shutting down reader.");
  reader.end();
  esp_camera_deinit();
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Crucial delay

  // 3. Connect to WiFi
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi OK");

  String macAddress = WiFi.macAddress();
  macAddress.replace(":", "");
  cameraId = "CAM_" + macAddress;
  Serial.println("Camera ID: " + cameraId);

  // 4. Re-initialize camera for streaming
  if (init_camera_for_streaming() != ESP_OK) {
    Serial.println("Failed to init camera for streaming. Restarting...");
    ESP.restart();
  }
  Serial.println("Streaming camera init OK");

  // 4.5. Register camera via HTTP first (more reliable than WebSocket)
  Serial.println("Registering camera via HTTP...");
  HTTPClient http;
  http.begin("http://" + String(WEBSOCKET_SERVER_HOST) + ":" + String(WEBSOCKET_SERVER_PORT) + "/api/camera/register");
  http.addHeader("Content-Type", "application/json");
  
  String payload = "{\"cameraId\":\"" + cameraId + "\"}";
  int httpResponseCode = http.POST(payload);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Registration Response: " + response);
  } else {
    Serial.println("HTTP Registration failed: " + String(httpResponseCode));
  }
  http.end();

  // 5. Connect to WebSocket Server
  String host = WEBSOCKET_SERVER_HOST;
  uint16_t port = WEBSOCKET_SERVER_PORT;
  String path = "/" + cameraId;

  Serial.println("Connecting to WS: " + host + ":" + String(port) + path);
  client.onMessage(onMessageCallback);
  
  // Configure WebSocket for better Node.js compatibility
  client.setInsecure(); // For development - remove in production
  
  for (int i = 1; i <= 50; i++) {
    if (!client.connect(host, port, path)) { 
        Serial.println("WS connect failed! Trying Again..."); 
    }
    else {
        Serial.println("WS OK");
        break;
    }
    delay(200);
  }

  /*if (!client.connect(host, port, path)) {
    Serial.print("Failed to connect 50 times! Restarting...");
    ESP.restart();
  } */

  // 6. Streaming Loop
  while (true) {
    if (client.available()) {
      client.poll();
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) { Serial.println("Camera capture failed"); continue; }
      client.sendBinary((const char *)fb->buf, fb->len);
      esp_camera_fb_return(fb);
    } else { Serial.println("WebSocket disconnected. Restarting..."); ESP.restart(); }
    vTaskDelay(20 / portTICK_PERIOD_MS); // FPS Control
  }
}

void setup() {
  // Disable brownout detector to prevent resets during camera init
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(2000); // Give time for serial to initialize
  
  Serial.println("\n\n=== ESP32-S3 WROOM FREENOVE QR Code Reader ===");
  Serial.println("Starting initialization...");
  
  // Check available memory
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  Serial.printf("PSRAM found: %s\n", psramFound() ? "YES" : "NO");
  
  // Add delay before camera initialization
  Serial.println("Waiting 3 seconds before camera init...");
  delay(3000);
  
  Serial.println("Initializing QR Code Reader...");
  
  // Enable debug for troubleshooting
  reader.setDebug(true);
  
  QRCodeReaderSetupErr setupResult = reader.setup();
  
  if (setupResult == SETUP_OK) {
    Serial.println("✓ QR Code Reader setup successful!");
    
    Serial.println("Starting QR Reader on Core 1...");
    reader.beginOnCore(1);
    Serial.println("✓ QR Reader started on Core 1");

    Serial.println("Creating QR Code task...");
    BaseType_t taskResult = xTaskCreate(onQrCodeTask, "onQrCodeTask", 12288, NULL, 5, NULL); // Increased stack size
    
    if (taskResult == pdPASS) {
      Serial.println("✓ QR Code task created successfully!");
      Serial.println("=== Ready to scan QR codes ===");
    } else {
      Serial.println("✗ Failed to create QR Code task!");
    }
    
  } else if (setupResult == SETUP_NO_PSRAM_ERROR) {
    Serial.println("⚠ Warning: PSRAM not found, using regular RAM");
    Serial.println("This may cause stability issues. Consider using a board with PSRAM.");
    
  } else if (setupResult == SETUP_CAMERA_INIT_ERROR) {
    Serial.println("✗ Error: QR Camera initialization failed!");
    Serial.println("Check your camera connections and pin configuration.");
    Serial.println("System will not proceed with QR scanning.");
    return;
  }
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}