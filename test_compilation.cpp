#include "src/ESP32QRCodeReader.h"

// Test compilation of ESP32S3 configuration
void test_compilation() {
    ESP32QRCodeReader reader(CAMERA_MODEL_ESP32S3_EYE, FRAMESIZE_QVGA);
    ESP32QRCodeReader reader2(CAMERA_MODEL_ESP32S3_WROOM_FREENOVE, FRAMESIZE_QVGA);
    
    // Test default constructor (should use ESP32S3_EYE now)
    ESP32QRCodeReader reader3;
    ESP32QRCodeReader reader4(FRAMESIZE_VGA);
}