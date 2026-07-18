# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0
#
# ESP-VISION board init - mounts internal flash FAT partitions

import esp32
import os
import machine

def format_and_mount(label, mount_point):
    """Find, format (if needed) and mount a FAT partition by label."""
    try:
        parts = esp32.Partition.find(esp32.Partition.TYPE_DATA, label=label)
        if not parts:
            print("[esp-vision] Partition '%s' not found" % label)
            return False

        bdev = parts[0]

        # Try to mount existing filesystem first
        try:
            vfs = os.VfsFat(bdev)
            os.mount(vfs, mount_point)
            print("[esp-vision] '%s' mounted at %s" % (label, mount_point))
            return True
        except:
            # Need to format
            print("[esp-vision] Formatting '%s' partition..." % label)
            try:
                os.VfsFat.mkfs(bdev)
                vfs = os.VfsFat(bdev)
                os.mount(vfs, mount_point)
                print("[esp-vision] '%s' formatted and mounted at %s" % (label, mount_point))
                return True
            except Exception as e:
                print("[esp-vision] mkfs '%s' failed:" % label, e)
                return False
    except Exception as e:
        print("[esp-vision] Error with '%s':" % label, e)
        return False

def mount_vfs():
    # Mount vfs as /sdcard (main user storage)
    format_and_mount("vfs", "/sdcard")
    # Also format cvs if it exists to prevent USB format dialog
    format_and_mount("cvs", "/cvs")

mount_vfs()
