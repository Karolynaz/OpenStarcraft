#include "hud_layout.h"
#include "command_grid.h"
#include "selection_info.h"
#include "controls.h"
#include "ui_art.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <string>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char**argv){assert(argc==2);using namespace port3ds;
 for(unsigned i=0;i<12;++i){const auto r=HudLayout::selection(i);assert(r.x>=19 && r.y>=16 && r.x+r.w<=96 && r.y+r.h<=225);assert(r.hit(r.x+35,r.y+33));assert(!r.hit(r.x+36,r.y));}
 for(unsigned i=0;i<5;++i){const auto c=ProductionPanel::cell(i);SelectionInfo busy;busy.valid=busy.own=true;busy.count=1;busy.work=WorkKind::training;busy.queue_count=5;assert(ProductionPanel::hit(c.x+2,c.y+2,busy)==int(i));assert(c.x>=19 && c.x+c.w<=97 && c.y+c.h<=225);}
 assert(HudLayout::menu().hit(152,137) && !HudLayout::menu().hit(205,137));
 Controls controls;controls.map_size(128,96);auto m=controls.minimap_bounds();assert(m.x==206 && m.y==142 && m.w==96 && m.h==72);assert(!controls.minimap_touch(210,135));assert(controls.minimap_touch(254,178));assert(controls.camera_x==1848 && controls.camera_y==1416);
 for(unsigned race=0;race<3;++race){UiArt art;assert(art.load((std::string(argv[1])+"/ui/"+HudLayout::skin(race)).c_str()));unsigned w=0,h=0;assert(art.size("hud",w,h) && w==320 && h==240);assert(fake::allocations==1);
  for(const char* key:{"key_A","key_X","key_Y","key_R+A","key_R+B","key_L+A"}){assert(art.size(key,w,h) && h==12);assert(w==12 || w==23);}
  assert(art.size("t10_W_41",w,h) && h==13);fake::draws.clear();assert(art.draw("hud",0,0));auto d=fake::draws.back();assert(d.sub.width==320 && d.sub.height==240 && d.sx==1 && d.sy==1);
  for(unsigned i=0;i<9;++i){auto c=CommandGrid::cell(i);assert(c.w==36 && c.h==34);assert(c.x>=122 && c.x+c.w<=306 && c.y+c.h<=104);}
 }
 assert(fake::allocations==0);puts("Three race HUD pages/native badges; Figma selection/5-queue/menu bounds; aspect-fit minimap and camera touch PASS");
}
