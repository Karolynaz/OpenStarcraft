#!/usr/bin/env python3
"""Build private CIA/CCI with read-only owned assets; never include user saves.

Requires Pillow, makerom and bannertool. Runs locally; creates no ZIP or upload.
"""
from pathlib import Path
import argparse, hashlib, json, shutil, subprocess, wave
from PIL import Image


def build(stage, elf, work, output, tools, title_image):
    stage, elf, work, output, tools = (p.resolve() for p in (stage, elf, work, output, tools))
    work.mkdir(parents=True, exist_ok=True)
    output.mkdir(parents=True, exist_ok=True)
    bundle = work / 'romfs/openbw'
    bundle.mkdir(parents=True, exist_ok=True)
    roots = {'StarDat.mpq', 'BrooDat.mpq', 'Patch_rt.mpq', 'graphics.bin', 'graphics-arena.bin'}
    folders = {'audio', 'campaign', 'maps', 'portraits', 'ui'}
    suffixes = {'.mpq', '.bin', '.json', '.pcm', '.chk', '.scm', '.scx', '.brief',
                '.brief-audio', '.brief-info', '.brief-cast', '.intro', '.location', '.motion', '.layout'}
    entries = []
    allowed = set()
    for source in sorted(stage.rglob('*')):
        if not source.is_file():
            continue
        rel = source.relative_to(stage)
        if not (str(rel) in roots or (rel.parts[0] in folders and source.suffix.lower() in suffixes)):
            continue
        if source.name.startswith('.') or any(p in ('saves', 'logs') for p in rel.parts):
            raise ValueError(f'Private user state in assets: {rel}')
        target = bundle / rel
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        allowed.add(rel)
        entries.append({'path': rel.as_posix(), 'bytes': target.stat().st_size,
                        'sha256': hashlib.sha256(target.read_bytes()).hexdigest()})
    for target in bundle.rglob('*'):
        if target.is_file() and target.relative_to(bundle) not in allowed:
            target.unlink()
    assert all((bundle / p).is_file() for p in roots)
    assert not list(bundle.rglob('*.ob3')) and not list(bundle.rglob('*.cfg'))
    repo = Path(__file__).resolve().parents[1]
    shutil.copy2(repo / 'app.rsf', work / 'app.rsf')
    image = Image.open(title_image).convert('RGB')
    image.resize((48, 48), Image.Resampling.LANCZOS).save(work / 'icon.png')
    image.resize((256, 128), Image.Resampling.LANCZOS).save(work / 'banner.png')
    with wave.open(str(work / 'silence.wav'), 'wb') as f:
        f.setnchannels(1); f.setsampwidth(2); f.setframerate(22050); f.writeframes(bytes(22050 * 2))
    def run(args):
        subprocess.run([str(x) for x in args], cwd=work, check=True)
    run([tools/'bannertool', 'makesmdh', '-s', 'StarCraft 3DS', '-l', 'OpenBW StarCraft / Brood War',
         '-p', 'Private test build', '-i', 'icon.png', '-o', 'icon.smdh'])
    run([tools/'bannertool', 'makebanner', '-i', 'banner.png', '-a', 'silence.wav', '-o', 'banner.bin'])
    results = []
    for fmt, ext in (('cia', 'cia'), ('cci', '3ds')):
        dest = output / f'StarCraft-3DS.{ext}'
        run([tools/'makerom', '-f', fmt, '-target', 't', '-rsf', 'app.rsf', '-elf', elf,
             '-icon', 'icon.smdh', '-banner', 'banner.bin', '-o', dest])
        results.append({'file': str(dest), 'bytes': dest.stat().st_size,
                        'sha256': hashlib.sha256(dest.read_bytes()).hexdigest()})
    (work/'package-manifest.json').write_text(json.dumps({'title_id': '000400000F9B3000',
        'assets': entries, 'files': results, 'private_state_excluded': True}, indent=2)+'\n')
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    for key in ('stage', 'elf', 'work', 'output', 'tools', 'title_image'):
        p.add_argument(key, type=Path)
    a = p.parse_args()
    build(a.stage, a.elf, a.work, a.output, a.tools, a.title_image)
