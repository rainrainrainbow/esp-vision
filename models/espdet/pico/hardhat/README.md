# ESPDet Pico Hardhat

## Overview

This 320x320 ESPDet Pico model detects safety helmets in RGB565 camera images with `espdl.ESPDet`.

The training dataset is derived from the [Hard Hat Workers Computer Vision Model](https://universe.roboflow.com/workspace-sergio-flores/hard-hat-workers-g8ot5) published on Roboflow Universe.

## Quantization

The int8 export for `esp32p4` uses percentile calibration, bias correction, and fusion settings tuned for scale consistency. Simulator validation reports `mAP50=0.9417` and `mAP50-95=0.6135`.

## Usage

Use RGB565 images from `sensor.snapshot()`.

```python
import espdl

det = espdl.ESPDet("/flash/models/espdet/pico/hardhat/espdet_pico_320_320_hardhat.espdl", score=0.5, nms=0.7)
```
