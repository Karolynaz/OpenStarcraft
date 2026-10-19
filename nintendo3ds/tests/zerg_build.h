#pragma once
#include "engine.h"
#include "zerg_catalogue.h"
#include <functional>
#include <string>
#include <cstdio>
#include <stdexcept>
inline void zerg_construction_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* why){if(!ok){std::fprintf(stderr,"Zerg construction: %s / %s\n",why,e.command_status());throw std::runtime_error(why);}};
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"tick");if(observe)observe(e);}};
 auto select=[&](UnitTypes type){UnitView units[512];const auto n=e.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==unsigned(type) && units[i].completed)return e.select(units[i].x,units[i].y);return false;};
 auto build=[&](UnitTypes type){
  require(select(UnitTypes::Zerg_Drone),"select Drone");bool placed=false;
  for(int y=96;y<1952 && !placed;y+=32)for(int x=96;x<1952 && !placed;x+=32)if(e.placement_unit(unsigned(type),x,y).valid)placed=e.perform(option_code(OptionKind::build,unsigned(type)),x,y);
  require(placed,"free legal creep construction site");
 };
 require(e.load(data,arena,Scenario::zerg_lab),"load construction lab");tick(500);
 require(select(UnitTypes::Zerg_Drone),"Drone present");require(!e.placement_unit(unsigned(UnitTypes::Zerg_Spawning_Pool),64,1952).valid,"off-creep Pool rejected");
 const auto cancel_money=e.snapshot().minerals;const auto cancel_drones=e.count_units(0,unsigned(UnitTypes::Zerg_Drone));build(UnitTypes::Zerg_Spawning_Pool);
 bool cancelled=false;for(unsigned frame=0;frame<1800 && !cancelled;++frame){tick(1);UnitView units[512];const auto n=e.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==unsigned(UnitTypes::Zerg_Spawning_Pool) && !units[i].completed){require(e.select(units[i].x,units[i].y),"select incomplete Drone building");MenuOption rows[8];require(e.options(OptionPage::primary,rows,8)==1 && rows[0].code==option_code(OptionKind::cancel),"construction cancel menu");require(e.perform(rows[0].code),"cancel Drone construction");cancelled=true;break;}}
 require(cancelled,"construction started before cancellation");tick(24);require(e.count_units(0,unsigned(UnitTypes::Zerg_Drone))==cancel_drones,"cancel returns Drone");require(e.snapshot().minerals==cancel_money-50,"native 75 percent construction refund");
 for(const auto& r:zerg::buildings){
  const unsigned before=e.count_units(0,unsigned(r.type));const unsigned drones=e.count_units(0,unsigned(UnitTypes::Zerg_Drone));
  if(r.parent==UnitTypes::Zerg_Drone)build(r.type);
  else {if(r.type==UnitTypes::Zerg_Spore_Colony && !e.count_units(0,unsigned(UnitTypes::Zerg_Creep_Colony))){build(UnitTypes::Zerg_Creep_Colony);tick(3000);}require(select(r.parent) && e.perform(option_code(OptionKind::morph,unsigned(r.type))),"building morph accepted");}
  for(unsigned n=0;n<6000 && e.count_units(0,unsigned(r.type))==before;++n)tick(1);
  require(e.count_units(0,unsigned(r.type))==before+1,"building complete");
  if(r.parent==UnitTypes::Zerg_Drone)require(e.count_units(0,unsigned(UnitTypes::Zerg_Drone))==drones-1,"Drone consumed by construction");
  std::printf("Zerg constructed %s\n",r.name);
 }
 require(select(UnitTypes::Zerg_Nydus_Canal),"select Nydus");const auto count=e.count_units(0,unsigned(UnitTypes::Zerg_Nydus_Canal));bool exit=false;PlacementView exit_site;
 for(int y=96;y<1952 && !exit;y+=32)for(int x=96;x<1952 && !exit;x+=32){const auto site=e.placement_unit(unsigned(UnitTypes::Zerg_Nydus_Canal),x,y);if(site.valid && e.perform(option_code(OptionKind::nydus),x,y)){exit=true;exit_site=site;}}
 require(exit,"native Nydus exit accepted");for(unsigned n=0;n<6000 && e.count_units(0,unsigned(UnitTypes::Zerg_Nydus_Canal))==count;++n)tick(1);require(e.count_units(0,unsigned(UnitTypes::Zerg_Nydus_Canal))==count+1,"paired Nydus exit completed");
 UnitView view[512];auto visible=e.visible_units(view,512);UnitView entrance{};for(unsigned i=0;i<visible;++i)if(view[i].selected)entrance=view[i];require(entrance.type==unsigned(UnitTypes::Zerg_Nydus_Canal),"Nydus entrance selection retained");
 require(select(UnitTypes::Zerg_Hatchery) && e.activate(entrance.x,entrance.y) && e.snapshot().selected_name==std::string("Nydus Canal"),"building selection stays selection");
 require(select(UnitTypes::Zerg_Drone) && e.activate(entrance.x,entrance.y),"contextual EnterNydusCanal order");bool travelled=false;
 for(unsigned frame=0;frame<3000 && !travelled;++frame){tick(1);visible=e.visible_units(view,512);for(unsigned i=0;i<visible;++i)if(view[i].selected && std::abs(view[i].x-(exit_site.x+exit_site.width/2))<160 && std::abs(view[i].y-(exit_site.y+exit_site.height/2))<160)travelled=true;}
 require(travelled,"Drone emerges at paired Nydus exit");
 std::puts("Zerg sixteen structures / creep enforcement / Drone consumption / paired Nydus construction and transit PASS");
}
