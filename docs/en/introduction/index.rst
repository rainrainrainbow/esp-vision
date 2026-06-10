Introduction
============

:link_to_translation:`zh_CN:[中文]`

What Is ESP-VISION
------------------

ESP-VISION is Espressif's low-code framework for intelligent edge vision. It deeply integrates essential capabilities including camera capture, image processing, video encoding and decoding, network transmission, model deployment, and AI inference, while providing unified and standardized Python APIs that enable developers to rapidly build edge applications combining visual capture, intelligent recognition, display, and media streaming.

Key Features
------------

- Unified camera, image, display, video encoding, preview, and streaming APIs across supported chips and boards.
- Image processing capabilities covering drawing, filtering, color tracking, feature detection, QR codes, barcodes, and AprilTags.
- ESP-DL-powered object detection, pose estimation, and image classification, with a streamlined path for deploying models to edge devices.
- Efficient C/C++ foundation components work closely with on-chip multimedia peripherals and hardware acceleration modules to deliver high-performance, real-time application execution.
- Development through a VSCode-based host tool or Web IDE, with firmware builds managed through ``idf.py``.

Supported Boards
----------------

ESP-VISION targets ESP32-P4, ESP32-S3, and ESP32-S31 boards. The supported board packages are ``ESP32_P4X_EYE``, ``ESP32_P4X_FUNCTION_EV_BOARD``, ``ESP32_S3_EYE``, and ``ESP32_S31_KORVO``. ``TEMPLATE`` is provided for new-board bring-up. See :doc:`../target-support/index` for target-specific modules and constraints.

See :doc:`../get-started/index` to build and flash the firmware, and :doc:`../architecture/index` for how the pieces fit together.
