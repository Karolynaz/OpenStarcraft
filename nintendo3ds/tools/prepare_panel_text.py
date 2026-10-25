#!/usr/bin/env python3
"""Bake Roboto Bold 10px glyphs into unused HUD space, preserving UI colors."""
from pathlib import Path
import argparse,math,json,struct,hashlib
from PIL import Image,ImageFont,ImageDraw
ART=Path(__file__).resolve().parents[1]/'art/panel-text'
COLORS={'W':(190,200,245),'G':(70,240,85),'B':(45,110,230)}
def generate(font):
 ART.mkdir(exist_ok=True);f=ImageFont.truetype(str(font),10);im=Image.new('L',(192,78));d=ImageDraw.Draw(im);advances={}
 for i,c in enumerate(range(32,127)):
  x,y=(i%16)*12,(i//16)*13;ch=chr(c);advances[str(c)]=math.ceil(f.getlength(ch));d.text((x,y+9),ch,font=f,fill=255,anchor='ls')
 im.save(ART/'font10.png');(ART/'metrics.json').write_text(json.dumps({'family':'Roboto Bold','size':10,'line_height':12,'advances':advances},indent=2)+'\n')
def install(root):
 b=(root/'ui/hud.bin').read_bytes();w,h,pages,count=struct.unpack_from('<4I',b,8);assert(w,h,pages)==(512,256,1)
 entries=[b[24+i*60:24+(i+1)*60] for i in range(count) if not b[24+i*60:64+i*60].split(b'\0')[0].startswith(b't10_')]
 pixels=bytearray(b[24+count*60:]);source=Image.open(ART/'font10.png').convert('L');metrics=json.loads((ART/'metrics.json').read_text());n=0
 for color,rgb in COLORS.items():
  codes=range(32,127) if color=='W' else [*range(48,58),47]
  for code in codes:
   advance=metrics['advances'][str(code)];i=code-32;sx,sy=(i%16)*12,(i//16)*13;x0,y0=324+(n%15)*12,112+(n//15)*13;n+=1
   assert x0+advance<=512 and y0+13<=256
   name=f't10_{color}_{code:02x}';entries.append(name.encode().ljust(40,b'\0')+struct.pack('<5I',0,x0,y0,advance,13))
   for y in range(13):
    for x in range(advance):
     a=source.getpixel((sx+x,sy+y));r,g,blue=[v*a//255 for v in rgb];v=(r>>3)<<11|(g>>3)<<6|(blue>>3)<<1|bool(a)
     xx,yy=x0+x,y0+y;m=(xx&1)|((yy&1)<<1)|((xx&2)<<1)|((yy&2)<<2)|((xx&4)<<2)|((yy&4)<<3)
     struct.pack_into('<H',pixels,(((yy//8)*64+xx//8)*64+m)*2,v)
 output=b'OB3UI001'+struct.pack('<4I',512,256,1,len(entries))+b''.join(entries)+pixels;(root/'ui/hud.bin').write_bytes(output)
 mp=root/'ui/manifest.json';manifest=json.loads(mp.read_text());manifest['hud'].update(sprites=len(entries),bytes=len(output),sha256=hashlib.sha256(output).hexdigest(),panel_font_size=10);mp.write_text(json.dumps(manifest,indent=2)+'\n')
 print(f'{n} native Roboto Bold10 glyphs, existing UI colors; unchanged 256 KiB HUD page')
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path,nargs='?');p.add_argument('--font',type=Path);a=p.parse_args()
 if a.font:generate(a.font)
 if a.root:install(a.root)
