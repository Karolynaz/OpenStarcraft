#pragma once
// Native trigger presentation for the first audited retail mission. Text stays
// in the user's map string table; no purchased dialogue is compiled into code.
namespace port3ds { namespace detail {
struct CampaignRuntime {
 bool active=false,cinematic=false;unsigned objective=0,message=0,message_until=0,camera_serial=0;
 bwgame::xy camera{};
 static std::string text(const bwgame::state& st,unsigned id){
  if(!id || id>st.game->map_strings.size())return {};
  if(id==20)return "TIP\nCircle Pad moves the cursor. A selects or gives an order; B cancels. X selects the army on screen. L+X selects workers.";
  const auto& raw=st.game->map_strings[id-1];std::string out;bool space=false;
  for(unsigned char c:raw){if(c=='\r')continue;if(c=='\n'){while(!out.empty() && out.back()==' ')out.pop_back();out+='\n';space=false;}
   else if(c>=32){if(c==' '){if(space || out.empty() || out.back()=='\n')continue;space=true;}else space=false;out+=char(c);}}
  return out;
 }
 void rescue(bwgame::state_functions& f){
  using namespace bwgame;
  for(unsigned p=1;p<8;++p)if(f.st.players[p].controller==player_t::controller_rescue_passive){
   for(auto it=f.st.player_units[p].begin();it!=f.st.player_units[p].end();){unit_t* u=&*it++;if(f.unit_dead(u) || f.ut_turret(u))continue;
    bool close=false;for(auto* v:ptr(f.st.player_units[0])){if(f.unit_dead(v) || f.ut_building(v) || f.us_hidden(v))continue;auto d=v->sprite->position-u->sprite->position;if(d.x*d.x+d.y*d.y<=96*96){close=true;break;}}
    if(close){f.give_unit_to(u,0);f.set_unit_order(u,u->unit_type->human_ai_idle);}
   }
  }
 }
};
}}
