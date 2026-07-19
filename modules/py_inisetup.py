# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

import os
import vfs

from flashbdev import bdev

_MARKER = "/.esp_vision_disk"

_DEFAULT_BOOT_PY = """\
# This file is executed on every boot.
"""

_DEFAULT_MAIN_PY = """\
import time

print("ESP-VISION HIWONDER_AI_MODULE ready")

while True:
    time.sleep_ms(1000)
"""

_DEFAULT_README_TXT = """\
ESP-VISION HIWONDER_AI_MODULE

Edit main.py to run your Python vision script.
Use the ESP-VISION VSCode extension to run scripts and preview frames.
The default main.py keeps the board idle so host tools can take control.
"""

def _board_default(name, default):
    try:
        import board_inisetup
        return getattr(board_inisetup, name, default)
    except (ImportError, AttributeError):
        return default

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

def _boot_sector_empty():
    if bdev is None:
        return False
    buf = bytearray(bdev.ioctl(5, 0))  # 5 is SEC_SIZE
    bdev.readblocks(0, buf)
    for b in buf:
        if b != 0xFF:
            return False
    return True

def _format_flash():
    if bdev is None:
        return False
    try:
        vfs.umount("/")
    except OSError:
        pass
    label = bdev.info()[4]
    if label == "ffat":
        vfs.VfsFat.mkfs(bdev)
        fs = vfs.VfsFat(bdev)
    elif label == "vfs":
        vfs.VfsLfs2.mkfs(bdev)
        fs = vfs.VfsLfs2(bdev)
    else:
        return False
    vfs.mount(fs, "/")
    return True

def _write_default_files():
    if _exists(_MARKER):
        return
    _write_if_missing("/boot.py", _board_default("BOOT_PY", _DEFAULT_BOOT_PY))
    _write_if_missing("/main.py", _board_default("MAIN_PY", _DEFAULT_MAIN_PY))
    _write_if_missing("/README.txt", _board_default("README_TXT", _DEFAULT_README_TXT))
    _write_if_missing(_MARKER, "ESP-VISION flash filesystem\n")

def setup():
    if _boot_sector_empty():
        print("Performing ESP-VISION initial setup")
        _format_flash()
    try:
        _write_default_files()
    except (OSError, UnicodeError):
        print("Repairing ESP-VISION flash filesystem")
        if _format_flash():
            _write_default_files()

try:
    setup()
except (OSError, UnicodeError) as e:
    print("ESP-VISION flash setup failed:", e)
