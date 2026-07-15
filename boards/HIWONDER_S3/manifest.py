# MicroPython frozen manifest for Hiwonder S3
freeze("$(PORT_DIR)/modules")
freeze("$(ESP_VISION_ROOT)/modules", "py_inisetup.py")
freeze("$(ESP_VISION_ROOT)/boards/HIWONDER_S3", "board_inisetup.py")
include("$(MPY_DIR)/extmod/asyncio")
require("webrepl")
