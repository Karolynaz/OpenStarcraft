#include "engine.h"
#include <cstdio>
#include <stdexcept>
int main(int argc,char** argv){
 if(argc!=3)return 2;
 using namespace port3ds;Engine engine;
 const Scenario modes[]={Scenario::terran_lab,Scenario::protoss_lab,Scenario::zerg_lab,
   Scenario::terran_battle,Scenario::protoss_battle,Scenario::zerg_battle,
   Scenario::protoss_lab,Scenario::zerg_lab,Scenario::terran_lab};
 for(unsigned pass=0;pass<2;++pass)for(auto mode:modes){
  if(!engine.load(argv[1],argv[2],mode))throw std::runtime_error(engine.error());
  for(unsigned frame=0;frame<24;++frame)if(!engine.tick())throw std::runtime_error(engine.error());
  const auto state=engine.snapshot();if(!engine.ready() || state.frame!=24 || !state.units)throw std::runtime_error("Reload lost native state");
  UnitView units[512];const auto n=engine.visible_units(units,512);bool selected=false;
  for(unsigned i=0;i<n && !selected;++i)if(units[i].owner==0)selected=engine.select(units[i].x,units[i].y);
  if(!selected || !engine.snapshot().selected)throw std::runtime_error("Selection after reload");
  SpriteView images[2048];const auto sprites=engine.visible_images(images,2048,0,0);
  for(unsigned i=0;i<sprites;++i)if(!engine.sprite_info(images[i].image,images[i].frame).valid)throw std::runtime_error("Stale image after reload");
 }
 std::puts("Single-engine cross-race battle/lab reload / native state / selection / GRP lifetime (18 loads) PASS");
}
