// Original Egression completed by actual pickup/escort/beacon movement.
#include "engine.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::zerg_campaign_2));assert(e.start_recording("/tmp/sc3ds-chrysalis-session"));e.set_fog_enabled(false);unsigned stage=0;bool picked=false,saved=false;
const int route[][2]={{335,123},{2416,2432},{2416,2432},{2416,2432}};
for(unsigned frame=0;frame<20000 && e.snapshot().winner<0;++frame){assert(e.tick());if(frame%24)continue;UnitView list[1700];unsigned n=e.visible_units(list,1700);if(frame==0)for(unsigned i=0;i<n;++i)if(list[i].type==216)printf("Artifact xy%d,%d order%u owner%u\n",list[i].x,list[i].y,list[i].order,list[i].owner);const UnitView* carrier=nullptr;int best=0x7fffffff;for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==41){int dx=list[i].x-route[stage][0],dy=list[i].y-route[stage][1],d=dx*dx+dy*dy;if(list[i].carried_powerup==216){carrier=&list[i];break;}if(stage==0 && d<best){best=d;carrier=&list[i];}}
if(!carrier){printf("Carrier lost at frame%u stage%u\n",e.frame(),stage);for(unsigned i=0;i<n;++i)if(list[i].type==41 || list[i].type==216)printf("unit%u owner%u xy%d,%d hp%d held%u order%u\n",list[i].type,list[i].owner,list[i].x,list[i].y,list[i].hp,list[i].carried_powerup,list[i].order);return 1;}
if(carrier->carried_powerup==216){picked=true;if(stage==0)stage=1;if(!saved){SaveView v;v.camera_x=200;v.camera_y=0;v.cursor_x=195;v.cursor_y=86;assert(e.save_game("/tmp/sc3ds-chrysalis-held.sav",v));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-chrysalis-held.sav","/tmp/sc3ds-chrysalis-replay","/tmp/sc3ds-chrysalis-map",loaded));assert(e.state_digest()==replay.state_digest());saved=true;}}
int dx=carrier->x-route[stage][0],dy=carrier->y-route[stage][1];if(stage && stage<3 && dx*dx+dy*dy<96*96)++stage;
assert(e.select(carrier->x,carrier->y));if(!stage)e.command(Command::context,335,123);else if(frame>5000)e.move_selected(route[stage][0],route[stage][1]);else e.command(Command::stop,carrier->x,carrier->y);
if(frame%240==0)for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==53 || list[i].type==38 || list[i].type==43) && e.select(list[i].x,list[i].y)){if(frame<600 && list[i].type==43)e.move_selected(560,220);else if(frame>=600)e.command(Command::attack,2416,2432);}
if(frame%1200==0){printf("Chrysalis frame%u stage%u xy%d,%d hp%d carrying%u\n",e.frame(),stage,carrier->x,carrier->y,carrier->hp,carrier->carried_powerup);fflush(stdout);}
}
printf("Egression result%d frame%u picked%d saved%d\n",e.snapshot().winner,e.frame(),picked,saved);assert(picked && saved && e.snapshot().winner==0);SaveView v;assert(e.save_game("/tmp/sc3ds-zerg02-completed.sav",v));puts("Original Chrysalis pickup/escort/beacon/victory and held-item save replay PASS");}
