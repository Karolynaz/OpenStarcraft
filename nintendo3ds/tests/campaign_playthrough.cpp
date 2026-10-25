// Automated player using public controls and paid native production only.
// Fog is disabled for repeatable test navigation; no map/triggers/unit stats edited.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==4);unsigned mission=unsigned(std::atoi(argv[3]));assert(mission==1 || mission==2);const auto& m=campaign::missions[mission];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const bool z=mission==1;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 unsigned build_pending=0,build_at=0;bool attack=false;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<120000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==(z?5u:7u))enemies.push_back(u);}
  // Meet the rescue colony and Fenix with the starting force before producing.
  if(!z && !home_x){UnitView* rescue=nullptr;for(unsigned i=0;i<n;++i)if((list[i].owner==1 && list[i].type==154) || (list[i].owner==2 && list[i].type==77))rescue=&list[i];if(rescue){for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==65 || list[i].type==66 || list[i].type==77))if(e.select(list[i].x,list[i].y))e.command(Command::attack,rescue->x,rescue->y);}continue;}
  if(!home_x)continue;
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66);
  if((z && e.count_units(0,38)>=26) || (!z && army>=24))attack=true;
  if(attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==(z?37u:65u) || list[i].type==(z?38u:66u) || (!z && list[i].type==77)))if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop)){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  // Assign three workers to the paid refinery once it has completed.
  if(frame%480==0 && e.count_units(0,gas) && workers.size()>=7){UnitView* refinery=nullptr;for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==gas && list[i].completed)refinery=&list[i];if(refinery)for(unsigned i=0;i<3;++i)if(e.select(workers[i].x,workers[i].y))e.command(Command::context,refinery->x,refinery->y);}
  if(build_pending && (e.count_units(0,build_pending) || frame>build_at+1500))build_pending=0;
  unsigned want=0;
  if(state.supply_max-state.supply<5 && !z)want=supply;
  if(!want && !e.count_units(0,z?142:160))want=z?142:160;
  if(!want && !e.count_units(0,gas))want=gas;
  if(!want && z && !e.count_units(0,135))want=135;
  if(!want && !z && !e.count_units(0,164))want=164;
  if(!build_pending && want && !workers.empty()){
   bool placed=false;auto w=workers.back();if(e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=home_x+x*32,py=home_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<9?worker:e.count_units(0,135)?38:37;e.perform(option_code(OptionKind::morph,next));}
   else if(!z && p.type==base && e.count_units(0,worker)<10)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==160){unsigned next=e.count_units(0,164) && e.count_units(0,66)<12?66:65;e.perform(option_code(OptionKind::train,next));}
  }
  if(frame%4800==0){printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);if(s.winner!=0)return 1;SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));puts("Original mission completed via paid native production, mining, rescue and combat PASS");
}
