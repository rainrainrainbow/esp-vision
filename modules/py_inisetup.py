# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0
#
# ESP-VISION board init - mounts internal flash FAT partition as /sdcard

import esp32
import os
import machine

def mount_vfs():
    try:
        parts = esp32.Partition.find(esp32.Partition.TYPE_DATA, label="ffat")
        if not parts:
            print("[esp-vision] No ffat partition found")
            return False

        bdev = parts[0]

        # Try to mount existing filesystem first
        try:
            vfs = os.VfsFat(bdev)
            os.mount(vfs, "/sdcard")
            print("[esp-vision] ffat mounted at /sdcard")
            return True
        except:
            # Need to format first
            print("[esp-vision] Formatting ffat partition...")
            try:
                os.VfsFat.mkfs(bdev)
                vfs = os.VfsFat(bdev)
                os.mount(vfs, "/sdcard")
                print("[esp-vision] ffat formatted and mounted")
                return True
            except Exception as e:
                print("[esp-vision] mkfs failed:", e)
                return False
    except Exception as e:
        print("[esp-vision] VFS mount error:", e)
        return False

mount_vfs()
