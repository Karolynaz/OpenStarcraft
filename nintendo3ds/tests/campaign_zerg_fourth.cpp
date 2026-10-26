// Original Zerg4: protect the Chrysalis and Kerrigan; a paid Mutalisk fleet
// crosses the lava, destroys Raynor's CC and reaches the original victory.
// Uses public controls and native restrictions, resources and placement only.
// Fog is disabled for repeatable test navigation; no map/triggers/unit stats edited.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);const auto& m=campaign::missions[campaign::key_index("zerg04")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const bool z=true;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 unsigned build_pending=0,build_at=0,build_count=0;bool attack=false,countdown_seen=false,changed_objective=false;unsigned hatch_frame=0,voices=0;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<220000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}SoundEvent events[256];auto sounds=e.drain_sounds(events,256);for(unsigned j=0;j<sounds;++j)voices+=events[j].id>=1200;auto snapshot=e.snapshot();countdown_seen|=snapshot.countdown_active && snapshot.countdown_seconds>0;if(!hatch_frame && e.count_units(0,51))hatch_frame=e.frame();changed_objective|=hatch_frame && snapshot.objectives.find("Raynor")!=std::string::npos;
if(frame<12000)assert(e.count_units(2,106)==1 && !e.count_units(0,51) && snapshot.winner<0);if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==2)enemies.push_back(u);}
  if(!home_x)continue;
  
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66)+e.count_units(0,z?43:70);
  if(e.count_units(0,43)>=20)attack=true;
  if(e.count_units(0,43)<6)attack=false;
  if(attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(!z)for(const auto& u:enemies)if(u.type>=106 && u.type<202){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(target->type<106 || d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==43)if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
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
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<18?worker:e.count_units(0,141)?43:e.count_units(0,135)?38:37;e.perform(option_code(OptionKind::morph,next));}
   else if(z && p.type==131 && !e.count_units(0,132,false))e.perform(option_code(OptionKind::morph,132));
   else if(!z && p.type==base && e.count_units(0,worker)<20)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==167)e.perform(option_code(OptionKind::train,70));
   else if(!z && p.type==160 && e.count_units(0,65)+e.count_units(0,66)<18){unsigned next=e.count_units(0,164) && e.count_units(0,66)<30?66:65;if(!e.perform(option_code(OptionKind::train,next)) && next==66)e.perform(option_code(OptionKind::train,65));}
  }
  if(frame%4800==0){printf("Queens%u Mutas%u Hydras%u Kerrigan%u CC2%u supply%d/%d ",e.count_units(0,45),e.count_units(0,43),e.count_units(0,38),e.count_units(0,51),e.count_units(2,106),state.supply,state.supply_max);for(const auto&u:enemies)if(u.type==106)printf("CC2HP%d at%d,%d ",u.hp,u.x,u.y);if(!z)printf("Scouts%u Stargates%u enemies%zu ",e.count_units(0,70),e.count_units(0,167),enemies.size());printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);if(s.winner!=0)return 1;assert(countdown_seen && hatch_frame>=14000 && changed_objective && e.count_units(0,51)==1 && e.count_units(2,106)==0 && voices>=10);printf("Chrysalis timer/hatch%u, changed objectives, paid Mutalisk fleet, living Kerrigan and%u speech events PASS\n",hatch_frame,voices);SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),(std::string("/tmp/sc3ds-third-replay-")+m.key).c_str(),(std::string("/tmp/sc3ds-third-map-")+m.key).c_str(),restored));assert(e.state_digest()==replay.state_digest());puts("Agents of the Swarm original countdown/hatch/progression + paid air assault/victory/completed save replay PASS");
}
