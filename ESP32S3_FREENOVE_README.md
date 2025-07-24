# ESP32S3 WROOM FREENOVE QR Code Reader Adaptation

This adaptation of the ESP32QRCodeReader library has been specifically configured to work with the **ESP32S3 WROOM FREENOVE** board using the `CAMERA_MODEL_ESP32S3_EYE` pin configuration.

## Pin Configuration

The ESP32S3 WROOM FREENOVE board uses the following camera pin mapping:

| Camera Pin | GPIO Pin | Description |
|------------|----------|-------------|
| PWDN       | -1       | Power Down (not used) |
| RESET      | -1       | Reset (not used) |
| XCLK       | 15       | External Clock |
| SIOD       | 4        | I2C Data (SDA) |
| SIOC       | 5        | I2C Clock (SCL) |
| Y9         | 16       | Data bit 9 |
| Y8         | 17       | Data bit 8 |
| Y7         | 18       | Data bit 7 |
| Y6         | 12       | Data bit 6 |
| Y5         | 10       | Data bit 5 |
| Y4         | 8        | Data bit 4 |
| Y3         | 9        | Data bit 3 |
| Y2         | 11       | Data bit 2 |
| VSYNC      | 6        | Vertical Sync |
| HREF       | 7        | Horizontal Reference |
| PCLK       | 13       | Pixel Clock |

## Key Adaptations Made

### 1. Pin Configuration Fix
- Fixed the malformed `CAMERA_MODEL_ESP32S3_EYE` definition in `ESP32CameraPins.h`
- Added proper struct initialization syntax
- Added alternative `CAMERA_MODEL_ESP32S3_WROOM_FREENOVE` definition for future use

### 2. ESP32-S3 Optimizations
- **PSRAM Handling**: Made PSRAM optional instead of required (ESP32-S3 can work without PSRAM)
- **Clock Frequency**: Increased XCLK frequency to 20MHz for better ESP32-S3 performance
- **Grab Mode**: Set to `CAMERA_GRAB_WHEN_EMPTY` for optimal ESP32-S3 performance
- **Frame Buffer**: Optimized for single buffer usage

### 3. Default Configuration Changes
- Changed default camera model from `CAMERA_MODEL_AI_THINKER` to `CAMERA_MODEL_ESP32S3_EYE`
- Set default frame size to `FRAMESIZE_QVGA` for optimal performance

### 4. Enhanced Error Handling
- Improved setup error reporting
- Added debug output for troubleshooting
- Better memory status monitoring

## Usage Examples

### Basic Usage
```cpp
#include <ESP32QRCodeReader.h>

ESP32QRCodeReader reader(CAMERA_MODEL_ESP32S3_EYE, FRAMESIZE_QVGA);

void setup() {
  Serial.begin(115200);
  
  QRCodeReaderSetupErr result = reader.setup();
  if (result == SETUP_OK) {
    reader.beginOnCore(1);
  }
}
```

### With Custom Pin Configuration
```cpp
// If you need to use different pins
CameraPins customPins = CAMERA_MODEL_ESP32S3_EYE;
ESP32QRCodeReader reader(customPins, FRAMESIZE_QVGA);
```

## Comparison with DeneyapKart Adaptation

The DeneyapKart adaptation approach was studied and similar principles were applied:

1. **Custom Pin Definitions**: Like DeneyapKart used `DK_Kamera` with custom pin mappings
2. **Simplified Constructor**: Removed unnecessary default constructors
3. **Board-Specific Optimizations**: Tailored camera settings for the specific board

### Key Differences:
- **PSRAM Requirement**: DeneyapKart required PSRAM, this adaptation makes it optional
- **Clock Settings**: Optimized for ESP32-S3 performance (20MHz vs 10MHz)
- **Error Handling**: Enhanced error reporting and debugging capabilities

## Troubleshooting

### Camera Initialization Failed
- Check all pin connections match the pin configuration table
- Ensure camera module is properly connected
- Verify power supply is adequate (3.3V)

### Poor QR Code Detection
- Ensure adequate lighting
- Try different frame sizes: `FRAMESIZE_QVGA`, `FRAMESIZE_VGA`
- Check camera focus and positioning

### Memory Issues
- Monitor heap usage with `ESP.getFreeHeap()`
- Consider using PSRAM if available
- Reduce frame size if memory is limited

## Files Modified

1. `src/ESP32CameraPins.h` - Fixed ESP32S3_EYE pin definition, added FREENOVE variant
2. `src/ESP32QRCodeReader.cpp` - ESP32-S3 optimizations and default model change
3. `examples/basic/basic.ino` - Enhanced with better error handling
4. `examples/esp32s3_freenove/esp32s3_freenove.ino` - New ESP32-S3 specific example

## Performance Notes

- **Frame Rate**: Optimized for real-time QR code detection
- **Memory Usage**: ~40KB stack size for QR detection task
- **CPU Usage**: QR detection runs on Core 1, leaving Core 0 for WiFi/Bluetooth
- **Power Consumption**: Optimized clock settings for balance of performance and power

## Future Enhancements

- Add support for different camera modules on ESP32-S3
- Implement auto-focus control if supported by camera
- Add image preprocessing for better QR detection in low light
- Support for multiple QR codes in single frame