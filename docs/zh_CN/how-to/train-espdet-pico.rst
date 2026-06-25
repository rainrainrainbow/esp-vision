训练 ESPDet Pico 模型
=====================

:link_to_translation:`en:[English]`

ESP-VISION 通过 :py:class:`espdl.ESPDet` 运行 ESPDet Pico 目标检测模型。模型训练、导出和量化在 ESP-Detection 项目中完成，ESP-VISION 侧只负责加载导出的 ``.espdl`` 文件，并通过 ``sensor``、``image`` 和 ``espdl`` 模块完成实时推理。

本文以单类别目标检测模型为例，说明从环境安装、数据集准备、训练、量化导出到 ESP-VISION 上运行的完整流程。

1. 准备 ESP-Detection 环境
--------------------------

训练仓库地址如下：

.. code-block:: bash

   git clone -b feat/esp-vision-train https://github.com/YanKE01/esp-detection.git
   cd esp-detection

ESP-Detection 使用 Python 训练环境，不需要先安装 ESP-IDF。只有编译 ESP-VISION 固件，或者运行 ESP-DL 生成的 C++ 示例工程时才需要 ESP-IDF。

ESP-Detection 使用 ``uv`` 管理依赖。根据操作系统选择对应安装方式。

Linux/macOS：

.. code-block:: bash

   curl -LsSf https://astral.sh/uv/install.sh | sh

Windows PowerShell：

.. code-block:: powershell

   powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

安装完成后，重新打开终端，或根据安装脚本提示刷新当前终端的 ``PATH``。确认 ``uv`` 可用后，同步项目环境：

.. code-block:: bash

   uv sync

``uv sync`` 会安装 Ultralytics、PyTorch、ONNX、ONNX Runtime、OpenCV 和 ``esp-ppq``。如果机器有 NVIDIA GPU，训练和量化会明显更快；没有 GPU 时也可以走 CPU，但训练时间会长很多。

环境安装完成后，可以先确认命令入口可用：

.. code-block:: bash

   uv run python espdet_train.py --help
   uv run python -m deploy.quantize_aligned --help

2. 准备数据集
-------------

使用 Roboflow 准备数据集时，推荐导出 ``YOLOv11`` 格式。导出后可将数据集整理到 ESP-Detection 仓库，例如：

.. code-block:: text

   datasets/my_object/
     images/
       train/
       val/
     labels/
       train/
       val/

ESP-Detection 读取的是 YOLO detection 标签格式。每张图片在 ``labels`` 目录下有一个同名 ``.txt`` 标签文件。例如 ``images/train/0001.jpg`` 对应 ``labels/train/0001.txt``。标签文件每一行表示一个目标框：

.. code-block:: text

   class_id x_center y_center width height

其中 ``x_center``、``y_center``、``width`` 和 ``height`` 都是相对图片宽高归一化后的浮点数，范围通常为 ``0.0`` 到 ``1.0``。

例如单类别安全帽检测的数据可以写成：

.. code-block:: text

   0 0.5123 0.4381 0.1840 0.1365

数据集标签应和后续 ESP-VISION 端显示的类别名保持一致。

接着在 ``cfg/datasets/`` 下创建数据集配置，例如 ``cfg/datasets/my_object.yaml``：

.. code-block:: yaml

   path: datasets/my_object
   train: images/train
   val: images/val
   test:

   names:
     0: my_object

``path`` 是数据集根目录，``train`` 和 ``val`` 是相对 ``path`` 的图片目录。``names`` 的顺序非常重要，它决定了板端 ``espdl.ESPDet.detect()`` 返回的 ``category`` 含义。训练完成后不要随意改变这个顺序。

如果是多类别模型，继续添加类别即可：

.. code-block:: yaml

   names:
     0: helmet
     1: person
     2: vest

3. 训练 ESPDet Pico
-------------------

ESP-Detection 提供了 ``espdet_train.py`` 作为只训练入口。它会使用 ``cfg/models/espdet_pico.yaml`` 构建 ESPDet Pico 网络，并输出最佳权重。

