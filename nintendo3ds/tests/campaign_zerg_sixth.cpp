// Original Zerg6: native paid economy/combat, researched Overlord transport,
// surviving Kerrigan alone at the original challenge, original victory/save.
// Fog-off navigation; no units, resources, stats, AI or map overrides.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);const auto& m=campaign::missions[campaign::key_index("zerg06")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const bool z=true;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 unsigned build_pending=0,build_at=0,build_count=0;bool loaded_hero=false,transported=false;bool attack=false;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<220000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==(z?1u:7u) || (!z && u.owner==6))enemies.push_back(u);}
  if(!home_x)continue;
  bool challenge=state.objectives.find("alone")!=std::string::npos;
  for(const auto& hero:parents)if(hero.type==51 && frame%240==0 && e.select(hero.x,hero.y)){int tx=home_x,ty=home_y;if(challenge && enemies.empty()){tx=1904;ty=2032;}e.move_selected(tx,ty);}

  
  if(challenge && enemies.empty() && frame%240==0)for(const auto& actor:parents)if(actor.type<106 && actor.type!=51 && actor.type!=42 && !actor.worker && e.select(actor.x,actor.y))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),home_x,home_y);
  if(challenge && enemies.empty()){
   UnitView* hero=nullptr;UnitView* ship=nullptr;for(unsigned j=0;j<n;++j)if(list[j].owner==0){if(list[j].type==51)hero=&list[j];if(list[j].type==42 && (!ship || list[j].cargo || (hero && (abs(list[j].x-hero->x)+abs(list[j].y-hero->y)<abs(ship->x-hero->x)+abs(ship->y-hero->y)))))ship=&list[j];}
   if(ship && e.select(ship->x,ship->y)){if(!ship->cargo && hero && hero->x>1700 && hero->y>1792 && hero->y<2272)transported=true;if(transported)e.move_selected(home_x,home_y);else if(ship->cargo){loaded_hero=true;int dx=ship->x-1904,dy=ship->y-2032;if(dx*dx+dy*dy<48*48)e.perform(option_code(OptionKind::unload));else e.move_selected(1904,2032);}else if(hero)e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::PickupTransport)),hero->x,hero->y);}
  }
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66)+e.count_units(0,z?43:70);
  if(army>=40 || (z && e.count_units(0,43)>=16) || (!z && e.count_units(0,70)>=16))attack=true;
  if(army<12)attack=false;
  if(attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(!z)for(const auto& u:enemies)if(u.type>=106 && u.type<202){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(target->type<106 || d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==(z?37u:65u) || list[i].type==(z?38u:66u) || list[i].type==(z?43u:70u) || (z && list[i].type==44)))if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  // Assign three workers to the paid refinery once it has completed.
  if(frame%480==0 && e.count_units(0,gas) && workers.size()>=7){UnitView* refinery=nullptr;for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==gas && list[i].completed)refinery=&list[i];if(refinery)for(unsigned i=0;i<3;++i)if(e.select(workers[i].x,workers[i].y))e.command(Command::context,refinery->x,refinery->y);}
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
  if(!build_pending && want && !workers.empty()){
   bool placed=false;auto w=workers.back();if(e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=anchor_x+x*32,py=anchor_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;build_count=e.count_units(0,want);}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){if(challenge && !e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs)))continue;unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<18?worker:e.count_units(0,141)?43:e.count_units(0,135)?38:37;e.perform(option_code(OptionKind::morph,next));}
   else if(z && p.type==132 && challenge)e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Ventral_Sacs)));
   else if(z && p.type==131 && !e.count_units(0,132,false))e.perform(option_code(OptionKind::morph,132));
   else if(!z && p.type==base && e.count_units(0,worker)<20)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==167)e.perform(option_code(OptionKind::train,70));
   else if(!z && p.type==160 && e.count_units(0,65)+e.count_units(0,66)<18){unsigned next=e.count_units(0,164) && e.count_units(0,66)<30?66:65;if(!e.perform(option_code(OptionKind::train,next)) && next==66)e.perform(option_code(OptionKind::train,65));}
  }
  if(frame%4800==0){if(!z)printf("Scouts%u Stargates%u enemies%zu ",e.count_units(0,70),e.count_units(0,167),enemies.size());printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);assert(s.winner==0 && loaded_hero && transported && e.count_units(0,51)==1 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs))==1);SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),(std::string("/tmp/sc3ds-third-replay-")+m.key).c_str(),(std::string("/tmp/sc3ds-third-map-")+m.key).c_str(),restored));assert(e.state_digest()==replay.state_digest());puts("The Dark Templar original victory, paid army/transport research, Overlord flight, lone surviving Kerrigan challenge and exact completed replay PASS");
}
