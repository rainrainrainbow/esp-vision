# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

import os


_MARKER = "/.esp_vision_disk"

_MAIN_PY = """\
import sensor
import time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=1000)

while True:
    img = sensor.snapshot()
    img.flush()
    time.sleep_ms(20)
"""

_README_TXT = """\
ESP-VISION

Edit main.py to run your Python vision script.
Use the ESP-VISION VSCode extension to run scripts and preview frames.
The default main.py starts the camera and sends preview frames with img.flush().
"""


def _exists(path):
    try:
        os.stat(path)
        return True
    except OSError:
        return False


def _write_if_missing(path, data):
    if _exists(path):
        return
    with open(path, "w") as f:
        f.write(data)


def setup():
    if _exists(_MARKER):
        return

    _write_if_missing("/main.py", _MAIN_PY)
    _write_if_missing("/README.txt", _README_TXT)
    _write_if_missing(_MARKER, "ESP-VISION flash filesystem\n")


try:
    setup()
except OSError:
    pass
