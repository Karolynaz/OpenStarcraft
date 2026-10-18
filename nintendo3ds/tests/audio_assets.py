#!/usr/bin/env python3
"""Check packaged PCM against independently decoded WAVs and all effect bounds."""
import pathlib,sys,struct,subprocess,hashlib,json
src,out=map(pathlib.Path,sys.argv[1:]);data=(out/'effects.bin').read_bytes()
magic,n,size=struct.unpack_from('<8sII',data)
width=struct.unpack_from('<I',data,16)[0] if magic==b'OBWAUD2\0' else 2
header=20 if magic==b'OBWAUD2\0' else 16
assert magic in (b'OBWAUD1\0',b'OBWAUD2\0') and width in (1,2) and 0<n<=1200 and size<=8*1024*1024 and len(data)==header+20*n+size
payload=data[header+20*n:];ids=set();end=0
for i in range(n):
    ident,offset,samples,priority,flags=struct.unpack_from('<IIIII',data,header+20*i)
    assert ident not in ids and ident<1200 and offset==end and 0<samples*width<=256*1024
    pcm=payload[offset:offset+samples*width];assert len(pcm)==samples*width and any(pcm)
    ref=subprocess.check_output(['ffmpeg','-v','error','-i',str(src/f'{ident}.wav'),'-f','s8' if width==1 else 's16le','-ac','1','-ar','22050','-'])
    assert ref==pcm;ids.add(ident);end=offset+len(pcm)
assert end==size
for i in range(1,4):
    ref=subprocess.check_output(['ffmpeg','-v','error','-i',str(src/f'terran{i}.wav'),'-f','s16le','-ac','2','-ar','22050','-'])
    music=(out/f'terran{i}.pcm').read_bytes();assert len(music)%4==0 and any(music) and music==ref
print(f'Audio assets PASS: {n} complete effects, 3 complete non-silent music tracks, exact PCM equality')
