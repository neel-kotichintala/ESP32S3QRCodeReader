#include <Arduino.h>
#include <ESP32QRCodeReader.h>

// ESP32S3 WROOM FREENOVE board QR Code Reader
ESP32QRCodeReader reader(CAMERA_MODEL_ESP32S3_EYE, FRAMESIZE_QVGA);

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("========== QR Code Detected ==========");
      if (qrCodeData.valid)
      {
        Serial.print("Valid QR Code - Payload: ");
        Serial.println((const char *)qrCodeData.payload);
        Serial.printf("Data Type: %d\n", qrCodeData.dataType);
        Serial.printf("Payload Length: %d\n", qrCodeData.payloadLen);
      }
      else
      {
        Serial.print("Invalid QR Code - Error: ");
        Serial.println((const char *)qrCodeData.payload);
      }
      Serial.println("=====================================");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  Serial.println("ESP32-S3 WROOM FREENOVE QR Code Reader");
  Serial.println("=====================================");
  
  // Enable debug output for troubleshooting
  reader.setDebug(true);
  
  Serial.println("Initializing camera...");
  
  QRCodeReaderSetupErr setupResult = reader.setup();
  
  switch(setupResult) {
    case SETUP_OK:
      Serial.println("✓ Camera setup successful!");
      break;
    case SETUP_NO_PSRAM_ERROR:
      Serial.println("⚠ Warning: PSRAM not found, using regular RAM");
      Serial.println("  Performance may be limited but should still work");
      break;
    case SETUP_CAMERA_INIT_ERROR:
      Serial.println("✗ Error: Camera initialization failed!");
      Serial.println("  Check your wiring and pin connections");
      return;
  }

  Serial.println("Starting QR Code detection...");
  Serial.println("Point a QR code at the camera");
  
  // Start QR detection on Core 1 (Core 0 is used by WiFi)
  reader.beginOnCore(1);

  // Create QR code processing task
  xTaskCreate(onQrCodeTask, "QRCodeTask", 4 * 1024, NULL, 4, NULL);
  
  Serial.println("Ready to scan QR codes!");
}

void loop()
{
  // Print memory status every 10 seconds
  static unsigned long lastMemCheck = 0;
  if (millis() - lastMemCheck > 10000) {
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
    lastMemCheck = millis();
  }
  
  delay(100);
}