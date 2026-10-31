"""Verify geometric crop boundaries and actual twiddled story-bank pixels."""
from pathlib import Path
from PIL import Image
import struct, sys
sys.path.insert(0, str(Path(__file__).parents[1] / 'tools'))
from prepare_campaign_screens import split_screens

canvas = Image.new('RGBA', (800, 960))
canvas.paste((16, 32, 48, 255), (0, 0, 800, 480))
canvas.paste((64, 80, 96, 255), (0, 480, 800, 960))
canvas.paste((200, 8, 8, 255), (0, 480, 80, 960))
canvas.paste((8, 200, 8, 255), (720, 480, 800, 960))
top, bottom = split_screens(canvas)
assert top.size == (400, 240) and bottom.size == (320, 240)
assert top.getpixel((200, 0)) == (16, 32, 48, 255)
assert bottom.getpixel((160, 239)) == (64, 80, 96, 255)
assert top.tobytes() == canvas.resize((400, 480), Image.Resampling.LANCZOS).crop((0, 0, 400, 240)).tobytes()
assert bottom.tobytes() == canvas.resize((400, 480), Image.Resampling.LANCZOS).crop((40, 240, 360, 480)).tobytes()

if len(sys.argv) == 3:
    import json
    screens, campaign = map(Path, sys.argv[1:])
    manifest = json.loads((screens / 'screens-manifest.json').read_text())
    assert len(manifest['images']) == 18 and len(manifest['missions']) > 0
    for key, source in manifest['missions'].items():
        data = (campaign / f'{key}-story.bin').read_bytes()
        assert data[:8] == b'OB3UI001'
        width, height, pages, count = struct.unpack_from('<4I', data, 8)
        assert (width, height, pages, count) == (512, 256, 2, 2)
        start = 24 + count * 60
        assert len(data) == start + width * height * pages * 2
        for i, screen in enumerate(('top', 'bottom')):
            name, page, x, y, w, h = struct.unpack_from('<40s5I', data, 24 + i * 60)
            assert name.rstrip(b'\0').decode() == screen
            image = Image.open(screens / screen / source).convert('RGBA')
            assert image.size == (w, h) and image.getchannel('A').getextrema() == (255, 255)
            for yy in range(h):
                for xx in range(w):
                    r, g, b, _ = image.getpixel((xx, yy))
                    expected = (r >> 3) << 11 | (g >> 3) << 6 | (b >> 3) << 1 | 1
                    px, py = x + xx, y + yy
                    morton = (px & 1) | ((py & 1) << 1) | ((px & 2) << 1) | ((py & 2) << 2) | ((px & 4) << 2) | ((py & 4) << 3)
                    offset = start + page * width * height * 2 + (((py // 8) * (width // 8) + px // 8) * 64 + morton) * 2
                    assert struct.unpack_from('<H', data, offset)[0] == expected
    print(f"18 PNG pairs and {len(manifest['missions'])} two-page GPU banks: all pixels/crops/sizes/opacity PASS")
else:
    print('Exact screen crop geometry PASS')
