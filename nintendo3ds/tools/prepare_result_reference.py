#!/usr/bin/env python3
"""Export privately owned classic results PCXs as native PNG drawing references.
Usage: prepare_result_reference.py EXTRACTED_PCX_DIRECTORY REFERENCE_DIRECTORY
Does not alter gameplay assets or overwrite artist redraws.
"""
from pathlib import Path
from PIL import Image, ImageDraw
import argparse, hashlib, json
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('raw',type=Path);p.add_argument('reference',type=Path);a=p.parse_args()
folder=a.reference/'03 laimejus pralaimejus';folder.mkdir(parents=True,exist_ok=True)
preview=a.reference/'400x240 perziuros'/'03 laimejus pralaimejus';preview.mkdir(parents=True,exist_ok=True)
entries=[];scenes=[]
for race,code in [('terran','t'),('protoss','p'),('zerg','z')]:
 for result,suffix in [('laimejus','v'),('pralaimejus','d')]:
  source=f'glue/pal{code}{suffix}/backgnd.pcx';raw=a.raw/source.replace('/','_')
  im=Image.open(raw).convert('RGB');assert im.size==(640,480)
  name=f'{race}-{result}.png';out=folder/name;im.save(out)
  assert Image.open(out).convert('RGB').tobytes()==im.tobytes()
  scaled=im.resize((400,300),Image.Resampling.LANCZOS).crop((0,30,400,270));scaled.save(preview/name)
  scenes.append((name,im))
  entries.append(dict(file=str(out.relative_to(a.reference)),source=source,width=640,height=480,source_sha256=hashlib.sha256(raw.read_bytes()).hexdigest(),purpose='Originalus rezultato fonas be kintamo teksto ir statistikos.'))
  for part in ('scorebox','pinset','untab'):
   source=f'glue/score{code}{suffix}/{part}.pcx';raw=a.raw/source.replace('/','_');piece=Image.open(raw).convert('RGB')
   target=folder/'UI dalys'/f'{race}-{result}-{part}.png';target.parent.mkdir(exist_ok=True);piece.save(target)
   assert Image.open(target).convert('RGB').tobytes()==piece.tobytes()
   entries.append(dict(file=str(target.relative_to(a.reference)),source=source,width=piece.width,height=piece.height,source_sha256=hashlib.sha256(raw.read_bytes()).hexdigest(),purpose='Originalus rezultatu UI fragmentas, nekeiciant juodu pikseliu skaidrumo.'))
sheet=Image.new('RGB',(960,528),(20,20,25));draw=ImageDraw.Draw(sheet)
for i,(name,im) in enumerate(scenes):
 im=im.resize((320,240),Image.Resampling.LANCZOS);x=i%3*320;y=i//3*264;sheet.paste(im,(x,y));draw.text((x+8,y+244),name,fill='white')
sheet.save(a.reference/'REZULTATU FONAI.png')
manifest=a.reference/'paveiksliuku-sarasas.json';old=json.loads(manifest.read_text()) if manifest.exists() else []
new_paths={v['file'] for v in entries};old=[v for v in old if v['file'] not in new_paths];manifest.write_text(json.dumps(old+entries,ensure_ascii=False,indent=2)+'\n')
(folder/'SKAITYK.txt').write_text('''Originalūs tavo žaidimo rezultatų fonai (640×480), po laimėjimo ir pralaimėjimo: Terran, Protoss, Zerg. Šaltiniai: glue/paltv, paltd, palpv, palpd, palzv, palzd/backgnd.pcx.

Tai švarūs originalūs fonai, be užrašų ir žaidėjų statistikos. UI dalys kataloge yra originalūs scorebox, pinset ir untab fragmentai. Kintančius tekstus, pergalių užrašus ir skaičius vėliau pieš kodas. 400x240 peržiūros yra atskirame bendro katalogo aplanke; originalai neištempti ir neapkarpyti.

Rezultatų UI originalus bendras aprašas: rez/gluscore.bin. Dabartinis 3DS portas vis dar naudoja savo Victory/Defeat dialogą; šio eksporto failai prie žaidimo dar neprijungti. Šie rasės/baigties fonai yra rezultatų ekrano medžiaga, o ne atskiri kiekvienos kampanijos misijos paveikslai.

Perpieštus 400×240 variantus dėk į „perpiesta 400x240“, išlaikydamas failų vardus. Privatūs tavo įsigyto žaidimo duomenys.
''')
readme=a.reference/'SKAITYK MANE.md'
if readme.exists():
 text=readme.read_text().replace('**03 laimejus pralaimejus** — paaiškinimas: dabartiniame projekte atskiro rezultato fono paveiksliuko nėra.','**03 laimejus pralaimejus** — šeši originalūs Terran/Protoss/Zerg laimėjimo ir pralaimėjimo fonai (640×480), 18 rezultatų UI fragmentų ir atskiros 400×240 peržiūros. Į 3DS rezultatų dialogą šie fonai dar neįjungti.')
 if '**REZULTATU FONAI.png**' not in text:text+='\n**REZULTATU FONAI.png** — visų šešių originalių rezultatų fonų peržiūra.\n'
 readme.write_text(text)
for entry in entries:
 with Image.open(a.reference/entry['file']) as im:im.verify()
for name,_ in scenes:
 with Image.open(preview/name) as im:
  assert im.size==(400,240)
  im.verify()
print(json.dumps(dict(original_backgrounds=6,original_ui_pieces=18,previews=6,pixel_exact_native_png=True,verified=True)))
