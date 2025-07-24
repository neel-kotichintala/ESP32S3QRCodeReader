#include <Arduino.h>
#include <ESP32QRCodeReader.h>

// Use ESP32S3_EYE pin configuration for FREENOVE ESP32S3 WROOM board
ESP32QRCodeReader reader(CAMERA_MODEL_ESP32S3_EYE, FRAMESIZE_QVGA);

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  // Enable debug output
  reader.setDebug(true);
  
  Serial.println("Initializing ESP32-S3 WROOM FREENOVE QR Code Reader...");
  
  QRCodeReaderSetupErr setupResult = reader.setup();
  
  if (setupResult == SETUP_OK) {
    Serial.println("QR Code Reader setup successful!");
  } else if (setupResult == SETUP_NO_PSRAM_ERROR) {
    Serial.println("Warning: PSRAM not found, but continuing...");
  } else if (setupResult == SETUP_CAMERA_INIT_ERROR) {
    Serial.println("Error: Camera initialization failed!");
    return;
  }

  Serial.println("Starting QR Code detection on Core 1...");
  reader.beginOnCore(1);

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop()
{
  delay(100);
}