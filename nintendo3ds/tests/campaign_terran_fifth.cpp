// Public-control playthrough: original map, paid economy, rescue and combat.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
#include <set>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_campaign_5));assert(e.start_recording("/tmp/sc3ds-terran05-session"));e.set_fog_enabled(false);unsigned pending=0,pending_at=0;bool attack=false;unsigned voices=0;bool cloaked=false,objectives_changed=false;std::set<unsigned> gas_ids;
for(unsigned frame=0;frame<200000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}SoundEvent sounds[256];auto ns=e.drain_sounds(sounds,256);for(unsigned i=0;i<ns;++i)if(sounds[i].id>=1200)++voices;if(frame%48)continue;
UnitView units[1700];unsigned n=e.visible_units(units,1700);std::vector<UnitView> workers,parents,enemies;int home_x=2720,home_y=464;
for(unsigned i=0;i<n;++i){const auto& u=units[i];if(u.owner==0 && u.completed){parents.push_back(u);if(u.type==7)workers.push_back(u);}if(u.owner==6 && u.type>=106 && u.type<176)enemies.push_back(u);}
// Preserve Raynor while the original rescue/infiltration triggers unlock the colony.
if(!e.count_units(0,16) || !e.count_units(0,106)){
 int tx=!e.count_units(0,16)?149:2720,ty=!e.count_units(0,16)?1233:464;
 if(frame%240==0)for(const auto& u:parents)if((u.type==0 || u.type==16) && e.select(u.x,u.y)){if(u.type==16 && !cloaked){MenuOption rows[32];auto count=e.options(OptionPage::primary,rows,32);bool offered=false;for(unsigned j=0;j<count;++j)if(rows[j].code==option_code(OptionKind::cloak,0) && rows[j].enabled)offered=true;assert(offered);cloaked=e.perform(option_code(OptionKind::cloak,0));assert(cloaked);}if(u.type==16)e.move_selected(tx,ty);else e.command(Command::attack,tx,ty);}
 if(frame%2400==0)for(const auto& k:parents)if(k.type==16)printf("K pos%d,%d order%u cloak%d\n",k.x,k.y,k.order,cloaked);if(frame%2400==0){printf("rescue frame%u Kerrigan%u CC%u marines%u winner%d\n",e.frame(),e.count_units(0,16),e.count_units(0,106),e.count_units(0,0),e.snapshot().winner);fflush(stdout);}continue;
}
objectives_changed|=e.snapshot().objectives.find("Destroy the Confederate base")!=std::string::npos;
if(frame%240==0)for(const auto& u:parents)if((u.type==19 || u.type==16) && e.select(u.x,u.y))e.move_selected(home_x-64,home_y+256);
const UnitView* refinery=nullptr;for(const auto& q:parents)if(q.type==110)refinery=&q;
for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){const UnitView* target=nullptr;auto wid=e.own_unit_at(w.x,w.y);if(refinery && (gas_ids.count(wid) || gas_ids.size()<3)){target=refinery;gas_ids.insert(wid);}int distance=target?0:0x7fffffff;for(unsigned i=0;i<n;++i)if(units[i].mineral){int dx=units[i].x-w.x,dy=units[i].y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&units[i];}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);}
if(pending && (e.count_units(0,pending) || frame>pending_at+1500))pending=0;
auto s=e.snapshot();unsigned want=s.supply_max-s.supply<6?109:e.count_units(0,114)<3 && s.minerals>=300?114:0;if(!pending && want && !workers.empty()){bool placed=false;if(e.select(workers.back().x,workers.back().y))for(int ring=2;ring<10 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(abs(x)!=ring && abs(y)!=ring)continue;int px=home_x+x*32,py=home_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}if(placed){pending=want;pending_at=frame;}}
for(const auto& p:parents)if(e.select(p.x,p.y)){if(p.type==106 && e.count_units(0,7)<16)e.perform(option_code(OptionKind::train,7));else if(p.type==111 && e.count_units(0,0)<20)e.perform(option_code(OptionKind::train,0));else if(p.type==113 && e.count_units(0,2)<6)e.perform(option_code(OptionKind::train,2));else if(p.type==114)e.perform(option_code(OptionKind::train,8));}
if(e.count_units(0,8)>=20)attack=true;
if(attack && frame%240==0 && !enemies.empty()){const UnitView* target=&enemies[0];int distance=0x7fffffff;for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}for(const auto& u:parents)if((u.type==8) && e.select(u.x,u.y))e.command(Command::attack,target->x,target->y);}
if(frame%4800==0){printf("Revolution frame%u minerals%d marines%u Raynor%u wraith%u gas%d enemyCC%u attack%d\n",e.frame(),s.minerals,e.count_units(0,0),e.count_units(0,19),e.count_units(0,8),s.gas,e.count_units(6,106),attack);fflush(stdout);}
}
printf("Revolution result%d frame%u Raynor%u transmissions%u\n",e.snapshot().winner,e.frame(),e.count_units(0,19),voices);assert(e.snapshot().winner==0 && e.count_units(0,19)==1 && e.count_units(0,16)==1 && objectives_changed && cloaked && voices>=10);SaveView view;assert(e.save_game("/tmp/sc3ds-terran05-completed.sav",view));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-terran05-completed.sav","/tmp/sc3ds-terran05-replay","/tmp/sc3ds-terran05-map",loaded));assert(e.state_digest()==replay.state_digest());puts("Revolution original rescue/infiltration/colony transfer, paid mining/air fleet, both heroes survive, original victory and exact completed-save replay PASS");}
