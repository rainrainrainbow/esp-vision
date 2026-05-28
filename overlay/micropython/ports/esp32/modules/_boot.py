# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

import gc
import vfs

from flashbdev import bdev


try:
    if bdev:
        vfs.mount(bdev, "/")
except OSError:
    # ESP-VISION formats or repairs the flash filesystem in py_inisetup.py.
    pass

gc.collect()
