MicroPython 语言基础
====================

:link_to_translation:`en:[English]`

MicroPython 是面向微控制器直接运行的紧凑型 Python 语言实现。ESP-VISION 使用 MicroPython v1.28.0 作为应用开发语言，同时通过 C/C++ 与 ESP-IDF 组件实现硬件访问和性能关键的视觉处理。本章介绍阅读和编写 ESP-VISION 应用所需的语言模型；完整权威说明仍以 `MicroPython v1.28.0 语言与实现参考 <https://docs.micropython.org/en/v1.28.0/reference/index.html>`_ 为准。

执行模型
--------

上游 MicroPython 以实现 Python 3.4 语法为目标，并选择性支持后续 Python 版本的部分特性。ESP-VISION 以 ``EXTRA_FEATURES`` 配置级别构建 ESP32 port，并启用设备端编译器、垃圾回收器、MPZ 长整数、单精度浮点、调度器、VFS 和持久字节码加载。源代码仍是动态类型的 Python，但会被编译为紧凑字节码，并由 MicroPython 虚拟机执行。原生模块将 Python 对象连接到 ESP-VISION C/C++ 组件、ESP-IDF 驱动和芯片硬件：

.. blockdiag::

   blockdiag {
     orientation = portrait;

     source   [label = "Python 源代码\n.py"];
     frozen   [label = "预编译或冻结模块\n.mpy / 固件"];
     bytecode [label = "MicroPython 字节码"];
     vm       [label = "MicroPython 虚拟机"];
     modules  [label = "Python 与原生模块\nsensor / image / espdl / machine"];
     native   [label = "ESP-VISION C/C++ 组件\n与 ESP-IDF 驱动"];
     hardware [label = "摄像头、显示、存储、\n网络和芯片加速器"];

     source -> bytecode;
     frozen -> bytecode;
     bytecode -> vm -> modules -> native -> hardware;
   }

从 ``.py`` 加载的代码会在设备上编译，并在编译期间占用 RAM。预编译的 ``.mpy`` 文件和冻结到固件中的模块可以减少运行时编译开销；冻结模块中的不可变常量可以保留在 Flash 中。Python 仍负责应用策略和对象生命周期，而原生组件负责图像帧采集、图像处理、编解码和 AI 推理等操作。

基本语法
--------

ESP-VISION 当前配置使用缩进定义代码块，并支持赋值、表达式、条件、循环、函数、类、异常、上下文管理器、推导式、生成器、导入以及 ``async``/``await`` 等常用 Python 形式。名称会动态绑定到对象，因此声明时不需要指定 C 风格的存储类型：

.. code-block:: python

   from micropython import const

   _MIN_PIXELS = const(80)

   class Detection:
       def __init__(self, label, score):
           self.label = label
           self.score = score

       def accepted(self, threshold=0.5):
           return self.score >= threshold

   def select_results(results):
       selected = []
       for label, score in results:
           item = Detection(label, score)
           if item.accepted():
               selected.append(item)
       return selected

   try:
       detections = select_results((("person", 0.91), ("chair", 0.42)))
       for item in detections:
           print(item.label, item.score)
   except (ValueError, TypeError) as error:
       print("invalid result:", error)
   finally:
       print("processing complete")

``const()`` 是 MicroPython 扩展，允许编译器直接替换常量值，并可减少字节码和全局字典开销。类型注解有助于开发者和编辑器理解代码，但 MicroPython 不会借此提供 C 风格的静态类型或自动内存布局。

主要数据结构
------------

在微控制器上，可变对象与不可变对象的选择十分重要，因为创建、扩容或拼接对象都会分配堆内存：

.. list-table::
   :header-rows: 1
   :widths: 20 18 62

   * - 类型
     - 可变性
     - ESP-VISION 使用建议
   * - ``None`` 与 ``bool``
     - 不可变
     - 表示值缺失和状态标志，无需额外定义自有哨兵对象。
   * - ``int`` 与 ``float``
     - 不可变
     - 存储计数、坐标、阈值和分数。重复算术运算可能创建新对象；硬中断上下文中应避免浮点运算。
   * - ``str``
     - 不可变
     - 存储文本、路径、标签和 JSON 键。拼接会创建新字符串，因此应避免每帧构造大型字符串。
   * - ``bytes``
     - 不可变
     - 存储紧凑的二进制常量、编码数据包以及只读模型或协议数据。
   * - ``bytearray``
     - 可变
     - 为外设 I/O 和数据包组装提供可复用二进制缓冲，避免每次操作都分配新对象。
   * - ``list``
     - 可变
     - 存储长度变化的有序结果。``append()`` 和扩容可能分配内存，因此持续帧循环中的列表应限制大小或重复使用。
   * - ``tuple``
     - 不可变
     - 表示矩形和检测结果等固定记录。常量元组通常比反复创建列表更节省内存。
   * - ``dict``
     - 可变
     - 表示配置和共享标量状态。新增键可能触发表扩容，应在初始化期间创建预期结构。
   * - ``set``
     - 可变
     - 在额外哈希表内存开销合理时，用于成员关系和唯一性检查。
   * - ``range``
     - 不可变
     - 描述整数迭代范围，无需构造包含全部数值的列表。
   * - ``memoryview`` 与 ``array``
     - 视图 / 可变
     - 以较少拷贝访问类型化或切片缓冲数据；视图有效期间必须保持底层缓冲存活且不能调整其大小。

