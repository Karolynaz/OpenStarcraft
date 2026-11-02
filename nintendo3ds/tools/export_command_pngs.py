#!/usr/bin/env python3
"""Export private original command art from the independently decoded UI oracle."""
from pathlib import Path
import argparse,struct,json,shutil
from PIL import Image,ImageDraw
p=argparse.ArgumentParser();p.add_argument('reference',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
data=a.reference.read_bytes();assert data[:8]==b'OBWUI02\0';races,count=struct.unpack_from('<II',data,8);assert races==3;at=16
a.output.mkdir(parents=True,exist_ok=True);manifest=[]
for race in ('zerg','terran','protoss'):
 sheet=Image.new('RGB',(20*54,((count+19)//20)*52),(12,12,16));draw=ImageDraw.Draw(sheet)
 for frame in range(count):
  x,y,w,h=struct.unpack_from('<IIII',data,at);at+=16
  for state in ('enabled','disabled'):
   colors=struct.unpack_from('<'+'H'*(w*h),data,at);at+=w*h*2
   im=Image.new('RGBA',(w,h));im.putdata([(((c>>11)&31)*255//31,((c>>6)&31)*255//31,((c>>1)&31)*255//31,255 if c&1 else 0) for c in colors])
   canvas=Image.new('RGBA',(36,34));canvas.paste(im,(x,y));name=f'{race}-{frame:03}-{state}.png';canvas.save(a.output/name)
   manifest.append({'file':name,'frame':frame,'race':race,'state':state,'original_crop':[x,y,w,h]})
   if state=='enabled':
    xx,yy=frame%20*54,frame//20*52;sheet.paste(canvas,(xx,yy),canvas);draw.text((xx,yy+35),str(frame),fill='white')
 at+=48*40*2
 sheet.save(a.output/f'_index-{race}.png')
assert at==len(data)
for file in (Path(__file__).resolve().parents[1]/'art/key-badges').glob('*.png'):
 if not file.name.startswith('reference_'):shutil.copy2(file,a.output/('key-'+file.name))
(a.output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(a.output/'README.txt').write_text(f'Private owned StarCraft/Brood War command icons. {count} frames per race, enabled and disabled, native 36x34 transparent canvas. Numeric filenames are original GRP frame IDs. Key badges are separate key-*.png. _index-*.png are contact sheets. Do not publish owned game art.\n')
print(f'{len(manifest)} original icon PNGs plus badges and three index sheets: {a.output}')
