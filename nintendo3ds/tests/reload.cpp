#include "engine.h"
#include <cstdio>
#include <stdexcept>
int main(int argc,char** argv){
 if(argc!=3 && argc!=4)return 2;
 using namespace port3ds;Engine engine;unsigned loads=0;
 const Scenario modes[]={Scenario::terran_lab,Scenario::protoss_lab,Scenario::zerg_lab,
   Scenario::terran_battle,Scenario::protoss_battle,Scenario::zerg_battle,
   Scenario::protoss_lab,Scenario::zerg_lab,Scenario::terran_lab};
 auto reload=[&](Scenario mode,const char* map){
  if(!engine.load(argv[1],map,mode))throw std::runtime_error(engine.error());
  for(unsigned frame=0;frame<24;++frame)if(!engine.tick())throw std::runtime_error(engine.error());
  const auto state=engine.snapshot();if(!engine.ready() || state.frame!=24 || !state.units)throw std::runtime_error("Reload lost native state");
  UnitView units[512];const auto n=engine.visible_units(units,512);bool selected=false;
  for(unsigned i=0;i<n && !selected;++i)if(units[i].owner==0)selected=engine.select(units[i].x,units[i].y);
  if(!selected || !engine.snapshot().selected)throw std::runtime_error("Selection after reload");
  SpriteView images[2048];const auto sprites=engine.visible_images(images,2048,0,0);
  for(unsigned i=0;i<sprites;++i)if(!engine.sprite_info(images[i].image,images[i].frame).valid)throw std::runtime_error("Stale image after reload");
  ++loads;
 };
 for(unsigned pass=0;pass<2;++pass){
  for(auto mode:modes)reload(mode,argv[2]);
  if(argc==4){
   reload(Scenario::protoss_ai_battle,argv[3]);
   if(!engine.snapshot().observer || !engine.snapshot().players[0].ai)throw std::runtime_error("Protoss observer flags");
   reload(Scenario::zerg_ai_battle,argv[3]);
   if(!engine.snapshot().observer || scenario_race(Scenario::zerg_ai_battle)!=0 || arena_scenario(Scenario::zerg_ai_battle))throw std::runtime_error("Zerg economy classification");
   reload(Scenario::zerg_versus_ai,argv[3]);
   if(!engine.fog_enabled() || engine.snapshot().players[0].ai || !engine.snapshot().players[1].ai)throw std::runtime_error("Zerg manual ownership / fog after reload");
   reload(Scenario::zerg_lab,argv[2]);
   reload(Scenario::protoss_versus_ai,argv[3]);
   const auto s=engine.snapshot();if(s.observer || s.players[0].ai || !s.players[1].ai)throw std::runtime_error("Manual Protoss ownership after reload");
  }
 }
 std::printf("Single-engine cross-race battle/lab/economy map reload / native state / selection / GRP lifetime (%u loads) PASS\n",loads);
}
