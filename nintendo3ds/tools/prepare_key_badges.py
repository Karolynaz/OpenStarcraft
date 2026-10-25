#!/usr/bin/env python3
"""Native-pixel key art, following Diablo3Ds' exported 23px HUD badges.

Stores badges in unused columns of the existing HUD page: no extra texture.
Run after prepare_hud.py, or pass --art-only to regenerate generic source PNGs.
"""
from pathlib import Path
import argparse, hashlib, json, struct
from PIL import Image, ImageDraw, ImageFont

ART = Path(__file__).resolve().parents[1] / 'art/key-badges'
KEYS = ('A', 'B', 'X', 'Y', 'L+A', 'L+B', 'L+X', 'L+Y', 'L+R', 'R+A', 'R+B', 'R+X', 'R+Y')

def generate(font):
    scale = 4
    for key in KEYS:
        target = ART / (key + '.png')
        if key in ('X', 'Y'):
            target.write_bytes((ART / ('reference_' + key.lower() + '.png')).read_bytes())
            continue
        width = 23 if len(key) == 1 else 35
        im = Image.new('RGBA', (width * scale, 23 * scale))
        d = ImageDraw.Draw(im)
        # 19px face, with the same four-pixel shadow allowance as Diablo.
        box = (scale, 0, (width - 2) * scale, 19 * scale)
        d.rounded_rectangle((box[0], box[1]+3*scale, box[2]+scale, box[3]+3*scale), 9*scale, fill=(0,0,0,160))
        d.rounded_rectangle(box, 9*scale, fill=(238,238,238,255), outline=(55,55,55,255), width=scale)
        f = ImageFont.truetype(str(font), 14 * scale)
        bounds = d.textbbox((0,0), key, font=f)
        x = ((width-1)*scale - (bounds[2]-bounds[0]))/2 - bounds[0]
        y = (19*scale - (bounds[3]-bounds[1]))/2 - bounds[1]
        d.text((x,y), key, font=f, fill=(8,8,8,255))
        im.resize((width,23),Image.Resampling.LANCZOS).save(target)

def install(root):
    path = root/'ui/hud.bin'
    data = bytearray(path.read_bytes())
    assert data[:8] == b'OB3UI001'
    width,height,pages,count = struct.unpack_from('<4I',data,8)
    assert (width,height,pages) == (512,256,1)
    entries=[]
    for i in range(count):
        entry = data[24+i*60:24+(i+1)*60]
        if not entry[:40].split(b'\0')[0].startswith(b'key_'):
            entries.append(bytes(entry))
    pixels = data[24+count*60:]
    assert len(pixels)==width*height*2
    for i,key in enumerate(KEYS):
        im = Image.open(ART/(key+'.png')).convert('RGBA')
        x0,y0 = 324+(i%4)*40, (i//4)*28
        w,h=im.size
        entries.append(('key_'+key).encode().ljust(40,b'\0')+struct.pack('<5I',0,x0,y0,w,h))
        # Composite fractional edge alpha over black before 5551 conversion.
        # Glyph antialiasing stays on the opaque face, never GPU-resampled.
        for y in range(h):
            for x in range(w):
                r,g,b,a=im.getpixel((x,y));r=r*a//255;g=g*a//255;b=b*a//255
                v=(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|bool(a)
                xx,yy=x0+x,y0+y
                m=(xx&1)|((yy&1)<<1)|((xx&2)<<1)|((yy&2)<<2)|((xx&4)<<2)|((yy&4)<<3)
                struct.pack_into('<H',pixels,(((yy//8)*64+xx//8)*64+m)*2,v)
    output=b'OB3UI001'+struct.pack('<4I',width,height,pages,len(entries))+b''.join(entries)+pixels
    path.write_bytes(output)
    mp=root/'ui/manifest.json';manifest=json.loads(mp.read_text());manifest['hud'].update(sprites=len(entries),bytes=len(output),sha256=hashlib.sha256(output).hexdigest(),key_badges=len(KEYS));mp.write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'{len(KEYS)} native key badges; same 256 KiB HUD texture')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('root',type=Path,nargs='?')
    p.add_argument('--font',type=Path,help='Regenerate generic badges using this Arial-compatible font')
    a=p.parse_args()
    if a.font:generate(a.font)
    if a.root:install(a.root)
