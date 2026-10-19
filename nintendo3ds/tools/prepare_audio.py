#!/usr/bin/env python3
"""Offline conversion: private classic WAV -> bounded DSP PCM assets; no runtime decode."""
import argparse, json, pathlib, struct, subprocess, hashlib
p=argparse.ArgumentParser();p.add_argument('source',type=pathlib.Path);p.add_argument('output',type=pathlib.Path);p.add_argument('--race',default='terran',choices=['terran','protoss','zerg']);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True)
entries=[];payload=bytearray()
for line in (a.source/'manifest.tsv').read_text().splitlines():
    ident,priority,flags,name=line.split('\t');ident,priority,flags=map(int,(ident,priority,flags))
    pcm=subprocess.check_output(['ffmpeg','-v','error','-i',str(a.source/f'{ident}.wav'),'-f','s8','-ac','1','-ar','22050','-'])
    if not pcm or len(pcm)>256*1024: raise ValueError(f'Unbounded effect {ident}')
    entries.append((ident,len(payload),len(pcm),priority,flags));payload+=pcm
if len(payload)>8*1024*1024: raise ValueError('SFX exceed normal-heap budget')
header=struct.pack('<8sIII',b'OBWAUD2\0',len(entries),len(payload),1)
(a.output/'effects.bin').write_bytes(header+b''.join(struct.pack('<IIIII',*e) for e in entries)+payload)
tracks=[]
for n in range(1,4):
    target=a.output/f'{a.race}{n}.pcm'
    subprocess.run(['ffmpeg','-v','error','-y','-i',str(a.source/f'{a.race}{n}.wav'),'-f','s16le','-ac','2','-ar','22050',str(target)],check=True)
    if not target.stat().st_size or target.stat().st_size%4: raise ValueError('Invalid music')
    tracks.append({'track':n,'bytes':target.stat().st_size,'sha256':hashlib.sha256(target.read_bytes()).hexdigest()})
(a.output/'manifest.json').write_text(json.dumps({'race':a.race,'rate':22050,'sample_bytes':1,'effects':len(entries),'effect_pcm_bytes':len(payload),'longest_effect_bytes':max(e[2] for e in entries),'music':tracks},indent=2)+'\n')
print((a.output/'manifest.json').read_text())
