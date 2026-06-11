Chip and Board Support
======================

:link_to_translation:`zh_CN:[中文]`

ESP-VISION documentation is built separately for each supported chip. Select the chip used by the board firmware so that the documented modules and APIs match the default build.

Supported Development Boards
----------------------------

.. include:: _generated/boards.rst

Chip Versus Board Capabilities
------------------------------

The selected chip controls chip-level source selection. For example, ``h264`` and ``rtsp`` are added by ``micropython.cmake`` only when ``IDF_TARGET`` is ``esp32p4``. A board profile can further enable or disable features such as the ZXing-C++ barcode backend through ``boards/<BOARD>/board.cmake``.

The API navigation and chip-specific symbol conditions are generated from these build files. This makes ``micropython.cmake``, ``boards/*/port/mpconfigboard.cmake``, and board ``board.cmake`` files authoritative for ESP-VISION API availability.

Standard MicroPython Features
-----------------------------

Standard MicroPython features use a separate configuration path: ``overlay/micropython/ports/esp32/mpconfigport.h`` plus each board's ``mpconfigboard.h``. The table above indexes the explicitly enabled baseline features from those files, while additional conditions can depend on chip capabilities and firmware configuration.

The current board headers disable Bluetooth and ESP-NOW; the S31 board additionally disables ``machine.ADC`` and ``machine.ADCBlock``. A board must still provide suitable networking hardware and firmware configuration before ``network.WLAN`` can connect.

Because these conditions are not determined solely by the chip, the chip selector filters the ESP-VISION API reference but does not represent a complete standard MicroPython module manifest. Build-environment compatibility details are maintained in the project README.

The chip documentation describes the defaults shared by the current board profiles. A custom board or modified build option can therefore differ from the published chip page. When that happens, the firmware configuration is authoritative.
