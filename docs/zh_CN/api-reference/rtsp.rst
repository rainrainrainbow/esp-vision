rtsp -- RTSP 推流
=================

:link_to_translation:`en:[English]`

``rtsp`` 模块通过 RTSP 协议发送 H.264 NAL 单元，使 VLC、ffplay 等客户端能通过网络观看摄像头画面。需与 :doc:`h264` 配合使用。该模块可用于 ESP32-P4 构建。

采集、编码与推流
----------------

.. code-block:: python

   import network, sensor, h264, rtsp, time
   from machine import Pin

   WIDTH, HEIGHT, FPS = 320, 240, 30

   lan = network.LAN(
       mdc=Pin(31),
       mdio=Pin(52),
       reset=Pin(51),
       phy_addr=1,
       phy_type=network.PHY_IP101,
   )
   lan.active(True)

   for _ in range(100):
       if lan.isconnected():
           break
       time.sleep_ms(100)
   if not lan.isconnected():
       raise OSError("Ethernet connection failed")

   sensor.reset()
   sensor.set_pixformat(sensor.RGB565)
   sensor.set_framesize(sensor.QVGA)
   sensor.skip_frames(time=1000)

   enc = h264.H264Encoder(WIDTH, HEIGHT, fps=FPS, bitrate=3_000_000)
   server = rtsp.RTSPServer(WIDTH, HEIGHT, fps=FPS, listen_port=8554)

   print("rtsp://%s:8554/" % lan.ifconfig()[0])
   try:
       while True:
           server.send(enc.encode(sensor.snapshot()))
   finally:
       server.stop()
       enc.close()

上述以太网引脚映射适用于带 IP101 RMII PHY 的 ESP32-P4X-Function-EV-Board，其他产品开发板应使用对应的网络配置。``RTSPServer`` 公布的宽度、高度和帧率必须与编码器配置一致。

客户端与队列行为
----------------

可使用 ``ffplay rtsp://<board-ip>:8554/`` 或其他支持 RTSP 的播放器打开输出地址。``send()`` 会将一帧编码数据加入队列，并且不会等待客户端；没有客户端连接或客户端处理过慢时，帧可能被丢弃，从而避免阻塞采集循环。如果高分辨率或高码率编码帧超过默认限制，可增大 ``max_frame_len``。

停止服务
--------

释放网络或编码器资源前必须调用 ``server.stop()``。使用 ``try``/``finally`` 可以确保脚本被中断或抛出异常时仍会完成资源清理。

.. seealso::

   :doc:`../concepts/codec-streaming` 完整介绍了采集、编码与推流的整个流程。

   可运行示例：``example/01-Camera/02-RTSP/stream_rtsp.py``\ 。

.. include:: _generated/rtsp.rst
