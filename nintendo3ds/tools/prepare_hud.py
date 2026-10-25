#!/usr/bin/env python3
"""Convert the user's HUD and private first-frame portraits; no runtime video."""
from pathlib import Path
import argparse,struct,json,hashlib,subprocess,io
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('background',type=Path);p.add_argument('portrait_raw',type=Path);p.add_argument('root',type=Path);a=p.parse_args()
im=Image.open(a.background).convert('RGBA');assert im.size==(320,240)
out=bytearray(512*256*2)
for y in range(240):
 for x in range(320):
  r,g,b,alpha=im.getpixel((x,y));v=(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|(alpha>=128);m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3)
  struct.pack_into('<H',out,(((y//8)*64+x//8)*64+m)*2,v)
hud=b'OB3UI001'+struct.pack('<4I',512,256,1,1)+b'hud'.ljust(40,b'\0')+struct.pack('<5I',0,0,0,320,240)+out
(a.root/'ui/hud.bin').write_bytes(hud);manifest=json.loads((a.root/'ui/manifest.json').read_text());manifest['hud']={'pages':1,'sprites':1,'bytes':len(hud),'sha256':hashlib.sha256(hud).hexdigest()};(a.root/'ui/manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
folder=a.root/'portraits';folder.mkdir(exist_ok=True);portraits={}
for source in sorted(a.portrait_raw.glob('*.smk')):
 raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(source),'-frames:v','1','-vf','scale=48:48:flags=area','-f','rawvideo','-pix_fmt','rgb24','-'])
 assert len(raw)==48*48*3;pcm=bytearray()
 for i in range(0,len(raw),3):
  r,g,b=raw[i:i+3];pcm+=struct.pack('<H',(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|1)
 target=folder/(source.stem+'.bin');target.write_bytes(pcm);portraits[target.name]={'sha256':hashlib.sha256(pcm).hexdigest(),'bytes':len(pcm)}
(folder/'manifest.json').write_text(json.dumps(portraits,indent=2)+'\n')
from prepare_key_badges import install
install(a.root)
from prepare_panel_text import install as install_panel_text
install_panel_text(a.root)
print(f'HUD 1 page; {len(portraits)} static original portraits, on-demand in existing atlas')
