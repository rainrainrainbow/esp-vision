image.ImageIO -- Image Stream
=============================

:link_to_translation:`zh_CN:[中文]`

The ``image.ImageIO`` type records and replays sequences of images while preserving inter-frame timing. A file stream reads/writes a container on storage; a memory stream keeps frames in a pre-allocated buffer. It is exposed on the ``image`` module as ``image.ImageIO``; the type stub lives in ``stubs/imageio.pyi``.

Record to Storage
-----------------

.. code-block:: python

   import sensor, image

   stream = image.ImageIO("/sdcard/stream.bin", "w")
   for _ in range(30):
       stream.write(sensor.snapshot())
   stream.sync()
   print("frames:", stream.count(), "bytes:", stream.size())
   stream.close()

Each ``write()`` stores the image and the elapsed time since the previous frame. ``sync()`` flushes buffered file data without closing the stream. Always close a file stream so the container metadata and storage buffers are finalized.

Replay a Recorded Stream
------------------------

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

``pause=True`` reproduces the recorded frame timing, while ``loop=False`` returns ``None`` at the end instead of rewinding. Set ``pause=False`` when frames should be consumed as quickly as processing permits.

Use an In-Memory Stream
-----------------------

.. code-block:: python

   stream = image.ImageIO((320, 240, image.RGB565), 10)
   for _ in range(10):
       stream.write(sensor.snapshot())

   stream.seek(0)
   first = stream.read(pause=False)
   print("buffer bytes:", stream.buffer_size())
   stream.close()

Memory streams avoid storage I/O and are useful for short frame histories or frame-difference workflows, but their full capacity is allocated in RAM or PSRAM when the stream is created.

.. seealso::

   :doc:`../concepts/codec-streaming` covers ImageIO alongside JPEG, H.264, and the USB CDC preview path.

   Runnable example: ``example/02-Image-Processing/03-Frame-Differencing/in_memory_frame_differencing.py`` uses an in-memory ImageIO stream.

.. include:: _generated/imageio.rst
