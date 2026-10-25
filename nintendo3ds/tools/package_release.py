#!/usr/bin/env python3
"""Package a tested private SD tree, dereferencing owned data-file symlinks."""
import argparse
import hashlib
import json
import struct
from pathlib import Path
import zipfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('source', type=Path)
p.add_argument('output', type=Path)
a = p.parse_args()
required = ['openbw-3ds.3dsx', 'StarDat.mpq', 'BrooDat.mpq', 'Patch_rt.mpq',
            'graphics.bin', 'graphics-arena.bin', 'maps/probe.scx', 'maps/arena.chk',
            'README.txt', 'campaign/terran01.chk', 'campaign/terran01.brief', 'maps/AI/islands.scm', 'maps/AI/three-islands.scm', 'observer-maps.json', 'ui/menu.bin', 'ui/hud.bin', 'ui/manifest.json', 'ui/animations.bin', 'audio/effects-all.bin', 'audio/effects-all.json']
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
# Include the complete bounded private map catalogue, not just the test map.
required += [p.relative_to(a.source).as_posix() for p in sorted((a.source/'maps').rglob('*'))
             if p.is_file() and p.suffix.lower() in ('.scm','.scx','.chk')
             and p.relative_to(a.source).as_posix() not in required]
if (a.source/'map-catalogue.json').is_file():
    catalogue=json.loads((a.source/'map-catalogue.json').read_text())
    for name, expected in catalogue['maps'].items():
        if hashlib.sha256((a.source/name).read_bytes()).hexdigest()!=expected['sha256']:
            p.error(f'Changed owned map: {name}')
    required.append('map-catalogue.json')
observer_manifest=json.loads((a.source/'observer-maps.json').read_text())
for name, expected in observer_manifest['maps'].items():
    if hashlib.sha256((a.source/name).read_bytes()).hexdigest()!=expected['sha256']:
        p.error(f'Changed private observer map: {name}')
audio_manifest=json.loads((a.source/'audio/effects-all.json').read_text())
if hashlib.sha256((a.source/'audio/effects-all.bin').read_bytes()).hexdigest()!=audio_manifest['sha256']:
    p.error('Changed combined private audio bank')
bank=(a.source/'audio/effects-all.bin').read_bytes()
count=struct.unpack_from('<I',bank,8)[0]
sound_ids={struct.unpack_from('<I',bank,20+i*20)[0] for i in range(count)}
if not {423,427,1357,1532,1604}.issubset(sound_ids):
    p.error('Combined bank lacks Raynor responses or Wasteland speech')
ui_manifest=json.loads((a.source/'ui/manifest.json').read_text())
for name in ('menu','hud','animations'):
    if hashlib.sha256((a.source/f'ui/{name}.bin').read_bytes()).hexdigest()!=ui_manifest[name]['sha256']:
        p.error(f'Changed UI asset: {name}')
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
