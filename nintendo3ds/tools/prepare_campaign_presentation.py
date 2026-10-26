#!/usr/bin/env python3
"""Build private campaign shots/briefing frames from owned extracted resources.
Usage: prepare_campaign_presentation.py RAW STAGED_ROOT
Extract RAW using campaign_presentation_export; originals remain private.
"""
from pathlib import Path
from PIL import Image
import struct,re,sys,json,hashlib,subprocess,io
raw,root=map(Path,sys.argv[1:]);campaign=root/'campaign'
def bank(path,items,side=512,height=256,positions=None):
 pages=[];entries=[];x=y=row=0
 for index,(name,im) in enumerate(items):
  im=im.convert('RGBA');w,h=im.size
  if positions is not None:
   x,y=positions[index];assert x+w<=side and y+h<=height
   if not pages:pages.append(Image.new('RGBA',(side,height)))
   pages[0].paste(im,(x,y));entries.append((name,0,x,y,w,h));continue
  if x+w>side:x=0;y+=row;row=0
  if not pages or y+h>height:pages.append(Image.new('RGBA',(side,height)));x=y=row=0
  pages[-1].paste(im,(x,y));entries.append((name,len(pages)-1,x,y,w,h));x+=w+1;row=max(row,h+1)
 with path.open('wb') as f:
  f.write(b'OB3UI001'+struct.pack('<4I',side,height,len(pages),len(entries)))
  for name,*v in entries:f.write(name.encode().ljust(40,b'\0')+struct.pack('<5I',*v))
  for p in pages:
   b=bytearray(side*height*2)
   for yy in range(height):
    for xx in range(side):
     r,g,blue,a=p.getpixel((xx,yy));v=(r>>3)<<11|(g>>3)<<6|(blue>>3)<<1|(a>=128)
     m=(xx&1)|((yy&1)<<1)|((xx&2)<<1)|((yy&2)<<2)|((xx&4)<<2)|((yy&4)<<3)
     struct.pack_into('<H',b,(((yy//8)*(side//8)+xx//8)*64+m)*2,v)
   f.write(b)
def owned(name):return raw/name.lower().replace('\\','_').replace('/','_')
def pages(name):
 result=[]
 for page in owned(name).read_text().split('</PAGE>'):
  bg=re.search(r'</BACKGROUND\s+([^>]+)>',page,re.I)
  if not bg:continue
  text=re.sub(r'</[^>]*>','',page).strip();result.append((bg[1],text))
 return result
summaries={
 'terran02':'Raynor has helped your colonists survive the journey. A nearby outpost has fallen silent as the alien infestation spreads. Investigate Backwater Station and bring its surviving defenders home.',
 'terran03':'The infestation on Mar Sara is spreading beyond the Confederacy\'s control. Your colonists need to evacuate. Hold your position until the rescue transports arrive.',
 'terran04':'After leaving Mar Sara, Raynor and the Sons of Korhal need answers about the Confederacy\'s involvement with the Zerg. Infiltrate the Jacobs Installation and recover the data held inside.',
 'zerg02':'The young Cerebrate has established its brood on Tarsonis. The Overmind now orders the precious Chrysalis moved to a new location. Protect it throughout the journey.',
 'zerg03':'The Chrysalis has reached Char. Establish your brood on this hostile world and protect the Overmind\'s creation while the Terrans threaten the Swarm.',
 'zerg04':'The Chrysalis is nearly ready to hatch. Terran forces are closing in on Char. Keep it safe until the new servant of the Swarm emerges.',
 'protoss02':'The Zerg invasion of Aiur continues. Fenix and the remaining defenders must hold their ground while reinforcements arrive. Prepare your defenses and strike back against the infestation.',
 'protoss03':'The Conclave demands that the Zerg be driven from Aiur. With Fenix fighting elsewhere, your forces must assault the entrenched enemy colonies and reclaim the high ground.',
 'protoss04':'The Conclave has sent you to find Tassadar. On Char, his forces remain surrounded by the Zerg. Locate the missing Executor and bring the survivors to safety.'}
portraits=list(map(int,(raw/'unit-portraits.txt').read_text().split()))
def dialog_widgets(path):
 # Read only documented classic BIN fields; positions remain in native pixels.
 # Format reference: PyMS/FileFormats/DialogBIN.py (BINWidget/BINSMK).
 data=path.read_bytes();offset=0;seen=set();widgets=[]
 def string(at):
  if not at:return ''
  assert at<len(data);return data[at:data.index(0,at)].decode('latin1')
 while True:
  assert offset not in seen and offset+86<=len(data);seen.add(offset)
  rect=struct.unpack_from('<6H',data,offset+4)
  name,flags=struct.unpack_from('<II',data,offset+20)
  identifier=struct.unpack_from('<H',data,offset+32)[0];typ=struct.unpack_from('<I',data,offset+34)[0]
  smk=struct.unpack_from('<I',data,offset+66)[0];movies=[];movie_seen=set()
  if offset:
   at=smk
   while at:
    assert at not in movie_seen and at+30<=len(data);movie_seen.add(at)
    overlay,_,_,filename,_,x,y,_,_=struct.unpack_from('<IHIIIHHII',data,at)
    movies.append((string(filename),x,y));at=overlay
  widgets.append(dict(id=identifier,type=typ,rect=rect,flags=flags,text=string(name),movies=movies))
  nxt=smk if not offset else struct.unpack_from('<I',data,offset)[0]
  if not nxt:break
  offset=nxt
 return widgets

def keyed(im,alpha=255):
 # Palette index zero is transparent; dark panel pixels are not discarded.
 mask=Image.frombytes('L',im.size,im.tobytes()).point(lambda v:0 if v==0 else alpha) if im.mode=='P' else im.convert('L').point(lambda v:alpha if v else 0)
 out=im.convert('RGBA');out.putalpha(mask.convert('L'));return out

def movie_still(name):
 data=subprocess.check_output(['ffmpeg','-v','error','-i',str(owned(name)),'-frames:v','1','-f','image2pipe','-vcodec','png','-'])
 return keyed(Image.open(io.BytesIO(data)))

for race,code,stem in [('terran','t','terr'),('zerg','z','zerg'),('protoss','p','prot')]:
 widgets=dialog_widgets(owned(f'rez/glurdy{code}.bin'))
 room=Image.open(owned(f'glue/palr{code}/backgnd.pcx')).convert('RGBA')
 for widget in widgets[1:]:
  if not widget['flags']&8:continue
  x,y=widget['rect'][:2]
  for name,ox,oy in widget['movies']:
   room.alpha_composite(movie_still(name),(x+ox,y+oy))
  if widget['type']==5 and widget['text']:
   sprite=keyed(Image.open(owned(widget['text'])))
   room.alpha_composite(sprite,(x,y))
 # Compose at the original 640x480 first, then scale the entire scene once.
 # No frame is moved, mirrored, independently resized or removed.
 room=room.resize((320,240),Image.Resampling.LANCZOS)
 items=[('room',room)]
 for i in range(4):
  frame=keyed(Image.open(owned(f'glue/ready{code}/{code}frameh{i+1}.pcx'))).resize((64,59),Image.Resampling.LANCZOS)
  items.append((f'active{i}',frame))
 bank(campaign/f'briefing-{race}.bin',items,positions=[(0,0),(320,0),(384,0),(320,59),(384,59)])
 room.save(raw/f'briefing-{race}-preview.png')
 (raw/f'layout-{code}.json').write_text(json.dumps(widgets,indent=2))
# Restore the same campaign metal decorations used by the earlier top dialog.
# A small retained frame texture stays inside original-model linear headroom.
frame_source=owned('glue/palcs/backgnd.pcx')
frame=keyed(Image.open(frame_source)).resize((128,128),Image.Resampling.LANCZOS)
bank(campaign/'briefing-frame.bin',[('frame',frame)],128,128)
cat=Path(__file__).parents[1]/'include/campaign_catalogue.h'
for key,title,scenario,race,bw in re.findall(r'\{"([^"]+)","([^"]+)",(\d+),(\d+),(true|false)\}',cat.read_text()):
 race=int(race);bw=bw=='true';r=['z','t','p'][race];number=key[-2:]
 shots=pages(f'rez/est{r}{number}{"x" if bw else ""}.txt')
 bg,location=next((s for s in reversed(shots) if 'blank.pcx' not in s[0].lower()),shots[-1])
 story='\n\n'.join(t for b,t in shots if 'blank.pcx' in b.lower())
 if not story and key=='terran01':story=pages('rez/estt0t.txt')[0][1]
 if not story:story=summaries[key]
 (campaign/f'{key}.intro').write_text(story)
 (campaign/f'{key}.location').write_text(location)
 im=Image.open(owned(bg)).convert('RGBA');im=im.resize((400,round(im.height*400/im.width)),Image.Resampling.LANCZOS)
 y=(im.height-240)//2;im=im.crop((0,y,400,y+240));bank(campaign/f'{key}-shot.bin',[('shot',im.resize((256,128),Image.Resampling.LANCZOS))],256,128)
 # Preserve source action order, speaker slot assignments and timings. Original
 # type 4 objectives are metadata, never inserted in the spoken dialogue.
 chk=(campaign/f'{key}.chk').read_bytes();at=0;meta=[];casts=[]
 while at+8<=len(chk):
  tag,n=struct.unpack_from('<4sI',chk,at);at+=8
  if tag==b'MBRF':
   slots={}
   for j in range(0,n,2400):
    for a in range(64):
     p=chk[at+j+320+a*32:at+j+352+a*32];typ=p[26];slot=struct.unpack_from('<I',p,16)[0]
     if p[28]&2:continue
     if typ==5:slots[slot]=struct.unpack_from('<H',p,24)[0]
     if typ==6:slots.pop(slot,None)
     if typ in (4,8) and struct.unpack_from('<I',p,4)[0]:
      unit=slots.get(slot,228);portrait=portraits[unit] if unit<228 else 110
      meta.append((typ,portrait,struct.unpack_from('<I',p,12)[0]))
      casts.append(tuple(portraits[slots[i]] if i in slots and slots[i]<228 else 110 for i in range(4))+(slot if typ==8 and slot<4 else 4,))
  at+=n
 brief=(campaign/f'{key}.brief').read_text().split('\f');assert len(meta)==len(brief),(key,len(meta),len(brief))
 (campaign/f'{key}.brief-cast').write_text(''.join(' '.join(map(str,c))+'\n' for c in casts))
 (campaign/f'{key}.brief-info').write_text(''.join(f'{t} {p} {ms}\n' for t,p,ms in meta))
manifest={p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in campaign.iterdir() if p.suffix in ('.intro','.location','.brief-info','.brief-cast') or p.name.endswith('-shot.bin') or p.name.startswith('briefing-')}
(campaign/'presentation-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'{len(manifest)} private presentation resources prepared')
