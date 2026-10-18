#!/usr/bin/env python3
"""Summarize the last run in a CSV, preserving startup/catch-up/partial samples."""
import argparse,csv,json,pathlib
p=argparse.ArgumentParser();p.add_argument('csv',type=pathlib.Path);p.add_argument('--output',type=pathlib.Path);a=p.parse_args()
runs=[];run=[];header=None
for row in csv.reader(a.csv.open()):
    if row and row[0]=='scenario':
        if run:runs.append(run);run=[]
        header=row;continue
    if not row:continue
    if header is None or len(row)!=len(header):raise ValueError('Malformed CSV row')
    r=dict(zip(header,row))
    if run and (int(r['frame'])<int(run[-1]['frame']) or r['scenario']!=run[-1]['scenario']):runs.append(run);run=[]
    run.append(r)
if run:runs.append(run)
if not runs:raise ValueError('No recorded samples')
r=runs[-1];active=[x for x in r if float(x['tps'])>0]
if not active:raise ValueError('No active simulation samples')
mean=lambda rows,k:sum(float(x[k]) for x in rows)/len(rows)
peak=lambda k:max(float(x[k]) for x in r)
s={'runs_in_file':len(runs),'selected_run':'last','samples':len(r),'active_samples':len(active),
   'excluded_zero_tps_samples':len(r)-len(active),'partial_samples':'retained',
   'first_frame':int(r[0]['frame']),'last_frame':int(r[-1]['frame']),'winner':int(r[-1]['winner']),
   'mean_fps':mean(active,'fps'),'mean_tps':mean(active,'tps'),
   'peak_units':int(peak('units')),'max_tick_us':peak('max_tick_us'),
   'peak_heap_bytes':int(peak('heap_bytes')),'peak_linear_bytes':int(peak('linear_used')),
   'max_backlog':int(peak('backlog')),'missing_sprites':int(peak('missing_images'))}
combat=[x for x in active if int(x['frame'])>=5000]
if combat:s['combat_from_frame5000']={'samples':len(combat),'mean_fps':mean(combat,'fps'),'mean_tps':mean(combat,'tps')}
if 'audio_ready' in r[0]:
    keys=['sfx_played','sfx_culled','sfx_missing','sfx_voice_drops','event_drops','music_buffers','music_underruns','audio_io_errors','dsp_drops']
    s['audio']={'all_samples_ready':all(x['audio_ready']=='1' for x in r),
                **{k:int(peak(k)) for k in keys},'tracks_seen':sorted({int(x['music_track']) for x in r})}
text=json.dumps(s,indent=2)+'\n'
if a.output:a.output.write_text(text)
print(text,end='')
