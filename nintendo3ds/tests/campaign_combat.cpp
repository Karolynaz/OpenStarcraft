#include "bwgame.h"
#include "campaign_runtime.h"
#include "campaign_ai.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;
struct Adapter:state_functions{
 port3ds::detail::CampaignRuntime campaign;port3ds::detail::CampaignAI campaign_ai;
 #include "campaign_triggers.h"
 explicit Adapter(state& s):state_functions(s){campaign.active=true;}
};
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(in)),{});loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=true;for(auto& p:player.st().players)p.controller=player_t::controller_occupied;});Adapter f(player.st());auto& st=player.st();st.game->triggers.clear();for(auto& r:st.running_triggers)r.clear();st.alliances[0][1]=st.alliances[1][0]=2;
auto create=[&](UnitTypes type,xy pos,unsigned owner){auto* u=f.trigger_create_unit(f.get_unit_type(type),pos,owner);assert(u);return u;};auto* killer=create(UnitTypes::Terran_Marine,{384,384},0);auto* ally=create(UnitTypes::Terran_Marine,{512,512},1);f.unit_deal_damage(ally,fp8::integer(100),killer,0);assert(killer->kill_count==1 && f.campaign.kills[0][0]==1);f.unit_deal_damage(ally,fp8::integer(100),killer,0);assert(killer->kill_count==1);
auto* own=create(UnitTypes::Terran_Marine,{576,512},0);f.unit_deal_damage(own,fp8::integer(100),killer,0);assert(killer->kill_count==1 && f.campaign.kills[0][0]==1);auto* removed=create(UnitTypes::Terran_Marine,{640,512},1);f.kill_unit(removed);assert(f.campaign.kills[0][0]==1);
trigger::condition c{};c.type=5;c.group=0;c.unit_id=0;c.num_n=10;c.count_n=1;assert(f.test_trigger_condition(c,0));c.unit_id=230;assert(f.test_trigger_condition(c,0));
auto* hallu=create(UnitTypes::Terran_Marine,{704,512},1);f.make_unit_hallucination(hallu);f.unit_deal_damage(hallu,fp8::integer(100),killer,0);assert(f.campaign.kills[0][0]==1 && killer->kill_count==1);
auto* tank=create(UnitTypes::Terran_Goliath,{768,768},0);auto* turret=f.unit_turret(tank);assert(turret);auto* target=create(UnitTypes::Terran_Marine,{864,768},1);f.unit_deal_damage(target,fp8::integer(100),turret,0);assert(tank->kill_count==1 && turret->kill_count==0 && f.campaign.kills[0][0]==2);
auto* transport=create(UnitTypes::Protoss_Shuttle,{1100,900},1);auto* passenger=create(UnitTypes::Terran_Marine,{1100,960},1);f.unit_load_target(transport,passenger);assert(f.u_loaded(passenger));killer->kill_count=254;f.unit_deal_damage(transport,fp8::integer(1000),killer,0);assert(killer->kill_count==255 && f.campaign.kills[0][0]==3 && f.campaign.kills[0][unsigned(UnitTypes::Protoss_Shuttle)]==1);
auto* queen=create(UnitTypes::Zerg_Queen,{1200,1200},0);auto* victim=create(UnitTypes::Protoss_Zealot,{1300,1200},1);f.spawn_broodlings(victim,queen);assert(queen->kill_count==1 && f.campaign.kills[0][unsigned(UnitTypes::Protoss_Zealot)]==1);
puts("Combat Kill conditions/UI counts: ally yes, self/trigger/hallucination no; repeat hit, turret parent, cargo,255 cap and Broodlings PASS");}
