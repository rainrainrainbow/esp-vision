# Copy this directory to overlay/micropython/ports/esp32/boards/<BOARD>
# and replace TEMPLATE settings before building a real board.

set(IDF_TARGET esp32p4)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    boards/sdkconfig.p4x_template
    boards/TEMPLATE/sdkconfig.board
)

# Keep early bring-up independent of optional MicroPython submodules.
set(MICROPY_PY_BTREE OFF)
