"""Shared private UI texture-bank writer (RGBA5551, native 8x8 tiling)."""
from PIL import Image
import struct

def bank(path,items,side=512,height=256,positions=None):
 pages=[];entries=[];x=y=row=0
 for index,(name,im) in enumerate(items):
  im=im.convert('RGBA');w,h=im.size
  if positions is not None:
   position=positions[index];page,x,y=position if len(position)==3 else (0,*position);assert x+w<=side and y+h<=height
   while len(pages)<=page:pages.append(Image.new('RGBA',(side,height)))
   pages[page].paste(im,(x,y));entries.append((name,page,x,y,w,h));continue
  if x+w>side:x=0;y+=row;row=0
  if not pages or y+h>height:pages.append(Image.new('RGBA',(side,height)));x=y=row=0
  pages[-1].paste(im,(x,y));entries.append((name,len(pages)-1,x,y,w,h));x+=w+1;row=max(row,h+1)
 with path.open('wb') as f:
  f.write(b'OB3UI001'+struct.pack('<4I',side,height,len(pages),len(entries)))
  for name,*v in entries:f.write(name.encode().ljust(40,b'\0')+struct.pack('<5I',*v))
  for p in pages:
   b=bytearray(side*height*2)
   for yy in range(height):
    for xx in range(side):
     r,g,blue,a=p.getpixel((xx,yy));v=(r>>3)<<11|(g>>3)<<6|(blue>>3)<<1|(a>=128)
     m=(xx&1)|((yy&1)<<1)|((xx&2)<<1)|((yy&2)<<2)|((xx&4)<<2)|((yy&4)<<3)
     struct.pack_into('<H',b,(((yy//8)*(side//8)+xx//8)*64+m)*2,v)
   f.write(b)
