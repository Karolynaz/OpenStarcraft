// Original SC Zerg7: no starting base. Public controls pay for the first
// Hatchery, clear/reuse the local gas, capture a western resource site, upgrade
// Hydralisks and research/fly/unload Overlords to finish the island forces.
// Fog-off information and a separate read-only native terrain model guide test
// navigation. That model never advances or mutates the actual campaign engine;
// no original map, AI, triggers, units, costs or resources are overridden.
#include "engine.h"
#include "bwenums.h"
#include "bwgame.h"
#include <fstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
#include <set>
#include <map>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);const unsigned mission=10;const auto& m=campaign::missions[campaign::key_index("zerg07")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 bwgame::game_player navigation(bwgame::data_loading::data_files_directory(argv[1]));std::ifstream nav_input(argv[2],std::ios::binary);bwgame::a_vector<uint8_t> nav_bytes((std::istreambuf_iterator<char>(nav_input)),{});bwgame::game_load_functions nav_loader(navigation.st());nav_loader.load_map_data(nav_bytes.data(),nav_bytes.size(),[&]{for(auto& p:navigation.st().players)p.controller=bwgame::player_t::controller_occupied;},false);auto& nav=navigation.funcs();bwgame::unit_t* nav_hydra=nullptr;for(auto* u:bwgame::ptr(nav.st.player_units[0]))if(nav.unit_is(u,bwgame::UnitTypes::Zerg_Hydralisk)){nav_hydra=u;break;}assert(nav_hydra);
 const bool z=mission==10;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 std::set<unsigned> gas_ids;std::map<unsigned,unsigned> gas_seen;unsigned build_pending=0,build_at=0,build_count=0;bool first_ordered=false;unsigned first_at=0;bool attack=false;int home_x=0,home_y=0,attack_x=352,attack_y=3056;bool islands=false,used_transport=false;unsigned landed=0,island_infantry=0;bool expanded=false;unsigned expansion_at=0;
 for(unsigned frame=0;frame<220000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==131 || u.type==132){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==(z?4u:7u) || (!z && u.owner==6))enemies.push_back(u);}
  if(!home_x){
   if(frame%240==0)for(const auto& actor:parents)if(actor.type==38 || actor.type==43)if(e.select(actor.x,actor.y))e.command(Command::attack,1664,3168);
   if((!first_ordered || frame>first_at+1800) && !e.count_units(0,131,false) && state.minerals>=300)for(const auto& w:workers){if(!e.select(w.x,w.y))continue;bool placed=false;for(int ring=0;ring<10 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(ring && abs(x)!=ring && abs(y)!=ring)continue;int px=1984+x*32,py=3360+y*32;if(e.placement_unit(131,px,py).valid)placed=e.perform(option_code(OptionKind::build,131),px,py);}if(placed){first_ordered=true;first_at=frame;printf("First paid Hatchery frame%u\n",e.frame());break;}}
   continue;
  }
  
  if(frame<7200 && frame%240==0)for(const auto& p:parents)if((p.type==38 || p.type==43) && e.select(p.x,p.y))e.command(Command::attack,1664,3168);
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66)+e.count_units(0,z?43:70);
  if((army>=55 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Zerg_Carapace))>=1) || (z && e.count_units(0,43)>=16) || (!z && e.count_units(0,70)>=16))attack=true;
  if(army<12)attack=false;
  bool mainland_enemy=false;for(const auto& u:enemies)if(!u.flying && nav.is_reachable(nav_hydra,bwgame::xy(u.x,u.y)))mainland_enemy=true;
  if(!mainland_enemy && frame>50000)islands=true;
  if(islands)for(const auto& p:parents)if(p.type==38 && !nav.is_reachable(nav_hydra,bwgame::xy(p.x,p.y)))++island_infantry;
  if(islands && !enemies.empty()){
   const UnitView* goal=nullptr;int gd=0x7fffffff;for(const auto& u:enemies)if(!u.flying){int dx=u.x-attack_x,dy=u.y-attack_y,d=dx*dx+dy*dy;if(d<gd){goal=&u;gd=d;}}
   if(!goal)goal=&enemies.front();
   int lx=goal->x,ly=goal->y,best=0x7fffffff;
   for(int yy=goal->y-384;yy<=goal->y+384;yy+=32)for(int xx=goal->x-384;xx<=goal->x+384;xx+=32){if(xx<32 || yy<32 || xx>=int(state.width)-32 || yy>=int(state.height)-32)continue;bwgame::xy pos(xx,yy);if(!nav.unit_type_can_fit_at(nav.get_unit_type(bwgame::UnitTypes::Zerg_Hydralisk),pos) || !nav.is_reachable(pos,bwgame::xy(goal->x,goal->y)))continue;int score=abs(xx-attack_x)+abs(yy-attack_y);for(const auto& u:enemies){int dx=u.x-xx,dy=u.y-yy,d=dx*dx+dy*dy;if((u.type==144 || u.type==38) && d<384*384)score+=1000000+(384*384-d);}if(score<best){best=score;lx=xx;ly=yy;}}
   for(const auto& p:parents)if(p.type==38 || p.type==43){const UnitView* target=nullptr;int dmin=0x7fffffff;for(const auto& u:enemies){if(p.type==38 && !nav.is_reachable(bwgame::xy(p.x,p.y),bwgame::xy(u.x,u.y)))continue;int dx=u.x-p.x,dy=u.y-p.y,d=dx*dx+dy*dy;if(u.type==42)d+=100000000;if(d<dmin){target=&u;dmin=d;}}if(target && frame%240==0 && e.select(p.x,p.y))e.command(Command::attack,target->x,target->y);}
   if(e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs))){std::set<unsigned> reserved;for(const auto& ship:parents)if(ship.type==42){if(ship.cargo>=4){used_transport=true;if(e.select(ship.x,ship.y)){int dx=ship.x-lx,dy=ship.y-ly;if(dx*dx+dy*dy<48*48){if(e.perform(option_code(OptionKind::unload)))++landed;}else e.move_selected(lx,ly);}}else{const UnitView* passenger=nullptr;int dmin=0x7fffffff;for(const auto& p:parents)if(p.type==38 && !nav.is_reachable(bwgame::xy(p.x,p.y),bwgame::xy(goal->x,goal->y))){unsigned id=e.own_unit_at(p.x,p.y);if(reserved.count(id))continue;int dx=p.x-ship.x,dy=p.y-ship.y,d=dx*dx+dy*dy;if(d<dmin){dmin=d;passenger=&p;}}if(passenger){reserved.insert(e.own_unit_at(passenger->x,passenger->y));if(e.select(ship.x,ship.y))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::PickupTransport)),passenger->x,passenger->y);}else if(ship.cargo && e.select(ship.x,ship.y)){used_transport=true;int dx=ship.x-lx,dy=ship.y-ly;if(dx*dx+dy*dy<48*48)e.perform(option_code(OptionKind::unload));else e.move_selected(lx,ly);}}}}
   if(frame%2400==0)printf("ISLAND target%d,%d landing%d,%d drops%u enemies%zu\n",goal->x,goal->y,lx,ly,landed,enemies.size());
  }
  if(!islands && attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){if(!nav.is_reachable(nav_hydra,bwgame::xy(u.x,u.y)))continue;int dx=u.x-attack_x,dy=u.y-attack_y,d=dx*dx+dy*dy;if(u.type==42)d+=100000000;if(u.type<106)d+=25000000;if(d<distance){distance=d;target=&u;}}
   if(!target){printf("MAINLAND clear frame%u remaining%zu\n",e.frame(),enemies.size());fflush(stdout);break;}
   bool left_enemy=false,left_owned=false;for(const auto& u:enemies)if(u.type>=106 && u.x<1000 && u.y>2600)left_enemy=true;for(const auto& p:parents)if((p.type==131 || p.type==132) && p.x<1000 && p.y>2600)left_owned=true;
   if(!left_enemy && !left_owned && state.minerals>=300 && (!expanded || frame>expansion_at+2400))for(const auto& w:workers){if(gas_ids.count(e.own_unit_at(w.x,w.y)) || !e.select(w.x,w.y))continue;bool built=false;for(int r=0;r<8 && !built;++r)for(int dy=-r;dy<=r && !built;++dy)for(int dx=-r;dx<=r && !built;++dx){int px=352+dx*32,py=3056+dy*32;if(e.placement_unit(131,px,py).valid)built=e.perform(option_code(OptionKind::build,131),px,py);}if(built){expanded=true;expansion_at=frame;printf("PAID expansion at%u\n",e.frame());break;}}
   UnitView holding;holding.x=448;holding.y=3008;if(!left_enemy && (!left_owned || frame<50000))target=&holding;
   attack_x=target->x;attack_y=target->y;
   const UnitView* front=nullptr;int front_d=0x7fffffff;for(const auto& p:parents)if(p.type==38){int dx=p.x-target->x,dy=p.y-target->y,d=dx*dx+dy*dy;if(d<front_d){front=&p;front_d=d;}}
   if(front){unsigned escort=0;for(const auto& p:parents)if(p.type==42 && ++escort<=6 && e.select(p.x,p.y))e.move_selected(front->x,front->y+64);}

   if(frame%2400==0)printf("TARGET type%u pos%d,%d enemies%zu\n",target->type,target->x,target->y,enemies.size());
   if(!z)for(const auto& u:enemies)if(u.type>=106 && u.type<202){int dx=u.x-attack_x,dy=u.y-attack_y,d=dx*dx+dy*dy;if(target->type<106 || d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==(z?37u:65u) || list[i].type==(z?38u:66u) || list[i].type==(z?43u:70u)))if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(u.type==42)d+=100000000;if(u.type<106)d+=25000000;if(d<distance){distance=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  UnitView* refinery=nullptr;for(unsigned j=0;j<n;++j)if(list[j].owner==0 && list[j].type==gas && list[j].completed)refinery=&list[j];
  for(const auto& w:workers){auto id=e.own_unit_at(w.x,w.y);if(gas_ids.count(id))gas_seen[id]=frame;}for(auto it=gas_ids.begin();it!=gas_ids.end();)if(frame>gas_seen[*it]+240){gas_seen.erase(*it);it=gas_ids.erase(it);}else ++it;
  if(refinery && workers.size()>6 && gas_ids.size()<3)for(const auto& w:workers){if(gas_ids.size()>=3)break;if(w.order==unsigned(bwgame::Orders::DroneStartBuild) || w.order==unsigned(bwgame::Orders::DroneLand))continue;unsigned id=e.own_unit_at(w.x,w.y);if(!gas_ids.count(id) && e.select(w.x,w.y) && e.command(Command::context,refinery->x+40,refinery->y+16)){gas_ids.insert(id);gas_seen[id]=frame;}}
  if(refinery && frame%480==0)for(const auto& w:workers)if(gas_ids.count(e.own_unit_at(w.x,w.y)) && e.select(w.x,w.y))e.command(Command::context,refinery->x+40,refinery->y+16);
  if(frame>=7200 && frame<9600 && frame%240==0)for(const auto& p:parents)if(p.type==43 && e.select(p.x,p.y))e.move_selected(2048,3504);
  if(build_pending && (e.count_units(0,build_pending)>build_count || frame>build_at+1500))build_pending=0;
  unsigned want=0;int anchor_x=home_x,anchor_y=home_y;
  if(!z)for(const auto& p:parents)if(p.type==160 && p.disabled){want=supply;anchor_x=p.x;anchor_y=p.y;break;}
  if(!want && state.supply_max-state.supply<5 && !z)want=supply;
  if(!want && !e.count_units(0,z?142:160))want=z?142:160;
  if(!want && !e.count_units(0,gas))want=gas;
  if(!want && z && !e.count_units(0,135))want=135;
  if(!want && !z && !e.count_units(0,164))want=164;
  if(!want && !z && e.count_units(0,167,false)<3)want=167;
  if(!want && z && e.count_units(0,131,false)+e.count_units(0,132,false)<2)want=131;
  if(!want && z && e.count_units(0,135) && e.count_units(0,143,false)+e.count_units(0,146,false)<3)want=143;
  if(!want && z && !e.count_units(0,139))want=139;
  if(!build_pending && want && !workers.empty()){
   bool placed=false;auto w=workers.back();if(e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && abs(list[i].x-home_x)<800 && abs(list[i].y-home_y)<800 && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=anchor_x+x*32,py=anchor_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;build_count=e.count_units(0,want);}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){if(islands && !e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs)))continue;unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<24?worker:e.count_units(0,135)?38:37;if(next!=38 || (e.snapshot().gas>=175 && e.snapshot().minerals>=225) || (e.upgrade_level(unsigned(bwgame::UpgradeTypes::Zerg_Carapace))>=2 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Zerg_Missile_Attacks))>=2))e.perform(option_code(OptionKind::morph,next));}
   else if(z && p.type==131 && !e.count_units(0,132,false) && e.count_units(0,135) && e.count_units(0,worker)>=20)e.perform(option_code(OptionKind::morph,132));
   else if(z && p.type==132 && islands){e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Ventral_Sacs)));e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace)));}
   else if(z && p.type==143)e.perform(option_code(OptionKind::morph,146));
   else if(z && p.type==135){e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Grooved_Spines)));e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Muscular_Augments)));}
   else if(z && p.type==139){e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Zerg_Carapace)));e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Zerg_Missile_Attacks)));}
   else if(!z && p.type==base && e.count_units(0,worker)<20)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==167)e.perform(option_code(OptionKind::train,70));
   else if(!z && p.type==160 && e.count_units(0,65)+e.count_units(0,66)<18){unsigned next=e.count_units(0,164) && e.count_units(0,66)<30?66:65;if(!e.perform(option_code(OptionKind::train,next)) && next==66)e.perform(option_code(OptionKind::train,65));}
  }
  if(frame%4800==0){printf("Enemy%zu Supply%d/%d HydraSpeed%u Range%u Atk%u Def%u ",enemies.size(),state.supply,state.supply_max,e.upgrade_level(unsigned(bwgame::UpgradeTypes::Muscular_Augments)),e.upgrade_level(unsigned(bwgame::UpgradeTypes::Grooved_Spines)),e.upgrade_level(unsigned(bwgame::UpgradeTypes::Zerg_Missile_Attacks)),e.upgrade_level(unsigned(bwgame::UpgradeTypes::Zerg_Carapace)));if(!z)printf("Scouts%u Stargates%u enemies%zu ",e.count_units(0,70),e.count_units(0,167),enemies.size());printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);if(s.winner!=0)return 1;assert(used_transport && landed && island_infantry && expanded && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs))==1);SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),(std::string("/tmp/sc3ds-zerg-seventh-replay-")+m.key).c_str(),(std::string("/tmp/sc3ds-zerg-seventh-map-")+m.key).c_str(),restored));assert(e.state_digest()==replay.state_digest());puts("The Culling original victory: paid first base, economy, west expansion, upgrades, researched Overlord cargo, island infantry and exact completed replay PASS");
}
