image.ImageIO -- 图像流
=======================

:link_to_translation:`en:[English]`

``image.ImageIO`` 类型用于录制和回放图像序列，并保留帧间时间间隔。文件流在存储上 读写容器文件；内存流则把帧保存在预分配的缓冲区中。它以 ``image.ImageIO`` 的形式 暴露在 ``image`` 模块上；类型存根位于 ``stubs/imageio.pyi``\ 。

录制到存储
----------

.. code-block:: python

   import sensor, image

   stream = image.ImageIO("/sdcard/stream.bin", "w")
   for _ in range(30):
       stream.write(sensor.snapshot())
   stream.sync()
   print("frames:", stream.count(), "bytes:", stream.size())
   stream.close()

每次 ``write()`` 都会保存图像以及与上一帧之间的时间间隔。``sync()`` 可在不关闭流的情况下将文件缓冲数据写入存储。文件流使用完毕后必须关闭，以完成容器元数据和存储缓冲区的写入。

回放已录制的图像流
------------------

.. code-block:: python

   import image

   stream = image.ImageIO("/sdcard/stream.bin", "r")
   try:
       while True:
           img = stream.read(loop=False, pause=True)
           if img is None:
               break
           img.flush()
   finally:
       stream.close()

``pause=True`` 会按照录制时的帧间隔进行回放，``loop=False`` 则会在流结束时返回 ``None``，而不是回到第一帧。需要以处理能力允许的最快速度读取时，可设置 ``pause=False``。

使用内存图像流
--------------

.. code-block:: python

   stream = image.ImageIO((320, 240, image.RGB565), 10)
   for _ in range(10):
       stream.write(sensor.snapshot())

   stream.seek(0)
   first = stream.read(pause=False)
   print("buffer bytes:", stream.buffer_size())
   stream.close()

内存流不产生存储 I/O，适合保存短时间图像历史或实现帧差处理，但创建流时会在 RAM 或 PSRAM 中一次性分配全部容量。

.. seealso::

   :doc:`../concepts/codec-streaming` 一并介绍了 ImageIO、JPEG、H.264 以及 USB CDC 预览通路。

   可运行示例：``example/02-Image-Processing/03-Frame-Differencing/in_memory_frame_differencing.py`` 使用内存 ImageIO 流。

.. include:: _generated/imageio.rst
