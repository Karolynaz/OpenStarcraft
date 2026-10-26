// Public-control playthrough: original map, paid economy, rescue and combat.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_campaign_2));assert(e.start_recording("/tmp/sc3ds-terran02-session"));e.set_fog_enabled(false);unsigned pending=0,pending_at=0;bool attack=false;unsigned voices=0;
for(unsigned frame=0;frame<100000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}SoundEvent sounds[256];auto ns=e.drain_sounds(sounds,256);for(unsigned i=0;i<ns;++i)if(sounds[i].id>=1200)++voices;if(frame%48)continue;
UnitView units[1700];unsigned n=e.visible_units(units,1700);std::vector<UnitView> workers,parents,enemies;int home_x=192,home_y=1616;
for(unsigned i=0;i<n;++i){const auto& u=units[i];if(u.owner==0 && u.completed){parents.push_back(u);if(u.type==7)workers.push_back(u);}if(u.owner==4)enemies.push_back(u);}
// Starting Raynor is a rescuable ally beside the initial force.
if(!e.count_units(0,19)){for(const auto& u:parents)if(u.type==0 && e.select(u.x,u.y))e.command(Command::attack,290,1796);}
for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){const UnitView* target=nullptr;int distance=0x7fffffff;for(unsigned i=0;i<n;++i)if(units[i].mineral){int dx=units[i].x-w.x,dy=units[i].y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&units[i];}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);}
if(pending && (e.count_units(0,pending) || frame>pending_at+1500))pending=0;
auto s=e.snapshot();unsigned want=s.supply_max-s.supply<5?109:0;if(!pending && want && !workers.empty()){bool placed=false;if(e.select(workers.back().x,workers.back().y))for(int ring=2;ring<10 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(abs(x)!=ring && abs(y)!=ring)continue;int px=home_x+x*32,py=home_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}if(placed){pending=want;pending_at=frame;}}
for(const auto& p:parents)if(e.select(p.x,p.y)){if(p.type==106 && e.count_units(0,7)<10)e.perform(option_code(OptionKind::train,7));else if(p.type==111)e.perform(option_code(OptionKind::train,0));}
if(e.count_units(0,0)>=24)attack=true;
if(attack && frame%240==0 && !enemies.empty()){const UnitView* target=&enemies[0];int distance=0x7fffffff;for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}for(const auto& u:parents)if((u.type==0 || u.type==32) && e.select(u.x,u.y))e.command(Command::attack,target->x,target->y);}
if(frame%4800==0){printf("Backwater frame%u minerals%d marines%u Raynor%u infestation%u attack%d\n",e.frame(),s.minerals,e.count_units(0,0),e.count_units(0,19),e.count_units(4,130),attack);fflush(stdout);}
}
printf("Backwater result%d frame%u Raynor%u transmissions%u\n",e.snapshot().winner,e.frame(),e.count_units(0,19),voices);assert(e.snapshot().winner==0 && e.count_units(0,19)==1 && voices>=5);SaveView view;assert(e.save_game("/tmp/sc3ds-terran02-completed.sav",view));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-terran02-completed.sav","/tmp/sc3ds-terran02-replay","/tmp/sc3ds-terran02-map",loaded));assert(e.state_digest()==replay.state_digest());puts("Backwater Station paid playthrough / surviving Raynor / original victory dialogue / save replay PASS");}
