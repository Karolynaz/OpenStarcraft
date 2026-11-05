// Normal paid public commands on the original map. Fog off for repeatable test navigation only.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
using namespace port3ds;using namespace bwgame;
int main(int argc,char**v){assert(argc==3);Engine e;assert(e.load(v[1],v[2],Scenario::bw_terran_campaign_3));assert(e.start_recording("/tmp/nov03-bwt3.journal"));e.set_fog_enabled(false);unsigned pending=0,pending_at=0,pending_count=0;std::set<unsigned> gas;bool attack=false;
for(unsigned frame=0;frame<180000 && e.snapshot().winner<0;++frame){if(!e.tick()){printf("ERROR %u %s\n",e.frame(),e.error());return 2;}SoundEvent sounds[256];e.drain_sounds(sounds,256);if(frame%48)continue;
UnitView list[1700];unsigned n=e.visible_units(list,1700);std::vector<UnitView> own,workers,enemies;int hx=1376,hy=3376;
for(unsigned i=0;i<n;++i){const auto& u=list[i];if(u.owner==0){if(u.completed)own.push_back(u);if(u.type==7 && u.completed)workers.push_back(u);if(u.type==106){hx=u.x;hy=u.y;}}if(u.owner>=3 && u.owner<=6 && u.type<176)enemies.push_back(u);}
if(frame%240==0 && !e.count_units(0,106))for(const auto&u:own)if((u.type==99 || u.type==0) && e.select(u.x,u.y))e.move_selected(hx,hy);
for(const auto&u:own)if(u.type==99 && frame%480==0 && e.select(u.x,u.y))e.move_selected(hx,hy+100);
for(const auto&w:workers)if(w.order==unsigned(Orders::PlayerGuard) || w.order==unsigned(Orders::Guard) || w.order==unsigned(Orders::Stop)){
 const UnitView* target=nullptr;int best=99999999;auto id=e.own_unit_at(w.x,w.y);for(const auto&u:own)if(u.type==110 && (workers.size()>=10 && (gas.count(id) || gas.size()<3))){target=&u;gas.insert(id);best=0;break;}
 for(unsigned i=0;i<n;++i)if(list[i].mineral){int dx=list[i].x-w.x,dy=list[i].y-w.y,d=dx*dx+dy*dy;if(d<best){best=d;target=&list[i];}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
}
if(pending && (e.count_units(0,pending,false)>pending_count || frame>pending_at+1800))pending=0;
auto s=e.snapshot();unsigned want=0;if(s.supply_max-s.supply<6)want=109;else if(workers.size()<12)want=0;else if(!e.count_units(0,110,false))want=110;else if(!e.count_units(0,113,false))want=113;else if(!e.count_units(0,112,false))want=112;else if(!e.count_units(0,123,false))want=123;else if(e.count_units(0,113,false)<3 && s.minerals>250)want=113;else if(e.count_units(0,111,false)<2 && s.minerals>300)want=111;
if(!pending && want && !workers.empty()){const UnitView* worker=&workers.back();int best=99999999;for(const auto&w:workers){int dx=w.x-hx,dy=w.y-hy,d=dx*dx+dy*dy;if(!gas.count(e.own_unit_at(w.x,w.y)) && d<best){best=d;worker=&w;}}bool built=false;if(e.select(worker->x,worker->y)){
 if(want==110){for(unsigned i=0;i<n && !built;++i)if(list[i].geyser && abs(list[i].x-hx)<700 && abs(list[i].y-hy)<700 && e.placement_unit(want,list[i].x,list[i].y).valid)built=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
 else for(int r=3;r<12 && !built;++r)for(int dy=-r;dy<=r && !built;++dy)for(int dx=-r;dx<=r && !built;++dx){if(abs(dx)!=r && abs(dy)!=r)continue;int x=hx+dx*32,y=hy+dy*32;if(e.placement_unit(want,x,y).valid)built=e.perform(option_code(OptionKind::build,want),x,y);}}
 if(built){pending=want;pending_at=frame;pending_count=e.count_units(0,want,false);}}
for(const auto&u:own)if(e.select(u.x,u.y)){auto info=e.selection_info();if(info.queue_count)continue;if(u.type==106 && workers.size()<24)e.perform(option_code(OptionKind::train,7));else if(u.type==111 && workers.size()>=12 && e.count_units(0,0)+e.count_units(0,34)<45)e.perform(option_code(OptionKind::train,e.count_units(0,112) && e.count_units(0,34)*5<e.count_units(0,0)?34:0));else if(u.type==113)e.perform(option_code(OptionKind::train,3));}
unsigned army=e.count_units(0,3)+e.count_units(0,0);if(army>=40)attack=true;if(army<12)attack=false;
const UnitView* target=nullptr;int best=99999999;
for(const auto&u:enemies){int dx=u.x-hx,dy=u.y-hy,d=dx*dx+dy*dy;if(u.type<106 && d<650*650){target=&u;best=d-10000000;}else if(attack && u.type==133 && d<best){target=&u;best=d;}}
if(frame%240==0 && target)for(const auto&u:own)if((u.type==0 || u.type==3 || u.type==34) && e.select(u.x,u.y)){if(u.type==34)e.move_selected(target->x,target->y);else e.command(Command::attack,target->x,target->y);}
if(!attack && frame%240==0)for(const auto& u:own)if(u.type==0 && e.select(u.x,u.y)){for(const auto& bunker:own)if(bunker.type==125 && bunker.cargo<4){e.command(Command::context,bunker.x,bunker.y);break;}}
if(!e.count_units(3,133) && !e.count_units(4,133) && !e.count_units(5,133) && !e.count_units(6,133))for(const auto&u:own)if(u.type==99 && e.select(u.x,u.y))e.move_selected(1360,256);
if(frame%4800==0){printf("BWT3 f%u min%d gas%d workers%zu marines%u medics%u goliaths%u CC%u hives%u Duran%u attack%d\n",e.frame(),s.minerals,s.gas,workers.size(),e.count_units(0,0),e.count_units(0,34),e.count_units(0,3),e.count_units(0,106),e.count_units(3,133)+e.count_units(4,133)+e.count_units(5,133)+e.count_units(6,133),e.count_units(0,99),attack);fflush(stdout);}
}
printf("BWT3 result%d frame%u\n",e.snapshot().winner,e.frame());assert(e.snapshot().winner==0 && e.count_units(0,99)==1 && e.snapshot().next_scenario.empty());assert(!e.count_units(3,133) && !e.count_units(4,133) && !e.count_units(5,133) && !e.count_units(6,133));SaveView save;assert(e.save_game("/tmp/nov03-bwt3-completed.sav",save));Engine replay;SaveView restored;assert(replay.load_save(v[1],"/tmp/nov03-bwt3-completed.sav","/tmp/nov03-bwt3-replay","/tmp/nov03-bwt3-map",restored));assert(e.state_digest()==replay.state_digest());puts("Ruins of Tarsonis original handover, four Hive destruction, surviving Duran extraction, victory/branch and exact completed replay PASS");}
