# ESP-VISION

[English](README.md) | [简体中文](README_ZH.md)

## 概述

ESP-VISION 是乐鑫面向端侧智能视觉打造的低代码开发框架，深度整合了摄像头采集、图像处理、视频编解码、网络传输、模型部署和 AI 推理等核心能力，并提供统一标准化的 Python 接口，赋能开发者快速构建集视觉采集、智能识别、画面显示与流媒体传输于一体的边缘应用。

## 主要特性

- 为受支持的芯片和开发板统一提供摄像头采集、图像输入、显示、视频编码、预览与推流 API。
- 提供绘图、滤波、颜色追踪、特征检测、二维码、条码和 AprilTag 等图像处理能力。
- 基于 ESP-DL 提供目标检测、姿态估计和图像分类等 AI 推理能力，并简化端侧模型部署流程。
- 通过一致的 Python API 实现低代码应用开发，并由高性能 C/C++ 组件提供底层支撑。
- 底层高效的 C/C++ 基础组件深度协同芯片的多媒体外设与硬件加速模块，切实保障应用的高效与实时运行性能。
- 可通过 VSCode 上位机工具或 [Web IDE](https://esp-vision-ide.espressif.tools/) 开发，并使用 `idf.py` 管理固件构建。

## 资源

- [编程指南](https://docs.espressif.com/projects/esp-vision/zh_CN/latest/)
- [Target 与开发板支持](https://docs.espressif.com/projects/esp-vision/zh_CN/latest/target-support/index.html)
- [Web IDE](https://esp-vision-ide.espressif.tools/)
- [示例](example/)
- [Python API 类型存根](stubs/)
- [客制化固件功能](https://docs.espressif.com/projects/esp-vision/zh_CN/latest/how-to/customize-firmware.html)
- [方案架构与许可证](https://docs.espressif.com/projects/esp-vision/zh_CN/latest/architecture/index.html)

## 快速开始

环境要求：ESP-IDF release/v5.5、release/v6.0 或 master（已 source `export.sh`， `idf.py` 可用），以及一块 [受支持的开发板](https://docs.espressif.com/projects/esp-vision/zh_CN/latest/target-support/index.html)。

```bash
git clone --recursive <本仓库> esp-vision
cd esp-vision
idf.py --board ESP32_P4X_EYE -p /dev/ttyACM0 build flash monitor
```
