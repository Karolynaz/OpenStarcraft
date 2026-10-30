// Original owned BW Zerg2. Autonomous original intro, neutral Psi Emitter
// pickup/tracking, real Drone/Hunter Killer rescue, paid first Hatchery,
// economy/tech/24-Mutalisk strike and native Psi Disrupter victory.
// Fog-off navigation only; no original map, units, funds, stats, AI or triggers edited.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
#include <map>
#include <limits>
using namespace port3ds;using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_zerg_campaign_2));assert(e.start_recording("/tmp/sc3ds-campaign-playthrough-expzerg02"));e.set_fog_enabled(false);
 std::vector<std::pair<int,int>> route{{1184,3488},{1408,3840},{1408,3648},{900,3888},{1408,3648}};unsigned step=0,arrived=0,first_at=0,pending=0,pending_at=0,pending_count=0;bool first_ordered=false,lair_ordered=false,attacking=false,emitter_carried=false,drones_rescued=false;unsigned max_mutas=0;std::set<unsigned>gas_ids;std::map<unsigned,unsigned>seen;
 for(unsigned frame=0;frame<160000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}if(frame%48)continue;UnitView list[1700];unsigned n=e.visible_units(list,1700);auto s=e.snapshot();std::vector<UnitView> own,workers,minerals;UnitView* gas=nullptr;UnitView* goal=nullptr;int hx=1536,hy=3552;bool home=false;
  for(unsigned i=0;i<n;++i){const auto& u=list[i];if(u.owner==0){if(u.completed)own.push_back(u);if(u.type==41 && u.completed)workers.push_back(u);if((u.type==131 || u.type==132) && !home){hx=u.x;hy=u.y;home=true;}if(u.type==149 && u.completed)gas=&list[i];}if(u.mineral)minerals.push_back(u);if(u.owner==7 && u.type==190)goal=&list[i];}
  max_mutas=std::max(max_mutas,e.count_units(0,43));if(e.count_units(0,41)>=2 && !home)drones_rescued=true;for(const auto& p:own)if(p.type==7 && p.carried_powerup==217)emitter_carried=true;
  for(const auto& p:own)if(p.type==7 && e.select(p.x,p.y)){auto t=route[step];int dx=p.x-t.first,dy=p.y-t.second;if(dx*dx+dy*dy<64*64){if(!arrived)arrived=frame;if(frame>arrived+240 && step+1<route.size()){++step;arrived=0;}}e.move_selected(t.first,t.second);}
  if(!home){if(e.count_units(0,53)>=3 && !workers.empty() && s.minerals>=300 && (!first_ordered || frame>first_at+1800)){auto w=workers.front();if(e.select(w.x,w.y)){bool placed=false;for(int r=0;r<10 && !placed;++r)for(int dy=-r;dy<=r && !placed;++dy)for(int dx=-r;dx<=r && !placed;++dx){int x=1536+dx*32,y=3552+dy*32;if(e.placement_unit(131,x,y).valid)placed=e.perform(option_code(OptionKind::build,131),x,y);}if(placed){first_ordered=true;first_at=frame;printf("First paid Hatchery f%u\n",e.frame());}}}continue;}
  for(const auto& w:workers){auto id=e.own_unit_at(w.x,w.y);if(gas_ids.count(id))seen[id]=frame;}
  for(auto it=gas_ids.begin();it!=gas_ids.end();)if(frame>seen[*it]+240){seen.erase(*it);it=gas_ids.erase(it);}else ++it;
  for(const auto& w:workers)if(!gas_ids.count(e.own_unit_at(w.x,w.y)) && (w.order==unsigned(Orders::PlayerGuard) || w.order==unsigned(Orders::Guard) || w.order==unsigned(Orders::Stop))){const UnitView* target=nullptr;int best=0x7fffffff;for(const auto& m:minerals){int dx=m.x-w.x,dy=m.y-w.y,d=dx*dx+dy*dy;if(d<best){best=d;target=&m;}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);}
  if(gas && workers.size()>7)for(const auto& w:workers){auto id=e.own_unit_at(w.x,w.y);bool assigned=gas_ids.count(id);if(!assigned && gas_ids.size()>=3)continue;if(w.order==unsigned(Orders::DroneStartBuild) || w.order==unsigned(Orders::DroneLand))continue;if((!assigned || frame%480==0) && e.select(w.x,w.y) && e.command(Command::context,gas->x+40,gas->y+16)){gas_ids.insert(id);seen[id]=frame;}}
  if(pending && (e.count_units(0,pending)>pending_count || frame>pending_at+1800))pending=0;
  unsigned want=0;
  if(!e.count_units(0,142,false))want=142;
  else if(!e.count_units(0,149,false))want=149;
  else if(!e.count_units(0,135,false))want=135;
  else if(e.count_units(0,132) && !e.count_units(0,141,false))want=141;
  else if(e.count_units(0,131,false)+e.count_units(0,132,false)<2)want=131;
  else if(e.count_units(0,143,false)+e.count_units(0,146,false)<2)want=143;
  if(!pending && want && workers.size()>=4){bool placed=false;for(const auto& w:workers){if(placed)break;if(gas_ids.count(e.own_unit_at(w.x,w.y)) || w.order==unsigned(Orders::DroneStartBuild) || w.order==unsigned(Orders::DroneLand) || !e.select(w.x,w.y))continue;
    if(want==149){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && abs(list[i].x-hx)+abs(list[i].y-hy)<700 && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int r=2;r<12 && !placed;++r)for(int dy=-r;dy<=r && !placed;++dy)for(int dx=-r;dx<=r && !placed;++dx){if(abs(dx)!=r && abs(dy)!=r)continue;int x=hx+dx*32,y=hy+dy*32;if(e.placement_unit(want,x,y).valid)placed=e.perform(option_code(OptionKind::build,want),x,y);}
   }if(placed){pending=want;pending_at=frame;pending_count=e.count_units(0,want);}}
  for(const auto& p:own){if(!e.select(p.x,p.y))continue;
   if(p.type==35){unsigned next=s.supply_max-s.supply<6?42:e.count_units(0,41)<20?41:e.count_units(0,141)?43:e.count_units(0,135)?38:37;if(next==38 && e.count_units(0,38)>=8)continue;if(next==37 && e.count_units(0,37)>=12)continue;e.perform(option_code(OptionKind::morph,next));}
   else if(p.type==131 && !lair_ordered && e.count_units(0,135) && workers.size()>=10){if(e.perform(option_code(OptionKind::morph,132)))lair_ordered=true;}
   else if(p.type==143)e.perform(option_code(OptionKind::morph,146));
   else if(p.type==141){if(e.upgrade_level(unsigned(UpgradeTypes::Zerg_Flyer_Attacks))<1)e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Zerg_Flyer_Attacks)));else if(e.upgrade_level(unsigned(UpgradeTypes::Zerg_Flyer_Carapace))<1)e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Zerg_Flyer_Carapace)));}
  }
  if(e.count_units(0,43)>=24)attacking=true;
  if(frame%240==0){for(const auto& p:own)if(p.type==37 || p.type==38 || p.type==48 || p.type==53 || (p.type==43 && !attacking)){const UnitView* danger=nullptr;int best=768*768;for(unsigned j=0;j<n;++j){const auto& t=list[j];if(t.owner!=7 || t.type==101)continue;int dx=t.x-hx,dy=t.y-hy,d=dx*dx+dy*dy;if(d<best){best=d;danger=&t;}}if(danger && e.select(p.x,p.y))e.command(Command::context,danger->x,danger->y);else if(p.type!=43 && (p.order==unsigned(Orders::PlayerGuard) || p.order==unsigned(Orders::Guard)) && e.select(p.x,p.y))e.command(Command::attack,1344,3424);}
   if(attacking && goal)for(const auto& p:own)if(p.type==43 && e.select(p.x,p.y))e.command(Command::context,goal->x,goal->y);
  }
  if(frame%2400==0){printf("f%u funds%d/%d drones%u hunter%u muta%u pool%u den%u spire%u pending%u attack%d goalhp%d winner%d\n",e.frame(),s.minerals,s.gas,e.count_units(0,41),e.count_units(0,53),e.count_units(0,43),e.count_units(0,142),e.count_units(0,135),e.count_units(0,141),pending,attacking,goal?goal->hp:0,s.winner);fflush(stdout);}
 }
 printf("BWZ2 result%d frame%u\n",e.snapshot().winner,e.frame());if(e.snapshot().winner!=0)return 1;assert(emitter_carried && drones_rescued && first_ordered && max_mutas>=24 && e.count_units(7,190)==0);SaveView view;assert(e.save_game("/tmp/sc3ds-expzerg02-completed.sav",view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],"/tmp/sc3ds-expzerg02-completed.sav","/tmp/sc3ds-bwz2-completed-replay","/tmp/sc3ds-bwz2-completed-map",restored));assert(e.state_digest()==replay.state_digest());puts("Original BW Zerg2 paid economy/air army/Psi Disrupter victory/exact completed replay PASS");
}
