# YOLO11n Detection Models

## Overview

YOLO11n models are COCO object detection models for ESP-DL.

Use these models with `espdl.YOLO11`.

Do not load YOLO11n models with `espdl.ESPDet`.

## Usage

The input image should be RGB565.

Copy the model to `/sdcard/models/espdet/yolo11n/` or `/flash/models/espdet/yolo11n/`.

```python
import espdl

det = espdl.YOLO11("/flash/models/espdet/yolo11n/espdet_yolo11n_160_160_coco.espdl", score=0.35, nms=0.7, topk=5)
```
