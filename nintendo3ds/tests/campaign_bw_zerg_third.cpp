// Original Kel-Morian Combine: establish a paid colony, defend its miners and
// bank10,000 minerals while preserving Fenix. Public controls and normal costs;
// fog-off navigation only. No original actors, triggers, AI or funds modified.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
#include <algorithm>
#include <map>
#include <set>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_zerg_campaign_3));assert(e.start_recording("/tmp/sc3ds-bwz3.journal"));e.set_fog_enabled(false);
 int hx=1376,hy=3680;unsigned stage=0,swarm=0;unsigned pending=0,pending_at=0,pending_count=0;std::set<unsigned>gas_workers;std::map<unsigned,unsigned>seen;
 for(unsigned f=0;f<60000 && e.snapshot().winner<0;++f){if(!e.tick()){printf("ERROR f%u %s\n",e.frame(),e.error());return 2;}SoundEvent sounds[256];e.drain_sounds(sounds,256);if(f%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto s=e.snapshot();std::vector<UnitView> workers,own,minerals,enemies;
  for(unsigned i=0;i<n;++i){const auto&u=list[i];if(u.owner==0){own.push_back(u);if(u.type==41 && u.completed)workers.push_back(u);if(u.type==131 && u.completed){hx=u.x;hy=u.y;}}if(u.mineral)minerals.push_back(u);if(u.owner!=0 && u.owner!=2 && !u.mineral && !u.geyser && u.type<176)enemies.push_back(u);}
  stage=2;const int tx=hx,ty=hy;
  const UnitView* threat=nullptr;int best=650*650;
  for(const auto&u:enemies){int dx=u.x-tx,dy=u.y-ty,d=dx*dx+dy*dy;if(d<best && u.type!=13 && !u.burrowed){best=d;threat=&u;}}
  if(!e.count_units(0,131,false) && f%240==0)for(const auto&w:workers){
   if(w.order!=unsigned(bwgame::Orders::DroneStartBuild) && e.select(w.x,w.y))e.move_selected(hx,hy);
  }
  if(f%120==0)for(const auto&u:own)if(!u.worker && u.type<106 && !u.flying && u.completed && e.select(u.x,u.y)){
   if(u.type==78 && e.count_units(0,131)){e.move_selected(448,3888);continue;}
   if(u.type==45){
    const UnitView* tank=nullptr;int near=1000*1000;for(const auto&t:enemies)if(t.type==3 || t.type==5 || t.type==30){int dx=u.x-t.x,dy=u.y-t.y,d=dx*dx+dy*dy;if(d<near){near=d;tank=&t;}}
    if(tank && u.energy>=150 && f>swarm+240){if(e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastSpawnBroodlings)),tank->x,tank->y)){swarm=f;printf("BROODLINGS f%u\n",e.frame());}}
    else if(f>swarm+240)e.move_selected(tx-250,ty+150);
   }else e.command(threat?Command::context:Command::attack,threat?threat->x:tx,threat?threat->y:ty);
  }
  for(const auto&w:workers)if(e.count_units(0,131) && (w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop))){
   const UnitView*target=nullptr;int best=99999999;for(const auto&u:minerals){int dx=u.x-hx,dy=u.y-hy,d=dx*dx+dy*dy;if(d<best){best=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  for(const auto&w:workers){unsigned id=e.own_unit_at(w.x,w.y);if(gas_workers.count(id))seen[id]=f;}
  for(auto it=gas_workers.begin();it!=gas_workers.end();)if(f>seen[*it]+240){seen.erase(*it);it=gas_workers.erase(it);}else ++it;
  if(workers.size()>10 && e.count_units(0,149))for(const auto&u:own)if(u.type==149 && u.completed)for(const auto&w:workers){unsigned id=e.own_unit_at(w.x,w.y);if(!gas_workers.count(id) && gas_workers.size()>=3)continue;if(e.select(w.x,w.y) && e.command(Command::context,u.x+40,u.y+16)){gas_workers.insert(id);seen[id]=f;}}
  if(pending && (e.count_units(0,pending,false)>pending_count || f>pending_at+1800))pending=0;
  unsigned want=0;
  if(!e.count_units(0,131,false))want=131;
  else if(!e.count_units(0,142,false))want=142;
  else if(e.count_units(0,146,false)+e.count_units(0,143,false)<3)want=143;
  else if(!e.count_units(0,139,false))want=139;
  else if(e.count_units(0,144,false)+e.count_units(0,143,false)<5)want=143;
  else if(e.count_units(0,131,false)<2 && workers.size()>12 && s.minerals>500)want=131;
  if(s.minerals<8500 && stage==2 && !pending && want && !workers.empty()){
   bool placed=false;const auto&w=workers.back();if(e.select(w.x,w.y)){
    if(want==149){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && abs(list[i].x-hx)<800 && abs(list[i].y-hy)<800 && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int r=0;r<12 && !placed;++r)for(int dy=-r;dy<=r && !placed;++dy)for(int dx=-r;dx<=r && !placed;++dx){if(abs(dx)!=r && abs(dy)!=r)continue;int x=hx+dx*32+(want==143?128:0),y=hy+dy*32;if(e.placement_unit(want,x,y).valid)placed=e.perform(option_code(OptionKind::build,want),x,y);}
   }if(placed){pending=want;pending_count=e.count_units(0,want,false);pending_at=f;}
  }
  for(const auto&u:own)if(u.completed && u.type==143 && e.select(u.x,u.y))e.perform(option_code(OptionKind::morph,e.count_units(0,146,false)<3?146:144));
  for(const auto&u:own)if(u.completed && u.type==35 && e.select(u.x,u.y)){
   unsigned type=s.supply_max-s.supply<4?42:workers.size()<36 && s.minerals<8500?41:e.count_units(0,38)<12 && e.count_units(0,135)?38:0;
   if(type)e.perform(option_code(OptionKind::morph,type));
  }
  if(f%2400==0){printf("BWZ3 f%u minerals%d workers%zu Hatch%u Sunken%u Spore%u Fenix%u\n",e.frame(),s.minerals,workers.size(),e.count_units(0,131),e.count_units(0,146),e.count_units(0,144),e.count_units(0,78));fflush(stdout);}
 }
 auto s=e.snapshot();printf("BWZ3 result%d frame%u min%d Fenix%u next%s\n",s.winner,e.frame(),s.minerals,e.count_units(0,78),s.next_scenario.c_str());
 assert(s.winner==0 && s.minerals>=10000 && e.count_units(0,78)==1 && s.next_scenario=="Zerg04a.scx");
 SaveView view;assert(e.save_game("/tmp/sc3ds-expzerg03-completed.sav",view));
 Engine replay;SaveView restored;assert(replay.load_save(argv[1],"/tmp/sc3ds-expzerg03-completed.sav","/tmp/sc3ds-expzerg03-replay","/tmp/sc3ds-expzerg03-map",restored));assert(e.state_digest()==replay.state_digest());
 puts("Kel-Morian original10k quota, surviving Fenix, paid colony/defenses, authored next branch and exact completed replay PASS");return 0;
}
