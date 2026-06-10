# ESP-VISION

[English](README.md) | [简体中文](README_ZH.md)

## Overview

ESP-VISION is Espressif's low-code framework for intelligent edge vision. It deeply integrates essential capabilities including camera capture, image processing, video encoding and decoding, network transmission, model deployment, and AI inference, while providing unified and standardized Python APIs that enable developers to rapidly build edge applications combining visual capture, intelligent recognition, display, and media streaming.

## Key Features

- Unified APIs for camera capture, image input, display, video encoding, preview, and streaming across supported chips and boards.
- Image processing capabilities covering drawing, filtering, color tracking, feature detection, QR codes, barcodes, and AprilTags.
- ESP-DL-powered AI inference for object detection, pose estimation, and image classification, with a streamlined path for deploying models to edge devices.
- A consistent Python API for low-code application development, backed by performance-oriented C/C++ components.
- Efficient C/C++ foundation components work closely with on-chip multimedia peripherals and hardware acceleration modules to deliver high-performance, real-time application execution.
- Development through a VSCode-based host tool or [Web IDE](https://esp-vision-ide.espressif.tools/), with firmware builds managed through `idf.py`.

## Resources

- [Programming Guide](https://docs.espressif.com/projects/esp-vision/en/latest/)
- [Supported Targets and Boards](https://docs.espressif.com/projects/esp-vision/en/latest/target-support/index.html)
- [Web IDE](https://esp-vision-ide.espressif.tools/)
- [Examples](example/)
- [Python API type stubs](stubs/)
- [Customize firmware features](https://docs.espressif.com/projects/esp-vision/en/latest/how-to/customize-firmware.html)
- [Solution architecture and licensing](https://docs.espressif.com/projects/esp-vision/en/latest/architecture/index.html)

## Quickstart

Prerequisites: ESP-IDF release/v5.5, release/v6.0, or master with the export script sourced (`idf.py` on `PATH`), and a [supported board](https://docs.espressif.com/projects/esp-vision/en/latest/target-support/index.html).

```bash
git clone --recursive <this-repo> esp-vision
cd esp-vision
idf.py --board ESP32_P4X_EYE -p /dev/ttyACM0 build flash monitor
```
