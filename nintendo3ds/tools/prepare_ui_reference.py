#!/usr/bin/env python3
"""Independent bounded PCX decoder for a private original-command-art oracle."""
import pathlib,struct,sys
root=pathlib.Path(sys.argv[1])
def pcx(name):
    src=(root/name).read_bytes()
    assert src[0]==10 and src[2]==1 and src[3]==8 and src[65]==1
    left,top,right,bottom=struct.unpack_from('<HHHH',src,4)
    width,height=right-left+1,bottom-top+1
    stride=struct.unpack_from('<H',src,66)[0]
    assert width<=stride<=1024 and 0<height<=1024 and src[-769]==12
    pixels=bytearray();at=128
    while len(pixels)<stride*height:
        token=src[at];at+=1
        if token>=192:
            count=token&63;value=src[at];at+=1
        else:count,value=1,token
        assert count and len(pixels)+count<=stride*height
        pixels.extend(bytes([value])*count)
    return width,height,stride,pixels,src[-768:]
_,_,_,remap,_=pcx('ticon.pcx')
raw=(root/'icons.raw').read_bytes();count=struct.unpack_from('<I',raw)[0];at=4;frames=[]
for _ in range(count):
    x,y,w,h=struct.unpack_from('<IIII',raw,at);at+=16
    indices=struct.unpack_from('<'+'H'*(w*h),raw,at);at+=2*w*h
    assert all(v<=256 for v in indices)
    frames.append((x,y,w,h,indices))
assert at==len(raw)
with (root/'reference.bin').open('wb') as out:
    out.write(b'OBWUI02\0');out.write(struct.pack('<II',3,count))
    for race,name in enumerate(['zconsole.pcx','tconsole.pcx','pconsole.pcx']):
        width,height,stride,screen,palette=pcx(name);assert (width,height)==(640,480)
        def rgba(index):
            r,g,b=palette[index*3:index*3+3]
            return (r>>3)<<11 | (g>>3)<<6 | (b>>3)<<1 | 1
        for x,y,w,h,indices in frames:
            out.write(struct.pack('<IIII',x,y,w,h))
            for state in range(2):
                values=[0 if v==256 else rgba(remap[state*16+v] if v<16 else v) for v in indices]
                out.write(struct.pack('<'+'H'*len(values),*values))
        values=[rgba(screen[(354+y)*stride+490+x]) for y in range(40) for x in range(48)]
        out.write(struct.pack('<'+'H'*len(values),*values))
print(f'Independent PCX/GRP palette/coverage oracle: {count} frames, three races, enabled/disabled')
