#!/usr/bin/env python3
"""Prepare private per-mission banks; CHK string IDs are local to each map."""
import argparse, hashlib, json, struct
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('sd_root',type=Path);p.add_argument('prepared_root',type=Path);a=p.parse_args()
def read(path):
 raw=path.read_bytes();magic,count,size,width=struct.unpack_from('<8sIII',raw)
 if magic!=b'OBWAUD2\0' or width!=1 or count>1200 or len(raw)!=20+count*20+size:raise ValueError(f'Invalid bank: {path}')
 out={};start=20+count*20
 for i in range(count):
  ident,offset,samples,priority,flags=struct.unpack_from('<IIIII',raw,20+i*20)
  if ident>=2300 or not 0<samples<=(2*1024*1024 if ident>=1200 else 256*1024) or offset+samples>size or ident in out:raise ValueError(f'Invalid effect: {path}/{ident}')
  out[ident]=(samples,priority,flags,raw[start+offset:start+offset+samples])
 return out
base={i:v for i,v in read(a.sd_root/'audio/effects-all.bin').items() if i<1200}
manifest={}
for key in ('terran01','zerg01','protoss01','expprotoss01','expterran01','expzerg01'):
 entries=dict(base)
 for ident,entry in read(a.prepared_root/key/'prepared/effects.bin').items():
  if ident in entries and entries[ident]!=entry:raise ValueError(f'Conflicting native sound: {key}/{ident}')
  entries[ident]=entry
 payload=bytearray();records=bytearray()
 for ident,(samples,priority,flags,pcm) in sorted(entries.items()):records+=struct.pack('<IIIII',ident,len(payload),samples,priority,flags);payload+=pcm
 if len(payload)>64*1024*1024 or len(entries)>1200:raise ValueError('Bounded native bank size exceeded')
 raw=struct.pack('<8sIII',b'OBWAUD2\0',len(entries),len(payload),1)+records+payload
 name=f'{key}-effects.bin';(a.sd_root/'campaign'/name).write_bytes(raw)
 manifest[name]={'sha256':hashlib.sha256(raw).hexdigest(),'bytes':len(raw),'effects':len(entries),'mission_speech':sum(i>=1200 for i in entries)}
 print(key,len(entries),len(payload),'PCM bytes')
(a.sd_root/'campaign/audio-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
