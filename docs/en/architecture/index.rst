Solution Architecture
=====================

:link_to_translation:`zh_CN:[中文]`

ESP-VISION is organized around a MicroPython firmware build, board-specific hardware backends, shared platform services, and Python-facing vision modules. Code is layered by whether it touches MicroPython (``mp_obj_t`` / ``py/*.h``).

Layered Overview
----------------

.. blockdiag::

   blockdiag {
     orientation = portrait;
     default_group_color = none;

     scripts  [label = "MicroPython scripts\n(example/)"];
     bindings [label = "Bindings (modules/)\nsensor / image / display / espdl"];
     platform [label = "Platform services\n(platform/)"];
     imlib    [label = "imlib component\n(components/imlib)"];
     boards   [label = "Board backends\n(boards/<BOARD>)"];
     mp       [label = "MicroPython + overlay\n(build/, overlay/)"];

     scripts  -> bindings;
     bindings -> platform;
     bindings -> imlib;
     platform -> boards;
     boards   -> mp;
   }

- **Bindings** (``modules/``): the ``USER_C_MODULES`` layer. The four real modules ``image``, ``sensor``, ``display``, and ``espdl`` self-register via ``MP_REGISTER_MODULE``. ``py_imageio.c`` provides the ``image.ImageIO`` type, and ``py_helper.c`` is shared helper code. Bindings only do object conversion and light API adaptation; heavy logic lives in pure C or ``platform/``.
- **Platform services** (``platform/``): self-written ESP32 glue. ``preview.c`` (EVFRAME JPEG preview over USB CDC), ``display.c`` (generic display layer), ``sdcard.c`` (mount at ``/sdcard``), ``usb_msc.c`` (exposes the ``ffat`` partition over TinyUSB MSC), ``jpeg.c`` (hardware or software JPEG), ``debug.c``, and ``main.c`` (startup init plus the soft-reset loop).
- **imlib component** (``components/imlib/``): pure-C vision algorithms, an IDF component maintained as MIT, derived from OpenMV ``lib/imlib``.
- **Board backends** (``boards/<BOARD>/``): per-board configuration and the real camera/display/sdcard implementations. P4X and S31 use ``esp_video``/V4L2; P4X also uses PPA, while S3 uses ``esp32-camera``.
- **MicroPython + overlay**: MicroPython v1.28.0 is the fixed baseline; project changes live in ``overlay/micropython/`` and are applied to a generated build copy under ``build/micropython/``.

Capture-to-Output Data Flow
---------------------------

.. blockdiag::

   blockdiag {
     orientation = portrait;

     sensor [label = "Camera sensor"];
     snap   [label = "sensor.snapshot()"];
     img    [label = "image.Image\n(reusable framebuffer)"];
     proc   [label = "imlib processing /\nespdl inference"];
     lcd    [label = "display.write()\n-> LCD"];
     prev   [label = "img.flush()\n-> USB CDC preview"];

     sensor -> snap -> img;
     img -> proc;
     img -> lcd;
     img -> prev;
   }

``sensor.snapshot()`` captures a frame into a reusable framebuffer wrapped as an ``image.Image``. Scripts then run ``imlib`` processing or ``espdl`` inference on the image, and send it to the LCD with ``display.write()`` or to the host preview with ``img.flush()``.

Source Tree
-----------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Path
     - Responsibility
   * - ``idf_ext.py``
     - Board-aware ``idf.py`` extension for the repository root.
   * - ``micropython.cmake``
     - Integration hub: registers user modules, platform and board sources, include paths, conditional ``zxing``, and ``ulab``.
   * - ``lib/``
     - Pinned third-party submodules (MicroPython, ``ulab``, ZXing-C++).
   * - ``overlay/micropython/``
     - ESP-VISION MicroPython delta, using the MicroPython path layout.
   * - ``boards/``
     - Per-board config, frozen manifests, and board peripheral backends.
   * - ``platform/``
     - Shared runtime services (camera, preview, storage, display, USB, JPEG).
   * - ``modules/``
     - MicroPython C/C++ bindings (``sensor``, ``image``, ``display``, ``imageio``, ``espdl``, plus target-dependent ``h264`` and ``rtsp``).
   * - ``components/``
     - ESP-IDF components, including OpenMV ``imlib`` and the ZXing backend.
   * - ``models/``
     - Optional ``.espdl`` model assets loaded from board storage at runtime.
   * - ``example/``
     - MicroPython example scripts.
   * - ``stubs/``
     - ``.pyi`` type stubs describing the C modules.

Board Composition
-----------------

A board's definition is split across two trees:

- MicroPython port side: ``overlay/micropython/ports/esp32/boards/<BOARD>/`` (IDF target, sdkconfig, partitions, USB strings).
- ESP-VISION side: ``boards/<BOARD>/`` (``boardconfig.h``, ``imlib_config.h``, ``manifest.py``, and optional ``camera.c`` / ``display.c`` / ``sdcard.c``).

See :doc:`../how-to/add-board` for the step-by-step procedure.

Target-Dependent Sources
------------------------

``micropython.cmake`` selects modules from ``IDF_TARGET`` and the board profile. The ESP32-P4 build includes ``h264`` and ``rtsp``; the current P4 board profiles also enable the ZXing-C++ barcode backend. See :doc:`../target-support/index` for the resulting public API matrix.

MicroPython Overlay
-------------------

ESP-VISION uses MicroPython v1.28.0 as a fixed upstream baseline. Project changes to the ESP32 port live under ``overlay/micropython/``. The ``prepare-micropython`` build step applies that tree to a generated copy under ``build/micropython/idf<ESP_IDF_VERSION>/micropython/``; the ``lib/micropython`` submodule remains a clean upstream reference.

Adding a New Board
------------------

A board package is split across two locations:

.. list-table::
   :header-rows: 1
   :widths: 34 66

   * - Location
     - Contents
   * - ``overlay/micropython/ports/esp32/boards/<BOARD>/``
     - ``mpconfigboard.cmake`` for the IDF target and sdkconfig chain, ``mpconfigboard.h`` for MicroPython feature flags, partition tables, and board metadata.
   * - ``boards/<BOARD>/``
     - Pin and runtime configuration, ``imlib_config.h``, frozen manifests, and optional camera, display, and SD card backends.

See :doc:`../how-to/add-board` for the complete procedure.

Licensing
---------

ESP-VISION's own code is released under Apache License 2.0. Vendored code keeps the license declared by its source file, SPDX identifier, or upstream license.

.. list-table::
   :header-rows: 1
   :widths: 24 20 36 20

   * - Repository
     - Local path
     - Usage
     - License
   * - `MicroPython <https://github.com/micropython/micropython>`__
     - ``lib/micropython``
     - Runtime and ESP32 port baseline
     - MIT
   * - `micropython-ulab <https://github.com/v923z/micropython-ulab>`__
     - ``lib/ulab``
     - Numerical module
     - MIT
   * - `OpenMV <https://github.com/openmv/openmv>`__ ``imlib`` subset
     - ``components/imlib``
     - Image processing and drawing algorithms
     - MIT, with separately licensed files
   * - OpenMV AprilTag implementation
     - ``components/imlib/upstream/apriltag.c``
     - AprilTag and rectangle detection
     - BSD-2-Clause
   * - `ZXing-C++ <https://github.com/zxing-cpp/zxing-cpp>`__
     - ``lib/zxing-cpp``
     - 1D barcode reader backend
     - Apache-2.0
   * - `ESP-DL <https://github.com/espressif/esp-dl>`__
     - Component Registry
     - Model inference runtime
     - MIT
   * - `esp_new_jpeg <https://github.com/espressif/esp-adf-libs/tree/master/esp_new_jpeg>`__
     - Component Registry
     - Software JPEG codec
     - Espressif MIT
   * - `esp32-camera <https://github.com/espressif/esp32-camera>`__
     - Component Registry
     - Camera driver
     - Apache-2.0
   * - `ESP-IDF <https://github.com/espressif/esp-idf>`__
     - External SDK
     - Build system, drivers, and media components
     - Apache-2.0

For per-file obligations and the relationship to upstream projects, see :doc:`../project-relationship/index`.
