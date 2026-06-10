Get Started
===========

:link_to_translation:`zh_CN:[中文]`

Prerequisites
-------------

- ESP-IDF ``release/v5.5``, ``release/v6.0``, or ``master`` with the export script sourced so that ``idf.py`` is on ``PATH``.
- A board listed in :doc:`../target-support/index` for the selected target.

.. only:: esp32s31

   ESP32-S31 currently requires an ESP-IDF ``master`` environment.

Build, Flash, and Monitor
-------------------------

Clone the repository with submodules, then use the board-aware ``idf.py`` extension from the repository root:

.. code-block:: bash

   git clone --recursive <this-repo> esp-vision
   cd esp-vision

.. only:: esp32p4

   .. code-block:: bash

      idf.py --board ESP32_P4X_EYE -p /dev/ttyACM0 build flash monitor

.. only:: esp32s3

   .. code-block:: bash

      idf.py --board ESP32_S3_EYE -p /dev/ttyACM0 build flash monitor

.. only:: esp32s31

   .. code-block:: bash

      idf.py --board ESP32_S31_KORVO -p /dev/ttyACM0 build flash monitor

The command first runs ``prepare-micropython``: it verifies that ``lib/micropython`` is checked out at the pinned MicroPython v1.28.0 commit, exports a clean MicroPython build copy under ``build/micropython/``, then applies ``overlay/micropython/`` to that copy. ``lib/micropython`` is never dirtied.

Common idf.py Commands
----------------------

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Command
     - Description
   * - ``idf.py --board <BOARD> build``
     - Build firmware for a board.
   * - ``idf.py --board <BOARD> -p <PORT> flash``
     - Build and flash firmware.
   * - ``idf.py --board <BOARD> -p <PORT> monitor``
     - Open the serial monitor.
   * - ``idf.py --board <BOARD> menuconfig``
     - Open menuconfig.
   * - ``idf.py --board <BOARD> -p <PORT> erase-flash``
     - Erase flash.
   * - ``idf.py --board <BOARD> clean``
     - Clean board build output.
   * - ``idf.py --board <BOARD> fullclean``
     - Remove the complete build directory for the selected board.

Run Your First Script
---------------------

After flashing, connect over the REPL and try the camera. See :doc:`../examples/index` for ready-to-run scripts.
