#!/bin/bash
set -e
git config --global --add safe.directory "*"
. /opt/esp/idf/export.sh
export ESP_IDF_VERSION=6.0
make BOARD=HIWONDER_S3 prepare-micropython
python3 boards/HIWONDER_S3/patch_bdb.py
make BOARD=HIWONDER_S3 build
