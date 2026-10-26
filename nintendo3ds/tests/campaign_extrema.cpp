#include "bwgame.h"
#include "campaign_runtime.h"
#include "campaign_ai.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;
struct Adapter:state_functions{port3ds::detail::CampaignRuntime campaign;port3ds::detail::CampaignAI campaign_ai;
#include "campaign_triggers.h"
explicit Adapter(state& s):state_functions(s){campaign.active=true;}};
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(in)),{});loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=true;for(unsigned p=0;p<12;++p){player.st().players[p].controller=p<2?player_t::controller_occupied:player_t::controller_inactive;player.st().players[p].race=race_t::terran;}});Adapter f(player.st());auto& st=player.st();st.locations[0]={{xy(128,128),xy(768,768)},0};auto unit=[&](unsigned owner,xy pos){assert(f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_Marine),pos,owner));};unit(0,{384,384});unit(0,{448,384});unit(1,{512,384});unit(1,{1024,1024});trigger::condition c{};c.type=7;c.location=1;c.unit_id=0;assert(f.test_trigger_condition(c,0) && !f.test_trigger_condition(c,1));c.type=17;assert(!f.test_trigger_condition(c,0) && f.test_trigger_condition(c,1));unit(1,{576,384});for(int type:{7,17}){c.type=type;assert(f.test_trigger_condition(c,0) && f.test_trigger_condition(c,1));}c.type=6;assert(!f.test_trigger_condition(c,0) && f.test_trigger_condition(c,1));c.type=16;assert(f.test_trigger_condition(c,0) && !f.test_trigger_condition(c,1));c.type=7;c.unit_id=230;assert(f.test_trigger_condition(c,0));st.players[2].controller=player_t::controller_occupied;c.type=17;assert(!f.test_trigger_condition(c,0));puts("Command most/least globally and at location: active opponents, outside-unit exclusion, class counts and ties PASS");}
