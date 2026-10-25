#!/usr/bin/env python3
"""Build one private, lossless multi-race effect bank from owned prepared PCM."""
from pathlib import Path
import argparse, hashlib, json, struct
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('audio_root', type=Path)
a = p.parse_args()
effects = {}
folders=[('terran',a.audio_root),('protoss',a.audio_root/'protoss'),('zerg',a.audio_root/'zerg')]
if (a.audio_root/'campaign/effects.bin').is_file():folders.append(('campaign',a.audio_root/'campaign'))
for race, folder in folders:
    raw = (folder/'effects.bin').read_bytes()
    magic, count, size, width = struct.unpack_from('<8sIII', raw)
    if magic != b'OBWAUD2\0' or not 0 < count <= 1200 or width != 1 or len(raw) != 20+count*20+size:
        p.error(f'Invalid owned prepared bank: {race}')
    start = 20+count*20
    for i in range(count):
        ident, offset, samples, priority, flags = struct.unpack_from('<IIIII', raw, 20+i*20)
        if ident >= 2300 or not 0 < samples <= 256*1024 or offset+samples > size:
            p.error(f'Invalid sample bounds: {race}/{ident}')
        entry = (samples, priority, flags, raw[start+offset:start+offset+samples])
        if ident in effects and effects[ident] != entry:
            p.error(f'Conflicting owned sound ID: {ident}')
        effects[ident] = entry
payload = bytearray()
records = bytearray()
for ident, (samples, priority, flags, pcm) in sorted(effects.items()):
    records += struct.pack('<IIIII', ident, len(payload), samples, priority, flags)
    payload += pcm
if len(payload) > 24*1024*1024:
    p.error('Combined effect bank exceeds the bounded native file limit')
result = struct.pack('<8sIII', b'OBWAUD2\0', len(effects), len(payload), 1)+records+payload
output = a.audio_root/'effects-all.bin'
output.write_bytes(result)
manifest = {'effects': len(effects), 'pcm_bytes': len(payload), 'bytes': len(result),
            'sha256': hashlib.sha256(result).hexdigest(), 'format': 'OBWAUD2 / mono PCM8 22050Hz',
            'provenance': 'Lossless merge of the privately owned race and optional campaign banks; identical IDs deduplicated.'}
(a.audio_root/'effects-all.json').write_text(json.dumps(manifest, indent=2)+'\n')
print(json.dumps(manifest, indent=2))
