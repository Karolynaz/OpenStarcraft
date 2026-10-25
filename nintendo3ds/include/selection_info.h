#pragma once
#include <array>
namespace port3ds {
enum class WorkKind {none,training,construction,morph,addon,research,upgrade};
struct SelectionInfo {
 bool valid=false,own=false,building=false,completed=false;
 unsigned type=0,count=0,queue_count=0,work_icon=~0u;
 const char* name="";const char* work_name="";
 int hp=0,max_hp=0,shields=0,max_shields=0,energy=0,max_energy=0;
 int armor=0,ground_damage=0,air_damage=0,kills=0;
 int armor_level=0,shield_level=0,ground_level=0,air_level=0;bool ground_weapon=false,air_weapon=false;
 int supply_used=0,supply_provided=0,supply_total=0;
 int remaining=0,total=0;WorkKind work=WorkKind::none;
 std::array<unsigned,5> queue{{~0u,~0u,~0u,~0u,~0u}};
};
// Shared drawing/touch geometry in the 224x78 bottom selection region.
struct ProductionPanel {
 struct Cell{int x,y,w,h;};
 static Cell cell(unsigned slot){return slot==0?Cell{94,174,26,26}:Cell{134+int(slot-1)*26,205,24,24};}
 static int hit(int x,int y,const SelectionInfo& info){
  if(!info.valid || info.count!=1 || !info.own || info.work==WorkKind::none)return -1;
  const unsigned count=info.work==WorkKind::training?info.queue_count:1;
  for(unsigned i=0;i<count && i<5;++i){const auto c=cell(i);if(x>=c.x && y>=c.y && x<c.x+c.w && y<c.y+c.h)return int(i);}return -1;
 }
};
}
