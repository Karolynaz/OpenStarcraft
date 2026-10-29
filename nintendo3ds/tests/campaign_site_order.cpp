// Start-site numbering must survive campaign human-owner normalization.
#include "bwgame.h"
#include "campaign_map.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});a_vector<xy> expected;
 for(unsigned pass=0;pass<2;++pass){if(pass)port3ds::campaign::normalize(map,4);game_player p(assets);game_load_functions loader(p.st());loader.load_map_data(map.data(),map.size(),[&]{loader.setup_info.create_no_units=true;for(auto& player:p.st().players)player.controller=player_t::controller_occupied;});a_vector<xy> positions;for(unsigned n=0;n<p.st().game->start_location_count;++n)positions.push_back(p.st().game->start_locations[p.st().game->start_location_order[n]]);assert(!positions.empty());if(pass)assert(expected==positions);else expected=positions;
  loader.load_map_data(map.data(),map.size(),[&]{loader.setup_info.create_no_units=true;for(auto& player:p.st().players)player.controller=player_t::controller_occupied;});assert(p.st().game->start_location_count==positions.size());
 }puts("CHK start-site order preserved across owner normalization and reload without duplicates PASS");}
