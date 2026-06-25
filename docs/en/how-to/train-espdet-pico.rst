Train an ESPDet Pico Model
==========================

:link_to_translation:`zh_CN:[中文]`

ESP-VISION runs ESPDet Pico object detection models through :py:class:`espdl.ESPDet`. Model training, export, and quantization are handled by the ESP-Detection project. ESP-VISION loads the exported ``.espdl`` file and runs real-time inference through the ``sensor``, ``image``, and ``espdl`` modules.

This guide uses a single-class object detection model as an example and covers the complete workflow from environment setup, dataset preparation, training, quantized export, and running the model on ESP-VISION.

1. Prepare the ESP-Detection Environment
----------------------------------------

Use the following training repository:

.. code-block:: bash

   git clone -b feat/esp-vision-train https://github.com/YanKE01/esp-detection.git
   cd esp-detection

ESP-Detection uses a Python training environment. ESP-IDF is not required for training. ESP-IDF is only required when building ESP-VISION firmware or running a C++ example project generated for ESP-DL.

ESP-Detection uses ``uv`` to manage dependencies. Select the installation command for the operating system.

Linux/macOS:

.. code-block:: bash

   curl -LsSf https://astral.sh/uv/install.sh | sh

Windows PowerShell:

.. code-block:: powershell

   powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

After installation, reopen the terminal or refresh the current terminal's ``PATH`` as instructed by the installer. After confirming that ``uv`` is available, synchronize the project environment:

.. code-block:: bash

   uv sync

``uv sync`` installs Ultralytics, PyTorch, ONNX, ONNX Runtime, OpenCV, and ``esp-ppq``. Training and quantization are much faster on machines with an NVIDIA GPU. CPU execution is also supported, but training time is significantly longer.

After the environment is ready, check that the command entry points are available:

.. code-block:: bash

   uv run python espdet_train.py --help
   uv run python -m deploy.quantize_aligned --help

2. Prepare the Dataset
----------------------

When preparing a dataset with Roboflow, export it in ``YOLOv11`` format. After export, organize the dataset in the ESP-Detection repository, for example:

.. code-block:: text

   datasets/my_object/
     images/
       train/
       val/
     labels/
       train/
       val/

ESP-Detection reads the YOLO detection label format. Each image has a same-named ``.txt`` label file under ``labels``. For example, ``images/train/0001.jpg`` corresponds to ``labels/train/0001.txt``. Each line in a label file describes one bounding box:

.. code-block:: text

   class_id x_center y_center width height

``x_center``, ``y_center``, ``width``, and ``height`` are floating-point values normalized by image width and height, usually in the ``0.0`` to ``1.0`` range.

For example, a single-class safety helmet dataset can use:

.. code-block:: text

   0 0.5123 0.4381 0.1840 0.1365

Dataset labels should match the class names shown later on the ESP-VISION side.

Then create a dataset configuration under ``cfg/datasets/``, for example ``cfg/datasets/my_object.yaml``:

.. code-block:: yaml

   path: datasets/my_object
   train: images/train
   val: images/val
   test:

   names:
     0: my_object

``path`` is the dataset root directory. ``train`` and ``val`` are image directories relative to ``path``. The order of ``names`` is important because it defines the meaning of the ``category`` value returned by ``espdl.ESPDet.detect()`` on the board. Keep this order stable after training.

For a multi-class model, add more classes:

.. code-block:: yaml

   names:
     0: helmet
     1: person
     2: vest

3. Train ESPDet Pico
--------------------

ESP-Detection provides ``espdet_train.py`` as the train-only entry point. It builds the ESPDet Pico network from ``cfg/models/espdet_pico.yaml`` and prints the best checkpoint path.

Single-class training example:

.. code-block:: bash

   uv run python espdet_train.py \
     --class_name my_object \
     --pretrained_path None \
     --dataset cfg/datasets/my_object.yaml \
     --size 320 320

Key parameters:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Parameter
     - Description
   * - ``--class_name``
     - Training task name. It affects the Ultralytics output directory name.
   * - ``--pretrained_path``
     - Pretrained checkpoint path. Use ``None`` when no pretrained checkpoint is available; the script builds a new model from ``cfg/models/espdet_pico.yaml``.
   * - ``--dataset``
     - Dataset YAML path.
   * - ``--size``
     - Input size in ``height width`` format. For example, ``320 320`` means 320x320 input.

The training script uses long training by default, with a large batch size, RAM cache, and data augmentation. For small datasets, monitor validation curves to avoid overfitting. When training finishes, the terminal prints output similar to:

.. code-block:: text

   Training complete: runs/detect/my_object
   Best weights: runs/detect/my_object/weights/best.pt

Use ``weights/best.pt`` for quantization.

4. Prepare Calibration Data
---------------------------

Exporting ``.espdl`` requires int8 quantization, and quantization requires calibration images. Calibration images should cover the real deployment distribution, including different lighting conditions, distances, angles, backgrounds, and object sizes.

