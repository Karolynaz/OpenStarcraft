#!/usr/bin/env python3
"""Package a tested private SD tree, dereferencing owned data-file symlinks."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('source', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
required = ['openbw-3ds.3dsx', 'StarDat.mpq', 'BrooDat.mpq', 'Patch_rt.mpq',
            'graphics.bin', 'graphics-arena.bin', 'maps/probe.scx', 'maps/arena.chk',
            'README.txt']
for race in ('terran', 'protoss', 'zerg'):
    folder = 'audio' if race == 'terran' else f'audio/{race}'
    required += [f'{folder}/effects.bin', f'{folder}/manifest.json']
    required += [f'{folder}/{race}{n}.pcm' for n in range(1, 4)]
for name in required:
    path = a.source / name
    if not path.is_file() or not path.stat().st_size:
        p.error(f'Missing/empty required file: {path}')
# Verify owned inputs against the original staging manifest before packaging.
manifest_path = a.source / 'assets-manifest.json'
if manifest_path.is_file():
    for name, expected in json.loads(manifest_path.read_text()).items():
        with (a.source / name).open('rb') as f:
            if hashlib.file_digest(f, 'sha256').hexdigest() != expected['sha256']:
                p.error(f'Changed owned input: {name}')
    required.append('assets-manifest.json')
a.output.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(a.output, 'w', compression=zipfile.ZIP_STORED) as package:
    for name in required:
        package.write(a.source / name, f'3ds/openbw/{name}')
with zipfile.ZipFile(a.output) as package:
    bad = package.testzip()
    if bad:
        raise RuntimeError(f'Corrupt package entry: {bad}')
    binary = package.read('3ds/openbw/openbw-3ds.3dsx')
    if hashlib.sha256(binary).digest() != hashlib.sha256((a.source / 'openbw-3ds.3dsx').read_bytes()).digest():
        raise RuntimeError('Packaged executable mismatch')
with a.output.open('rb') as f:
    digest = hashlib.file_digest(f, 'sha256').hexdigest()
print(json.dumps({'package': str(a.output), 'files': len(required),
                  'bytes': a.output.stat().st_size, 'sha256': digest,
                  'sd_updated': False}, indent=2))
