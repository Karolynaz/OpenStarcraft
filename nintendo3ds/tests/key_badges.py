"""Check native badge atlas pixels, dimensions and unchanged HUD art."""
from pathlib import Path
import struct,sys
from PIL import Image
root=Path(sys.argv[1]); art=Path(__file__).resolve().parents[1]/'art/key-badges'
b=(root/'ui/hud.bin').read_bytes();w,h,pages,count=struct.unpack_from('<4I',b,8)
assert (w,h,pages,count)==(512,256,1,14)
pixels=b[24+count*60:];assert len(pixels)==512*256*2
if len(sys.argv)>2:
 original=Image.open(sys.argv[2]).convert('RGBA');assert original.size==(320,240)
 for y in range(240):
  for x in range(320):
   m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3)
   actual=struct.unpack_from('<H',pixels,(((y//8)*64+x//8)*64+m)*2)[0]
   r,g,blue,a=original.getpixel((x,y))
   assert actual==(r>>3)<<11|(g>>3)<<6|(blue>>3)<<1|(a>=128)
names=set()
for i in range(count):
 e=b[24+i*60:24+(i+1)*60];name=e[:40].split(b'\0')[0].decode();page,x0,y0,width,height=struct.unpack_from('<5I',e,40)
 if name=='hud':assert (x0,y0,width,height)==(0,0,320,240);continue
 assert name.startswith('key_') and name not in names;names.add(name)
 key=name[4:];im=Image.open(art/(key+'.png')).convert('RGBA')
 assert im.size==(width,height)==(23 if len(key)==1 else 35,23)
 assert x0>=324 and x0+width<=512 and y0+height<=256
 dark=light=0
 for y in range(height):
  for x in range(width):
   xx,yy=x0+x,y0+y;m=(xx&1)|((yy&1)<<1)|((xx&2)<<1)|((yy&2)<<2)|((xx&4)<<2)|((yy&4)<<3)
   actual=struct.unpack_from('<H',pixels,(((yy//8)*64+xx//8)*64+m)*2)[0]
   r,g,blue,a=im.getpixel((x,y));r=r*a//255;g=g*a//255;blue=blue*a//255
   assert actual==(r>>3)<<11|(g>>3)<<6|(blue>>3)<<1|bool(a)
   if a>128:dark+=r<70;light+=r>190
 assert dark>=6 and light>50
assert len(names)==13
for key in ('X','Y'):assert (art/(key+'.png')).read_bytes()==(art/('reference_'+key.lower()+'.png')).read_bytes()
print('13 native 23px/35px key badges, exact 5551 atlas, readable contrast, unchanged 256 KiB GPU page and Diablo X/Y references PASS')
