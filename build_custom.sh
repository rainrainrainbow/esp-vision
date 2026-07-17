#!/bin/bash
# Build script for ESP32_S3_CUSTOM board
set -e

echo "=== ESP32_S3_CUSTOM Build Script ==="
echo "Board: ESP32_S3_CUSTOM | ESP-IDF: release/v6.1"

git config --global --add safe.directory "*"
. /opt/esp/idf/export.sh
export ESP_IDF_VERSION=6.1

# Fix micropython submodule commit
echo "=== Step 0: Ensure submodules at right commit ==="
if [ -d "lib/micropython" ]; then
    cd lib/micropython
    git fetch origin v1.28.0 --depth=100 2>&1 || true
    git checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>&1 || git checkout v1.28.0 2>&1 || true
    cd /project
fi

echo "Running prepare-micropython..."
make BOARD=ESP32_S3_CUSTOM prepare-micropython 2>&1

echo "=== Patching berkeley-db headers ==="
find build/micropython/ -path "*/berkeley-db/include/berkeley-db/*.h" 2>/dev/null | while read f; do
    if ! grep -q "#ifndef __P" "$f" 2>/dev/null; then
        printf '#ifndef __P\n#define __P(x) x\n#endif\n#ifndef __BEGIN_DECLS\n#define __BEGIN_DECLS\n#endif\n#ifndef __END_DECLS\n#define __END_DECLS\n#endif\n' > "$f.tmp"
        cat "$f" >> "$f.tmp"
        mv "$f.tmp" "$f"
    fi
done

echo "=== Building ==="
make BOARD=ESP32_S3_CUSTOM build 2>&1
BUILD_EXIT=$?
echo "Build exit code: $BUILD_EXIT"

if [ $BUILD_EXIT -eq 0 ]; then
    echo "BUILD SUCCESS!"
else
    echo "BUILD FAILED!"
fi