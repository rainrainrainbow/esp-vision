#!/bin/bash
# Build script for ESP32_S3_CUSTOM board
# Used by GitHub Actions CI

set -e

echo "=== ESP32_S3_CUSTOM Build Script ==="
echo "Board: ESP32_S3_CUSTOM"
echo "ESP-IDF: release/v6.1"

# Configure git safe directory
git config --global --add safe.directory "*"

# Source ESP-IDF
. /opt/esp/idf/export.sh
export ESP_IDF_VERSION=6.1

echo "=== Step 0: Ensure submodules at right commit ==="

if [ -d "lib/micropython" ]; then
    echo "Fixing lib/micropython commit..."
    cd lib/micropython
    git fetch origin v1.28.0 --depth=100 2>&1 || true
    git checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>&1 || git checkout v1.28.0 2>&1 || true
    cd /project
    echo "MICROPYTHON: $(git -C lib/micropython rev-parse HEAD)"
else
    echo "WARNING: lib/micropython missing!"
    git submodule sync --recursive
    git submodule update --init --recursive --depth=100 2>&1 || true
    if [ -d "lib/micropython" ]; then
        cd lib/micropython
        git fetch origin v1.28.0 --depth=100 2>&1 || true
        git checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>&1 || git checkout v1.28.0 2>&1 || true
        cd /project
        echo "MICROPYTHON: $(git -C lib/micropython rev-parse HEAD)"
    fi
fi

echo "Running prepare-micropython..."
make BOARD=ESP32_S3_CUSTOM prepare-micropython 2>&1
MAKE_RETRY=$?
echo "prepare-micropython exit code: $MAKE_RETRY"

if [ $MAKE_RETRY -ne 0 ]; then
    echo "prepare failed, continuing anyway..."
fi

echo "=== Patching berkeley-db ==="
find build/micropython/ -path "*/berkeley-db/include/berkeley-db/*.h" 2>/dev/null | while read f; do
    if ! grep -q "#ifndef __P" "$f" 2>/dev/null; then
        printf '#ifndef __P\n#define __P(x) x\n#endif\n#ifndef __BEGIN_DECLS\n#define __BEGIN_DECLS\n#endif\n#ifndef __END_DECLS\n#define __END_DECLS\n#endif\n' > "$f.tmp"
        cat "$f" >> "$f.tmp"
        mv "$f.tmp" "$f"
        echo "Patched: $f"
    fi
done

echo "=== Building ==="
make BOARD=ESP32_S3_CUSTOM build 2>&1
BUILD_EXIT=$?
echo "Build exit code: $BUILD_EXIT"

if [ $BUILD_EXIT -eq 0 ]; then
    echo "BUILD SUCCESS!"
    ls -lah build/ESP32_S3_CUSTOM/idf6.1/ 2>/dev/null || true
    ls -lah build/micropython/idf6.1/micropython/build/ESP32_S3_CUSTOM/idf6.1/ 2>/dev/null || true
else
    echo "BUILD FAILED!"
fi