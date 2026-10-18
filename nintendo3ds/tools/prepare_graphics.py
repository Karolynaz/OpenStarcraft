#!/usr/bin/env python3
"""Pack privately exported StarCraft graphics into bounded, pre-swizzled 3DS atlases."""
import argparse
from array import array
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib

SIDE = 512
MAX_PAGES = 8

def signature(width, height, tileset, tiles):
    h = 2166136261
    for value in (width, height, tileset, *tiles):
        for byte in (value & 255, value >> 8):
            h = ((h ^ byte) * 16777619) & 0xffffffff
    return h

def morton(x, y):
    return sum(((x >> i) & 1) << (i * 2) | ((y >> i) & 1) << (i * 2 + 1) for i in range(3))

def swizzle(pixels):
    # PICA200 8x8 Morton blocks. Store logical rows top-to-bottom, as tex3ds
    # does; C2D subtexture V coordinates already account for its convention.
    out = array('H', [0]) * (SIDE * SIDE)
    for y in range(SIDE):
        for x in range(SIDE):
            ty = y
            off = ((ty // 8) * (SIDE // 8) + x // 8) * 64 + morton(x & 7, ty & 7)
            out[off] = pixels[y * SIDE + x]
    if sys.byteorder != 'little': out.byteswap()
    return out.tobytes()

def png(path, pixels):
    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data))
    raw = bytearray()
    for y in range(SIDE):
        raw.append(0)
        for c in pixels[y * SIDE:(y + 1) * SIDE]:
            raw.extend((((c >> 11) & 31) * 255 // 31, ((c >> 6) & 31) * 255 // 31,
                        ((c >> 1) & 31) * 255 // 31, 255 if c & 1 else 0))
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>2I5B', SIDE, SIDE, 8, 6, 0, 0, 0)) +
                     chunk(b'IDAT', zlib.compress(bytes(raw))) + chunk(b'IEND', b''))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('raw', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--preview', type=Path)
    parser.add_argument('--static-sprites', action='store_true', help='Legacy diagnostic export only')
    args = parser.parse_args()
    data = args.raw.read_bytes(); at = 0
    def take(n):
        nonlocal at
        if at + n > len(data): raise ValueError('Truncated raw graphics')
        value = data[at:at + n]; at += n; return value
    def u16(): return struct.unpack('<H', take(2))[0]
    def u32(): return struct.unpack('<I', take(4))[0]
    if take(8) != b'OBWRAW1\0': raise ValueError('Wrong raw format')
    width, height, tileset = u32(), u32(), u32()
    if not 32 <= width <= 64 or not 32 <= height <= 64: raise ValueError('Map outside probe limits')
    palette = take(1024); teams = take(128)
    colors = [((palette[i * 4] >> 3) << 11) | ((palette[i * 4 + 1] >> 3) << 6) |
              ((palette[i * 4 + 2] >> 3) << 1) | 1 for i in range(256)]
    count = u32()
    if count != width * height: raise ValueError('Tile count mismatch')
    tilemap = [u16() for _ in range(count)]
    records = []; dedup = {}
    def add(w, h, pixels):
        key = (w, h, hashlib.sha256(pixels.tobytes()).digest())
        if key not in dedup:
            dedup[key] = len(records)
            records.append({'w': w, 'h': h, 'pixels': pixels})
        return dedup[key]
    tiles = {}; tile_ids = []
    for _ in range(u32()):
        tile_id = u16(); source = take(1024)
        tiles[tile_id] = source; tile_ids.append(tile_id)
    tile_refs = [add(32, 32, array('H', (colors[c] for c in tiles[t]))) for t in tile_ids]
    tile_lookup = {tile: i for i, tile in enumerate(tile_ids)}
    sprites = []
    groups = u32()
    for _ in range(groups):
        image, frames = u16(), u16()
        if image >= 999 or frames >= 16384: raise ValueError('Sprite ID outside format')
        for frame in range(frames):
            w, h = u16(), u16()
            indices = struct.unpack('<' + str(w * h) + 'H', take(w * h * 2))
            if args.static_sprites:
                for team in range(3):
                    remap = teams[(team if team < 2 else 11) * 8:][:8]
                    pixels = array('H', (0 if c == 65535 else colors[remap[c - 8] if 8 <= c < 16 else c] for c in indices))
                    sprites.append(((image << 16) | (frame << 2) | team, add(w, h, pixels)))
    if at != len(data): raise ValueError('Unexpected trailing raw data')
    # A real terrain minimap, prepared once here and uploaded with the atlases.
    mini = array('H')
    for y in range(128):
        wy = y * height * 32 // 128
        for x in range(128):
            wx = x * width * 32 // 128
            src = tiles[tilemap[(wy // 32) * width + wx // 32]]
            mini.append(colors[src[(wy % 32) * 32 + wx % 32]])
    mini_ref = add(128, 128, mini)
    # MaxRects best-short-side fit recovers the space below shorter sprites.
    # This keeps construction/combat in the same original-3DS 4 MiB budget.
    # One-pixel gutters and nearest filtering isolate adjacent frames.
    pages = []; free_pages = []
    for r in sorted(records, key=lambda r: (-r['h'], -r['w'])):
        w, h = max(1, r['w']), max(1, r['h'])
        if w + 2 > SIDE or h + 2 > SIDE: raise ValueError('Frame larger than atlas')
        rw, rh = w + 2, h + 2
        choices = []
        for p, free in enumerate(free_pages):
            for fx, fy, fw, fh in free:
                if rw <= fw and rh <= fh:
                    choices.append((min(fw-rw, fh-rh), max(fw-rw, fh-rh), fy, fx, p))
        if choices:
            _, _, py, px, page = min(choices)
        else:
            if len(pages) >= MAX_PAGES: raise ValueError('Graphics exceeds original-3DS 4 MiB atlas budget')
            page = len(pages); px = py = 0
            pages.append(array('H', [0]) * (SIDE * SIDE))
            free_pages.append([(0, 0, SIDE, SIDE)])
        split = []
        for fx, fy, fw, fh in free_pages[page]:
            if px >= fx+fw or px+rw <= fx or py >= fy+fh or py+rh <= fy:
                split.append((fx, fy, fw, fh)); continue
            if px > fx: split.append((fx, fy, px-fx, fh))
            if px+rw < fx+fw: split.append((px+rw, fy, fx+fw-px-rw, fh))
            if py > fy: split.append((fx, fy, fw, py-fy))
            if py+rh < fy+fh: split.append((fx, py+rh, fw, fy+fh-py-rh))
        split = list(dict.fromkeys(split))
        free_pages[page] = [a for i, a in enumerate(split) if not any(
            i != j and a[0] >= b[0] and a[1] >= b[1] and
            a[0]+a[2] <= b[0]+b[2] and a[1]+a[3] <= b[1]+b[3]
            for j, b in enumerate(split))]
        x, y = px + 1, py + 1
        r['rect'] = (page, x, y, r['w'], r['h'])
        for line in range(r['h']):
            pages[page][(y + line) * SIDE + x:(y + line) * SIDE + x + r['w']] = r['pixels'][line * r['w']:(line + 1) * r['w']]
    rect = lambda ref: struct.pack('<5H', *records[ref]['rect'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('wb') as out:
        out.write(struct.pack('<8s7I', b'OB3DGFX1' if args.static_sprites else b'OB3DGFX2', width, height, signature(width, height, tileset, tilemap),
                              len(pages), len(tile_refs), len(sprites), len(tilemap)))
        out.write(rect(mini_ref))
        for ref in tile_refs: out.write(rect(ref))
        out.write(struct.pack('<' + str(len(tilemap)) + 'H', *(tile_lookup[t] for t in tilemap)))
        for key, ref in sorted(sprites): out.write(struct.pack('<I', key) + rect(ref))
        for i, pixels in enumerate(pages):
            out.write(swizzle(pixels))
            if args.preview:
                args.preview.mkdir(parents=True, exist_ok=True)
                png(args.preview / f'atlas-{i}.png', pixels)
    stats = {'map_tiles': [width, height], 'image_types': groups, 'sprite_records': len(sprites), 'sprite_mode': 'static' if args.static_sprites else 'resident-GRP GPU cache',
             'unique_rects': len(records), 'pages': len(pages), 'texture_bytes': len(pages) * SIDE * SIDE * 2,
             'file_bytes': args.output.stat().st_size}
    args.output.with_suffix('.json').write_text(json.dumps(stats, indent=2) + '\n')
    print(json.dumps(stats))

if __name__ == '__main__': main()
