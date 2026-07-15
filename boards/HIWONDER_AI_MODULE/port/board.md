# HIWONDER AI Module

ESP-VISION port for the HIWONDER AI Module board (ESP32-S3).

## Specifications

- **SoC**: ESP32-S3, 240 MHz, onboard PSRAM
- **Camera**: OV2640 via DVP (8-bit), 320x240 JPEG @ 8 MHz XCLK
- **Display**: 320x240 ST7789 SPI LCD (DC=GPIO1, CS=GPIO2, BL=GPIO14)
- **Touch**: FT5x06 capacitive touch (I2C)
- **Audio**: ES8311 codec (I2C SDA=GPIO4/SCL=GPIO5, I2S)
- **Buttons**: BOOT (GPIO0), Right button (GPIO43)
- **UART0**: connected to voice co-processor (RXD=GPIO44)

## Building

```bash
idf.py --board HIWONDER_AI_MODULE build
```
