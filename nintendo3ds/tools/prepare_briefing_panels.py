#!/usr/bin/env python3
"""Pack user-supplied, native-screen briefing layers without moving their artwork.

The bottom exports have an opaque white matte. Remove only near-white pixels
connected to the canvas edge, retaining enclosed panel artwork and dark interiors.
"""
from pathlib import Path
from collections import deque
from PIL import Image
import argparse, hashlib, json
from ui_bank import bank


def unmatte(image):
    image = image.convert('RGBA')
    w, h = image.size
    pixels = image.load()
    seen = set()
    queue = deque([(x, y) for x in range(w) for y in (0, h-1)] +
                  [(x, y) for y in range(h) for x in (0, w-1)])
    while queue:
        x, y = queue.popleft()
        if (x, y) in seen or not (0 <= x < w and 0 <= y < h):
            continue
        seen.add((x, y))
        r, g, b, a = pixels[x, y]
        if a and min(r, g, b) < 245:
            continue
        pixels[x, y] = (0, 0, 0, 0)
        queue.extend(((x-1, y), (x+1, y), (x, y-1), (x, y+1)))
    # Undo the remaining pale antialias against the white matte at the edge.
    # Leave enclosed highlights untouched; only the immediate silhouette is
    # unpremultiplied, preventing a white halo over the dark briefing room.
    edge = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            low = min(r, g, b)
            if a == 255 and 128 < low < 245 and any(
                    0 <= xx < w and 0 <= yy < h and pixels[xx, yy][3] == 0
                    for xx, yy in ((x-1, y), (x+1, y), (x, y-1), (x, y+1))):
                alpha = 255-low
                edge.append((x, y, ((r-low)*255//alpha, (g-low)*255//alpha,
                                   (b-low)*255//alpha, alpha)))
    for x, y, color in edge:
        pixels[x, y] = color
    return image


def prepare(source, stage, output):
    campaign = stage / 'campaign'
    path = campaign / 'presentation-manifest.json'
    manifest = json.loads(path.read_text())
    output.mkdir(parents=True, exist_ok=True)
    for race in ('terran', 'protoss', 'zerg'):
        items, layout = [], []
        for layer in ('left', 'right', 'top', 'cancel', 'text'):
            name = (f'{race}-top-screen-text.png' if race == 'protoss' else
                    f'{race}-briefing-text-top.png') if layer == 'text' else f'{race}-{layer}.png'
            im = Image.open(source / name).convert('RGBA')
            assert im.size == ((400, 240) if layer == 'text' else (320, 240)), name
            if layer != 'text':
                im = unmatte(im)
            im.save(output / name)
            bounds = im.getchannel('A').getbbox()
            assert bounds, name
            items.append((layer, im.crop(bounds)))
            layout.append(f'{layer} {bounds[0]} {bounds[1]}')
            if layer == 'text':
                # Native upper frame survives only core loading; released before terrain/audio.
                frame = campaign / f'briefing-frame-{race}.bin'
                bank(frame, [('frame', im)], 512, 256)
        packed = campaign / f'briefing-panels-{race}.bin'
        bank(packed, items, 512, 512)
        geometry = campaign / f'briefing-panels-{race}.layout'
        geometry.write_text('\n'.join(layout) + '\n')
        for resource in (packed, geometry, campaign / f'briefing-frame-{race}.bin'):
            data = resource.read_bytes()
            manifest[resource.name] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
        print(race, packed.stat().st_size, '; '.join(layout))
    path.write_text(json.dumps(manifest, indent=2) + '\n')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('source', 'stage', 'output'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    prepare(a.source, a.stage, a.output)
