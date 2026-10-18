#pragma once
#include "engine.h"
#include "terran_catalogue.h"
#include <functional>
#include <vector>
#include <stdexcept>
#include <cstdio>
inline void terran_construction_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}) {
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* message){if(!ok){std::fprintf(stderr,"Construction: %s / %s\n",message,e.command_status());throw std::runtime_error(message);}};
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"tick");if(observe)observe(e);}};
 auto units=[&](){std::vector<UnitView> v(512);v.resize(e.visible_units(v.data(),v.size()));return v;};
 require(e.load(data,arena,Scenario::terran_lab),"load construction lab");tick(24);
 auto build=[&](const terran::Product& r){
  UnitView worker{};bool found=false;for(auto u:units())if(u.owner==0 && u.worker && u.completed){worker=u;found=true;break;}
  require(found && e.select(worker.x,worker.y),"select builder");
  const auto count=e.count_units(0,unsigned(r.type));bool placed=false;int px=0,py=0;
  if(r.type==UnitTypes::Terran_Refinery){for(auto u:units())if(u.geyser && e.placement_unit(unsigned(r.type),u.x,u.y).valid){px=u.x;py=u.y;placed=e.perform(option_code(OptionKind::build,unsigned(r.type)),px,py);if(placed)break;}}
  else for(int y=1100;y<1950 && !placed;y+=64)for(int x=128;x<1900 && !placed;x+=64){
   bool room=true;for(auto u:units())if(u.owner==0 && !u.worker && std::abs(x+64-u.x)<190 && std::abs(y-u.y)<125){room=false;break;}
   if(room && e.placement_unit(unsigned(r.type),x,y).valid){px=x;py=y;placed=e.perform(option_code(OptionKind::build,unsigned(r.type)),x,y);}
  }
  if(!placed)std::fprintf(stderr,"No site for %s\n",r.name);require(placed,"legal SCV construction");
  for(unsigned n=0;n<6000 && e.count_units(0,unsigned(r.type))==count;++n)tick(1);
  require(e.count_units(0,unsigned(r.type))>count,"construction completes");
  UnitView result{};int best=0x7fffffff;for(auto u:units())if(u.owner==0 && u.type==unsigned(r.type) && u.completed){int d=(u.x-px)*(u.x-px)+(u.y-py)*(u.y-py);if(d<best){best=d;result=u;}}
  tick(24); // Let the finishing SCV leave construction and restore collision.
  std::printf("Built %s\n",r.name);return result;
 };
 for(unsigned i=0;i<12;++i)build(terran::buildings[i]);
 for(unsigned i=12;i<18;++i){const auto& addon=terran::buildings[i];const terran::Product* recipe=nullptr;for(const auto& r:terran::buildings)if(r.type==addon.parent)recipe=&r;
  require(recipe,"addon parent recipe");auto parent=build(*recipe);require(e.select(parent.x,parent.y),"select new addon parent");
  auto count=e.count_units(0,unsigned(addon.type));require(e.perform(option_code(OptionKind::addon,unsigned(addon.type))),"native addon placement");
  for(unsigned n=0;n<5000 && e.count_units(0,unsigned(addon.type))==count;++n)tick(1);
  require(e.count_units(0,unsigned(addon.type))>count,"addon completes");std::printf("Built addon %s\n",addon.name);
 }
 std::puts("All twelve SCV buildings and all six legal add-ons built through public native orders PASS");
}
