#include "fog_memory.h"
#include "creep_graphics.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace bwgame;
int main(int argc,char** argv){
 assert(argc==3);game_player player;player.init(data_loading::data_files_directory(argv[1]));
 a_vector<uint8_t> chk;data_loading::mpq_file<> map(argv[2]);map(chk,"staredit/scenario.chk");
 game_load_functions loader(player.st());loader.load_map_data(chk.data(),chk.size(),[&]{
  loader.setup_info.create_no_units=false;loader.setup_info.victory_condition=0;
  for(unsigned p=0;p<12;++p){player.st().players[p].controller=p<2?player_t::controller_occupied:player_t::controller_inactive;player.st().players[p].race=race_t::protoss;loader.setup_info.create_melee_units_for_player[p]=p<2;}
 });
 auto& f=player.funcs();auto& st=player.st();unit_t* nexus=nullptr;
 for(auto* u:ptr(st.player_units[1]))if(f.unit_is(u,UnitTypes::Protoss_Nexus))nexus=u;assert(nexus);
 // Isolate presentation visibility without changing simulation orders/resources.
 for(auto& tile:st.tiles){tile.visible|=1;tile.explored&=~1;}
 for(auto& line:st.sprites_on_tile_line)for(auto& sprite:line)sprite.visibility_flags&=~1;
 const auto position=nexus->sprite->position;const unsigned index=position.y/32*st.game->map_tile_width+position.x/32;
 nexus->sprite->visibility_flags|=1;st.tiles[index].visible&=~1;st.tiles[index].flags|=tile_t::flag_has_creep;
 port3ds::detail::FogMemory memory;memory.reset();memory.update(f);
 port3ds::UnitView markers[128];assert(memory.markers(markers,128)==0);
 assert(memory.creep(position.x/32,position.y/32));
 nexus->sprite->visibility_flags&=~1;st.tiles[index].visible|=1;st.tiles[index].flags&=~tile_t::flag_has_creep;memory.update(f);
 assert(memory.creep(position.x/32,position.y/32));assert(memory.markers(markers,128)==1);
 port3ds::SpriteView before[128],after[128];const auto n=memory.images(before,128,position.x-200,position.y-120,f);assert(n>0);
 // Unobserved destruction must not erase the last-seen building or creep.
 f.kill_unit(nexus);memory.update(f);assert(memory.markers(markers,128)==1);
 assert(memory.images(after,128,position.x-200,position.y-120,f)==n);
 for(unsigned i=0;i<n;++i){assert(before[i].image==after[i].image && before[i].frame==after[i].frame && before[i].x==after[i].x && before[i].y==after[i].y);}
 assert(memory.creep(position.x/32,position.y/32));
 // Revisiting the now-empty footprint clears both memories, not before.
 st.tiles[index].visible&=~1;memory.update(f);assert(memory.markers(markers,128)==0 && !memory.creep(position.x/32,position.y/32));
 memory.reset();assert(memory.markers(markers,128)==0);
 unsigned common=0,rare=0;bool seen[13]{};
 for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x){const auto v=port3ds::detail::creep_variant(x,y);assert(v<13);seen[v]=true;v<6?++common:++rare;}
 for(bool b:seen)assert(b);assert(rare>100 && rare<220);
 std::printf("Frozen fog images survive unseen destruction; empty-footprint revisit clears building and creep; reset clears memories; valid creep variants %u common/%u rare PASS\n",common,rare);
}
