# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0
#
# ESP-VISION board init script - runs once at boot after _boot.py
# Handles VFS (flash FAT partition) mounting

import machine
import uos

# Try to mount the internal flash FAT partition
# The partition is created by boardctrl_startup() in main.c
# but needs to be formatted and mounted here

def mount_vfs():
    try:
        # Try to mount existing FAT partition
        fat = machine.FATFS()
        uos.mount(fat, "/sdcard")
        print("[esp-vision] VFS mounted at /sdcard")
        return True
    except Exception as e:
        print("[esp-vision] VFS mount failed:", e)
        try:
            # Try to format and mount
            fat = machine.FATFS()
            fat.mkfs()
            uos.mount(fat, "/sdcard")
            print("[esp-vision] VFS formatted and mounted at /sdcard")
            return True
        except Exception as e2:
            print("[esp-vision] VFS format also failed:", e2)
            return False

mount_vfs()
