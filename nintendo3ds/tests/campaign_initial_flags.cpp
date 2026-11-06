#include "bwgame.h"
#include "campaign_runtime.h"
#include <fstream>
#include <cassert>
#include <cstdio>
using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream input(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(input)),{});loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=false;loader.setup_info.victory_condition=0;for(auto& p:player.st().players)if(p.controller==player_t::controller_computer || p.controller==player_t::controller_open)p.controller=player_t::controller_occupied;});auto& f=player.funcs();port3ds::detail::CampaignRuntime::prepare_burrowed_ambushers(f);unsigned hives=0,burrowed=0,tunnels=0;
 for(auto* u:ptr(player.st().player_units[1]))if(f.unit_is(u,UnitTypes::Zerg_Hive)){assert(f.u_invincible(u));++hives;}
 for(unsigned owner:{4u,5u})for(auto* u:ptr(player.st().player_units[owner]))if(f.u_burrowed(u)){assert(f.u_cloaked(u) && f.u_requires_detector(u) && f.unit_is_undetected(u,0));++burrowed;}
 for(auto* u:ptr(player.st().player_units[1]))if(f.unit_is_nydus(u)){auto* exit=u->building.nydus.exit;assert(exit && exit!=u && f.unit_is_nydus(exit) && exit->owner==u->owner && exit->building.nydus.exit==u);++tunnels;}assert(tunnels==8);
 assert(hives==7 && burrowed==88);printf("Owned BWZerg opening: %u protected hives and %u pre-burrowed ambushers and %u serial-linked Nydus entrances PASS\n",hives,burrowed,tunnels);
}
