#!/usr/bin/env python3
"""Pack race HUDs and native 60x56 first-frame portraits; one resident HUD page."""
from pathlib import Path
import argparse, struct, json, hashlib, subprocess, shutil
from PIL import Image
from prepare_key_badges import install
from prepare_panel_text import install as install_panel_text

def prepare(background, portrait_raw, root):
    (root/'ui').mkdir(parents=True, exist_ok=True)
    manifest_path=root/'ui/manifest.json'
    manifest=json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
    for race in ('terran','protoss','zerg'):
        source=background/(race+'-320x240.png') if background.is_dir() else background
        im=Image.open(source).convert('RGBA')
        assert im.size==(320,240), (source, im.size)
        out=bytearray(512*256*2)
        for y in range(240):
            for x in range(320):
                r,g,b,alpha=im.getpixel((x,y))
                v=(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|(alpha>=128)
                m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3)
                struct.pack_into('<H',out,(((y//8)*64+x//8)*64+m)*2,v)
        hud=b'OB3UI001'+struct.pack('<4I',512,256,1,1)+b'hud'.ljust(40,b'\0')+struct.pack('<5I',0,0,0,320,240)+out
        name='hud-'+race
        (root/'ui'/(name+'.bin')).write_bytes(hud)
        manifest[name]={'pages':1,'sprites':1,'bytes':len(hud),'sha256':hashlib.sha256(hud).hexdigest(),'background_sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
    manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
    for race in ('terran','protoss','zerg'):
        install(root,'hud-'+race+'.bin');install_panel_text(root,'hud-'+race+'.bin')
    # Preserve the old asset name for tooling, not as a runtime fallback.
    shutil.copy2(root/'ui/hud-protoss.bin',root/'ui/hud.bin')
    manifest=json.loads(manifest_path.read_text());manifest['hud']=manifest['hud-protoss'].copy()
    manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
    folder=root/'portraits';folder.mkdir(exist_ok=True);portraits={}
    for source in sorted(portrait_raw.glob('*.smk')):
        raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(source),'-frames:v','1','-vf','scale=60:56:flags=area','-f','rawvideo','-pix_fmt','rgb24','-'])
        assert len(raw)==60*56*3
        pcm=bytearray()
        for i in range(0,len(raw),3):
            r,g,b=raw[i:i+3];pcm+=struct.pack('<H',(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|1)
        target=folder/(source.stem+'.bin');target.write_bytes(pcm)
        portraits[target.name]={'sha256':hashlib.sha256(pcm).hexdigest(),'bytes':len(pcm),'width':60,'height':56}
    (folder/'manifest.json').write_text(json.dumps(portraits,indent=2)+'\n')
    print(f'Three race HUDs, one 256 KiB page resident; {len(portraits)} native portraits')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('background',type=Path);p.add_argument('portrait_raw',type=Path);p.add_argument('root',type=Path)
    a=p.parse_args();prepare(a.background,a.portrait_raw,a.root)
