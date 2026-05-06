# ESPDet Pico Models

## Overview

ESPDet Pico models are small object detection models for ESP-DL.

Use these models with `espdl.ESPDet`.

## Usage

The input image should be RGB565.

Copy the model to `/sdcard/models/espdet/pico/` or `/flash/models/espdet/pico/`.

```python
import espdl

det = espdl.ESPDet("/flash/models/espdet/pico/espdet_pico_224_224_face.espdl", score=0.5, nms=0.7)
```
