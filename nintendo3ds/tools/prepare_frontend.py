#!/usr/bin/env python3
"""Prepare privately owned menu art and user-supplied HUD/title for the 3DS.
Run with the bundled Python (Pillow), ffmpeg, and the host MPQ extractor.
No original images are committed to the repository.
"""
from pathlib import Path
import argparse, struct, subprocess, json, hashlib, shutil
from PIL import Image
p=argparse.ArgumentParser();p.add_argument('work',type=Path);p.add_argument('output',type=Path);p.add_argument('--ffmpeg',default=shutil.which('ffmpeg') or '/opt/homebrew/bin/ffmpeg');p.add_argument('--custom-top',type=Path,required=True);p.add_argument('--custom-bottom',type=Path,required=True);p.add_argument('--title',type=Path,required=True);a=p.parse_args();r=a.work
images=[]
def add(name,im):
 im=im.convert('RGBA');images.append((name,im));return im

def pcx(name):return Image.open(r/(name+'.pcx')).convert('RGBA')
def over(dst,im,xy):
 im=im.copy();im.putalpha(Image.eval(im.convert('L'),lambda v:255 if v else 0));dst.alpha_composite(im,xy)
def frames(name,count=4):
 out=r/(name+'-%02d.png')
 subprocess.run([a.ffmpeg,'-v','error','-i',str(r/(name+'.smk')),'-vf','fps=8,scale=iw/2:ih/2:flags=area','-frames:v',str(count),'-y',str(out)],check=True)
 files=sorted(r.glob(name+'-[0-9][0-9].png'))
 result=[]
 for f in files:
  im=Image.open(f).convert('RGBA');im.putalpha(Image.eval(im.convert('L'),lambda v:255 if v else 0));result.append(im)
 return result
# Original 640x480 backgrounds, with their native BIN image positions halved.
bg=pcx('glue_PalCs_backgnd').resize((320,240),Image.Resampling.LANCZOS)
campaign=bg.copy()
for name,xy in [('glue_campaign_pBotR',(108,185)),('glue_campaign_pBotL',(0,176))]:
 im=pcx(name);over(campaign,im.resize((im.width//2,im.height//2),Image.Resampling.LANCZOS),xy)
add('campaign',campaign)
custom=bg.copy()
for name,xy in [('glue_create_pListMap2',(0,0)),('glue_create_pInfo',(200,0)),('glue_create_pSlots',(0,142)),('glue_PalNl_pCancel',(236,178)),('glue_PalNl_pOk',(218,168))]:
 im=pcx(name);over(custom,im.resize((im.width//2,im.height//2),Image.Resampling.LANCZOS),xy)
add('custom',custom)
add('custom_top',Image.open(a.custom_top))
add('custom_bottom',Image.open(a.custom_bottom))
frame=bg.copy();frame.putalpha(Image.eval(frame.convert('L'),lambda v:255 if v else 0))
add('frame_bottom',frame)
add('frame_top',frame.resize((400,240),Image.Resampling.LANCZOS))
add('title',Image.open(a.title))
logo=pcx('logo-bw')
add('logo_sc',logo.crop((20,40,235,83)).resize((245,49),Image.Resampling.LANCZOS))
add('logo_bw',logo.crop((129,114,340,160)).resize((245,53),Image.Resampling.LANCZOS))
# Original portrait idle/on animations. Keep four small frames per state;
# the hardware draws these over the static campaign backdrop.
for version,folder,prefix in [(0,'campaign',''),(1,'Expcampaign','X')]:
 for race in ['Terr','Prot','Zerg']:
  for state in ['', 'On']:
   fs=frames(f'glue_{folder}_{prefix}{race}{state}')
   for i in range(4):add(f'v{version}_{race}{state}{i}',fs[i%len(fs)])
for i,im in enumerate(frames('glue_campaign_disk')):add(f'disk{i}',im)
# Native menu cursor GRP (first frame); palette from the matching menu PCX.
b=(r/'cursor.grp').read_bytes();n,w,h=struct.unpack_from('<HHH',b);ox,oy,cw,ch,offset=struct.unpack_from('<BBBBI',b,6)
im=Image.new('RGBA',(cw,ch));pal=Image.open(r/'glue_PalCs_backgnd.pcx').getpalette()
for y in range(ch):
 at=offset+struct.unpack_from('<H',b,offset+2*y)[0];x=0
 while x<cw:
  t=b[at];at+=1
  if t&128:x+=t&127;continue
  count=t&63 if t&64 else t
  values=[b[at]]*count if t&64 else list(b[at:at+count]);at+=1 if t&64 else count
  for c in values:im.putpixel((x,y),tuple(pal[c*3:c*3+3])+(255,));x+=1
add('cursor',im.resize((max(1,cw//2),max(1,ch//2)),Image.Resampling.NEAREST))

def pack(path,items,side=512,height=512):
 pages=[];entries=[];x=y=row=0
 for name,im in sorted(items,key=lambda item:-item[1].height):
  assert im.width<=side and im.height<=height
  if x+im.width>side:x=0;y+=row;row=0
  if not pages or y+im.height>height:pages.append(Image.new('RGBA',(side,height)));x=y=row=0
  pages[-1].paste(im,(x,y));entries.append((name,len(pages)-1,x,y,im.width,im.height));x+=im.width+1;row=max(row,im.height+1)
 path.parent.mkdir(parents=True,exist_ok=True)
 with path.open('wb') as f:
  f.write(b'OB3UI001');f.write(struct.pack('<4I',side,height,len(pages),len(entries)))
  for name,*rect in entries:f.write(name.encode().ljust(40,b'\0'));f.write(struct.pack('<5I',*rect))
  for page in pages:
   out=bytearray(side*height*2)
   for py in range(height):
    for px in range(side):
     red,g,b,alpha=page.getpixel((px,py));v=(red>>3)<<11|(g>>3)<<6|(b>>3)<<1|(alpha>=128)
     low=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3)
     struct.pack_into('<H',out,(((py//8)*(side//8)+px//8)*64+low)*2,v)
   f.write(out)
 for i,page in enumerate(pages):page.save(r/(path.stem+f'-atlas{i}.png'))
 return {'pages':len(pages),'sprites':len(entries),'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()}
manifest={'menu':pack(a.output/'menu.bin',images),'hud':pack(a.output/'hud.bin',[('hud',Image.open(r/'hud.png'))],height=256)}
(a.output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
