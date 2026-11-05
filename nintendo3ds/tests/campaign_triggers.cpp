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
 void run(trigger::action a){execute_trigger_struct ets;running_trigger rt;running_trigger::action ra;bool handled=false;assert(campaign_action(ets,0,rt,ra,a,handled));if(!handled)assert(state_functions::execute_trigger_action(ets,0,rt,ra,a));}
};
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream input(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(input)),{});loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=true;for(auto& p:player.st().players)p.controller=player_t::controller_occupied;});Adapter f(player.st());auto& st=player.st();st.locations[0]={{xy(128,128),xy(768,768)},0};st.locations[1]={{xy(800,800),xy(864,864)},0};
 trigger::action a{};trigger::condition c{};
 a.type=13;a.group2_n=70;a.num_n=4;f.run(a);c.type=11;c.extra_n=70;c.num_n=2;assert(f.test_trigger_condition(c,0));a.num_n=6;f.run(a);assert(!f.test_trigger_condition(c,0));c.num_n=3;assert(f.test_trigger_condition(c,0));
 a={};a.type=27;a.group_n=13;a.extra_n=7;a.num_n=7;a.group2_n=100;f.run(a);a.num_n=9;a.group2_n=40;f.run(a);c={};c.type=21;c.group=0;c.extra_n=7;c.num_n=10;c.count_n=60;assert(f.test_trigger_condition(c,0));
 // Campaign authors also use unused player slots as integer storage (BW T2).
 st.players[7].controller=player_t::controller_inactive;
 a={};a.type=27;a.group_n=7;a.extra_n=7;a.num_n=8;a.group2_n=1;f.run(a);c={};c.type=21;c.group=7;c.extra_n=7;c.num_n=10;c.count_n=1;assert(f.test_trigger_condition(c,0));
 a={};a.type=45;a.group_n=7;a.extra_n=15;a.num_n=7;a.group2_n=4;f.run(a);c={};c.type=15;c.group=7;c.unit_id=15;c.num_n=10;c.count_n=4;assert(f.test_trigger_condition(c,0));assert(!f.player_slot_active(7));
 a={};a.type=26;a.group_n=0;a.extra_n=2;a.num_n=7;a.group2_n=100;f.run(a);a.num_n=9;a.group2_n=40;f.run(a);assert(st.current_minerals[0]==60 && st.current_gas[0]==60);assert(st.total_minerals_gathered[0]==100);
 auto& prop=f.campaign.properties[0];prop[0]=16;prop[2]=2|4|8;prop[5]=25;prop[6]=50;prop[7]=25;prop[14]=16;
 a={};a.type=11;a.location=1;a.group_n=0;a.group2_n=1;a.extra_n=64;a.num_n=1;a.flags=8;f.run(a);unit_t* probe=nullptr;for(auto* u:ptr(st.player_units[0]))if(f.unit_is(u,UnitTypes::Protoss_Probe))probe=u;assert(probe && f.u_invincible(probe) && probe->hp.integer_part()==5 && probe->shield_points.integer_part()==10);
 assert(f.unit_is_at_elevation_flags(probe,56) && !f.unit_is_at_elevation_flags(probe,7));auto* air=f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_Wraith),xy(700,700),0);assert(air && f.unit_is_at_elevation_flags(air,7) && !f.unit_is_at_elevation_flags(air,56));
 a={};a.type=43;a.location=1;a.group_n=0;a.extra_n=64;a.num_n=5;f.run(a);assert(!f.u_invincible(probe));
 a={};a.type=49;a.location=1;a.group_n=0;a.extra_n=64;a.group2_n=50;f.run(a);assert(probe->hp.integer_part()==10);
 a={};a.type=48;a.location=1;a.group_n=0;a.group2_n=1;a.extra_n=64;f.run(a);assert(probe->owner==1);
 a={};a.type=39;a.location=1;a.group_n=1;a.group2_n=2;a.extra_n=64;f.run(a);assert(probe->sprite->position.x>=768 && probe->sprite->position.y>=768);
 auto* removed=f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_Marine),xy(384,384),0);assert(removed);a={};a.type=25;a.location=1;a.group_n=0;a.extra_n=0;a.num_n=1;f.run(a);assert(f.campaign.deaths[0][0]==0);
 auto* killed=f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_Marine),xy(416,416),0);assert(killed);f.kill_unit(killed);assert(f.campaign.deaths[0][0]==1);c={};c.type=15;c.group=0;c.unit_id=0;c.num_n=10;c.count_n=1;assert(f.test_trigger_condition(c,0));
 // Original CHK countdown fields: seconds in time_n, arithmetic in num_n.
 a={};a.type=14;a.time_n=2;a.num_n=7;f.run(a);c={};c.type=1;c.num_n=10;c.count_n=2;assert(f.test_trigger_condition(c,0));
 for(unsigned i=0;i<23;++i)f.campaign.advance();assert(f.campaign.countdown()==2);f.campaign.advance();assert(f.campaign.countdown()==1);
 a={};a.type=54;f.run(a);for(unsigned i=0;i<48;++i)f.campaign.advance();assert(f.campaign.countdown()==1);a.type=55;f.run(a);for(unsigned i=0;i<24;++i)f.campaign.advance();assert(f.campaign.countdown()==0);
 a={};a.type=14;a.time_n=0xffffffffu;a.num_n=7;f.run(a);assert(f.campaign.countdown()==0xffffffffu);a.num_n=8;f.run(a);assert(f.campaign.countdown()==0xffffffffu);a.num_n=9;f.run(a);assert(!f.campaign.countdown());
 c={};c.type=23;assert(!f.test_trigger_condition(c,0));
 a={};a.type=45;a.group_n=0;a.extra_n=0;a.group2_n=4;a.num_n=7;f.run(a);a.num_n=8;f.run(a);assert(f.campaign.deaths[0][0]==8);a.num_n=9;a.group2_n=20;f.run(a);assert(!f.campaign.deaths[0][0]);
 // Retail campaign records use count zero for one creation (not zero).
 // https://docs.scmjs.dev/triggers/create-unit/
 a={};a.type=11;a.location=1;a.group_n=0;a.extra_n=0;a.num_n=0;unsigned before=st.completed_unit_counts[0][UnitTypes::Terran_Marine];f.run(a);assert(st.completed_unit_counts[0][UnitTypes::Terran_Marine]==before+1);a.type=44;f.run(a);assert(st.completed_unit_counts[0][UnitTypes::Terran_Marine]==before+2);
 a={};a.type=57;a.group_n=1;a.extra_n=1;f.run(a);assert(st.alliances[0][1]==2);a.extra_n=0;f.run(a);assert(st.alliances[0][1]==0);
 // Original Zer1 must produce an actual paid wave from a sparse town,
 // rather than merely dispatching units that were placed in the map.
 st.game->triggers.clear();for(auto& list:st.running_triggers)list.clear();f.hide_unit(air);f.kill_unit(air);
 for(auto* u:ptr(st.player_units[0]))if(f.unit_is(u,UnitTypes::Terran_Marine) && !f.unit_dying(u)){f.hide_unit(u);f.kill_unit(u);}
 st.players[1].race=race_t::terran;st.current_minerals[1]=4000;st.current_gas[1]=1000;
 unit_t* cc=nullptr;for(int y=512;y<int(st.game->map_height)-320 && !cc;y+=96)for(int x=512;x<int(st.game->map_width)-320 && !cc;x+=96)cc=loader.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Command_Center),xy(x,y),1);assert(cc);auto town=cc->sprite->position;
 for(unsigned i=0;i<4;++i)assert(f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_SCV),town+xy(-64+i*32,144),1));
 assert(f.trigger_create_unit(f.get_unit_type(UnitTypes(176)),town+xy(224,128),11));
 f.campaign_ai.load([&](a_vector<uint8_t>& bytes,const a_string& name){assets(bytes,name);});f.campaign_ai.start(f,0x3172655a,1,cc->sprite->position);
 const auto gathered_before=st.total_minerals_gathered[1];for(unsigned frame=0;frame<9000;++frame){f.next_frame();f.campaign_ai.step(f);}
 unsigned marines=0,barracks=0;for(auto* u:ptr(st.player_units[1]))if(!f.unit_dying(u) && f.u_completed(u)){if(f.unit_is(u,UnitTypes::Terran_Marine))++marines;if(f.unit_is(u,UnitTypes::Terran_Barracks))++barracks;}
 printf("Original Zer1 paid town: barracks%u marines%u minerals%d\n",barracks,marines,st.current_minerals[1]);const auto spent=4000+(st.total_minerals_gathered[1]-gathered_before)-st.current_minerals[1];printf("Paid native mineral expenditure%d\n",spent);assert(barracks && marines && spent>=300);
 puts("Native campaign switches, custom score, resource subtract, UPRP, invincibility, HP, ownership, relocation, deaths vs removal and alliances PASS");
}
