// Original Into the Flames: paid economy, timed rear-force arrival, Fenix
// survival, Scarab production and the original Cerebrate victory trigger.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::protoss_campaign_2));assert(e.start_recording("/tmp/sc3ds-campaign-playthrough-protoss02"));e.set_fog_enabled(false);
unsigned pending=0,build_at=0;bool attack=false,fenix_arrived=false;int home_x=0,home_y=0;
for(unsigned frame=0;frame<120000 && e.snapshot().winner<0;++frame){assert(e.tick());if(frame%48)continue;UnitView list[1700];unsigned n=e.visible_units(list,1700);auto s=e.snapshot();std::vector<UnitView> workers,parents,minerals,enemies;
for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==64 && u.completed)workers.push_back(u);if(u.type==154){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==6)enemies.push_back(u);}
assert(home_x);if(frame==48)assert(s.countdown_active && s.countdown_seconds>890);
if(frame%240==0)for(const auto& u:parents)if(u.type==77){fenix_arrived=true;if(e.select(u.x,u.y))e.move_selected(home_x,home_y-160);}
unsigned army=e.count_units(0,65)+e.count_units(0,66);if(army>=24)attack=true;
if(attack && frame%240==0 && !enemies.empty()){const UnitView* target=&enemies.front();int distance=0x7fffffff;for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}if(frame>=21600)for(const auto& u:enemies)if(u.type==151){target=&u;break;}for(const auto& u:parents)if((u.type==65 || u.type==66) && e.select(u.x,u.y))e.command(Command::attack,target->x,target->y);}
for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){const UnitView* target=nullptr;int distance=0x7fffffff;for(const auto& m:minerals){int dx=m.x-w.x,dy=m.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&m;}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);}
if(frame%480==0 && e.count_units(0,157) && workers.size()>=7){const UnitView* gas=nullptr;for(const auto& u:parents)if(u.type==157)gas=&u;if(gas)for(unsigned i=0;i<3;++i)if(e.select(workers[i].x,workers[i].y))e.command(Command::context,gas->x,gas->y);}
if(pending && (e.count_units(0,pending) || frame>build_at+1500))pending=0;
unsigned want=s.supply_max-s.supply<5?156:!e.count_units(0,160)?160:!e.count_units(0,157)?157:!e.count_units(0,164)?164:0;
if(!pending && want && !workers.empty()){bool placed=false;if(e.select(workers.back().x,workers.back().y)){if(want==157){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(abs(x)!=ring && abs(y)!=ring)continue;int px=home_x+x*32,py=home_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}}if(placed){pending=want;build_at=frame;}}
for(const auto& p:parents)if(e.select(p.x,p.y)){if(p.type==154 && e.count_units(0,64)<10)e.perform(option_code(OptionKind::train,64));else if(p.type==83)e.perform(option_code(OptionKind::train,85));else if(p.type==160)e.perform(option_code(OptionKind::train,e.count_units(0,164) && e.count_units(0,66)<12?66:65));}
if(frame%4800==0){printf("Into Flames frame%u minerals%d gas%d army%u Fenix%u timer%u\n",e.frame(),s.minerals,s.gas,army,e.count_units(0,77),s.countdown_seconds);fflush(stdout);}
}
printf("Into Flames result%d frame%u Fenix%u\n",e.snapshot().winner,e.frame(),e.count_units(0,77));assert(e.snapshot().winner==0 && fenix_arrived && e.count_units(0,77)==1);SaveView view;assert(e.save_game("/tmp/sc3ds-protoss02-completed.sav",view));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-protoss02-completed.sav","/tmp/sc3ds-protoss02-replay","/tmp/sc3ds-protoss02-map",loaded));assert(e.state_digest()==replay.state_digest());puts("Original Protoss2 timer/reinforcements/paid combat/Fenix victory + completed save replay PASS");}
