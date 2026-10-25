#pragma once
// Native trigger presentation for the first audited retail mission. Text stays
// in the user's map string table; no purchased dialogue is compiled into code.
namespace port3ds { namespace detail {
struct CampaignRuntime {
 bool active=false,cinematic=false,suppress_deaths=false;unsigned mission=0,objective=0,message=0,message_until=0,camera_serial=0;
 std::array<bool,256> switches{};std::array<int,12> scores{};
 std::array<std::array<unsigned,228>,12> deaths{};std::array<unsigned,1700> death_ids{};
 std::array<std::array<uint8_t,20>,64> properties{};
 unsigned ping_frame=0;bwgame::xy ping{};
 bwgame::xy camera{};
 std::string text(const bwgame::state& st,unsigned id)const{
  if(!id || id>st.game->map_strings.size())return {};
  if(mission==0 && id==20)return "TIP\nCircle Pad moves the cursor. A selects or gives an order; B cancels. Select selects units on screen. R+X selects the army; R+Y selects workers.";
  const auto& raw=st.game->map_strings[id-1];std::string out;bool space=false;
  for(unsigned char c:raw){if(c=='\r')continue;if(c=='\n'){while(!out.empty() && out.back()==' ')out.pop_back();out+='\n';space=false;}
   else if(c>=32){if(c==' '){if(space || out.empty() || out.back()=='\n')continue;space=true;}else space=false;out+=char(c);}}
  return out;
 }
 void rescue(bwgame::state_functions& f){
  using namespace bwgame;
  auto transfer=[&](unit_t* u){if(f.trigger_give_unit_to(u,0))f.set_unit_order(u,f.get_order_type(f.u_in_bunker(u)?Orders::BunkerGuard:u->unit_type->human_ai_idle->id));};
  for(unsigned p=1;p<8;++p)if(f.st.players[p].controller==player_t::controller_rescue_passive){
   // A rescued resource depot transfers the owner's entire colony. Other
   // rescuable actors transfer individually. Search uses the original 64px
   // rectangle against unit collision boxes, including friendly air units.
   for(auto it=f.st.player_units[p].begin();it!=f.st.player_units[p].end();){unit_t* u=&*it++;if(f.unit_dying(u) || f.ut_turret(u) || f.us_hidden(u))continue;
    bool close=false;auto pos=u->sprite->position;for(auto* v:f.find_units({pos-xy(64,64),pos+xy(64,64)}))if(v->owner==0 && !f.unit_dying(v) && !f.us_hidden(v)){close=true;break;}
    if(!close)continue;
    const bool depot=f.u_grounded_building(u) && f.ut_resource_depot(u) && (f.u_completed(u) || f.unit_is(u,UnitTypes::Zerg_Lair) || f.unit_is(u,UnitTypes::Zerg_Hive) || f.unit_is(u,UnitTypes::Zerg_Hatchery));
    if(depot){a_vector<unit_id_32> colony;for(auto* v:ptr(f.st.player_units[p]))if(!f.unit_dying(v) && !f.ut_turret(v))colony.push_back(f.get_unit_id_32(v));for(auto id:colony)if(auto* v=f.get_unit(id))if(v->owner==p && !f.unit_dying(v))transfer(v);break;}
    transfer(u);
   }
  }
 }
};
}}
