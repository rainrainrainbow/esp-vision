#!/usr/bin/env python3
"""Patch berkeley-db headers for GCC 15 compatibility."""
import glob, os
patch = "#ifndef __P\n#define __P(x) x\n#endif\n#ifndef __BEGIN_DECLS\n#define __BEGIN_DECLS\n#endif\n#ifndef __END_DECLS\n#define __END_DECLS\n#endif\n"
count = 0
for f in glob.glob("build/micropython/**/berkeley-db/include/berkeley-db/*.h", recursive=True):
    with open(f) as fh:
        content = fh.read()
    if "#ifndef __P" not in content:
        with open(f, "w") as fh:
            fh.write(patch + content)
        count += 1
        print(f"Patched {f}")
print(f"Total patched: {count} files")
