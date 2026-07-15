set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    boards/HIWONDER_AI_MODULE/sdkconfig.hiwonder
    boards/HIWONDER_AI_MODULE/sdkconfig.board
)

set(MICROPY_PY_BTREE OFF)