The simplest approach is to use validation images directly:

.. code-block:: text

   datasets/my_object/images/val

A separate calibration directory can also be prepared:

.. code-block:: text

   deploy/my_object_calib/
     0001.jpg
     0002.jpg
     0003.jpg

Calibration images do not need labels, but their distribution should be close to actual camera input. If the calibration set is too narrow, simulator metrics may look normal while on-board scores are low or detections are missing.

5. Quantize and Export .espdl
-----------------------------

Use ``deploy.quantize_aligned`` to export the trained ``best.pt`` checkpoint to an ESP-DL ``.espdl`` model:

.. code-block:: bash

   uv run python -m deploy.quantize_aligned \
     --model runs/detect/my_object/weights/best.pt \
     --size 320 320 \
     --target esp32p4 \
     --calib_data datasets/my_object/images/val \
     --espdl deploy/espdet_pico_320_320_my_object.espdl \
     --data cfg/datasets/my_object.yaml

Parameters:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Parameter
     - Description
   * - ``--model``
     - Trained ``best.pt`` checkpoint. An exported ONNX model can also be used.
   * - ``--size``
     - Model input size. It must match the training size.
   * - ``--target``
     - Target chip, such as ``esp32p4`` or ``esp32s3``. It should match the chip that will run the model.
   * - ``--calib_data``
     - Calibration image directory.
   * - ``--espdl``
     - Output ESP-DL model path.
   * - ``--data``
     - Dataset YAML. When provided, the script also reports simulator validation metrics.

The quantization script prepares ONNX from ``pt`` internally, then exports an int8 ESP-DL model through ESP-PPQ. The current configuration uses percentile calibration, bias correction, and fusion settings tuned for scale consistency. The script prints the exported model's operator histogram. When ``--data`` is provided, it also prints simulator ``mAP50`` and ``mAP50-95``.

After quantization, confirm that the ``.espdl`` file exists:

.. code-block:: bash

   ls -lh deploy/espdet_pico_320_320_my_object.espdl

6. Add Model Metadata
---------------------

To publish the model as a shared ESP-VISION asset, place it under ``models/espdet/pico/<name>/``:

.. code-block:: text

   models/espdet/pico/my_object/
     README.md
     espdet_pico_320_320_my_object.espdl
     espdet_pico_320_320_my_object.json

The ``.espdl`` file is the model binary. The ``.json`` file is sidecar metadata read by model lists and tooling. Minimal JSON example:

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

``labels`` must follow the same order as ``names`` in the dataset YAML. ``sizeBytes`` should be the actual model file size. On Linux, use:

.. code-block:: bash

   stat -c '%s' models/espdet/pico/my_object/espdet_pico_320_320_my_object.espdl

``README.md`` should describe the model purpose, dataset source, input size, quantization metrics, and basic usage. For public datasets, keep the dataset link in README instead of the JSON file.

7. Copy the Model to Board Storage
----------------------------------

ESP-VISION loads models from the file system at runtime. Two common paths are:

.. list-table::
   :header-rows: 1
   :widths: 28 72

   * - Path
     - Use
   * - ``/sdcard``
     - SD card. Suitable for multiple models, large files, or frequent model replacement.
   * - ``/flash``
     - On-flash FAT data partition. It is usually exposed as a USB MSC drive and is suitable for a small number of frequently used models.

For example, after copying the model to the SD card root directory, use:

.. code-block:: python

   MODEL = "/sdcard/espdet_pico_320_320_my_object.espdl"

If the model is copied to the flash root directory, use:

.. code-block:: python

   MODEL = "/flash/espdet_pico_320_320_my_object.espdl"

8. Run on ESP-VISION
--------------------

The following is a complete MicroPython inference script:

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

``sensor`` output should use ``RGB565``. ``score`` is the confidence threshold, and ``nms`` is the NMS threshold. During the first on-board test, start with a lower ``score`` to check whether the model can produce stable boxes, then adjust it based on false positives and missed detections.

9. Validate and Tune
--------------------

Use the following validation order:

- Check the training log and validation metrics first to confirm that the floating-point model does not obviously underfit or overfit.
- Check the simulator metrics reported by the quantization script to confirm that the int8 model does not drop significantly.
- Run the model with real camera input on ESP-VISION, and inspect detections at different distances, angles, and lighting conditions.
- Tune ``score`` and ``nms``, then freeze the final model, labels, and example script.

If simulator metrics are normal but on-board results are poor, check these items first:

- Whether ``--size`` exactly matches the training size.
- Whether ``labels`` follows the same order as ``names`` in the dataset YAML.
- Whether calibration images cover the real camera input distribution.
- Whether the board camera image has exposure issues, color issues, or objects that are too small.
- Whether ``score`` is too high and filters out low-score detections.

After the model file, JSON metadata, README, and test script are verified, the model can be added to ESP-VISION's ``models/`` and ``example/`` directories as a reusable model.
