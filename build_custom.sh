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

# Ensure submodules are initialized
if [ -d "lib/micropython" ]; then
    echo "=== Submodule status ==="
    git submodule status --recursive
    
    # Check if micropython is at the correct commit
    MP_COMMIT=$(git -C lib/micropython rev-parse HEAD 2>/dev/null || echo "unknown")
    echo "lib/micropython HEAD: $MP_COMMIT"
    echo "Expected: e0e9fbb17ed6fd06bb76e266ae554784c9c80804"
    
    if [ "$MP_COMMIT" != "e0e9fbb17ed6fd06bb76e266ae554784c9c80804" ]; then
        echo "=== Fixing micropython submodule commit ==="
        git -C lib/micropython fetch --depth=50 origin v1.28.0 2>/dev/null || true
        git -C lib/micropython checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>/dev/null || \
        git -C lib/micropython checkout v1.28.0 2>/dev/null || true
        
        # If still not correct, try a different approach
        MP_COMMIT2=$(git -C lib/micropython rev-parse HEAD 2>/dev/null || echo "unknown")
        echo "lib/micropython HEAD after fix: $MP_COMMIT2"
    fi
fi

# Run prepare-micropython (this will verify the commit)
echo "=== Step 1: prepare-micropython ==="
make BOARD=ESP32_S3_CUSTOM prepare-micropython 2>&1 || {
    echo "prepare-micropython failed, attempting workaround..."
    # Workaround: init submodules manually then retry
    git submodule sync --recursive
    git submodule update --init --recursive --depth=50 2>/dev/null || true
    
    # Force checkout the required commit
    if [ -d "lib/micropython" ]; then
        cd lib/micropython
        git fetch --depth=100 origin v1.28.0 2>/dev/null || true
        git checkout v1.28.0 2>/dev/null || true
        cd /project
    fi
    
    make BOARD=ESP32_S3_CUSTOM prepare-micropython 2>&1
}

# Patch berkeley-db headers in the build copy
echo "=== Step 2: Patch berkeley-db headers ==="
find build/micropython/ -path "*/berkeley-db/include/berkeley-db/*.h" 2>/dev/null | while read f; do
    if ! grep -q "#ifndef __P" "$f" 2>/dev/null; then
        printf '#ifndef __P\n#define __P(x) x\n#endif\n#ifndef __BEGIN_DECLS\n#define __BEGIN_DECLS\n#endif\n#ifndef __END_DECLS\n#define __END_DECLS\n#endif\n' > "$f.tmp"
        cat "$f" >> "$f.tmp"
        mv "$f.tmp" "$f"
        echo "Patched: $f"
    fi
done

# Build
echo "=== Step 3: Build ==="
make BOARD=ESP32_S3_CUSTOM build 2>&1

echo "=== Build complete ==="
echo "Build exit code: $?"
ls -lah build/ESP32_S3_CUSTOM/idf6.1/ 2>/dev/null || ls -lah build/ 2>/dev/null
