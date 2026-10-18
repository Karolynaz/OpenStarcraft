#!/usr/bin/env python3
"""Check packaged PCM against independently decoded WAVs and all effect bounds."""
import pathlib,sys,struct,subprocess,hashlib,json
src,out=map(pathlib.Path,sys.argv[1:]);data=(out/'effects.bin').read_bytes()
magic,n,size=struct.unpack_from('<8sII',data)
assert magic==b'OBWAUD1\0' and 0<n<=1200 and size<=4*1024*1024 and len(data)==16+20*n+size
payload=data[16+20*n:];ids=set();end=0
for i in range(n):
    ident,offset,samples,priority,flags=struct.unpack_from('<IIIII',data,16+20*i)
    assert ident not in ids and ident<1200 and offset==end and 0<samples*2<=128*1024
    pcm=payload[offset:offset+samples*2];assert len(pcm)==samples*2 and any(pcm)
    ref=subprocess.check_output(['ffmpeg','-v','error','-i',str(src/f'{ident}.wav'),'-f','s16le','-ac','1','-ar','22050','-'])
    assert ref==pcm;ids.add(ident);end=offset+len(pcm)
assert end==size
for i in range(1,4):
    ref=subprocess.check_output(['ffmpeg','-v','error','-i',str(src/f'terran{i}.wav'),'-f','s16le','-ac','2','-ar','22050','-'])
    music=(out/f'terran{i}.pcm').read_bytes();assert len(music)%4==0 and any(music) and music==ref
print(f'Audio assets PASS: {n} complete effects, 3 complete non-silent music tracks, exact PCM equality')
