#pragma once
#include "engine.h"
#include "protoss_catalogue.h"
#include <functional>
#include <vector>
#include <cstdio>
#include <stdexcept>
inline void protoss_construction_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* why){if(!ok){std::fprintf(stderr,"Protoss construction: %s / %s\n",why,e.command_status());throw std::runtime_error(why);}};
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"construction tick");if(observe)observe(e);}};
 auto units=[&](){std::vector<UnitView>a(512);a.resize(e.visible_units(a.data(),a.size()));return a;};
 require(e.load(data,arena,Scenario::protoss_lab),"load construction lab");tick(24);
 auto builder=[&](){for(auto v:units())if(v.owner==0 && v.worker && e.select(v.x,v.y))return true;return false;};
 auto build=[&](UnitTypes type,int x,int y){
  require(builder(),"select Probe");const auto before=e.count_units(0,unsigned(type));const auto resources=e.snapshot();
  require(e.placement_unit(unsigned(type),x,y).valid && e.perform(option_code(OptionKind::build,unsigned(type)),x,y),"legal warp placement");
  for(unsigned n=0;n<6000 && e.count_units(0,unsigned(type))==before;++n)tick(1);
  require(e.count_units(0,unsigned(type))>before,"native warp completes");require(e.snapshot().minerals<resources.minerals,"warp pays resources");tick(24);
 };
 require(builder() && !e.placement_unit(unsigned(UnitTypes::Protoss_Gateway),128,1850).valid,"reject unpowered Gateway");
 // Expand power using ordinary Probe/Pylon construction, not a global psi cheat.
 for(int y:{1440,1792})for(int x:{128,448,800,1120})if(builder() && e.placement_unit(unsigned(UnitTypes::Protoss_Pylon),x,y).valid)build(UnitTypes::Protoss_Pylon,x,y);
 for(const auto& r:protoss::buildings){
  require(builder(),"choose builder");bool found=false;int px=0,py=0;
  if(r.type==UnitTypes::Protoss_Assimilator){for(auto u:units())if(u.geyser && e.placement_unit(unsigned(r.type),u.x,u.y).valid){px=u.x;py=u.y;found=true;break;}}
  else for(int y=1152;y<1950 && !found;y+=32)for(int x=96;x<1450 && !found;x+=32){
   bool free=true;for(auto u:units())if(!u.worker && std::abs(x-u.x)<128 && std::abs(y-u.y)<104){free=false;break;}
   if(free && e.placement_unit(unsigned(r.type),x,y).valid){px=x;py=y;found=true;}
  }
  if(!found)std::fprintf(stderr,"No powered site for %s\n",r.name);require(found,"powered unoccupied site");build(r.type,px,py);std::printf("Protoss constructed %s\n",r.name);
 }
 std::puts("Protoss all sixteen Probe buildings / legal psi / geyser construction PASS");
}
