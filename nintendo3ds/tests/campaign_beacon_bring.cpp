// Original shipyard terrain: solid-pad contact must satisfy its fitted Bring
// location. Isolated fixture; no shipped mission is substituted.
#include "actions.h"
#include <fstream>
#include <cstdio>
#include <cassert>
using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);
 for(auto type:{UnitTypes::Special_Zerg_Beacon,UnitTypes::Special_Terran_Beacon,UnitTypes::Special_Protoss_Beacon,UnitTypes::Terran_Supply_Depot}){
  game_player p(data_loading::data_files_directory(argv[1]));std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)),{});game_load_functions loader(p.st());loader.load_map_data(bytes.data(),bytes.size(),[&]{loader.setup_info.create_no_units=true;for(auto& v:p.st().players)v.controller=player_t::controller_occupied;},false);p.st().game->triggers.clear();for(auto& t:p.st().running_triggers)t.clear();
  auto& f=p.funcs();auto* pad=f.trigger_create_unit(f.get_unit_type(type),{4400,3328},0);auto* pilot=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Civilian),{4700,3328},0);assert(pad && pilot);
  action_state state;action_functions commands(p.st(),state);commands.action_select(0,pilot);assert(commands.action_order(0,commands.get_order_type(Orders::Move),{4400,3328},nullptr,nullptr,false));for(unsigned n=0;n<1000;++n)p.next_frame();
  auto delta=pilot->sprite->position-xy(4400,3328);assert(delta.x*delta.x+delta.y*delta.y>32*32);
  if(type!=UnitTypes::Terran_Supply_Depot){location loc;loc.area={xy(4352,3296),xy(4448,3360)};loc.elevation_flags=56;auto box=f.unit_sprite_inner_bounding_box(pilot);auto counts=f.trigger_bring_count(loc);printf("pad%u pilot%d,%d bbox%d,%d..%d,%d count%u\n",unsigned(type),pilot->sprite->position.x,pilot->sprite->position.y,box.from.x,box.from.y,box.to.x,box.to.y,unsigned(counts.unit_counts[0][UnitTypes::Terran_Civilian]));assert(counts.unit_counts[0][UnitTypes::Terran_Civilian]==1);}

 }
 puts("Civilian touching original control-pad location counts for Bring; all solid collision/placement retained PASS");
}
