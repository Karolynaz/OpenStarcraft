#!/usr/bin/env python3
"""Prepare six private result screen pairs from outpaints and original artwork.

The top is the upper half of the 800x960 outpaint, scaled to 400x240.
The bottom is the untouched original composition scaled from 640x480 to 320x240.
"""
from pathlib import Path
from PIL import Image
import argparse, hashlib, json
from ui_bank import bank


def prepare(originals, generated, output, stage):
    campaign = stage / 'campaign'
    manifest_path = campaign / 'presentation-manifest.json'
    manifest = json.loads(manifest_path.read_text())
    rows = []
    for race in ('terran', 'protoss', 'zerg'):
        for outcome, suffix in [('victory', 'laimejus'), ('defeat', 'pralaimejus')]:
            name = f'{race}-{suffix}.png'
            original = Image.open(originals / name).convert('RGBA')
            extension = Image.open(generated / name).convert('RGBA')
            if original.size == (800, 960):
                original = original.crop((80, 480, 720, 960))
            if original.size != (640, 480) or extension.size != (800, 960):
                raise ValueError(f'Unexpected source sizes for {name}')
            if any(im.getchannel('A').getextrema() != (255, 255) for im in (original, extension)):
                raise ValueError(f'Transparent result art: {name}')
            top = extension.resize((400, 480), Image.Resampling.LANCZOS).crop((0, 0, 400, 240))
            bottom = original.resize((320, 240), Image.Resampling.LANCZOS)
            for screen, image in [('top', top), ('bottom', bottom)]:
                folder = output / screen
                folder.mkdir(parents=True, exist_ok=True)
                image.save(folder / name)
            path = campaign / f'result-{race}-{outcome}.bin'
            bank(path, [('top', top), ('bottom', bottom)], positions=[(0, 0, 0), (1, 0, 0)])
            manifest[path.name] = {'bytes': path.stat().st_size, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
            rows.append({'source': name, 'bank': path.name})
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    (output / 'result-screens-manifest.json').write_text(json.dumps({
        'top_size': [400, 240], 'bottom_size': [320, 240],
        'bottom_from_original': True, 'images': rows,
    }, indent=2) + '\n')
    print('6 result pairs / 12 PNGs / 6 native two-page GPU banks prepared')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('originals', 'generated', 'output', 'stage'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    prepare(a.originals, a.generated, a.output, a.stage)
