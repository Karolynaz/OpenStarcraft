#!/usr/bin/env python3
"""Split completed scene PNGs and optionally prepare private campaign story art.

INPUT contains 800x960 completed PNGs. OUTPUT/top contains the top 400x240
crop after scaling to 400px wide; OUTPUT/bottom contains the lower half,
center-cropped to the touch screen's 320x240. Originals are never overwritten.
Use --raw RAW --stage STAGED_ROOT to map the owned location cards to missions.
"""
from pathlib import Path
from PIL import Image
import argparse, hashlib, json, re
from ui_bank import bank


def split_screens(image):
    if image.size != (800, 960):
        raise ValueError(f'Expected 800x960 completed canvas, got {image.size}')
    image = image.convert('RGBA')
    if image.getchannel('A').getextrema() != (255, 255):
        raise ValueError('Completed canvas must be fully opaque')
    scaled = image.resize((400, 480), Image.Resampling.LANCZOS)
    return scaled.crop((0, 0, 400, 240)), scaled.crop((40, 240, 360, 480))


def story_card(key, race, brood):
    if key == 'expbonus':
        return 'rez/estz09x.txt'
    if key.startswith('expzerg04'):
        return 'rez/estz04x.txt'
    code = ['z', 't', 'p'][race]
    number = '0t' if key == 'tutorial' else re.search(r'\d{2}[a-f]?$', key).group()
    return f'rez/est{code}{number}{"x" if brood else ""}.txt'


def stock_story_fallback(key):
    return key == 'expbonus' or (key.startswith('expzerg') and key[7:9] >= '04')


def background(raw, key, race, brood):
    path = raw / story_card(key, race, brood).replace('/', '_')
    pages = re.findall(r'</BACKGROUND\s+([^>]+)>', path.read_text(encoding='cp1252'), re.I)
    if not pages:
        raise ValueError(f'No background in {path}')
    source = next((name for name in reversed(pages) if 'blank.pcx' not in name.lower()), pages[-1])
    return source.lower().replace('\\', '_').replace('/', '_').removesuffix('.pcx') + '.png'


def prepare(source, output, raw=None, stage=None):
    if (raw is None) != (stage is None):
        raise ValueError('--raw and --stage must be supplied together')
    pairs = {}
    for path in sorted(source.glob('*.png')):
        top, bottom = split_screens(Image.open(path))
        for screen, image in [('top', top), ('bottom', bottom)]:
            folder = output / screen
            folder.mkdir(parents=True, exist_ok=True)
            image.save(folder / path.name)
        pairs[path.name.lower()] = (top, bottom)
    if not pairs:
        raise ValueError(f'No PNGs in {source}')
    mapping = {}
    if stage is not None:
        catalogue = Path(__file__).parents[1] / 'include/campaign_catalogue.h'
        campaign = stage / 'campaign'
        for key, _, _, race, brood in re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(\d+),(true|false)\}', catalogue.read_text()):
            name = background(raw, key, int(race), brood == 'true')
            if name not in pairs:
                if stock_story_fallback(key) and (campaign / f'{key}-story.bin').is_file():
                    continue
                raise ValueError(f'Missing completed scene {name} for {key}')
            top, bottom = pairs[name]
            # Native resolution on two pages, resident only during story view.
            bank(campaign / f'{key}-story.bin', [('top', top), ('bottom', bottom)],
                 positions=[(0, 0, 0), (1, 0, 0)])
            mapping[key] = name
        manifest_path = campaign / 'presentation-manifest.json'
        manifest = json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
        for key in mapping:
            path = campaign / f'{key}-story.bin'
            manifest[path.name] = {'bytes': path.stat().st_size, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
        manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    (output / 'screens-manifest.json').write_text(json.dumps({
        'top_size': [400, 240], 'bottom_size': [320, 240],
        'scaled_canvas': [400, 480], 'top_crop': [0, 0, 400, 240],
        'bottom_crop': [40, 240, 360, 480], 'images': sorted(pairs), 'missions': mapping,
    }, indent=2) + '\n')
    print(f'{len(pairs)} scene pairs exported; {len(mapping)} mission story banks prepared')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path); parser.add_argument('output', type=Path)
    parser.add_argument('--raw', type=Path); parser.add_argument('--stage', type=Path)
    args = parser.parse_args()
    prepare(args.input, args.output, args.raw, args.stage)
