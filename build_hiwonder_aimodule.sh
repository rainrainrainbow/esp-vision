#!/bin/bash
# Build script for HIWONDER_AI_MODULE
# Fix: camera capture loop bounds, audio I2S MCLK config

set -e
git config --global --add safe.directory "*"
. /opt/esp/idf/export.sh

make BOARD=HIWONDER_AI_MODULE prepare-micropython

# Patch berkeley-db headers
find build/micropython/ -path "*/berkeley-db/include/berkeley-db/*.h" -exec grep -qL "#ifndef __P" {} \; | while read f; do
  printf '#ifndef __P\n#define __P(x) x\n#endif\n#ifndef __BEGIN_DECLS\n#define __BEGIN_DECLS\n#endif\n#ifndef __END_DECLS\n#define __END_DECLS\n#endif\n' > "$f.tmp"
  cat "$f" >> "$f.tmp"
  mv "$f.tmp" "$f"
  echo "Patched: $f"
done

echo "Build starting..."
make BOARD=HIWONDER_AI_MODULE build
echo "Build exit code: $?"
