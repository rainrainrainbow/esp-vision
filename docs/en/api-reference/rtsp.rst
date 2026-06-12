rtsp -- RTSP Streaming
======================

:link_to_translation:`zh_CN:[中文]`

The ``rtsp`` module serves H.264 NAL units over RTSP so a client such as VLC or ffplay can view the camera stream over the network. Pair it with :doc:`h264`. It is available on ESP32-P4 builds.

Capture, Encode, and Stream
---------------------------

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

The Ethernet pin mapping above is for the ESP32-P4X-Function-EV-Board with its IP101 RMII PHY; use the network configuration appropriate for another product board. The width, height, and frame rate advertised by ``RTSPServer`` must match the encoder configuration.

Client and Queue Behavior
-------------------------

Open the printed URL with ``ffplay rtsp://<board-ip>:8554/`` or an RTSP-capable player. ``send()`` queues one encoded frame and does not wait for a client; when no client is connected or the client is too slow, frames may be dropped so the capture loop is not blocked. ``max_frame_len`` can be increased when high-resolution or high-bitrate encoded frames exceed the default limit.

Shutdown
--------

Always call ``server.stop()`` before releasing network or encoder resources. A ``try``/``finally`` block ensures cleanup also occurs when the script is interrupted or an exception is raised.

.. seealso::

   :doc:`../concepts/codec-streaming` covers the full capture, encode, and stream pipeline.

   Runnable example: ``example/01-Camera/02-RTSP/stream_rtsp.py``.

.. include:: _generated/rtsp.rst
