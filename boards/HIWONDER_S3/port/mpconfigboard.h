#define MICROPY_HW_BOARD_NAME "Hiwonder S3 Vision"
#define MICROPY_HW_MCU_NAME   "ESP32-S3"

// 禁用 btree 模块，绕过 berkeley-db 在 GCC 15 下的兼容性问题
#define MICROPY_PY_BTREE (0)
