#!/usr/bin/env python3
"""Build private campaign shots/briefing frames from owned extracted resources.
Usage: prepare_campaign_presentation.py RAW STAGED_ROOT [--screens FILLED_PNGS]
Extract RAW using campaign_presentation_export; originals remain private.
"""
from pathlib import Path
from PIL import Image
import struct,re,sys,json,hashlib,subprocess,io
import argparse
from ui_bank import bank
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('raw',type=Path);parser.add_argument('root',type=Path)
parser.add_argument('--screens',type=Path,help='Completed 800x960 outpainted PNGs for dual-screen story scenes')
parser.add_argument('--panels',type=Path,help='Native-screen layered briefing PNG exports (new-brriefing)')
args=parser.parse_args();raw,root=args.raw,args.root;campaign=root/'campaign'
def owned(name):return raw/name.lower().replace('\\','_').replace('/','_')
def pages(name):
 result=[]
 for page in owned(name).read_text(encoding='cp1252').split('</PAGE>'):
  bg=re.search(r'</BACKGROUND\s+([^>]+)>',page,re.I)
  if not bg:continue
  text=re.sub(r'</[^>]*>','',page).strip();result.append((bg[1],text))
 return result
summaries={
 'expzerg02':'The UED Psi Disrupter prevents Kerrigan from controlling the renegade Zerg on Braxis. Raynor\'s raiders will cut its power, then Mengsk\'s SCV will carry a Psi Emitter. Gather the scattered Zerg, establish a colony and destroy the Disrupter.',
 'zerg08':'With the Garm Brood purged, Kerrigan turns back to the Protoss on Char. Destroy their bases, intercept the Dark Templar before they escape, and keep Kerrigan alive.',
 'zerg07':'While Kerrigan pursued Tassadar, the Dark Templar struck down the Cerebrate Zasz. Without its master, the Garm Brood has gone berserk. Establish a colony and destroy the uncontrolled brood, including its isolated outposts.',
 'zerg06':'Kerrigan has recovered the secrets of the Ghost program. The Protoss remain on Char, and Tassadar has drawn her attention. Drive back the intruders, then guide Kerrigan to the challenge that awaits her.',
 'zerg05':'Kerrigan has emerged from the Chrysalis. To recover the secrets of her suppressed powers, she infiltrates a Terran science vessel. Guide her through the installation, unlock its passages, and reach the supercomputer.',
 'terran08':'The Confederacy has blockaded Antiga Prime. Protect Kerrigan and carry the stolen Psi Emitter into the Confederate base so the Sons of Korhal can break the blockade.',
 'terran06':'Raynor must reach the downed Norad II. Secure the Terran colony, prepare transports, and bring Raynor with two Dropships to the crash site.',
 'expprotoss02':'The survivors of Aiur have reached Shakuras, the refuge of the Dark Templar. Build a colony, find their people, and confront the Zerg that have followed the refugees through the warp gate.',
 'terran05':'The Sons of Korhal turn to Antiga Prime, where colonists are ready to rise against Confederate rule. Join Sarah Kerrigan, help the rebels take control, and protect their new foothold while Confederate forces close in.',
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
    overlay,movie_flags,_,filename,_,x,y,_,_=struct.unpack_from('<IHIIIHHII',data,at)
    movies.append((string(filename),x,y,movie_flags));at=overlay
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

race_motions={}
for race,code,stem in [('terran','t','terr'),('zerg','z','zerg'),('protoss','p','prot')]:
 widgets=dialog_widgets(owned(f'rez/glurdy{code}.bin'))
 room=Image.open(owned(f'glue/palr{code}/backgnd.pcx')).convert('RGBA')
 foreground=Image.new('RGBA',(640,480));motions=[]
 for widget in widgets[1:]:
  if not widget['flags']&8:continue
  x,y=widget['rect'][:2]
  for name,ox,oy,flags in widget['movies']:
   info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_entries','stream=width,height','-of','json',str(owned(name))]))['streams'][0]
   motions.append(dict(name=name,x=(x+ox)/2,y=(y+oy)/2,w=info['width']/2,h=info['height']/2,flags=flags,widget=widget['id']))
  if widget['type']==5 and widget['text']:
   sprite=keyed(Image.open(owned(widget['text'])))
   foreground.alpha_composite(sprite,(x,y))
 # Compose at the original 640x480 first, then scale the entire scene once.
 # No frame is moved, mirrored, independently resized or removed.
 room=room.resize((320,240),Image.Resampling.LANCZOS)
 foreground=foreground.resize((320,240),Image.Resampling.LANCZOS)
 items=[('room',room),('overlay',foreground)]
 for i in range(4):
  frame=keyed(Image.open(owned(f'glue/ready{code}/{code}frameh{i+1}.pcx'))).resize((64,59),Image.Resampling.LANCZOS)
  items.append((f'active{i}',frame))
 bank(campaign/f'briefing-{race}.bin',items,positions=[(0,0,0),(1,0,0),(0,320,0),(0,384,0),(0,320,59),(0,384,59)])
 preview=room.copy();preview.alpha_composite(foreground);preview.save(raw/f'briefing-{race}-preview.png')
 race_motions[race]=motions
 (raw/f'layout-{code}.json').write_text(json.dumps(widgets,indent=2))
# Original portrait-window decorations supply each race's ornamented frame.
# Crop away its off-window mounting arm; retain the full border and corners.
for race,code,stem in [('terran','t','terr'),('protoss','p','prot'),('zerg','z','zerg')]:
 source=Image.open(owned(f'glue/ready{code}/p4{stem}.pcx'))
 frame=keyed(source.crop((12,42,157,source.height))).resize((128,128),Image.Resampling.LANCZOS)
 frame.save(raw/f'upper-frame-{race}.png')
 bank(campaign/f'briefing-frame-{race}.bin',[('frame',frame)],128,128)

clip_cache={}
def clip(name,path,width,height,opaque=False):
 cache_key=(str(path),width,height,opaque)
 if cache_key not in clip_cache:
  ratio=min(1,128/width,128/height);w=max(1,round(width*ratio));h=max(1,round(height*ratio))
  data=subprocess.check_output(['ffmpeg','-v','error','-i',str(path),'-vf',f'fps=8,scale={w}:{h}:flags=area','-f','rawvideo','-pix_fmt','rgb24','-'])
  assert len(data)%(w*h*3)==0;frames=len(data)//(w*h*3);assert 1<=frames<=256,(path,frames)
  pixels=bytearray(len(data)//3*2)
  for i in range(0,len(data),3):
   r,g,b=data[i:i+3];struct.pack_into('<H',pixels,i//3*2,(r>>3)<<11|(g>>3)<<6|(b>>3)<<1|bool(opaque or r or g or b))
  clip_cache[cache_key]=(w,h,frames,pixels)
 return (name,*clip_cache[cache_key])
def animations(path,entries):
 assert 0<len(entries)<=32
 offset=12+len(entries)*56
 with path.open('wb') as f:
  f.write(b'OB3AN001'+struct.pack('<I',len(entries)))
  for name,w,h,n,pixels in entries:
   f.write(name.encode().ljust(40,b'\0')+struct.pack('<4I',w,h,n,offset));offset+=len(pixels)
  for name,w,h,n,pixels in entries:f.write(pixels)
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
 entries=[];motion_lines=[]
 ids=sorted({v for cast in casts for v in cast[:4] if v<110})
 for portrait in ids:
  for mode in ('idle','talk'):
   source=raw/f'portrait-{portrait}-{mode}.smk'
   if not source.exists():raise RuntimeError(f'Missing owned portrait animation {source}')
   entries.append(clip(f'p{portrait}_{mode}',source,64,59,True))
 for i,motion in enumerate(race_motions[['zerg','terran','protoss'][race]]):
  name=f'm{i}';entries.append(clip(name,owned(motion['name']),motion['w'],motion['h']))
  hover='-'
  if motion['widget']==13 and motion['name'].lower().endswith('start.smk'):
   path=owned(motion['name'][:-9]+'starton.smk')
   if path.exists():hover=f'h{i}';entries.append(clip(hover,path,motion['w'],motion['h']))
  motion_lines.append(f"{name} {hover} {motion['x']} {motion['y']} {motion['w']} {motion['h']} {motion['flags']} {motion['widget']}\n")
 animations(campaign/f'{key}-animations.bin',entries)
 (campaign/f'{key}.motion').write_text(''.join(motion_lines))
 (campaign/f'{key}.brief-cast').write_text(''.join(' '.join(map(str,c))+'\n' for c in casts))
 (campaign/f'{key}.brief-info').write_text(''.join(f'{t} {p} {ms}\n' for t,p,ms in meta))
if args.screens:
 from prepare_campaign_screens import prepare
 prepare(args.screens,root/'campaign-screen-previews',raw,root)
manifest={p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in campaign.iterdir() if p.suffix in ('.intro','.location','.brief-info','.brief-cast','.motion') or p.name.endswith(('-shot.bin','-story.bin','-animations.bin')) or p.name.startswith(('briefing-','result-'))}
(campaign/'presentation-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
if args.panels:
 from prepare_briefing_panels import prepare
 prepare(args.panels,root,root/'briefing-panel-previews')
 manifest=json.loads((campaign/'presentation-manifest.json').read_text())
print(f'{len(manifest)} private presentation resources prepared')
