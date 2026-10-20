#include "graphics.h"
#include "ability_controls.h"
#include "bwenums.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char** argv){
 assert(argc==4);using namespace port3ds;using bwgame::UnitTypes;
 Engine engine;assert(engine.load(argv[1],argv[2],Scenario::protoss_versus_ai));assert(engine.fog_enabled());
 for(unsigned i=0;i<24;++i)assert(engine.tick());
 UnitView units[512];auto n=engine.visible_units(units,512);UnitView home{},probe{},enemy{};
 for(unsigned i=0;i<n;++i){assert(units[i].owner!=1);if(units[i].type==unsigned(UnitTypes::Protoss_Nexus))home=units[i];if(units[i].worker)probe=units[i];}
 assert(home.max_hp && probe.worker);assert(engine.fog_cell(home.x/32,home.y/32)==FogCell::visible);
 engine.set_fog_enabled(false);n=engine.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==1 && units[i].type==unsigned(UnitTypes::Protoss_Nexus))enemy=units[i];assert(enemy.max_hp);
 assert(engine.cursor_kind(enemy.x,enemy.y)==CursorKind::enemy);engine.set_fog_enabled(true);
 assert(engine.fog_cell(enemy.x/32,enemy.y/32)==FogCell::unexplored && engine.cursor_kind(enemy.x,enemy.y)==CursorKind::arrow);
 SpriteView images[2048];assert(engine.visible_images(images,2048,home.x-200,home.y-120)>0);assert(engine.visible_images(images,2048,enemy.x-200,enemy.y-120)==0);
 assert(engine.select(probe.x,probe.y) && engine.move_selected(enemy.x,enemy.y));
 unsigned scout=0;while(engine.fog_cell(enemy.x/32,enemy.y/32)!=FogCell::visible && scout<3600){assert(engine.tick());++scout;}
 assert(scout<3600);assert(engine.visible_images(images,2048,enemy.x-200,enemy.y-120)>0);assert(engine.cursor_kind(enemy.x,enemy.y)==CursorKind::enemy);
 assert(engine.move_selected(home.x,home.y));unsigned retreat=0;
 while(engine.fog_cell(enemy.x/32,enemy.y/32)==FogCell::visible && retreat<3600){assert(engine.tick());++retreat;}
 assert(retreat<3600 && engine.fog_cell(enemy.x/32,enemy.y/32)==FogCell::explored);
 assert(engine.cursor_kind(enemy.x,enemy.y)==CursorKind::arrow && engine.visible_images(images,2048,enemy.x-200,enemy.y-120)>0);
 const auto memories=engine.remembered_units(units,512);bool nexus_memory=false;
 for(unsigned i=0;i<memories;++i){assert(!units[i].selected && units[i].hp==0);if(units[i].type==unsigned(UnitTypes::Protoss_Nexus) && units[i].x==enemy.x && units[i].y==enemy.y)nexus_memory=true;}
 assert(nexus_memory);n=engine.visible_units(units,512);for(unsigned i=0;i<n;++i)assert(units[i].owner!=1);
 const auto frozen=engine.visible_images(images,2048,enemy.x-200,enemy.y-120);std::vector<SpriteView> memory_images(images,images+frozen);
 for(unsigned i=0;i<24;++i)assert(engine.tick());assert(engine.visible_images(images,2048,enemy.x-200,enemy.y-120)==frozen);
 for(unsigned i=0;i<frozen;++i)assert(images[i].image==memory_images[i].image && images[i].frame==memory_images[i].frame && images[i].x==memory_images[i].x && images[i].y==memory_images[i].y);
 assert(engine.move_selected(enemy.x,enemy.y));unsigned revisit=0;
 while(engine.fog_cell(enemy.x/32,enemy.y/32)!=FogCell::visible && revisit<3600){assert(engine.tick());++revisit;}assert(revisit<3600);
 const auto revisited=engine.remembered_units(units,512);for(unsigned i=0;i<revisited;++i)assert(units[i].x!=enemy.x || units[i].y!=enemy.y);
 assert(engine.move_selected(home.x,home.y));while(engine.fog_cell(enemy.x/32,enemy.y/32)==FogCell::visible)assert(engine.tick());
 {
  Graphics graphics;assert(graphics.load(argv[3],engine.terrain_signature()));assert(graphics.sprites(memory_images.data(),memory_images.size(),enemy.x-200,enemy.y-120,engine)==0);fake::draws.clear();graphics.fog(engine,0,0);assert(fake::draws.size()==1);const auto* tex=static_cast<const uint16_t*>(fake::draws[0].tex->data);unsigned states[3]{};
  for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x){const auto state=engine.fog_cell(x,y);++states[unsigned(state)];const unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);assert(tex[((y/8)*8+x/8)*64+m]==(state==FogCell::visible?0:state==FogCell::explored?10:15));}
  assert(states[0] && states[1] && states[2]);graphics.fog_minimap(engine,8,144,120,88);assert(fake::draws.size()==2 && fake::draws[0].tex==fake::draws[1].tex);
  engine.set_fog_enabled(false);graphics.fog(engine,0,0);graphics.fog_minimap(engine,8,144,120,88);assert(fake::draws.size()==2);
 }
 assert(fake::allocations==0);
 Engine observer;assert(observer.load(argv[1],argv[2],Scenario::protoss_ai_battle));for(unsigned i=0;i<24;++i)assert(observer.tick());
 assert(observer.select(enemy.x,enemy.y) && observer.store_group(0));observer.set_fog_enabled(true);
 int hidden_x=-1,hidden_y=-1;assert(!observer.recall_group(0,&hidden_x,&hidden_y) && hidden_x==-1 && hidden_y==-1 && observer.snapshot().selected==0);
 observer.set_fog_enabled(false);assert(observer.recall_group(0,&hidden_x,&hidden_y) && hidden_x==enemy.x && hidden_y==enemy.y);
 Engine radar;const std::string probe_map=argv[2];const auto arena=probe_map.substr(0,probe_map.find_last_of('/')+1)+"arena.chk";
 assert(radar.load(argv[1],arena.c_str(),Scenario::terran_lab));for(unsigned i=0;i<24;++i)assert(radar.tick());radar.set_fog_enabled(true);
 n=radar.visible_units(units,512);UnitView comsat{};for(unsigned i=0;i<n;++i)if(units[i].type==unsigned(UnitTypes::Terran_Comsat_Station))comsat=units[i];assert(comsat.max_hp);
 assert(radar.fog_cell(59,59)==FogCell::explored && radar.select(comsat.x,comsat.y));
 const auto energy=radar.snapshot().selected_energy;assert(energy>=50);assert(radar.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastScannerSweep)),1900,1900));
 unsigned reveal=0;while(radar.fog_cell(59,59)!=FogCell::visible && reveal<120){assert(radar.tick());++reveal;}assert(reveal<120 && radar.snapshot().selected_energy<energy);
 while(radar.snapshot().selected_energy>=50){
  const auto old_energy=radar.snapshot().selected_energy;assert(radar.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastScannerSweep)),1900,1900));
  unsigned wait=0;while(radar.snapshot().selected_energy>=old_energy && wait<120){assert(radar.tick());++wait;}assert(wait<120);
 }
 MenuOption options[8];const auto count=radar.options(OptionPage::primary,options,8);bool disabled=false;for(unsigned i=0;i<count;++i)if((options[i].code&65535)==unsigned(bwgame::Orders::CastScannerSweep)){disabled=!options[i].enabled;}assert(disabled);
 unsigned expire=0;while(radar.fog_cell(59,59)==FogCell::visible && expire<720){assert(radar.tick());++expire;}assert(expire<720 && radar.fog_cell(59,59)==FogCell::explored);
 std::printf("Native Scanner Sweep reveals fog in %u frames, expires in %u; real energy consumed and unavailable UI disabled PASS\n",reveal,expire);
 assert(ability_shortcut(8,false,true)==-1 && ability_shortcut(1,true,true)==-1);
 assert(ability_shortcut(0,false)==-1 && ability_shortcut(0,true)==-1);
 assert(ability_shortcut(1,false)==0 && ability_shortcut(1,true)==-1);
 assert(ability_shortcut(2,false)==0 && ability_shortcut(2,true)==1);
 assert(ability_shortcut(3,false)==0 && ability_shortcut(3,true)==-1 && ability_shortcut(4,true)==-1);
 std::printf("Native fog: scout reveals enemy in %u frames, retreat restores explored fog in %u; mobile units/hover withheld; frozen building/resource images and minimap memories persist until revisited; 4096 fog pixels and shared minimap mask; Y/L+Y policy PASS\n",scout,retreat);
}
