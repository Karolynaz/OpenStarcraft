// Owned original BW Terran2 terrain: script-created control-pad beacons may
// precede the player's exploration. Ordinary placement still requires vision.
#include "engine.h"
#include "bwgame.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace port3ds;
using namespace bwgame;
int main(int argc,char**argv){
 assert(argc==3);
 game_player arena(data_loading::data_files_directory(argv[1]));
 std::ifstream input(argv[2],std::ios::binary);
 a_vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)),{});
 game_load_functions loader(arena.st());
 loader.load_map_data(bytes.data(),bytes.size(),[&]{
  loader.setup_info.create_no_units=true;
  for(auto& player:arena.st().players)player.controller=player_t::controller_occupied;
 },false);
 auto& f=arena.funcs();const auto* beacon=f.get_unit_type(UnitTypes::Special_Terran_Beacon);
 for(unsigned location:{2u,5u,6u,28u,29u,35u,49u}){
  const auto& area=arena.st().locations[location-1].area;auto position=(area.from+area.to)/2;
  assert(!f.can_place_building(nullptr,0,beacon,position,true,false));
  assert(f.can_place_building(nullptr,0,beacon,position,true,false,false));
  const auto before=arena.st().tiles[f.tile_index({96,96})].explored;
  auto* unit=f.trigger_create_unit(beacon,position,0);
  assert(unit && f.u_completed(unit) && unit->sprite->position==position);
  assert(arena.st().tiles[f.tile_index({96,96})].explored==before);
  // Ignoring vision must not ignore the existing building footprint.
  assert(!f.trigger_create_unit(beacon,position,0));
 }
 // Original void terrain remains unsuitable, even for a scenario request.
 assert(!f.can_place_building(nullptr,0,beacon,{48,32},true,false,false));
 assert(!f.trigger_create_unit(beacon,{48,32},0));
 Engine engine;assert(engine.load(argv[1],argv[2],Scenario::bw_terran_campaign));
 assert(engine.start_recording("/tmp/sc3ds-trigger-beacons.journal"));
 for(unsigned i=0;i<600;++i){assert(engine.tick());SoundEvent sounds[256];engine.drain_sounds(sounds,256);}
 assert(engine.count_units(0,unsigned(UnitTypes::Special_Terran_Beacon))==3);
 SaveView view;view.camera_x=4200;view.camera_y=3100;assert(engine.save_game("/tmp/sc3ds-trigger-beacons.ob3",view));
 Engine replay;SaveView restored;
 assert(replay.load_save(argv[1],"/tmp/sc3ds-trigger-beacons.ob3","/tmp/sc3ds-trigger-beacons-replay","/tmp/sc3ds-trigger-beacons-map",restored));
 assert(engine.state_digest()==replay.state_digest());
 for(unsigned i=0;i<48;++i){assert(engine.tick() && replay.tick());assert(engine.state_digest()==replay.state_digest());}
 puts("Unexplored original control-pad markers, unchanged remote fog, normal vision/terrain/occupancy rejection, original three-beacon opening and exact save+48 PASS");
}
