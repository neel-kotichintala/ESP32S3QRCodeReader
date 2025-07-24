#include <ESP32QRCodeReader.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

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

ESP32QRCodeReader reader(ESP32S3_FREENOVE_PINS, FRAMESIZE_QVGA);

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(3000); // Give time for serial to stabilize
  
  Serial.println("\n=== ESP32-S3 FREENOVE Camera Test ===");
  
  // Check system info
  Serial.printf("ESP32-S3 Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  Serial.printf("PSRAM found: %s\n", psramFound() ? "YES" : "NO");
  
  Serial.println("\nTesting camera initialization...");
  
  QRCodeReaderSetupErr result = reader.setup();
  
  switch(result) {
    case SETUP_OK:
      Serial.println("✓ Camera initialized successfully!");
      Serial.println("✓ Ready for QR code scanning");
      break;
      
    case SETUP_NO_PSRAM_ERROR:
      Serial.println("⚠ PSRAM not found - this may cause issues");
      Serial.println("Consider using a board with PSRAM for better stability");
      break;
      
    case SETUP_CAMERA_INIT_ERROR:
      Serial.println("✗ Camera initialization FAILED!");
      Serial.println("Possible causes:");
      Serial.println("  - Incorrect pin connections");
      Serial.println("  - Camera module not connected");
      Serial.println("  - Power supply issues");
      Serial.println("  - Incompatible camera module");
      return;
  }
  
  Serial.println("\nIf you see this message, the camera is working!");
  Serial.println("You can now try the full QR code scanning program.");
}

void loop() {
  // Just blink the LED to show the system is stable
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink > 1000) {
    ledState = !ledState;
    pinMode(2, OUTPUT); // ESP32-S3 built-in LED
    digitalWrite(2, ledState);
    lastBlink = millis();
    
    // Print memory status
    Serial.printf("Heap: %u, PSRAM: %u\n", ESP.getFreeHeap(), ESP.getFreePsram());
  }
  
  delay(100);
}