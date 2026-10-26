#!/usr/bin/env python3
"""Validate six privately extracted mission openings and record their hashes."""
import argparse, hashlib, json,struct
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('directory',type=Path);a=p.parse_args();manifest={}
for key,race in [('terran01',1),('zerg01',0),('protoss01',2),('expprotoss01',2),('expterran01',1),('expzerg01',0)]:
 data=(a.directory/f'{key}.chk').read_bytes();at=0;chunks={}
 while at+8<=len(data):
  tag,n=struct.unpack_from('<4sI',data,at);at+=8
  if n>len(data)-at:raise ValueError('CHK bounds')
  chunks[tag]=data[at:at+n];at+=n
 if at!=len(data):raise ValueError('CHK trailing data')
 owners=chunks[b'OWNR'];side=chunks[b'SIDE'];dim=struct.unpack('<HH',chunks[b'DIM '])
 assert owners[0]==6 and owners[:8].count(6)==1 and side[0]==race and max(dim)<=128
 assert len(chunks[b'TRIG'])%2400==0 and len(chunks[b'UNIT'])%36==0
 pages=(a.directory/f'{key}.brief').read_text().split('\f');voices=[int(v) for v in (a.directory/f'{key}.brief-audio').read_text().splitlines()]
 assert len(pages)==len(voices) and all(v==0 or 1200<=v<2300 for v in voices)
 for suffix in ('.chk','.brief','.brief-audio'):
  name=key+suffix;b=(a.directory/name).read_bytes();assert b
  manifest[name]={'sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b)}
 manifest[key+'.chk'].update({'dimensions':dim,'human':0,'race':race})
 print(key,dim,'human0','race',race)
(a.directory/'maps-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
