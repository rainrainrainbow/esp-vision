软件包与部署
============

:link_to_translation:`en:[English]`

MicroPython 以模块和软件包组织可复用代码，但嵌入式产品还需要确定代码的存储位置和更新方式。本章基于 ESP-VISION 当前固定使用的 MicroPython v1.28.0 介绍软件包管理与部署；完整的通用行为请参考上游 `软件包管理 <https://docs.micropython.org/en/v1.28.0/reference/packages.html>`_ 和 `manifest <https://docs.micropython.org/en/v1.28.0/reference/manifest.html>`_ 文档。

模块与软件包结构
----------------

模块通常是 ``.py`` 源文件或预编译的 ``.mpy`` 文件。软件包是包含 ``__init__.py`` 文件以及一个或多个模块或子包的目录，因此可复用的视觉流水线可以采用以下结构：

.. code-block:: text

   my_vision/
       __init__.py
       pipeline.py
       transport.py

应用程序使用标准 Python 语法导入软件包成员：

.. code-block:: python

   from my_vision.pipeline import VisionPipeline

   pipeline = VisionPipeline()

当前固件的软件包管理
--------------------

MicroPython 使用 ``mip`` 而不是 CPython 的 ``pip``，可从 `micropython-lib <https://github.com/micropython/micropython-lib>`_、软件包索引、URL 或本地 ``package.json`` 描述中安装软件包。ESP-VISION 默认 manifest 当前没有嵌入设备端 ``mip`` 模块，因此标准固件不支持 ``import mip``；请在开发主机上使用 ``mpremote`` 将软件包安装到设备文件系统：

.. important::

   ESP-VISION 默认固件不支持设备端 ``mip``。在设备上执行 ``import mip`` 或 ``mip.install()`` 会引发 ``ImportError``。主机端 ``mpremote mip`` 命令仍然可用，并且是默认的软件包安装方式。

.. code-block:: shell

   mpremote connect <PORT> mip install --target=/lib <PACKAGE>
   mpremote connect <PORT> mip install --target=/lib <PACKAGE>@<VERSION>
   mpremote connect <PORT> mip install --target=/lib ./package.json

``package.json`` 用于描述 ``mip`` 在运行时或设备预配置阶段安装的文件和依赖项。它不是固件 manifest，也不决定哪些模块会被编译进 ESP-VISION。由于 MicroPython 软件包可能依赖固件功能或特定架构的 ``.mpy`` 文件，因此必须确认其兼容 MicroPython v1.28.0 和所选芯片架构。

在设备端使用 ``mip.install()`` 安装软件包，需要客制化固件显式加入 ``mip`` 以及所需的网络和 TLS 支持。默认推荐通过主机端 ``mpremote`` 安装，因为这种方式不会增大产品固件，也不要求设备通过网络下载代码。

文件系统中的软件包
------------------

ESP-VISION 将内部 Flash 文件系统挂载到 ``/``，并将 ``/lib`` 加入 ``sys.path``。可复用软件包应安装到 ``/lib``；此后应用程序无需修改代码，即可导入当前目录中的模块、冻结模块和 ``/lib`` 下的软件包。

启用 SD 卡后，SD 卡挂载到 ``/sdcard``，但其软件包目录不会自动加入 ``sys.path``。导入其中的软件包前需要显式添加路径：

.. code-block:: python

   import sys

   sys.path.append("/sdcard/lib")
   from my_vision.pipeline import VisionPipeline

``.py`` 源文件在导入时编译，会占用 RAM 保存字节码并增加导入延迟。预编译的 ``.mpy`` 文件无需在设备上编译并可减少源代码暴露，但加载后的字节码仍会占用 RAM，而且必须匹配 MicroPython 版本和架构。

嵌入固件的软件包
----------------

固件 manifest 用于选择需要编译并冻结到固件镜像中的 Python 模块和软件包。ESP-VISION 将开发板 manifest 保存在 ``boards/<BOARD>/manifest.py``，当前通过这些文件冻结端口模块、ESP-VISION 初始化模块、``asyncio`` 以及特定开发板需要的功能。

例如，将 ``my_vision/__init__.py`` 及其模块放在 ``boards/<BOARD>/packages`` 下，然后在该开发板的 manifest 中加入软件包：

.. code-block:: python

   package(
       "my_vision",
       base_path="$(ESP_VISION_ROOT)/boards/<BOARD>/packages",
   )

底层 ``freeze()`` 函数也可以冻结一个目录或指定模块，``include()`` 和 ``require()`` 则用于复用已配置 manifest 库中的 manifest 片段和软件包。修改 manifest 后，需要重新配置并构建所选开发板：

.. code-block:: shell

   idf.py --board <BOARD> reconfigure
   idf.py --board <BOARD> build
   idf.py --board <BOARD> flash

选择部署方式
------------

可以根据软件包的预期更新频率及其是否属于产品固定依赖来选择部署方式：

.. blockdiag::
   :caption: 软件包部署决策

   blockdiag {
       orientation = portrait;
       default_fontsize = 14;
       change [label = "软件包是否需要\n独立更新？", shape = diamond];
       source [label = "频繁更新\n将 .py 安装到 /lib"];
       precompiled [label = "受控更新\n将 .mpy 安装到 /lib"];
       product [label = "是否为稳定的\n产品依赖？", shape = diamond];
       frozen [label = "通过开发板\nmanifest 冻结"];
       filesystem [label = "保存在 /lib\n以便替换"];
       change -> source [label = "频繁"];
       change -> precompiled [label = "偶尔"];
       change -> product [label = "很少"];
       product -> frozen [label = "是"];
       product -> filesystem [label = "否"];
   }

.. list-table:: 部署方式对比
   :header-rows: 1
   :widths: 22 26 26 26

   * - 属性
     - ``/lib`` 下的 ``.py``
     - ``/lib`` 下的 ``.mpy``
     - 冻结到固件
   * - 存储位置
     - 可写文件系统
     - 可写文件系统
     - 固件 Flash 镜像
   * - 导入行为
     - 导入时编译
     - 无需编译源文件即可加载
     - 直接执行冻结字节码
   * - 运行时 RAM
     - 字节码和对象占用 RAM
     - 字节码和对象占用 RAM
     - 冻结字节码和常量可保留在 Flash
   * - 更新方式
     - 替换文件或使用 ``mpremote mip``
     - 替换兼容的 ``.mpy`` 文件
     - 重新构建并烧录固件
   * - 主要用途
     - 开发及频繁更新的应用代码
     - 导入开销较低且可替换的模块
     - 稳定的产品依赖和启动模块

导入优先级与版本管理
--------------------

当前固件依次搜索当前目录、通过 ``.frozen`` 提供的冻结模块，然后搜索 ``/lib``。因此，同名的文件系统软件包在正常导入时不能覆盖冻结软件包；需要从 manifest 中移除冻结软件包并重新构建固件，或使用不同的软件包名称。

应将开发板 manifest 和冻结软件包版本纳入源码管理，以确保固件构建可复现。``package.json`` 用于描述 ``mip`` 安装的文件和依赖项，``manifest.py`` 用于定义固件构建时嵌入的模块；两者服务于不同的部署阶段，不能相互替代。

对于 ESP-VISION 产品，建议冻结稳定的框架扩展和启动依赖，将产品脚本、配置及需要现场更新的模块保存在 ``/lib``。大型 AI 模型、图像和视频资源通常应作为数据文件保存在 Flash、SD 卡或外部存储中，而不是作为 Python 软件包数据嵌入固件，以便更新并由适合的运行时组件进行映射。
