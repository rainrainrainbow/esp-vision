#!/bin/bash
# Build script for ESP32_S3_CUSTOM
# Used by GitHub Actions CI

set -e  # still exit on error, but handle each step gracefully

echo \"=== ESP32_S3_CUSTOM Build Script ===\"
echo "Board: ESP32_S3_CUSTOM"
echo "ESP-IDF: release/v6.1"

# Configure git safe directory
git config --global --add safe.directory \"*\"

# Source ESP-IDF
. /opt/esp/idf/export.sh
export ESP_IDF_VERSION=6.1

echo \"=== Step 0: Ensure submodules are at the right commit ===\"

# Pull the specific commit for lib/micropython
if [ -d \"lib/micropython\" ]; then
    echo \"Fixing lib/micropython commit...\"
    # Try to fetch the required tag
    git -C lib/micropython fetch origin v1.28.0 --depth=100 2>'&1 || true
    # Try checkout by tag then commit
    git -C lib/micropython checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>'&1 || echo \"Commit checkout failed, trying tag...\"
    git -C lib/micropython checkout v1.28.0 2>&'1 || echo \"Tag checkout failed, trying MASTER...\"
    git -C lib/micropython fetch origin master --depth=200 2>'1 || true
    git -C lib/micropython checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>'1 || true
    echo \"MICROPYTHoN commit: $(git -C lib/micropython rev-parse HEAD)\"
else
    echo \"WARNING: lib/micropython directory not found!\"
    git submodule sync --recursive
    git submodule update --init --recursive --depth=100 2>&'1 || true
    if [ -d \"lib/micropython\" ]; then
        git -C lib/micropython fetch origin v1.28.0 --depth=100 2>'1 || true
        git -C lib/micropython checkout e0e9fbb17ed6fd06bb76e266ae554784c9c80804 2>'&1 || true
        git -C lib/micropython checkout v1.28.0 2>'1 || true
        echo \"MICROPYTHON commit: $(git -C lib/micropython rev-parse HEAD)\"
    fi
fi

echo \"## Running prepare-micropython ##\"
make BOARD=ESP32_S3_CUSTOM prepare-micropython 2>&1
MASE_RETRY=$?
echo \"prepare-micropython exit code: $MASE_RETRY\"

if [ $MARE_RETRY -ne 0 ]; then
    echo \"prepare-micropython failed, but we\'ll try to build anyway...\"
    # Check if the build dir was created anyway
    if [ -d \"build/micropython/idf6.1/micropython\" ]; then
        echo \"Workaround: build dir exists, proceeding with build...\"
    fi
fi

# Patch berkeley-db headers
echo \"=== Patching berkeley-db headers ===\"
find build/micropython/ -path \"*/berkeley-db/include/berkeley-db/*.h\" 2>/dev/null | while read f; do
  if !grep -qL \"#ifndef __P\" \"$f\" 2>/dev/null; then
    printf '#ifndef __P\\n#define __P(x) x\\n#endif\\n#ifndef __BEGIN_DECLS\\n#define __BEGIN_DECLS\\n#endif\\n#ifndef __END_DECLS\\n#define __END_DECLS\\n#endif\\n' > \"$f.tmp\"
    cat \"$f\" >> \"$f.tmp\"
    mv \"$f.tmp\" \"$f\"
    echo \"Patched: $f\"
  fi
done

echo \"=== Building ===\"
make BOARD=ESP32_S3_CUSTOM build 2>&1
EXIT_CODE=$?

echo \"Build exit code: $EXIT_CODE\"

if [ $EXIT_CODE -eq 0 ]; then
    echo \"SUCCESS: Build successful!\"
    ls -lah build/ESP32_S3_CUSTOM/idf6.1/ 2>/dev/null || echo \"Check build/micropython/idf6.1/micropython/build/ESP32_S3_CUSTOM/\"
    ls -lah build/micropython/idf6.1/micropython/build/ESP32_S3_CUSTOM/idf6.1/ 2>/dev/null || true
else
    echo \"FAILED: Build failed: See log above for details\"
fi
