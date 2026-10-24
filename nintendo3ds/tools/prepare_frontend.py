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
animations=[]
def add(name,im):
 im=im.convert('RGBA');images.append((name,im));return im

def pcx(name):return Image.open(r/(name+'.pcx')).convert('RGBA')
def over(dst,im,xy):
 im=im.copy();im.putalpha(Image.eval(im.convert('L'),lambda v:255 if v else 0));dst.alpha_composite(im,xy)
def animation(name,key,scale=.325):
 # Decode the complete source cycle, not an initial still segment.
 source=r/(name+'.smk')
 probe=subprocess.run([str(Path(a.ffmpeg).with_name('ffprobe')),'-v','error','-show_entries','stream=width,height','-of','json',str(source)],capture_output=True,text=True,check=True)
 stream=json.loads(probe.stdout)['streams'][0];w=max(1,round(stream['width']*scale));h=max(1,round(stream['height']*scale))
 assert w<=128 and h<=128
 raw=subprocess.run([a.ffmpeg,'-v','error','-i',str(source),'-vf',f'fps=8,scale={w}:{h}:flags=area','-f','rawvideo','-pix_fmt','rgba','-'],capture_output=True,check=True).stdout
 frames=[]
 for at in range(0,len(raw),w*h*4):
  im=Image.frombytes('RGBA',(w,h),raw[at:at+w*h*4]);im.putalpha(Image.eval(im.convert('L'),lambda v:255 if v else 0));frames.append(im)
 assert 1<len(frames)<=256
 animations.append((key,w,h,frames))

# Original 640x480 backgrounds, with their native BIN image positions halved.
bg=pcx('glue_PalCs_backgnd').resize((320,240),Image.Resampling.LANCZOS)
add('campaign_left',bg.crop((0,0,160,240)))
add('campaign_right',bg.crop((160,0,320,240)))
custom=bg.copy()
for name,xy in [('glue_create_pListMap2',(0,0)),('glue_create_pInfo',(200,0)),('glue_create_pSlots',(0,142)),('glue_PalNl_pCancel',(236,178)),('glue_PalNl_pOk',(218,168))]:
 im=pcx(name);over(custom,im.resize((im.width//2,im.height//2),Image.Resampling.LANCZOS),xy)
add('custom',custom)
add('custom_top',Image.open(a.custom_top))
add('custom_bottom',Image.open(a.custom_bottom))
frame=bg.copy();frame.putalpha(Image.eval(frame.convert('L'),lambda v:255 if v else 0))
add('frame_bottom',frame)
add('frame_top',frame.resize((400,240),Image.Resampling.LANCZOS))
# Keep original asymmetric metal mounts, instead of squeezing the entire
# screen's outer frame into a short flat rectangle.
def keyed(im):
 im=im.convert('RGBA');im.putalpha(Image.eval(im.convert('L'),lambda v:255 if v else 0));return im
panel=keyed(pcx('glue_create_pSlots')).resize((292,166),Image.Resampling.LANCZOS)
beam=keyed(pcx('glue_create_pListMap2').crop((0,0,364,100))).resize((292,30),Image.Resampling.LANCZOS)
panel.alpha_composite(beam,(0,0))
right=keyed(pcx('glue_create_pInfo').crop((190,0,240,120))).resize((25,60),Image.Resampling.LANCZOS)
panel.alpha_composite(right,(267,0));add('custom_panel',panel)
for key,operation in [('custom_cancel',Image.Transpose.ROTATE_180),('custom_ok',Image.Transpose.FLIP_TOP_BOTTOM)]:
 native=keyed(pcx('glue_PalNl_pCancel')).transpose(operation)
 mount=Image.new('RGBA',(142,44))
 mount.alpha_composite(native.crop((0,0,168,33)).resize((142,28),Image.Resampling.LANCZOS),(0,0))
 mount.alpha_composite(native.crop((0,33,168,91)).resize((142,16),Image.Resampling.LANCZOS),(0,28))
 add(key,mount)
 if key=='custom_ok':add('menu_button',mount.resize((146,34),Image.Resampling.LANCZOS))

add('title',Image.open(a.title))
logo=pcx('logo-bw')
add('logo_sc',logo.crop((20,40,235,83)).resize((245,49),Image.Resampling.LANCZOS))
add('logo_bw',logo.crop((129,114,340,160)).resize((245,53),Image.Resampling.LANCZOS))
# Full native idle and hover cycles, streamed through four bounded surfaces.
for version,folder,prefix in [(0,'campaign',''),(1,'Expcampaign','X')]:
 for race in ['Terr','Prot','Zerg']:
  for state in ['', 'On']:animation(f'glue_{folder}_{prefix}{race}{state}',f'v{version}_{race}{state}')
animation('glue_campaign_disk','disk',.40)
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
for yy in range(im.height):
 for xx in range(im.width):
  red,g,blue,alpha=im.getpixel((xx,yy));light=max(red,g,blue)
  if alpha and light:im.putpixel((xx,yy),(light//4,max(64,light),light//5,alpha))
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
def pack_animations(path):
 entries=[];offset=12+len(animations)*56
 with path.open('wb') as f:
  f.write(b'OB3AN001');f.write(struct.pack('<I',len(animations)))
  for key,w,h,frames in animations:
   entries.append((key,w,h,len(frames),offset));f.write(key.encode().ljust(40,b'\0'));f.write(struct.pack('<4I',w,h,len(frames),offset));offset+=w*h*len(frames)*2
  for key,w,h,frames in animations:
   for im in frames:
    pixels=bytearray(w*h*2)
    for i,(red,g,blue,alpha) in enumerate(im.getdata()):struct.pack_into('<H',pixels,i*2,(red>>3)<<11|(g>>3)<<6|(blue>>3)<<1|(alpha>=128))
    f.write(pixels)
 return {'sequences':len(entries),'frames':sum(e[3] for e in entries),'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'entries':entries}
manifest={'menu':pack(a.output/'menu.bin',images),'hud':pack(a.output/'hud.bin',[('hud',Image.open(r/'hud.png'))],height=256),'animations':pack_animations(a.output/'animations.bin')}
(a.output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps(manifest))
