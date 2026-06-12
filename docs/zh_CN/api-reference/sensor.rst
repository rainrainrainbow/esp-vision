sensor -- 摄像头
================

:link_to_translation:`en:[English]`

``sensor`` 模块用于控制摄像头并采集图像帧。它的接口与 OpenMV 的 ``sensor`` 保持一致，因此大多数 OpenMV 脚本无需改动即可移植。

摄像头初始化与连续采集
----------------------

典型流程是复位传感器、选择像素格式与分辨率、等待自动曝光和自动白平衡稳定，然后连续采集图像：

.. code-block:: python

   import sensor

   sensor.reset()
   sensor.set_pixformat(sensor.RGB565)
   sensor.set_framesize(sensor.QVGA)
   sensor.skip_frames(time=2000)

   while True:
       img = sensor.snapshot()

``sensor.reset()`` 会应用开发板的默认摄像头配置。``RGB565`` 适用于显示、绘图和大多数 AI 工作流，``GRAYSCALE`` 则可降低许多检测算法的内存占用和处理开销。``sensor.snapshot()`` 返回由可复用帧缓冲承载的 :py:class:`image.Image`，因此需要在下一次采集后继续保持图像内容不变时，应先复制该图像。

图像方向与摄像头状态
--------------------

摄像头传感器的实际安装方向可能导致画面翻转，可通过水平镜像和垂直翻转进行校正。``status()`` 返回当前分辨率、像素格式、传感器 ID、图像方向和裁剪信息，可用于诊断：

.. code-block:: python

   import sensor

   sensor.reset()
   sensor.set_hmirror(True)
   sensor.set_vflip(False)

   info = sensor.status()
   print("sensor id:", info["id"])
   print("output:", info["width"], "x", info["height"])
   print("ready:", info["ready"])

镜像和翻转设置会影响后续采集的图像。产品代码可在初始化后调用 ``status()``，确认摄像头已经就绪，并验证协商得到的输出尺寸与处理链路一致。

暂时停止采集
------------

不需要采集时可以停止摄像头流，并在下一次采集前重新启动：

.. code-block:: python

   sensor.shutdown(True)
   # 执行不需要摄像头的任务。
   sensor.shutdown(False)
   sensor.skip_frames(n=3)
   img = sensor.snapshot()

重新启动摄像头流后，如果曝光或输入帧队列需要重新稳定，应丢弃少量图像帧。

.. seealso::

   :doc:`../concepts/camera-pipeline` 说明了一帧图像如何从图像传感器到达 :py:class:`image.Image`\ ；:doc:`../concepts/image-model` 介绍像素格式与 色彩空间。

   可运行示例：``example/01-Camera/00-Snapshot``\ （采集并保存）与 ``example/01-Camera/03-MJPEG``\ （Wi-Fi MJPEG 串流）。

.. include:: _generated/sensor.rst