单类别模型训练示例：

.. code-block:: bash

   uv run python espdet_train.py \
     --class_name my_object \
     --pretrained_path None \
     --dataset cfg/datasets/my_object.yaml \
     --size 320 320

关键参数说明：

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - 参数
     - 说明
   * - ``--class_name``
     - 训练任务名称，会影响 Ultralytics 输出目录名称。
   * - ``--pretrained_path``
     - 预训练权重路径。没有预训练权重时使用 ``None``，脚本会从 ``cfg/models/espdet_pico.yaml`` 构建新模型。
   * - ``--dataset``
     - 数据集 YAML 路径。
   * - ``--size``
     - 输入尺寸，格式为 ``height width``。例如 ``320 320`` 表示 320x320 输入。

训练脚本内部默认使用较长训练轮数、较大 batch、RAM cache 和数据增强策略。对于小数据集，训练可能需要观察验证集曲线，避免过拟合。训练完成后，终端会打印类似下面的信息：

.. code-block:: text

   Training complete: runs/detect/my_object
   Best weights: runs/detect/my_object/weights/best.pt

后续量化时使用 ``weights/best.pt``。

4. 准备校准数据
---------------

导出 ``.espdl`` 前需要做 int8 量化，量化需要校准图片。校准图片应尽量覆盖真实使用场景，例如不同光照、距离、角度、背景和目标大小。

最简单的做法是直接使用验证集图片：

.. code-block:: text

   datasets/my_object/images/val

也可以单独准备一个校准目录：

.. code-block:: text

   deploy/my_object_calib/
     0001.jpg
     0002.jpg
     0003.jpg

校准图片不需要标签，但分布要接近摄像头实际输入。如果校准集过于单一，可能出现模拟器指标正常、板端分数偏低或漏检的问题。

5. 量化并导出 .espdl
--------------------

使用 ``deploy.quantize_aligned`` 将训练好的 ``best.pt`` 导出为 ESP-DL 可加载的 ``.espdl`` 模型：

.. code-block:: bash

   uv run python -m deploy.quantize_aligned \
     --model runs/detect/my_object/weights/best.pt \
     --size 320 320 \
     --target esp32p4 \
     --calib_data datasets/my_object/images/val \
     --espdl deploy/espdet_pico_320_320_my_object.espdl \
     --data cfg/datasets/my_object.yaml

参数说明：

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - 参数
     - 说明
   * - ``--model``
     - 训练得到的 ``best.pt``，也可以是已导出的 ONNX。
   * - ``--size``
     - 模型输入尺寸，必须和训练尺寸一致。
   * - ``--target``
     - 目标芯片，例如 ``esp32p4`` 或 ``esp32s3``。应和最终运行模型的芯片匹配。
   * - ``--calib_data``
     - 校准图片目录。
   * - ``--espdl``
     - 导出的 ESP-DL 模型路径。
   * - ``--data``
     - 数据集 YAML。传入后脚本会额外输出模拟器验证指标。

该量化脚本会在内部完成 ``pt`` 到 ONNX 的准备，然后使用 ESP-PPQ 导出 int8 ESP-DL 模型。当前配置使用 percentile 校准、bias correction 和面向 scale 一致性的 fusion 设置。脚本会打印导出模型的算子统计；如果传入 ``--data``，还会打印模拟器 ``mAP50`` 和 ``mAP50-95``。

量化完成后，确认 ``.espdl`` 文件存在：

.. code-block:: bash

   ls -lh deploy/espdet_pico_320_320_my_object.espdl

6. 添加模型元数据
-----------------

如果模型需要作为 ESP-VISION 仓库中的共享模型发布，建议放在 ``models/espdet/pico/<name>/`` 下：

.. code-block:: text

   models/espdet/pico/my_object/
     README.md
     espdet_pico_320_320_my_object.espdl
     espdet_pico_320_320_my_object.json

其中 ``.espdl`` 是实际模型文件，``.json`` 是模型列表和工具链读取的 sidecar 元数据。最小 JSON 示例：

