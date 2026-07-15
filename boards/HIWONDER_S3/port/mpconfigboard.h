#ifndef MICROPY_BOARD_H
#define MICROPY_BOARD_H

#define MICROPY_HW_BOARD_NAME "Hiwonder S3 Vision"
#define MICROPY_HW_MCU_NAME   "ESP32-S3"

// Disable btree (berkeley-db has GCC 15 incompatibility in ESP-IDF 6.0)
#ifndef MICROPY_PY_BTREE
#define MICROPY_PY_BTREE (0)
#endif

#endif
