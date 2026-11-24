#!/usr/bin/env python3
"""Build private CIA/CCI and a separate SD asset folder; never include user saves.

Requires ffmpeg, makerom and bannertool. Runs locally; creates no ZIP or upload.
"""
from pathlib import Path
import argparse, hashlib, json, shutil, subprocess, wave


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def build(stage, elf, work, output, tools, title_image, bundle_assets=False):
    stage, elf, work, output, tools = (p.resolve() for p in (stage, elf, work, output, tools))
    work.mkdir(parents=True, exist_ok=True)
    output.mkdir(parents=True, exist_ok=True)
    bundle = work / 'romfs/openbw' if bundle_assets else output / 'SD/3ds/openbw'
    if bundle.is_relative_to(stage) or stage.is_relative_to(bundle):
        raise ValueError('Use separate directories for the source and packaged game files')
    bundle.mkdir(parents=True, exist_ok=True)
    roots = {'StarDat.mpq', 'BrooDat.mpq', 'Patch_rt.mpq', 'graphics.bin', 'graphics-arena.bin'}
    folders = {'audio', 'campaign', 'maps', 'portraits', 'ui'}
    suffixes = {'.mpq', '.bin', '.json', '.pcm', '.chk', '.scm', '.scx', '.brief',
                '.brief-audio', '.brief-info', '.brief-cast', '.intro', '.location', '.motion', '.layout'}
    sources = {}
    for source in sorted(stage.rglob('*')):
        if not source.is_file():
            continue
        rel = source.relative_to(stage)
        if not (str(rel) in roots or (rel.parts[0] in folders and source.suffix.lower() in suffixes)):
            continue
        if source.name.startswith('.') or any(p in ('saves', 'logs') for p in rel.parts):
            raise ValueError(f'Private user state in assets: {rel}')
        sources[rel] = source
    missing = roots - {p.as_posix() for p in sources}
    if missing:
        raise ValueError(f'Missing required game files: {", ".join(sorted(missing))}')
    for rel in sources:
        if rel.parts[0] == 'campaign' and rel.suffix == '.intro':
            story = rel.with_name(f'{rel.stem}-story.bin')
            if rel.with_suffix('.brief-info') in sources and story not in sources:
                raise ValueError(f'Missing campaign story art: {story}')
    for target in bundle.rglob('*'):
        if target.is_file() and target.relative_to(bundle) not in sources:
            raise ValueError(f'Unknown file in output: {target}. Use a fresh output folder.')
    entries = []
    for rel, source in sources.items():
        target = bundle / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        entries.append({'path': rel.as_posix(), 'bytes': target.stat().st_size,
                        'sha256': digest(target)})
    repo = Path(__file__).resolve().parents[1]
    rsf = (repo / 'app.rsf').read_text()
    if not bundle_assets:
        rsf = rsf.replace('RomFs:\n  RootPath: romfs\n', '')
    (work / 'app.rsf').write_text(rsf)
    with wave.open(str(work / 'silence.wav'), 'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(22050); f.writeframes(bytes(22050 * 2))
    def run(args):
        subprocess.run([str(x) for x in args], cwd=work, check=True)
    for name, dimensions in (('icon', '48:48'), ('banner', '256:128')):
        run(['ffmpeg', '-v', 'error', '-y', '-i', title_image.resolve(),
             '-vf', f'scale={dimensions}:flags=lanczos', '-pix_fmt', 'rgb24',
             '-frames:v', 1, '-update', 1, f'{name}.png'])
    run([tools/'bannertool', 'makesmdh', '-s', 'StarCraft 3DS', '-l', 'OpenBW StarCraft / Brood War',
         '-p', 'Private test build', '-i', 'icon.png', '-o', 'icon.smdh'])
    run([tools/'bannertool', 'makebanner', '-i', 'banner.png', '-a', 'silence.wav', '-o', 'banner.bin'])
    results = []
    for fmt, ext in (('cia', 'cia'), ('cci', '3ds')):
        dest = output / f'StarCraft-3DS.{ext}'
        run([tools/'makerom', '-f', fmt, '-target', 't', '-rsf', 'app.rsf', '-elf', elf,
             '-icon', 'icon.smdh', '-banner', 'banner.bin', '-o', dest])
        results.append({'file': str(dest), 'bytes': dest.stat().st_size,
                        'sha256': digest(dest)})
    (work/'package-manifest.json').write_text(json.dumps({'title_id': '000400000F9B3000',
        'asset_location': 'romfs:/openbw/' if bundle_assets else 'sdmc:/3ds/openbw/',
        'assets': entries, 'files': results, 'private_state_excluded': True}, indent=2)+'\n')
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for key in ('stage', 'elf', 'work', 'output', 'tools', 'title_image'):
        p.add_argument(key, type=Path)
    p.add_argument('--bundle-assets', action='store_true', help='Embed game files in RomFS')
    a = p.parse_args()
    build(**vars(a))
