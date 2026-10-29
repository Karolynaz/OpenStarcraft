#!/usr/bin/env python3
"""Validate the privately extracted enabled campaign catalogue and record their hashes."""
import argparse, hashlib, json,struct,re
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('directory',type=Path);a=p.parse_args();manifest={}
catalogue=(Path(__file__).parents[1]/'include/campaign_catalogue.h').read_text()
missions=re.findall(r'\{"([^"\n]+)","[^"\n]+",(\d+),(\d+),(?:true|false)\}',catalogue)
assert missions
for key,scenario,race in missions:
 race=int(race)
 data=(a.directory/f'{key}.chk').read_bytes();at=0;chunks={}
 while at+8<=len(data):
  tag,n=struct.unpack_from('<4sI',data,at);at+=8
  if n>len(data)-at:raise ValueError('CHK bounds')
  chunks[tag]=data[at:at+n];at+=n
 if at!=len(data):raise ValueError('CHK trailing data')
 owners=chunks[b'OWNR'];side=chunks[b'SIDE'];dim=struct.unpack('<HH',chunks[b'DIM '])
 assert owners[0]==6 and owners[:8].count(6)==1 and side[0]==race and 0<min(dim) and max(dim)<=256
 assert len(chunks[b'TRIG'])%2400==0 and len(chunks[b'UNIT'])%36==0
 pages=(a.directory/f'{key}.brief').read_text().split('\f');voices=[int(v) for v in (a.directory/f'{key}.brief-audio').read_text().splitlines()]
 assert len(pages)==len(voices) and all(v==0 or 1200<=v<2300 for v in voices)
 for suffix in ('.chk','.brief','.brief-audio'):
  name=key+suffix;b=(a.directory/name).read_bytes();assert b
  manifest[name]={'sha256':hashlib.sha256(b).hexdigest(),'bytes':len(b)}
 manifest[key+'.chk'].update({'dimensions':dim,'human':0,'race':race,'scenario':int(scenario)})
 print(key,dim,'human0','race',race)
(a.directory/'maps-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
