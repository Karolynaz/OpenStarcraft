#!/usr/bin/env python3
"""Prepare race briefing ceilings at native resolution and bounded loading size.

GENERATED contains the completed800x960 canvases named RACE-fonas.png.
Only the upper400x240 crop is used; the original bottom room and UI stay intact.
"""
from pathlib import Path
from PIL import Image
import argparse, hashlib, json
from ui_bank import bank


def prepare(generated, output, stage):
    campaign = stage / 'campaign'
    manifest_path = campaign / 'presentation-manifest.json'
    manifest = json.loads(manifest_path.read_text())
    output.mkdir(parents=True, exist_ok=True)
    rows = []
    for race in ('terran', 'protoss', 'zerg'):
        image = Image.open(generated / f'{race}-fonas.png').convert('RGBA')
        if image.size != (800, 960) or image.getchannel('A').getextrema() != (255, 255):
            raise ValueError(f'Expected opaque800x960 canvas for {race}')
        top = image.resize((400, 480), Image.Resampling.LANCZOS).crop((0, 0, 400, 240))
        top.save(output / f'{race}-fonas.png')
        native = campaign / f'briefing-upper-{race}.bin'
        bank(native, [('top', top)])
        # The full256KiB native page is freed before simulation/core loading.
        # This64KiB thumbnail replaces the previous64KiB planet texture, so
        # the gameplay warming frame does not need any additional linear RAM.
        loading = campaign / f'briefing-loading-{race}.bin'
        bottom = image.crop((80, 480, 720, 960))
        bank(loading, [('shot', top.resize((128, 128), Image.Resampling.LANCZOS)),
                       ('bottom', bottom.resize((128, 128), Image.Resampling.LANCZOS))],
             256, 128, positions=[(0, 0), (128, 0)])
        for path in (native, loading):
            manifest[path.name] = {'bytes': path.stat().st_size, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
        rows.append({'race': race, 'native': native.name, 'loading': loading.name})
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    (output / 'briefing-screens-manifest.json').write_text(json.dumps({
        'top_size': [400, 240], 'crop_after_resize_400x480': [0, 0, 400, 240],
        'bottom_room_unchanged': True, 'races': rows,
    }, indent=2) + '\n')
    print('3 native400x240 briefing ceilings and 3 bounded loading banks prepared')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('generated', 'output', 'stage'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    prepare(a.generated, a.output, a.stage)
