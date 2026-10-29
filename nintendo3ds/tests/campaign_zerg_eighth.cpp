// Original SC Zerg8: real starting economy, linked Nydus transit, paid air
// army and detector speed, local guards outside the authored loss zones, and
// Kerrigan's paid-energy Ensnare aimed ahead of escaping Dark Templar.
// Fog-off navigation only; no map, original AI/triggers, units, funds, stats or
// research availability are overridden. The original result must be victory.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
#include <set>
#include <map>
#include <algorithm>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);const auto& m=campaign::missions[campaign::key_index("zerg08")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const bool z=true;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 std::set<unsigned>gas_ids;std::map<unsigned,unsigned>gas_seen;unsigned bottom_ultra=0;std::set<unsigned>north_muta,south_muta;unsigned build_pending=0,build_at=0,build_count=0;bool loaded_hero=false,transported=false,used_ensnare=false,nydus_transit=false;bool attack=false;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<220000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base && u.x>2500 && u.y<1000){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==1 || u.owner==2)enemies.push_back(u);}
  for(const auto& p:parents)if(p.type==39 && p.y>2500)nydus_transit=true;
  if(!home_x)continue;
  bool challenge=false;
  for(const auto& hero:parents)if(hero.type==51 && frame%240==0 && e.select(hero.x,hero.y)){int tx=3568,ty=1120;if(challenge && enemies.empty()){tx=1904;ty=2032;}e.move_selected(tx,ty);}

  
  if(challenge && enemies.empty() && frame%240==0)for(const auto& actor:parents)if(actor.type<106 && actor.type!=51 && actor.type!=42 && !actor.worker && e.select(actor.x,actor.y))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),home_x,home_y);
  if(challenge && enemies.empty()){
   UnitView* hero=nullptr;UnitView* ship=nullptr;for(unsigned j=0;j<n;++j)if(list[j].owner==0){if(list[j].type==51)hero=&list[j];if(list[j].type==42 && (!ship || list[j].cargo || (hero && (abs(list[j].x-hero->x)+abs(list[j].y-hero->y)<abs(ship->x-hero->x)+abs(ship->y-hero->y)))))ship=&list[j];}
   if(ship && e.select(ship->x,ship->y)){if(!ship->cargo && hero && hero->x>1700 && hero->y>1792 && hero->y<2272)transported=true;if(transported)e.move_selected(home_x,home_y);else if(ship->cargo){loaded_hero=true;int dx=ship->x-1904,dy=ship->y-2032;if(dx*dx+dy*dy<48*48)e.perform(option_code(OptionKind::unload));else e.move_selected(1904,2032);}else if(hero)e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::PickupTransport)),hero->x,hero->y);}
  }
  if(frame%240==0){unsigned lord=0;for(const auto& p:parents)if(p.type==42 && e.select(p.x,p.y)){int gate=0,near=0x7fffffff;for(int g=0;g<3;++g){int tx=g==0?432:g==1?3568:2704,ty=g==0?544:g==1?1024:3008;int dx=p.x-tx,dy=p.y-ty,d=dx*dx+dy*dy;if(d<near){near=d;gate=g;}}int gx=gate==0?256:gate==1?3568:2720,gy=gate==0?576:gate==1?1120:3584;e.move_selected(gx,gy);}}
  if(frame<1200 && frame%240==0)for(const auto& p:parents)if((p.type==38 || p.type==37) && p.x>2400 && p.y>2500 && e.select(p.x,p.y))e.move_selected(2688,3616);
  if(frame<1200 && frame%240==0)for(const auto& p:parents)if(p.type==38 && p.x>3000 && p.y<800 && e.select(p.x,p.y))e.move_selected(3568,1152);
  if(frame<1800 && frame%240==0)for(const auto& p:parents)if(p.type==38 && p.x<1200 && e.select(p.x,p.y))e.move_selected(256,672);
  for(const auto& p:parents)if(p.type==43 && p.x<1200 && p.y<1200 && north_muta.size()<4)north_muta.insert(e.own_unit_at(p.x,p.y));
  if(frame%240==0)for(const auto& p:parents)if(p.type==43 && north_muta.count(e.own_unit_at(p.x,p.y)) && e.select(p.x,p.y))e.move_selected(256,640);
  for(const auto& p:parents)if(p.type==43 && p.y>2500 && south_muta.size()<4)south_muta.insert(e.own_unit_at(p.x,p.y));
  if(frame%240==0)for(const auto& p:parents)if(p.type==43 && south_muta.count(e.own_unit_at(p.x,p.y)) && e.select(p.x,p.y))e.move_selected(2736,3584);
  const UnitView* templar=nullptr;for(const auto& u:enemies)if(u.type==61 || u.type==74)templar=&u;
  bool detected=false;if(templar)for(const auto& p:parents)if(p.type==42){int dx=p.x-templar->x,dy=p.y-templar->y;if(dx*dx+dy*dy<200*200)detected=true;}
  if(templar && detected && frame%96==0)for(const auto& p:parents)if(p.type==42 && abs(p.x-templar->x)+abs(p.y-templar->y)<500 && e.select(p.x,p.y))e.move_selected(templar->x,std::min(int(state.height)-32,templar->y+96));
  if(templar && detected && frame%96==0)for(const auto& p:parents)if((p.type==38 || p.type==43 || p.type==37 || p.type==39) && abs(p.x-templar->x)+abs(p.y-templar->y)<1000 && e.select(p.x,p.y))e.command(Command::context,templar->x,templar->y);
  if(frame%96==0)for(const auto& p:parents)if(p.type==39 && e.select(p.x,p.y)){if(p.y<1500)e.command(Command::context,3424,576);else if(!templar || !detected)e.move_selected(2768,3648);}
  if(templar && frame%96==0)for(const auto& p:parents)if(p.type==51 && abs(p.x-templar->x)+abs(p.y-templar->y)<600 && p.energy>=75 && e.select(p.x,p.y)){bool cast=e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastEnsnare)),templar->x+64,templar->y-96);used_ensnare=used_ensnare || cast;}
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66)+e.count_units(0,z?43:70);
  if(e.count_units(0,43)>=28)attack=true;
  if(army<12)attack=false;
  if(!templar && attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(!z)for(const auto& u:enemies)if(u.type>=106 && u.type<202){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(target->type<106 || d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==43) && !north_muta.count(e.own_unit_at(list[i].x,list[i].y)) && !south_muta.count(e.own_unit_at(list[i].x,list[i].y)))if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  UnitView* refinery=nullptr;for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==gas && list[i].completed)refinery=&list[i];
  for(const auto& w:workers){unsigned id=e.own_unit_at(w.x,w.y);if(gas_ids.count(id))gas_seen[id]=frame;}for(auto it=gas_ids.begin();it!=gas_ids.end();)if(frame>gas_seen[*it]+240){gas_seen.erase(*it);it=gas_ids.erase(it);}else ++it;
  if(refinery && workers.size()>8)for(const auto& w:workers){unsigned id=e.own_unit_at(w.x,w.y);bool assigned=gas_ids.count(id);if(!assigned && gas_ids.size()>=3)continue;if(abs(w.x-refinery->x)+abs(w.y-refinery->y)>700)continue;if(w.order==unsigned(bwgame::Orders::DroneStartBuild) || w.order==unsigned(bwgame::Orders::DroneLand))continue;if((!assigned || frame%480==0) && e.select(w.x,w.y) && e.command(Command::context,refinery->x+40,refinery->y+16)){gas_ids.insert(id);gas_seen[id]=frame;}}
  if(build_pending && (e.count_units(0,build_pending)>build_count || frame>build_at+1500))build_pending=0;
  unsigned want=0;int anchor_x=home_x,anchor_y=home_y;
  if(!z)for(const auto& p:parents)if(p.type==160 && p.disabled){want=supply;anchor_x=p.x;anchor_y=p.y;break;}
  if(!want && state.supply_max-state.supply<5 && !z)want=supply;
  if(!want && !e.count_units(0,z?142:160))want=z?142:160;
  if(!want && !e.count_units(0,gas))want=gas;
  if(!want && z && !e.count_units(0,135))want=135;
  if(!want && !z && !e.count_units(0,164))want=164;
  if(!want && !z && e.count_units(0,167,false)<3)want=167;
  if(!want && z && e.count_units(0,131,false)<2)want=131;
  if(!want && z && e.count_units(0,132) && !e.count_units(0,141))want=141;
  if(!build_pending && want && workers.size()>=8){
   std::sort(workers.begin(),workers.end(),[&](const UnitView&a,const UnitView&b){return abs(a.x-home_x)+abs(a.y-home_y)<abs(b.x-home_x)+abs(b.y-home_y);});bool placed=false;auto w=workers.front();if(e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && abs(list[i].x-home_x)<800 && abs(list[i].y-home_y)<800 && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=anchor_x+x*32,py=anchor_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;build_count=e.count_units(0,want);}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<18?worker:e.count_units(0,141)?43:e.count_units(0,135)?38:37;if((next!=37 || e.count_units(0,37)<12) && (next!=38 || e.count_units(0,38)<12))e.perform(option_code(OptionKind::morph,next));}
   else if(z && p.type==132)e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace)));
   else if(z && p.type==131 && !e.count_units(0,132,false))e.perform(option_code(OptionKind::morph,132));
   else if(!z && p.type==base && e.count_units(0,worker)<20)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==167)e.perform(option_code(OptionKind::train,70));
   else if(!z && p.type==160 && e.count_units(0,65)+e.count_units(0,66)<18){unsigned next=e.count_units(0,164) && e.count_units(0,66)<30?66:65;if(!e.perform(option_code(OptionKind::train,next)) && next==66)e.perform(option_code(OptionKind::train,65));}
  }
  if(frame%4800==0){printf("SentriesN%zu S%zu Muta%u Speed%u ",north_muta.size(),south_muta.size(),e.count_units(0,43),e.upgrade_level(unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace)));if(!z)printf("Scouts%u Stargates%u enemies%zu ",e.count_units(0,70),e.count_units(0,167),enemies.size());printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);if(s.winner!=0)return 1;assert(e.count_units(0,51)==1 && used_ensnare && nydus_transit && north_muta.size()==4 && south_muta.size()==4 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace))==1);SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),(std::string("/tmp/sc3ds-zerg-eighth-replay-")+m.key).c_str(),(std::string("/tmp/sc3ds-zerg-eighth-map-")+m.key).c_str(),restored));assert(e.state_digest()==replay.state_digest());puts("Eye for an Eye original victory: Kerrigan survives, paid economy/air army, native Nydus transit, leading Ensnare, researched detector speed, four northern/southern air sentries, exact completed replay PASS");
}
