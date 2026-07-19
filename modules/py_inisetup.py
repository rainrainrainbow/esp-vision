# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0
#
# ESP-VISION board init - mounts internal flash FAT partition as /sdcard

import esp32
import os
import machine

def is_fat_formatted(bdev):
    """Check if partition has a valid FAT boot sector by looking for 0x55 0xAA signature."""
    try:
        buf = bytearray(512)
        bdev.readblocks(0, buf)
        # FAT boot sector ends with 0x55 0xAA at offset 0x1FE
        return buf[0x1FE] == 0x55 and buf[0x1FF] == 0xAA
    except:
        return False

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
            # Only format if the partition doesn't have a valid FAT
            if is_fat_formatted(bdev):
                print("[esp-vision] FAT boot sector found but mount failed - trying mkfs")
            else:
                print("[esp-vision] No valid FAT found - formatting...")

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
