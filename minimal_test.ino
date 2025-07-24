// Minimal ESP32-S3 test - NO camera initialization
// This will help isolate if the issue is camera-related or board-related

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== ESP32-S3 MINIMAL TEST ===");
  Serial.println("If you see this, the board is working!");
  
  // Basic system info
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM Size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  Serial.printf("Flash Size: %u bytes\n", ESP.getFlashChipSize());
  
  // Test LED
  pinMode(2, OUTPUT); // ESP32-S3 built-in LED
  Serial.println("LED test starting...");
}

void loop() {
  static unsigned long lastTime = 0;
  static int counter = 0;
  
  if (millis() - lastTime > 1000) {
    counter++;
    Serial.printf("Running stable for %d seconds\n", counter);
    
    // Blink LED
    digitalWrite(2, counter % 2);
    
    lastTime = millis();
  }
  
  delay(100);
}