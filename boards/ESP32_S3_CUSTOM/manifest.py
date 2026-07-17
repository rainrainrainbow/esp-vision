# SPDX-FileCopyrightText: 2026
# SPDX-License-Identifier: Apache-2.0

freeze("$(PORT_DIR)/modules")
freeze("$(ESP_VISION_ROOT)/modules", "py_inisetup.py")
freeze("$(ESP_VISION_ROOT)/boards/ESP32_S3_CUSTOM", "board_inisetup.py")
include("$(MPY_DIR)/extmod/asyncio")
