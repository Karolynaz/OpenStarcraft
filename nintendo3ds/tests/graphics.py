"""Compare packed GPU texels against OpenBW's host-exported source pixels."""
from pathlib import Path
import struct
import sys

raw = memoryview(Path(sys.argv[1]).read_bytes())
packed = memoryview(Path(sys.argv[2]).read_bytes())
pos = 0

def read(fmt):
    global pos
    result = struct.unpack_from(fmt, packed, pos)
    pos += struct.calcsize(fmt)
    return result

magic, width, height, signature, pages, tile_count, sprite_count, cells = read('<8s7I')
assert magic == b'OB3DGFX1' and pages <= 8 and cells == width * height
mini = read('<5H')
tiles = [read('<5H') for _ in range(tile_count)]
map_indices = read('<' + str(cells) + 'H')
sprites = {}
for _ in range(sprite_count):
    key, = read('<I')
    assert key not in sprites
    sprites[key] = read('<5H')
texture_start = pos
assert len(packed) == texture_start + pages * 512 * 512 * 2
for page, x, y, w, h in [mini, *tiles, *sprites.values()]:
    assert page < pages and x + w <= 512 and y + h <= 512

def pixel(rect, x, y):
    page, rx, ry, _, _ = rect
    x += rx
    y += ry
    # Explicit coordinate bit interleave from the PICA200 address contract.
    low = (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)
    offset = ((y >> 3) * 64 + (x >> 3)) * 64 + low
    return struct.unpack_from('<H', packed, texture_start + page * 524288 + offset * 2)[0]

at = 8
rw, rh, era = struct.unpack_from('<3I', raw, at); at += 12
assert (rw, rh) == (width, height)
palette = raw[at:at + 1024]; at += 1024
teams = raw[at:at + 128]; at += 128
colors = [((palette[i * 4] >> 3) << 11) | ((palette[i * 4 + 1] >> 3) << 6) | ((palette[i * 4 + 2] >> 3) << 1) | 1 for i in range(256)]
n, = struct.unpack_from('<I', raw, at); at += 4
source_map = struct.unpack_from('<' + str(n) + 'H', raw, at); at += n * 2
h = 2166136261
for value in (width, height, era, *source_map):
    for byte in (value & 255, value >> 8): h = ((h ^ byte) * 16777619) & 0xffffffff
assert signature == h
n, = struct.unpack_from('<I', raw, at); at += 4
ids = []
for i in range(n):
    tile, = struct.unpack_from('<H', raw, at); at += 2
    ids.append(tile)
    for y in range(32):
        for x in range(32): assert pixel(tiles[i], x, y) == colors[raw[at + y * 32 + x]]
    at += 1024
assert all(source_map[i] == ids[index] for i, index in enumerate(map_indices))
image_count, = struct.unpack_from('<I', raw, at); at += 4
for _ in range(image_count):
    image, frames = struct.unpack_from('<2H', raw, at); at += 4
    for frame in range(frames):
        w, h = struct.unpack_from('<2H', raw, at); at += 4
        for team in range(3):
            rect = sprites[(image << 16) | (frame << 2) | team]
            assert rect[3:] == (w, h)
            for y in range(h):
                for x in range(w):
                    c, = struct.unpack_from('<H', raw, at + (y * w + x) * 2)
                    if c == 65535: expected = 0
                    else:
                        if 8 <= c < 16: c = teams[(team if team < 2 else 11) * 8 + c - 8]
                        expected = colors[c]
                    assert pixel(rect, x, y) == expected
        at += w * h * 2
assert at == len(raw)
print(f'All terrain/sprite/team/alpha texels and GPU layout verified: {pages} pages PASS')
