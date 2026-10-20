#!/usr/bin/env python3
"""Fingerprint native source inputs; never hash purchased game assets or paths."""
from pathlib import Path
import hashlib,sys
root=Path(__file__).resolve().parents[1]
files=sorted([*root.parent.glob('*.h'),*root.glob('include/*.h'),*root.glob('source/*.cpp'),root/'Makefile',root/'arm11.specs',Path(__file__).resolve()])
h=hashlib.sha256()
for path in files:
    name=path.relative_to(root.parent).as_posix().encode();data=path.read_bytes()
    h.update(len(name).to_bytes(4,'little'));h.update(name);h.update(len(data).to_bytes(8,'little'));h.update(data)
value='src-'+h.hexdigest()[:16]
output=Path(sys.argv[1]);text='#pragma once\nnamespace port3ds { constexpr const char* source_id="'+value+'"; }\n'
output.parent.mkdir(parents=True,exist_ok=True)
if not output.exists() or output.read_text()!=text:output.write_text(text)
print(value)
