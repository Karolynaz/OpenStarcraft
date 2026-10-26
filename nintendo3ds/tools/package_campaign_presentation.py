#!/usr/bin/env python3
"""Private presentation-only overlay; requires the existing campaign data tree."""
from pathlib import Path
import argparse,json,hashlib,zipfile
p=argparse.ArgumentParser(description=__doc__);p.add_argument('stage',type=Path);p.add_argument('executable',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
manifest=json.loads((a.stage/'campaign/presentation-manifest.json').read_text())
a.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(a.output,'w',compression=zipfile.ZIP_DEFLATED) as z:
 z.write(a.executable,'3ds/openbw/openbw-3ds.3dsx')
 z.write(a.stage/'campaign/presentation-manifest.json','3ds/openbw/campaign/presentation-manifest.json')
 for name,info in manifest.items():
  assert Path(name).name==name
  f=a.stage/'campaign'/name;assert f.stat().st_size==info['bytes'] and hashlib.sha256(f.read_bytes()).hexdigest()==info['sha256']
  z.write(f,'3ds/openbw/campaign/'+name)
 z.writestr('PRESENTATION-UPDATE.txt','Private update: overlay on the existing full campaign data tree. Includes executable, owned intro/shot/room assets and briefing metadata for 15 missions. No profiles, progress or saves. Keep existing portraits, campaign maps, dialogue audio and UI assets. Not a standalone installation.\n')
with zipfile.ZipFile(a.output) as z:
 assert z.testzip() is None
 assert len(z.namelist())==len(manifest)+3
 assert z.read('3ds/openbw/openbw-3ds.3dsx')==a.executable.read_bytes()
 assert not any(x.endswith(('player1.cfg','progress.cfg','.ob3')) for x in z.namelist())
print(json.dumps({'path':str(a.output),'bytes':a.output.stat().st_size,'sha256':hashlib.sha256(a.output.read_bytes()).hexdigest(),'resources':len(manifest),'verified':True},indent=2))
