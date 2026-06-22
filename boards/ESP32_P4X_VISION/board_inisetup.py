# SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
#
# SPDX-License-Identifier: Apache-2.0

MAIN_PY = """\
import time

print("ESP32_P4X_VISION ready")

while True:
    time.sleep_ms(1000)
"""

README_TXT = """\
ESP32_P4X_VISION

Edit main.py to run your Python vision script.
The default main.py keeps the board idle so host tools can take control.
"""
