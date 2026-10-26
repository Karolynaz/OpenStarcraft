// Original BW Terran1 contains six preplaced addon pairs, in both UNIT orders.
#include "bwgame.h"
#include <fstream>
#include <cassert>
#include <cstdio>
using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream input(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(input)),{});loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=false;loader.setup_info.victory_condition=0;for(auto& p:player.st().players)if(p.controller==player_t::controller_computer || p.controller==player_t::controller_open)p.controller=player_t::controller_occupied;});auto& f=player.funcs();unsigned links=0;unit_t* factory=nullptr;
 for(auto* u:ptr(player.st().visible_units))if(f.u_grounded_building(u) && !f.ut_addon(u) && u->building.addon){auto* addon=u->building.addon;assert(f.unit_can_attach_addon(u,addon) && u->owner==addon->owner);++links;if(u->owner==0 && f.unit_is(u,UnitTypes::Terran_Factory))factory=u;}
 assert(links==6 && factory);auto* tank=f.get_unit_type(UnitTypes::Terran_Siege_Tank_Tank_Mode);assert(f.unit_can_build(factory,tank));auto& st=player.st();st.current_minerals[0]=1000;st.current_gas[0]=1000;assert(f.train_unit(factory,tank));assert(st.current_minerals[0]==1000-tank->mineral_cost && st.current_gas[0]==1000-tank->gas_cost);
 printf("Original BW Terran opening: %u serial-linked addon pairs, player factory permits paid tank training PASS\n",links);
}