对象引用不会自动复制。执行 ``b = a`` 后，两个名称会引用同一个可变列表、字典、图像或缓冲；仅在确实需要独立数据时才应显式复制。对于由可复用摄像头帧缓冲支持的 :py:class:`image.Image` 对象，这一点尤其重要。

面向微控制器的设计理念
----------------------

MicroPython 优先考虑可移植性、交互式开发、紧凑固件集成和直接硬件访问，而不是完整覆盖 CPython 标准库。垃圾回收堆使动态应用代码可行，但 RAM、Flash、栈空间和内存分配时延仍然有限。`MicroPython 受限设备指南 <https://docs.micropython.org/en/v1.28.0/reference/constrained.html>`_ 建议减少重复对象创建、预分配缓冲、将常量和可复用模块放入冻结字节码，并使用 ``gc.mem_free()`` 与 ``gc.mem_alloc()`` 监控堆。

ESP-VISION 应用应在进入帧循环前初始化模块、模型、缓冲和结构稳定的字典。重复使用 ``bytearray`` 与图像缓冲，限制每帧临时数据量；除非已经复制，否则不要在下一次 ``sensor.snapshot()`` 后继续持有摄像头帧缓冲支持的图像。Python 适合表达编排与产品行为；确定性实时工作、ISR 处理、大批量像素处理、编解码和推理内核应保留在原生组件中。

MicroPython 与 CPython
----------------------

本文档在进行运行时对比时，“Python”指通常运行于 PC 和服务器的参考实现 CPython 3.x。MicroPython 的具体功能集取决于固件配置，因此下表描述的是 ESP-VISION 当前配置的 MicroPython v1.28.0：所有维护中的开发板 manifest 都会冻结 ``asyncio``，ESP32 port 启用了带 GIL 的 ``_thread``，硬件 API 则仍受芯片和开发板配置约束。

.. list-table::
   :header-rows: 1
   :widths: 22 39 39

   * - 对比项
     - ESP-VISION MicroPython
     - CPython
   * - 主要运行环境
     - 运行于 MCU 固件内部，可直接访问外设与板级服务。
     - 作为操作系统进程运行于 PC、服务器和较大型嵌入式系统。
   * - 语言语法
     - 实现 Python 3.4 语法并选择性支持后续特性；常用控制流、函数、类、异常、生成器和异步语法均可使用。
     - 跟随当前 Python 语言规范，通常最先引入新语法。
   * - 类型系统
     - 动态类型；类型注解不会使执行过程变为静态类型。
     - 动态类型；类型注解主要由工具和可选库使用。
   * - 标准库
     - 提供构建固件时选择的资源导向子集，并增加 ``machine``、``micropython``、``esp`` 和 ``esp32`` 等硬件与运行时模块。
     - 为安装的版本提供完整 CPython 标准库，但不包含标准 MCU 外设 API。
   * - 软件包部署
     - 模块可复制到存储、预编译为 ``.mpy`` 或冻结到固件中；不能假定支持桌面 ``pip`` 和任意二进制 wheel。
     - 通常使用 ``pip``、虚拟环境、源码发行包和平台 wheel。
   * - 内存模型
     - 使用与固件资源共享的小型垃圾回收堆，必须显式考虑分配失败和碎片化。
     - 使用容量大得多的进程堆，并采用引用计数和循环垃圾回收。
   * - 并发
     - 提供固件内冻结的 ``asyncio``、已启用且带 GIL 的 ``_thread``，以及经过调度的 GPIO IRQ 与 Timer 回调；外设可用性仍取决于所选芯片和开发板。
     - 提供 ``asyncio``、线程、进程和丰富的操作系统同步机制。
   * - 性能路径
     - 高负载操作调用原生 C/C++ 模块和硬件加速器；Python 更适合用于编排。
     - 可使用优化扩展模块、带 JIT 的其他运行时或多进程，但通常拥有更多 CPU 与内存资源。
   * - 兼容性细节
     - 为减少代码体积和 RAM 使用，部分内置行为、异常文本、反射能力、模块内容和边界情况存在差异。
     - 定义 MicroPython 差异文档所对照的参考行为。
   * - 可移植性
     - 使用受支持子集的纯 Python 代码通常可以移植；硬件模块和资源假设与设备相关。
     - 依赖可用时，纯 Python 代码通常可在受支持操作系统间移植。

不能只根据语法判断可移植性，还应检查导入模块、内存使用、文件系统路径、并发假设和硬件访问。上游 `MicroPython 与 CPython 差异 <https://docs.micropython.org/en/v1.28.0/genrst/index.html>`_ 记录了已知行为差异，:doc:`../api-reference/micropython` 则说明 ESP-VISION 可用的沿用 API 与并发模型。