.. code-block:: json

   {
       "name": "ESPDet Pico My Object",
       "architecture": "ESPDet Pico",
       "api": "espdl.ESPDet",
       "task": "detection",
       "input": "320x320",
       "inputFormat": "RGB565",
       "dataset": "My Object",
       "labels": ["my_object"],
       "sizeBytes": 574864,
       "description": "Detects my_object in camera images."
   }

``labels`` 必须和数据集 YAML 中 ``names`` 的顺序一致。``sizeBytes`` 应填写实际模型文件大小，在 Linux 上可以这样获取：

.. code-block:: bash

   stat -c '%s' models/espdet/pico/my_object/espdet_pico_320_320_my_object.espdl

``README.md`` 建议说明模型用途、数据集来源、输入尺寸、量化指标和基本使用方法。对于公开数据集，数据集链接放在 README 中即可，不建议写进 JSON。

7. 拷贝模型到板端存储
---------------------

ESP-VISION 运行时从文件系统加载模型。常用路径有两个：

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - 路径
     - 用途
   * - ``/sdcard``
     - SD 卡。适合模型较多、文件较大或频繁替换的场景。
   * - ``/flash``
     - 片上 FAT 数据分区。通常通过 USB MSC 暴露为 U 盘，适合放少量常用模型。

例如将模型复制到 SD 卡根目录后，板端路径可以写成：

.. code-block:: python

   MODEL = "/sdcard/espdet_pico_320_320_my_object.espdl"

如果复制到 flash 根目录，则路径通常为：

.. code-block:: python

   MODEL = "/flash/espdet_pico_320_320_my_object.espdl"

8. 在 ESP-VISION 上运行
-----------------------

下面是一个完整的 MicroPython 推理脚本：

.. code-block:: python

   import espdl
   import sensor
   import time

   MODEL = "/sdcard/espdet_pico_320_320_my_object.espdl"
   LABELS = ("my_object",)

   sensor.reset()
   sensor.set_pixformat(sensor.RGB565)
   sensor.set_framesize(sensor.QVGA)
   sensor.skip_frames(time=1000)

   det = espdl.ESPDet(MODEL, score=0.5, nms=0.7)

   try:
       while True:
           img = sensor.snapshot()
           for x, y, w, h, score, category in det.detect(img):
               label = LABELS[category] if category < len(LABELS) else str(category)
               img.draw_rectangle(x, y, w, h, color=(255, 0, 0), thickness=2)
               img.draw_string(x, max(0, y - 12), "%s %.2f" % (label, score), color=(255, 0, 0))
           img.flush()
           time.sleep_ms(20)
   finally:
       det.deinit()

``sensor`` 输出应使用 ``RGB565``。``score`` 是置信度阈值，``nms`` 是 NMS 阈值。第一次上板时建议先用较低 ``score`` 观察模型是否能稳定出框，再根据误检和漏检情况调整。

9. 验证和调参
-------------

建议按以下顺序验证：

- 先看训练日志和验证集指标，确认浮点模型没有明显欠拟合或过拟合。
- 再看量化脚本输出的模拟器指标，确认 int8 模型没有大幅掉点。
- 然后在 ESP-VISION 上运行真实摄像头脚本，观察不同距离、角度和光照下的检测框。
- 最后调节 ``score`` 和 ``nms``，并固定最终模型、标签和示例脚本。

如果模拟器指标正常但板端效果差，优先检查这些问题：

- ``--size`` 是否和训练尺寸完全一致。
- ``labels`` 是否和数据集 YAML 中 ``names`` 顺序一致。
- 校准图片是否覆盖真实摄像头输入分布。
- 板端摄像头画面是否曝光异常、颜色异常或目标过小。
- ``score`` 是否设得过高，导致低分检测结果被过滤。

模型文件、JSON 元数据、README 和测试脚本都确认后，就可以将模型加入 ESP-VISION 的 ``models/`` 和 ``example/`` 目录，作为可复用模型发布。
