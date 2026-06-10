芯片与开发板支持
================

:link_to_translation:`en:[English]`

ESP-VISION 文档按支持的芯片分别构建。请选择开发板固件所使用的芯片，使文档中的模块和 API 与默认固件保持一致。

支持的开发板
------------

.. include:: _generated/boards.rst

芯片能力与板级能力
------------------

所选芯片控制芯片级源文件选择。例如，只有 ``IDF_TARGET`` 为 ``esp32p4`` 时， ``micropython.cmake`` 才会加入 ``h264`` 和 ``rtsp``。板级配置还可以通过 ``boards/<BOARD>/board.cmake`` 进一步启用或关闭 ZXing-C++ 条形码后端等功能。

API 导航和芯片相关的符号条件会根据这些构建文件生成。因此， ``micropython.cmake``、 ``overlay/micropython/ports/esp32/boards/*/mpconfigboard.cmake`` 和各开发板的 ``board.cmake`` 是 ESP-VISION API 可用性的权威来源。

标准 MicroPython 功能
---------------------

标准 MicroPython 功能采用另一条配置路径：全局 ``overlay/micropython/ports/esp32/mpconfigport.h`` 与各开发板的 ``mpconfigboard.h``。上表会索引这些文件中显式启用的基础能力，其他条件还可能取决于芯片能力和固件配置。

当前开发板头文件也都关闭了 Bluetooth 和 ESP-NOW；S31 开发板还关闭了 ``machine.ADC`` 和 ``machine.ADCBlock``。开发板仍需具备相应网络硬件和固件配置， ``network.WLAN`` 才能建立连接。

这些条件并非只由芯片决定，因此芯片选择器会过滤 ESP-VISION API 参考，但不代表标准 MicroPython 模块的完整清单。构建环境兼容性说明统一维护在项目 README 中。

芯片文档描述当前开发板配置共同采用的默认能力。自定义开发板或修改编译选项后的固件可能与已发布芯片页面不同；发生差异时应以固件配置为准。
