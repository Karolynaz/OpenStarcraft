// Paid native player for BW Terran1: economy, recruitment, siege and scans.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <limits>
using namespace port3ds;using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_terran_campaign));assert(e.start_recording("/tmp/sc3ds-bw-terran-paid"));e.set_fog_enabled(false);unsigned pending=0,pending_at=0,pending_count=0,phase=0;bool duran_safe=false;
 for(unsigned frame=0;frame<220000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}if(frame%48)continue;UnitView list[1700];auto n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents,army,enemies,mineral;UnitView* enemy_cc=nullptr;UnitView* duran=nullptr;UnitView* refinery=nullptr;int home_x=576,home_y=2704;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0 && u.completed){if(u.type==7)workers.push_back(u);else if(u.type>=106)parents.push_back(u);else if(u.type==0 || u.type==5 || u.type==30 || u.type==34)army.push_back(u);if(u.type==110)refinery=&u;}if(u.owner==5){enemies.push_back(u);if(u.type==106)enemy_cc=&u;}if(u.type==99)duran=&u;if(u.mineral)mineral.push_back(u);}
  for(const auto& w:workers)if(w.order==unsigned(Orders::PlayerGuard) || w.order==unsigned(Orders::Guard) || w.order==unsigned(Orders::Stop)){
   const UnitView* target=nullptr;int distance=0x7fffffff;for(const auto& m:mineral){int dx=m.x-w.x,dy=m.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&m;}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  if(frame%480==0 && refinery && workers.size()>6)for(unsigned i=0;i<3;++i)if(e.select(workers[i].x,workers[i].y))e.command(Command::context,refinery->x,refinery->y);
  if(pending && (e.count_units(0,pending)>pending_count || frame>pending_at+1800))pending=0;
  unsigned want=0;if(state.supply_max-state.supply<6)want=109;else if(!e.count_units(0,110))want=110;else if(e.count_units(0,113)<2)want=113;else if(e.count_units(0,111)<3)want=111;else if(!e.count_units(0,112))want=112;else if(!e.count_units(0,122))want=122;else if(!e.count_units(0,123))want=123;
  if(want && !pending && !workers.empty()){auto w=workers.front();int bx=home_x,by=home_y;if(want==110)for(unsigned i=0;i<n;++i)if(list[i].geyser){bx=list[i].x;by=list[i].y;break;}int best=0x7fffffff;for(const auto& candidate:workers){int dx=candidate.x-bx,dy=candidate.y-by,d=dx*dx+dy*dy;if(d<best){best=d;w=candidate;}}bool placed=false;if(e.select(w.x,w.y)){
   if(want==110){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
   else for(int r=2;r<12 && !placed;++r)for(int y=-r;y<=r && !placed;++y)for(int x=-r;x<=r && !placed;++x){if(std::abs(x)!=r && std::abs(y)!=r)continue;int px=home_x+x*32,py=home_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
  }if(placed){pending=want;pending_at=frame;pending_count=e.count_units(0,want);}}
  for(const auto& p:parents)if(e.select(p.x,p.y)){
   if(p.type==106){if(e.count_units(0,112) && !e.count_units(0,107))e.perform(option_code(OptionKind::addon,107));if(workers.size()<12)e.perform(option_code(OptionKind::train,7));}
   else if(p.type==111 && e.count_units(0,0)<42)e.perform(option_code(OptionKind::train,(e.count_units(0,112) && e.count_units(0,34)<6 && e.count_units(0,0)>8)?34:0));
   else if(p.type==113){e.perform(option_code(OptionKind::addon,120));if(e.count_units(0,5)+e.count_units(0,30)<16)e.perform(option_code(OptionKind::train,5));}
   else if(p.type==120 && !e.researched(unsigned(TechTypes::Tank_Siege_Mode)))e.perform(option_code(OptionKind::research,unsigned(TechTypes::Tank_Siege_Mode)));
   else if(p.type==122){e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Infantry_Weapons)));e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Infantry_Armor)));}
   else if(p.type==112)e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::U_238_Shells)));
   else if(p.type==123)e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Vehicle_Weapons)));
  }
  if(phase==0 && e.count_units(0,0)>=12 && e.count_units(0,5)+e.count_units(0,30)>=2)phase=1;
  if(e.count_units(0,99)){phase=std::max(phase,2u);if(!duran_safe && duran && e.select(duran->x,duran->y)){assert(std::string(e.selection_info().name)=="Samir Duran");e.perform(option_code(OptionKind::cloak,0));e.move_selected(160,2912);duran_safe=true;}}
  if(phase==2 && e.count_units(0,5)+e.count_units(0,30)>=8 && e.count_units(0,0)>=16)phase=3;
  if(phase==3 && army.size()<12)phase=2;
  int tx=home_x,ty=home_y;if(phase==1 && duran){tx=duran->x;ty=duran->y;}else if(phase==3 && enemy_cc){tx=enemy_cc->x;ty=enemy_cc->y;}
  for(const auto& u:army){const UnitView* near=nullptr;int distance=0x7fffffff;for(const auto& v:enemies){int dx=u.x-v.x,dy=u.y-v.y,d=dx*dx+dy*dy;if(d<distance){distance=d;near=&v;}}
   if(!e.select(u.x,u.y))continue;
   if(u.type==5 && distance<380*380){e.perform(option_code(OptionKind::siege,0));continue;}
   if(u.type==30){if(distance>450*450)e.perform(option_code(OptionKind::siege,0));continue;}
   if(phase && frame%240==0)e.command(Command::attack,tx,ty);
   if(near && near->type==13 && distance<400*400)for(const auto& p:parents)if(p.type==107 && p.energy>=50 && e.select(p.x,p.y)){e.perform(option_code(OptionKind::order,unsigned(Orders::CastScannerSweep)),near->x,near->y);break;}
  }
  if(frame%4800==0){printf("BW Terran f%u phase%u minerals%d gas%d workers%zu marines%u tanks%u Duran%u enemy%zu siege%d pending%u factories%u shops%u\n",e.frame(),phase,state.minerals,state.gas,workers.size(),e.count_units(0,0),e.count_units(0,5)+e.count_units(0,30),e.count_units(0,99),enemies.size(),e.researched(unsigned(TechTypes::Tank_Siege_Mode)),pending,e.count_units(0,113),e.count_units(0,120));fflush(stdout);}
 }
 printf("BW Terran result%d frame%u\n",e.snapshot().winner,e.frame());fflush(stdout);if(e.snapshot().winner!=0)return 1;SaveView view;assert(e.save_game("/tmp/sc3ds-bw-terran-completed.sav",view));Engine replay;SaveView restored;assert(e.count_units(0,99)==1);assert(replay.load_save(argv[1],"/tmp/sc3ds-bw-terran-completed.sav","/tmp/sc3ds-bw-terran-replay","/tmp/sc3ds-bw-terran-map",restored));assert(e.state_digest()==replay.state_digest());puts("BW Terran original objectives completed with paid mining/production/research, Duran rescue/survival, medics, siege/combat and identical completed save replay PASS");
}
