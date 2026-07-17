set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    boards/ESP32_S3_CUSTOM/sdkconfig.s3_custom
    boards/ESP32_S3_CUSTOM/sdkconfig.board
)

set(MICROPY_PY_BTREE OFF)
