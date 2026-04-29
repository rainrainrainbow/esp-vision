# esp-vision top-level Makefile
# Wraps idf.py with the right flags for the MicroPython esp32 port.

ROOT      := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BOARD     ?= ESP32_P4X_EYE
BUILD     := $(ROOT)/build/$(BOARD)
MP_PORT   := $(ROOT)/lib/micropython/ports/esp32

IDF_ARGS := -C $(MP_PORT) \
            -B $(BUILD) \
            -D MICROPY_BOARD=$(BOARD) \
            -D USER_C_MODULES=$(ROOT)/micropython.cmake \
            -D MICROPY_FROZEN_MANIFEST=$(ROOT)/boards/$(BOARD)/manifest.py \
            -D EXTRA_COMPONENT_DIRS=$(ROOT)/components

ifdef ESPPORT
IDF_ARGS += -p $(ESPPORT)
endif

ifdef ESPBAUD
IDF_ARGS += -b $(ESPBAUD)
endif

.PHONY: all build flash monitor deploy erase menuconfig size clean distclean

all: build

build:
	idf.py $(IDF_ARGS) build

flash:
	idf.py $(IDF_ARGS) flash

monitor:
	idf.py $(IDF_ARGS) monitor

deploy: build flash

erase:
	idf.py $(IDF_ARGS) erase-flash

menuconfig:
	idf.py $(IDF_ARGS) menuconfig

size:
	idf.py $(IDF_ARGS) size

clean:
	rm -rf $(BUILD)

distclean:
	rm -rf $(ROOT)/build
