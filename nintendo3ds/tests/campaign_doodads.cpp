// Native iscript/collision/weapon checks on an owned installation map. No
// purchased map data is compiled in. Original triggers are suppressed only in
// this isolated state fixture; shipped map triggers remain intact.
#include "bwgame.h"
#include "campaign_runtime.h"
#include "campaign_ai.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;
struct Adapter:state_functions{port3ds::detail::CampaignRuntime campaign;port3ds::detail::CampaignAI campaign_ai;
#include "campaign_triggers.h"
explicit Adapter(state& s):state_functions(s){campaign.active=true;}
void run(const trigger::action& a){execute_trigger_struct ets{};running_trigger rt{};running_trigger::action ra{};bool handled=false;assert(campaign_action(ets,0,rt,ra,a,handled) && handled);}
};
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);game_player player(assets);game_load_functions loader(player.st());std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> chk((std::istreambuf_iterator<char>(in)),{});loader.load_map_data(chk.data(),chk.size(),[&]{for(auto& p:player.st().players){p.controller=player_t::controller_occupied;p.race=race_t::terran;}player.st().players[11].controller=player_t::controller_neutral;});Adapter f(player.st());auto& st=player.st();st.game->triggers.clear();for(auto& list:st.running_triggers)list.clear();std::array<unit_t*,4> doors{};unit_t* trap=nullptr;unit_t* marine=nullptr;
for(auto* u:ptr(st.visible_units)){if(f.unit_is_door(u))doors[unsigned(u->unit_type->id)-unsigned(UnitTypes::Special_Upper_Level_Door)]=u;if(f.unit_is(u,UnitTypes::Special_Floor_Gun_Trap))trap=u;if(f.unit_is(u,UnitTypes::Terran_Marine) && !marine)marine=u;if(f.unit_is_trap(u))f.set_doodad_state(u,false);}assert(trap && marine);
auto ticks=[&](unsigned n){while(n--)f.next_frame();};ticks(128);
for(auto* door:doors){assert(door);f.set_doodad_state(door,true);ticks(160);assert(!f.u_no_collide(door) && !f.u_disabled(door));assert(f.unit_can_collide_with(marine,door));f.set_doodad_state(door,false);assert(door->order_type->id==Orders::OpenDoor && !f.u_no_collide(door));ticks(160);assert(f.u_no_collide(door) && f.u_disabled(door) && door->sprite->elevation_level==1);assert(!f.unit_can_collide_with(marine,door));f.set_doodad_state(door,true);ticks(160);assert(!f.u_no_collide(door) && !f.u_disabled(door) && door->sprite->elevation_level==door->unit_type->elevation_level);}
// Location+owner filter and legacy zero/toggle are exercised through action42.
location area{};auto pos=doors[0]->sprite->position;area.area={pos-xy(16,16),pos+xy(16,16)};st.locations.push_back(area);trigger::action a{};a.type=42;a.location=st.locations.size();a.group_n=17;a.extra_n=int(doors[0]->unit_type->id);a.num_n=0;f.run(a);ticks(160);assert(f.u_no_collide(doors[0]));a.num_n=6;f.run(a);ticks(160);assert(!f.u_no_collide(doors[0]));
// Disabled floor gun cannot hurt a nearby hostile. Enabling it must reveal,
// fire the original weapon, then hide again after the target is removed.
auto* victim=f.trigger_create_unit(f.get_unit_type(UnitTypes::Terran_Marine),trap->sprite->position+xy(64,0),0);assert(victim);f.u_set_status_flag(victim,unit_t::status_flag_disabled);f.set_unit_hp(victim,fp8::integer(1000));int owner=trap->owner;assert(owner!=0);st.alliances[owner][0]=st.alliances[0][owner]=0;auto hp=victim->hp;ticks(120);assert(victim->hp==hp && f.u_no_collide(trap));f.set_doodad_state(trap,true);bool revealed=false,hit=false;for(unsigned i=0;i<600 && !hit;++i){f.next_frame();revealed|=!f.u_cloaked(trap);hit=victim->hp<hp;}assert(revealed && hit);f.kill_unit(victim);ticks(400);assert(trap->order_type->id==Orders::HiddenGun && f.u_cloaked(trap) && f.u_no_collide(trap));
printf("Installation: all4 original door animation/collision cycles, action42 location/legacy/toggle, disabled gun + real reveal/weapon damage/hide PASS frame%u\n",st.current_frame);}
