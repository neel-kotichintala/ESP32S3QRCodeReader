#include <ESP32QRCodeReader.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Use the standard ESP_EYE configuration (might work better)
ESP32QRCodeReader reader(CAMERA_MODEL_ESP_EYE, FRAMESIZE_QVGA);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(3000);
  
  Serial.println("\n=== ESP32-S3 with ESP_EYE Config Test ===");
  
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM found: %s\n", psramFound() ? "YES" : "NO");
  
  Serial.println("Testing with standard ESP_EYE pin configuration...");
  
  QRCodeReaderSetupErr result = reader.setup();
  
  if (result == SETUP_OK) {
    Serial.println("✓ SUCCESS! Camera initialized with ESP_EYE config");
  } else {
    Serial.printf("✗ FAILED with error code: %d\n", result);
  }
}

void loop() {
  delay(1000);
  Serial.println("System running stable...");
}