// Unchanged owned BW Zerg2 intro. No user controls or scenario replacements:
// scripted group rendezvous must destroy the generators, deliver the SCV and
// let it pick up the neutral-slot Psi Emitter before gameplay can begin.
#include "engine.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){
 assert(argc==3);Engine e;
 assert(e.load(argv[1],argv[2],Scenario::bw_zerg_campaign_2));
 assert(e.start_recording("/tmp/sc3ds-bwz2-intro.journal"));
 bool delivered=false;
 for(unsigned tick=0;tick<6000 && !delivered;++tick){
  assert(e.tick());UnitView units[1700];auto n=e.visible_units(units,1700);
  for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==7 && units[i].carried_powerup==217)delivered=true;
 }
 assert(delivered && e.count_units(0,7)==1 && e.count_units(7,200)==0 && e.snapshot().winner<0);
 for(unsigned tick=0;tick<2400 && (e.count_units(0,41)<2 || e.snapshot().minerals<850 || e.snapshot().gas<200);++tick){if(tick%48==0){UnitView units[1700];auto n=e.visible_units(units,1700);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==7 && e.select(units[i].x,units[i].y))assert(e.move_selected(1184,3488));}assert(e.tick());}
 assert(e.count_units(0,41)==2 && e.snapshot().minerals==850 && e.snapshot().gas==200);
 SaveView view;view.camera_x=1080;view.camera_y=3330;assert(e.save_game("/tmp/sc3ds-bwz2-intro.sav",view));
 Engine replay;SaveView restored;
 assert(replay.load_save(argv[1],"/tmp/sc3ds-bwz2-intro.sav","/tmp/sc3ds-bwz2-intro.replay","/tmp/sc3ds-bwz2-intro.map",restored));
 assert(e.state_digest()==replay.state_digest());
 for(unsigned tick=0;tick<96;++tick){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}
 printf("Original BW Zerg2 autonomous intro/generators/SCV/neutral Psi Emitter/carried-item tracking/first Drones/original funds/exact active replay PASS frame%u\n",e.frame());
}
