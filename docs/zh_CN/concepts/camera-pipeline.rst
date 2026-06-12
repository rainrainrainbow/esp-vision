摄像头流水线
============

:link_to_translation:`en:[English]`

:py:func:`sensor.snapshot` 看起来只是一次调用，但其背后是一条与硬件相关的采集流水线， 把光子变成 :py:class:`image.Image`\ 。了解这些阶段有助于理解配置调用 （``set_pixformat``\ 、``set_framesize``\ 、``skip_frames``\ ）以及各开发板的性能特征。

从传感器到图像
--------------

.. blockdiag::

   blockdiag {
     orientation = portrait;

     config  [label = "配置传感器\n复位 / 像素格式 / 分辨率"];
     stream  [label = "经 MIPI-CSI 或 DVP 输出像素\n并通过 DMA 写入帧缓冲"];
     convert [label = "使用 PPA 或软件\n缩放并转换像素格式"];
     settle  [label = "等待自动曝光和\n自动白平衡稳定"];
     capture [label = "sensor.snapshot()"];
     image   [label = "共享可复用帧缓冲的\nimage.Image"];

     config -> stream -> convert -> settle -> capture -> image;
   }

取流前，传感器控制总线会应用开发板默认配置以及请求的图像格式。双缓冲允许处理当前帧的同时填充下一帧。ESP32-P4 使用硬件 PPA 完成缩放和色彩转换，ESP32-S3 则由软件完成转换。配置后调用 ``sensor.skip_frames(time=2000)``，可为自动曝光和自动白平衡预留收敛时间。

各开发板后端
------------

摄像头后端在构建时按板选择：

.. list-table::
   :header-rows: 1
   :widths: 22 30 48

   * - 开发板系列
     - 后端
     - 说明
   * - ESP32-P4
     - ``esp_video`` / V4L2 + PPA
     - MIPI-CSI 传感器；硬件加速的缩放与色彩转换。
   * - ESP32-S3
     - ``esp32-camera``
     - DVP 传感器；软件转换，建议使用较小分辨率。

由于公开的 ``sensor`` API 在各板上完全一致，同一脚本可在两者上运行；区别仅在于可达到 的分辨率与帧率。

分辨率
------

``set_framesize`` 接受来自共享分辨率表的命名尺寸（``QQVGA``\ 、``QVGA``\ 、``VGA`` 等）。较小的帧在每个阶段都更省内存、更省处理时间，因此当模型或算法只需小输入时， 应直接采集小帧，而不是采集大帧再缩小。
