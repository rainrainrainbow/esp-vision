Target 与开发板支持
===================

:link_to_translation:`en:[English]`

ESP-VISION 文档按 IDF target 分别构建。请选择与开发板固件相同的 target，使模块和 API 可用性与默认固件保持一致。

支持的开发板
------------

.. include:: _generated/boards.rst

Target 能力与板级能力
---------------------

Target 控制芯片级源文件选择。例如，只有 ``IDF_TARGET`` 为 ``esp32p4`` 时， ``micropython.cmake`` 才会加入 ``h264`` 和 ``rtsp``。板级配置还可以通过 ``boards/<BOARD>/board.cmake`` 进一步启用或关闭 ZXing-C++ 条形码后端等功能。

API 导航和 target 相关符号条件会根据这些构建文件生成。因此， ``micropython.cmake``、 ``overlay/micropython/ports/esp32/boards/*/mpconfigboard.cmake`` 和各开发板的 ``board.cmake`` 是 ESP-VISION API 可用性的权威来源。

标准 MicroPython 功能
---------------------

标准 MicroPython 功能采用另一条配置路径：全局 ``overlay/micropython/ports/esp32/mpconfigport.h`` 与各开发板的 ``mpconfigboard.h``。上表会索引这些文件中显式启用的基础能力。部分条件还取决于 ESP-IDF 版本或 SoC 能力宏。

ESP-IDF 5.5 提供更完整的标准 MicroPython 功能。除 ``network`` 模块、WLAN 和 socket 外，还启用 SSL/TLS、``cryptolib``、WebSocket、WebREPL 和 socket events。 ``machine.I2S`` 与 ESP32 PCNT 也会在所选 SoC 支持时启用；支持的芯片还可使用 ESP-IDF 5.4 及以上版本提供的 I2C target 模式。

使用 ESP-VISION overlay 时，ESP-IDF 6.0 及以上版本仍支持标准 ``network`` 模块、 WLAN 和 socket；该 IDF 版本上的 SSL/TLS、``cryptolib``、WebSocket、WebREPL、 socket events、``machine.I2S`` 和 ESP32 PCNT 仍保持关闭。S31 port 还提供默认 hostname ``mpy-esp32s31``。

当前开发板头文件也都关闭了 Bluetooth 和 ESP-NOW；S31 开发板还关闭了 ``machine.ADC`` 和 ``machine.ADCBlock``。开发板仍需具备相应网络硬件和固件配置， ``network.WLAN`` 才能建立连接。

这些条件并非只由 target 决定，因此 target 选择器会过滤 ESP-VISION API 指南，但不把 自身作为标准 MicroPython 模块的完整清单。

Target 文档描述当前开发板配置共同采用的默认能力。自定义开发板或修改编译选项后的 固件可能与已发布 target 页面不同；发生差异时应以固件配置为准。

构建指定 Target
---------------

.. code-block:: bash

   build-docs -l zh_CN -t esp32p4
   build-docs -l zh_CN -t esp32s3
   build-docs -l zh_CN -t esp32s31

输出位于 ``docs/_build/<language>/<target>/html/``。
