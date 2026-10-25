#!/usr/bin/env python3
"""Prepare private short hero/mission speech for the existing bounded SFX mixer."""
import argparse,hashlib,json,struct,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True);payload=bytearray();records=bytearray();entries=[]
for line in (a.source/'manifest.tsv').read_text().splitlines():
    ident,priority,flags,name=line.split('\t');ident,priority,flags=map(int,(ident,priority,flags))
    pcm=subprocess.check_output(['ffmpeg','-v','error','-i',str(a.source/f'{ident}.wav'),'-f','s8','-ac','1','-ar','22050','-'])
    if not 0<len(pcm)<=256*1024:raise ValueError(f'Speech exceeds bounded voice: {ident}')
    records+=struct.pack('<IIIII',ident,len(payload),len(pcm),priority,flags);payload+=pcm
    entries.append({'id':ident,'samples':len(pcm),'source':name})
result=struct.pack('<8sIII',b'OBWAUD2\0',len(entries),len(payload),1)+records+payload
(a.output/'effects.bin').write_bytes(result)
(a.output/'manifest.json').write_text(json.dumps({'effects':entries,'sha256':hashlib.sha256(result).hexdigest(),'rate':22050,'sample_bytes':1},indent=2)+'\n')
print(f'{len(entries)} hero/mission effects, {len(payload)} PCM bytes')
