sensor -- Camera
================

:link_to_translation:`zh_CN:[中文]`

The ``sensor`` module controls the camera and captures frames. It mirrors the OpenMV ``sensor`` API so existing OpenMV scripts port with little change.

Camera Initialization and Continuous Capture
--------------------------------------------

A typical program resets the sensor, selects a pixel format and frame size, lets automatic exposure and white balance settle, and then captures frames continuously:

.. code-block:: python

   import sensor

   sensor.reset()
   sensor.set_pixformat(sensor.RGB565)
   sensor.set_framesize(sensor.QVGA)
   sensor.skip_frames(time=2000)

   while True:
       img = sensor.snapshot()

``sensor.reset()`` applies the board's default camera configuration. ``RGB565`` is suitable for display, drawing, and most AI workflows, while ``GRAYSCALE`` reduces memory and processing cost for many detection algorithms. ``sensor.snapshot()`` returns an :py:class:`image.Image` backed by the reusable frame buffer, so copy the image when it must remain unchanged after the next capture.

Image Orientation and Camera Status
-----------------------------------

Use horizontal mirror and vertical flip to correct the image orientation imposed by the physical sensor installation. ``status()`` provides the active dimensions, pixel format, sensor ID, orientation, and crop information for diagnostics:

.. code-block:: python

   import sensor

   sensor.reset()
   sensor.set_hmirror(True)
   sensor.set_vflip(False)

   info = sensor.status()
   print("sensor id:", info["id"])
   print("output:", info["width"], "x", info["height"])
   print("ready:", info["ready"])

Mirror and flip settings affect subsequently captured frames. Product code can call ``status()`` after initialization to verify that the camera is ready and that the negotiated output size matches the processing pipeline.

Temporarily Stop Capture
------------------------

The camera stream can be stopped when capture is not required and restarted before the next frame:

.. code-block:: python

   sensor.shutdown(True)
   # Perform work that does not require the camera.
   sensor.shutdown(False)
   sensor.skip_frames(n=3)
   img = sensor.snapshot()

After restarting the stream, discard a few frames if exposure or the incoming frame queue needs to stabilize.

.. seealso::

   :doc:`../concepts/camera-pipeline` explains how a frame travels from the image sensor to an :py:class:`image.Image`, and :doc:`../concepts/image-model` covers pixel formats and color spaces.

   Runnable examples: ``example/01-Camera/00-Snapshot`` (capture and save) and ``example/01-Camera/03-MJPEG`` (Wi-Fi MJPEG stream).

.. include:: _generated/sensor.rst
