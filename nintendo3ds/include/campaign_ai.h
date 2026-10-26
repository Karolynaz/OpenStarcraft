#pragma once
// Bounded interpreter for the original campaign AIScript bytecode.
// Scripts remain private assets. Purchases, placement, research and orders use
// native action validation. Only explicit retail scenario creation opcodes may
// create actors directly; ordinary economy requests always pay native costs.
#include "actions.h"
#include "race_catalogue.h"
namespace port3ds { namespace detail {
using namespace bwgame;
class CampaignAI {
 struct Thread {unsigned pc=0,wake=0,owner=0;bool brood=false,active=false;xy center{};std::array<unsigned,16> calls{};unsigned depth=0;bool killable=false,kill_pending=false;};
 struct Guard {bool active=false;unsigned owner=0,type=0,deaths=0;unit_id_32 unit{};xy position{};};
 struct Side {
  bool enabled=false,suicide=false,target_expansion=false;xy center{};unsigned search=0,build_frame=0;unit_id_32 builder{};
  bool nuclear=false;unsigned nuke_minutes=0,nuke_due=0;unit_id_32 spotter{},missile{};
  std::array<unsigned,228> desired{},defense{},attack{},maximum{};
  Side(){maximum.fill(255);}
  std::array<unsigned,61> upgrades{};std::array<bool,44> tech{};
 };
 std::array<unsigned,1700> committed_{};a_vector<uint8_t> original_,brood_;std::array<Thread,64> threads_{};std::array<Side,8> sides_{};std::array<Guard,128> guards_{};
 // Each bit records a player's defended region. Region indices are bounded by
 // the engine's original 5000-region limit; no per-tick allocation is needed.
 std::array<uint8_t,5000> defended_{};
 unsigned defending_owners_=0;
 unsigned read(const a_vector<uint8_t>& b,unsigned at,unsigned n)const{if(at>b.size() || n>b.size()-at)throw std::runtime_error("Campaign AI bytecode bounds");unsigned v=0;for(unsigned i=0;i<n;++i)v|=unsigned(b[at+i])<<(i*8);return v;}
 unsigned entry(unsigned id,bool& expansion)const{
  const auto table=read(original_,0,4);for(unsigned at=table;at+16<=original_.size();at+=16){unsigned key=read(original_,at,4);if(!key)break;if(key==id){auto pc=read(original_,at+4,4);if(pc){expansion=false;return pc;}break;}}
  const auto bwtable=read(brood_,0,4);for(unsigned at=bwtable;at+8<=brood_.size();at+=8){unsigned key=read(brood_,at,4);if(!key)break;if(key==id){expansion=true;return read(brood_,at+4,4);}}return 0;
 }
 static bool idle(const unit_t* u){auto o=u->order_type->id;return o==Orders::PlayerGuard || o==Orders::Guard || o==Orders::Stop || o==Orders::Nothing || o==u->unit_type->return_to_idle->id;}
 static xy spiral(unsigned n){unsigned r=1;while(n>=8*r){n-=8*r;++r;}int k=n%(2*r),d=r;switch(n/(2*r)){case 0:return {d,-d+k};case 1:return {d-k,d};case 2:return {-d,d-k};default:return {-d+k,-d};}}
 void attack(state_functions& f,unsigned owner,bool all){
  auto& side=sides_[owner];action_state state;action_functions a(f.st,state);auto remaining=side.attack;
  // Campaign attack scripts deliberately target the player's colony. Choose
  // the closest hostile target to the town, without altering map alliances.
  unit_t* target=nullptr;int distance=0x7fffffff;for(auto* u:ptr(f.st.visible_units))if(u->owner<8 && u->owner!=owner && f.st.alliances[owner][u->owner]==0 && !f.unit_dying(u) && !f.ut_turret(u)){
   auto d=u->sprite->position-side.center;int n=d.x*d.x+d.y*d.y;if(n<distance){distance=n;target=u;}}
  if(side.target_expansion){unit_t* expansion=nullptr;int nearest=0x7fffffff;for(auto* u:ptr(f.st.visible_units))if(u->owner<8 && u->owner!=owner && f.st.alliances[owner][u->owner]==0 && f.ut_resource_depot(u) && !f.unit_dying(u)){auto from=u->sprite->position-f.st.game->start_locations[u->owner];auto d=u->sprite->position-side.center;int n=d.x*d.x+d.y*d.y;if(from.x*from.x+from.y*from.y>512*512 && n<nearest){nearest=n;expansion=u;}}if(expansion)target=expansion;}
  if(!target)return;
  for(auto* u:ptr(f.st.player_units[owner])){unsigned type=unsigned(u->unit_type->id);if(type>=228 || f.ut_building(u) || f.ut_worker(u) || f.ut_turret(u) || !f.u_completed(u) || f.unit_dying(u) || f.us_hidden(u))continue;
   if(u->order_type->tech_type!=TechTypes::None || u->order_type->id==Orders::NukePaint || u->order_type->id==Orders::NukeTrack)continue;
   if(!all && u->index<committed_.size() && committed_[u->index]==f.get_unit_id_32(u).raw_value)continue;
   if(!(f.unit_or_subunit_ground_weapon(u) || f.unit_or_subunit_air_weapon(u)))continue;if(!all && !remaining[type])continue;
   a.action_select(owner,u);if(a.action_order(owner,a.get_order_type(Orders::AttackMove),target->sprite->position,nullptr,nullptr,false)){if(u->index<committed_.size())committed_[u->index]=f.get_unit_id_32(u).raw_value;if(remaining[type])--remaining[type];}
  }
 }
 bool hostile(state_functions& f,unsigned owner,unsigned other)const{const auto c=f.st.players[other].controller;return owner!=other && !f.st.alliances[owner][other] && c!=player_t::controller_inactive && c!=player_t::controller_neutral && c!=player_t::controller_rescue_passive;}
 void create_nuke(state_functions& f,unsigned owner){
  for(auto* silo:ptr(f.st.player_units[owner]))if(f.unit_is(silo,UnitTypes::Terran_Nuclear_Silo) && f.u_completed(silo) && !f.unit_dying(silo) && !silo->building.silo.nuke && !silo->current_build_unit && silo->build_queue.empty()){
   auto* missile=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Nuclear_Missile),silo->sprite->position,owner);
   if(missile){f.hide_unit(missile);missile->connected_unit=silo;silo->building.silo.nuke=missile;silo->building.silo.ready=true;}return;
  }
 }
 void nuclear_at(state_functions& f,unsigned owner,xy position){
  bool armed=false;for(auto* u:ptr(f.st.player_units[owner]))if(f.unit_is(u,UnitTypes::Terran_Nuclear_Silo) && !f.unit_dying(u) && u->building.silo.nuke && u->building.silo.ready){armed=true;break;}if(!armed)return;
  unit_t* ghost=nullptr;int nearest=0x7fffffff;
  for(auto* u:ptr(f.st.player_units[owner]))if(f.unit_is_ghost(u) && f.u_completed(u) && !f.unit_dying(u) && !f.us_hidden(u) && !f.unit_is_disabled(u) && u->order_type->tech_type==TechTypes::None && u->order_type->id!=Orders::NukeTrack && u->order_type->id!=Orders::NukePaint && u->order_type->id!=Orders::NukeUnit){auto d=u->sprite->position-position;int n=d.x*d.x+d.y*d.y;if(n<nearest){nearest=n;ghost=u;}}
  if(ghost){action_state state;action_functions a(f.st,state);a.action_select(owner,ghost);a.action_order(owner,a.get_order_type(Orders::NukePaint),position,nullptr,nullptr,false);}
 }
 void guard_regions(state_functions& f,unsigned owner){
  defending_owners_|=1u<<owner;
  for(auto* u:ptr(f.st.player_units[owner]))if(!f.unit_dying(u) && !f.us_hidden(u) && !f.ut_turret(u)){
   auto index=f.get_region_at(u->sprite->position)->index;
   if(index>=defended_.size())throw std::runtime_error("Campaign defense region bounds");
   defended_[index]|=1u<<owner;
  }
 }
 void defend(state_functions& f){
  if(!defending_owners_ || f.st.current_frame%96)return;
  action_state state;action_functions a(f.st,state);
  for(unsigned owner=0;owner<8;++owner){
   if(!(defending_owners_&(1u<<owner)))continue;
   static_vector<unit_t*,128> threats;
   for(auto* u:ptr(f.st.visible_units))if(u->owner<8 && hostile(f,owner,u->owner) && !f.unit_dying(u) && !f.u_invincible(u) && f.player_position_is_visible(owner,u->sprite->position) && !f.unit_is_undetected(u,owner)){
    auto index=f.get_region_at(u->sprite->position)->index;
    if(index<defended_.size() && (defended_[index]&(1u<<owner)) && threats.size()<128)threats.push_back(u);
   }
   if(threats.empty())continue;
   for(auto* u:ptr(f.st.player_units[owner]))if(f.u_completed(u) && !f.unit_dying(u) && !f.us_hidden(u) && !f.ut_worker(u) && !f.ut_building(u) && !f.ut_turret(u) && idle(u) && !(u->index<committed_.size() && committed_[u->index]==f.get_unit_id_32(u).raw_value)){
    unit_t* target=nullptr;int nearest=768*768;
    for(auto* enemy:threats)if(f.unit_can_attack_target(u,enemy)){auto d=enemy->sprite->position-u->sprite->position;int n=d.x*d.x+d.y*d.y;if(n<nearest){nearest=n;target=enemy;}}
    if(target){a.action_select(owner,u);a.action_order(owner,a.get_order_type(Orders::AttackMove),target->sprite->position,nullptr,nullptr,false);}
   }
  }
 }
 void nuclear(state_functions& f,unsigned owner){
  auto& side=sides_[owner];if(!side.nuclear || f.st.current_frame%24)return;
  const auto interval=side.nuke_minutes*60u*24u;
  if(auto* missile=f.get_unit(side.missile))if(missile->order_type->id==Orders::NukeLaunch){side.nuke_due=f.st.current_frame+interval;side.spotter={};side.missile={};}
  if(auto* ghost=f.get_unit(side.spotter)){
   if(!f.unit_dying(ghost) && ghost->owner==owner){
    if(ghost->order_type->id==Orders::NukeTrack && ghost->connected_unit){side.nuke_due=f.st.current_frame+interval;side.spotter={};}
    else if(ghost->order_type->id==Orders::NukePaint || ghost->order_type->id==Orders::NukeUnit || ghost->order_type->id==Orders::CastNuclearStrike)return;
    else side.spotter={};
   }else side.spotter={};
  }else side.spotter={};
  unsigned silos=0;unit_t* armed=nullptr;
  for(auto* u:ptr(f.st.player_units[owner]))if(f.unit_is(u,UnitTypes::Terran_Nuclear_Silo) && f.u_completed(u) && !f.unit_dying(u)){++silos;if(!armed && u->building.silo.ready && u->building.silo.nuke)armed=u->building.silo.nuke;}
  // Replacements are purchased by the ordinary native production path.
  if(silos)side.desired[unsigned(UnitTypes::Terran_Nuclear_Missile)]=silos;
  if(!armed || f.st.current_frame<side.nuke_due)return;
  unit_t* target=nullptr;unsigned best=0;
  for(auto* u:ptr(f.st.visible_units))if(u->owner<8 && hostile(f,owner,u->owner) && f.ut_building(u) && !f.unit_dying(u) && !f.u_invincible(u) && f.player_position_is_visible(owner,u->sprite->position)){
   bool friendly=false;for(auto* n:f.find_units({u->sprite->position-xy(256,256),u->sprite->position+xy(256,256)}))if(n->owner==owner && !f.unit_dying(n) && !f.unit_is_ghost(n)){friendly=true;break;}
   const unsigned value=(f.ut_resource_depot(u)?10000:0)+u->unit_type->mineral_cost+u->unit_type->gas_cost;
   if(!friendly && (!target || value>best)){target=u;best=value;}
  }
  if(!target)return;
  unit_t* chosen=nullptr;int nearest=0x7fffffff;
  for(auto* u:ptr(f.st.player_units[owner]))if(f.unit_is_ghost(u) && f.u_completed(u) && !f.unit_dying(u) && !f.us_hidden(u) && !f.unit_is_disabled(u) && u->order_type->tech_type==TechTypes::None && u->order_type->id!=Orders::NukeTrack && u->order_type->id!=Orders::NukePaint && u->order_type->id!=Orders::NukeUnit){auto d=u->sprite->position-target->sprite->position;int n=d.x*d.x+d.y*d.y;if(n<nearest && f.is_reachable(u,target)){nearest=n;chosen=u;}}
  if(chosen){action_state state;action_functions a(f.st,state);a.action_select(owner,chosen);if(a.action_order(owner,a.get_order_type(Orders::NukePaint),target->sprite->position,nullptr,nullptr,false)){side.spotter=f.get_unit_id_32(chosen);side.missile=f.get_unit_id_32(armed);}}
 }
 void place_guard(state_functions& f,unsigned owner,unsigned type,unsigned location,xy center){if(type>=228 || location>2)throw std::runtime_error("Campaign guard request bounds");xy position=center;if(location){int nearest=0x7fffffff;for(auto* u:ptr(f.st.visible_units))if(!f.unit_dying(u) && (location==1?f.unit_is_mineral_field(u):f.unit_is(u,UnitTypes::Resource_Vespene_Geyser) || (u->owner==owner && f.unit_is_refinery(u)))){auto d=u->sprite->position-center;int n=d.x*d.x+d.y*d.y;if(n<600*600 && n<nearest){nearest=n;position=u->sprite->position;}}}for(auto& g:guards_)if(!g.active){g={true,owner,type,0,{},position};return;}throw std::runtime_error("Campaign guard capacity");}
 void update_guards(state_functions& f){if(f.st.current_frame%24)return;action_state state;action_functions a(f.st,state);for(auto& g:guards_)if(g.active){auto* unit=f.get_unit(g.unit);if(g.unit.raw_value && (!unit || f.unit_dying(unit) || unit->owner!=g.owner || (unit->index<committed_.size() && committed_[unit->index]==g.unit.raw_value))){if(!unit || f.unit_dying(unit)){if(++g.deaths>=3){g.active=false;continue;}}g.unit={};unit=nullptr;}if(!unit){for(auto* candidate:ptr(f.st.player_units[g.owner]))if(unsigned(candidate->unit_type->id)==g.type && f.u_completed(candidate) && !f.unit_dying(candidate) && !f.us_hidden(candidate) && !f.ut_turret(candidate) && !(candidate->index<committed_.size() && committed_[candidate->index]==f.get_unit_id_32(candidate).raw_value)){auto id=f.get_unit_id_32(candidate);bool used=false;for(const auto& other:guards_)if(other.active && other.unit==id){used=true;break;}if(!used){unit=candidate;g.unit=id;break;}}}if(unit && idle(unit)){auto d=unit->sprite->position-g.position;if(d.x*d.x+d.y*d.y>96*96){a.action_select(g.owner,unit);a.action_order(g.owner,a.get_order_type(Orders::AttackMove),g.position,nullptr,nullptr,false);}}}}
 unsigned unit_count(state_functions& f,unsigned owner,unsigned type,bool completed)const{
  unsigned n=0;for(auto* u:ptr(f.st.player_units[owner]))if(!f.unit_dying(u) && !f.ut_turret(u)){
   if(unsigned(u->unit_type->id)==type && (!completed || f.u_completed(u)))++n;
   else if(!completed && (f.unit_is_egg(u) || f.unit_is_morphing_building(u)) && !u->build_queue.empty() && unsigned(u->build_queue.front()->id)==type)n+=f.ut_two_units_in_one_egg(f.get_unit_type(UnitTypes(type)))?2:1;
  }return n;
 }
 Thread& launch(unsigned pc,unsigned owner,bool brood,xy center,unsigned frame){for(auto& child:threads_)if(!child.active){child={pc,frame,owner,brood,true,center};return child;}throw std::runtime_error("Campaign AI thread capacity");}
 void interpret(state_functions& f,Thread& t){
  const auto& bytes=t.brood?brood_:original_;auto& side=sides_[t.owner];
  auto byte=[&](){return read(bytes,t.pc++,1);};auto word=[&](){unsigned v=read(bytes,t.pc,2);t.pc+=2;return v;};
  for(unsigned budget=0;budget<64 && t.active && f.st.current_frame>=t.wake;++budget){if(t.kill_pending){t.active=false;return;}const unsigned instruction=t.pc;unsigned op=byte();
   switch(op){
   case 0:t.pc=word();break;
   case 1:case 106:{unsigned id=word(),address=word();if(id>=228)throw std::runtime_error("Campaign AI ownership bounds");const bool owns=unit_count(f,t.owner,id,false)!=0;if(owns==(op==106))t.pc=address;break;}
   case 2:t.wake=f.st.current_frame+std::max(1u,word());return;
   case 3:case 4:side.enabled=true;side.center=t.center;break;
   case 6:{unsigned n=byte(),id=word();byte();if(id>=228)throw std::runtime_error("Campaign AI build bounds");side.desired[id]=std::max(side.desired[id],n);break;}
   case 7:{unsigned level=byte(),id=word();byte();if(id>=61)throw std::runtime_error("Campaign AI upgrade bounds");side.upgrades[id]=std::max(side.upgrades[id],level);break;}
   case 8:{unsigned id=word();byte();if(id>=44)throw std::runtime_error("Campaign AI research bounds");side.tech[id]=true;break;}
   case 9:case 10:case 78:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI wait type bounds");if(unit_count(f,t.owner,id,op!=10)<n){t.pc=instruction;t.wake=f.st.current_frame+24;return;}break;}
   case 11:side.attack.fill(0);break;
   case 12:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI attack bounds");side.attack[id]=std::min(128u,side.attack[id]+n);break;}
   case 13:break; // Persistent wave requests are paid by the town economy.
   case 14:attack(f,t.owner,false);break;
   case 16:break; // Captured town adoption already uses all existing owner units.
   case 19:case 20:case 21:case 22:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI defense bounds");side.defense[id]=std::max(side.defense[id],n);break;}
   case 23:case 24:case 25:case 26:byte();word();break; // Native guards engage nearby threats.
   case 31:{byte();side.enabled=true;side.suicide=true;side.center=t.center;attack(f,t.owner,true);break;}
   case 32:case 33:if(t.owner){f.st.alliances[t.owner][0]=f.st.alliances[0][t.owner]=op==32?0:2;}break;
   case 34:byte();break;
   case 35:case 56:break;
   case 36:t.active=false;break;
   case 97:for(auto& child:threads_)if(child.active && child.killable)child.kill_pending=true;break;
   case 98:t.killable=true;break;
   case 37:side.enabled=false;f.st.players[t.owner].controller=player_t::controller_rescue_passive;for(auto& child:threads_)if(child.owner==t.owner)child.active=false;break;
   case 61:{unsigned type=word(),location=byte();place_guard(f,t.owner,type,location,t.center);break;}
   case 62:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI force bounds");if(unit_count(f,t.owner,id,true)<n){side.desired[id]=std::max(side.desired[id],n);t.pc=instruction;t.wake=f.st.current_frame+24;return;}break;}
   case 64:{unsigned address=word();if(t.depth==t.calls.size())throw std::runtime_error("Campaign AI call depth");t.calls[t.depth++]=t.pc;t.pc=address;break;}
   case 65:if(!t.depth)throw std::runtime_error("Campaign AI return without call");t.pc=t.calls[--t.depth];break;
   case 72:{unsigned address=word();launch(address,t.owner,t.brood,t.center,f.st.current_frame);break;}
   case 75:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI maximum bounds");side.maximum[id]=n;break;}
   case 76:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI train bounds");side.desired[id]=std::max(side.desired[id],n);break;}
   case 77:side.target_expansion=true;break;
   case 87:case 88:{unsigned p=byte();if(p>=8)throw std::runtime_error("Campaign AI shared vision bounds");if(op==87)f.st.shared_vision[p]|=1<<t.owner;else f.st.shared_vision[p]&=~(1<<t.owner);f.st.shared_vision[p]|=1<<p;f.st.update_tiles_countdown=1;break;}
   case 83:case 94:{unsigned minerals=word(),gas=word(),address=word();bool enough=false;for(unsigned p=0;p<8;++p)if((op==83?p==t.owner:hostile(f,t.owner,p)) && f.st.current_minerals[p]>=int(minerals) && f.st.current_gas[p]>=int(gas))enough=true;if(enough)t.pc=address;break;}
   case 93:{unsigned type=word(),address=word();if(type>=228)throw std::runtime_error("Campaign enemy ownership bounds");for(unsigned p=0;p<8;++p)if(hostile(f,t.owner,p) && unit_count(f,p,type,false)){t.pc=address;break;}break;}
   case 105:f.st.lcg_rand_state=read(bytes,t.pc,4);t.pc+=4;break;
   case 45:side.nuclear=true;side.nuke_minutes=byte();side.nuke_due=f.st.current_frame+side.nuke_minutes*60u*24u;break;
   case 92:guard_regions(f,t.owner);break;
   case 102:side.nuke_due=f.st.current_frame+side.nuke_minutes*60u*24u;break;
   case 110:{guard_regions(f,t.owner);for(unsigned p=0;p<8;++p)if(p!=t.owner && sides_[p].enabled && !hostile(f,t.owner,p) && f.st.alliances[t.owner][p]){defending_owners_|=1u<<p;for(auto& region:defended_)if(region&(1u<<t.owner))region|=1u<<p;}break;}
   case 107:create_nuke(f,t.owner);break;
   case 89:nuclear_at(f,t.owner,t.center);break;
   case 109:{xy position;position.x=word();position.y=word();if(position.x>=int(f.st.game->map_width) || position.y>=int(f.st.game->map_height))throw std::runtime_error("Campaign nuclear position bounds");nuclear_at(f,t.owner,position);break;}
   case 41:{action_state state;action_functions a(f.st,state);for(auto* bunker:ptr(f.st.player_units[t.owner]))if(f.unit_is(bunker,UnitTypes::Terran_Bunker))for(auto* u:ptr(f.st.player_units[t.owner]))if(f.unit_is(u,UnitTypes::Terran_Marine) && !f.us_hidden(u)){auto d=u->sprite->position-bunker->sprite->position;if(d.x*d.x+d.y*d.y<256*256){a.action_select(t.owner,u);a.action_default_order(t.owner,bunker->sprite->position,bunker,nullptr,false);}}break;}
   case 46:word();break; // Original force limits exceed our bounded unit request cap.
   default:throw std::runtime_error("Unsupported campaign AI opcode "+std::to_string(op));
   }
  }
 }
 void economy(state_functions& f,unsigned owner){
  auto& side=sides_[owner];if(!side.enabled || f.st.current_frame%24)return;auto& st=f.st;action_state state;action_functions a(st,state);
  if(unsigned(st.players[owner].race)>=3)throw std::runtime_error("Campaign AI race bounds");
  const bool z=st.players[owner].race==race_t::zerg,p=st.players[owner].race==race_t::protoss;
  const auto worker=z?UnitTypes::Zerg_Drone:p?UnitTypes::Protoss_Probe:UnitTypes::Terran_SCV;
  const auto supply=z?UnitTypes::Zerg_Overlord:p?UnitTypes::Protoss_Pylon:UnitTypes::Terran_Supply_Depot;
  const auto gas=z?UnitTypes::Zerg_Extractor:p?UnitTypes::Protoss_Assimilator:UnitTypes::Terran_Refinery;
  std::array<unsigned,228> counts{},training{};static_vector<unit_t*,128> workers,parents;unit_t* refinery=nullptr;unsigned live=0;
  for(auto* u:ptr(st.player_units[owner]))if(!f.unit_dying(u) && !f.ut_turret(u)){++live;unsigned id=unsigned(u->unit_type->id);if(id<228 && !(u->index<committed_.size() && committed_[u->index]==f.get_unit_id_32(u).raw_value))++counts[id];for(auto* q:u->build_queue)if(unsigned(q->id)<228){unsigned n=q->id==UnitTypes::Zerg_Zergling?2:1;counts[unsigned(q->id)]+=n;training[unsigned(q->id)]+=n;}
   if(f.u_completed(u)){if(f.unit_is(u,worker))workers.push_back(u);else if((f.ut_building(u) || f.unit_is(u,UnitTypes::Zerg_Larva)) && parents.size()<128)parents.push_back(u);if(f.unit_is(u,gas))refinery=u;}}
  auto* builder=f.get_unit(side.builder);if(builder && (f.unit_dying(builder) || f.ut_building(builder) || (st.current_frame>side.build_frame+96 && builder->order_type->id!=Orders::PlaceBuilding && builder->order_type->id!=Orders::PlaceProtossBuilding && builder->order_type->id!=Orders::DroneStartBuild && builder->order_type->id!=Orders::ConstructingBuilding && builder->order_type->id!=Orders::DroneLand))){side.builder={};builder=nullptr;}
  unsigned gas_workers=0;for(auto* w:workers){auto o=w->order_type->id;if(o==Orders::HarvestGas || o==Orders::ReturnGas || o==Orders::MoveToGas || o==Orders::WaitForGas)++gas_workers;}
  for(auto* w:workers)if(w!=builder && !f.us_hidden(w) && idle(w)){
   unit_t* target=nullptr;if(refinery && gas_workers<(workers.size()>7?3u:1u)){target=refinery;++gas_workers;}else {int distance=0x7fffffff;for(auto* m:ptr(st.visible_units))if(f.unit_is_mineral_field(m) && !f.unit_dying(m) && f.is_reachable(w,m)){auto d=m->sprite->position-w->sprite->position;int n=d.x*d.x+d.y*d.y;if(n<distance){distance=n;target=m;}}}
   if(target){a.action_select(owner,w);a.action_default_order(owner,target->sprite->position,target,nullptr,false);}
  }
  if(side.suicide && st.current_frame%120==0)attack(f,owner,true);
  if(live>=160)return;
  auto wants=side.desired;for(unsigned i=0;i<228;++i)wants[i]=std::max(wants[i],side.defense[i]+side.attack[i]);
  unsigned r=unsigned(st.players[owner].race);if(st.supply_available[owner][r]-st.supply_used[owner][r]<=fp1::integer(4))wants[unsigned(supply)]=std::max(wants[unsigned(supply)],counts[unsigned(supply)]+1);
  // Native campaign AI maintains a modest mining staff even with defaultbuild off.
  if(!workers.empty())wants[unsigned(worker)]=std::max(wants[unsigned(worker)],8u);
  for(unsigned id=0;id<228;++id)if(wants[id]>counts[id] && f.get_unit_type(UnitTypes(id))->gas_cost)wants[unsigned(gas)]=std::max(wants[unsigned(gas)],1u);
  for(unsigned id=0;id<228;++id)wants[id]=std::min(wants[id],side.maximum[id]);
  for(unsigned id=0;id<228;++id){unsigned pending=0;for(const auto& g:guards_)if(g.active && g.owner==owner && g.type==id && !g.unit.raw_value)++pending;if(pending>training[id])wants[id]=std::max(wants[id],counts[id]+pending-training[id]);}
  for(unsigned id=0;id<228;++id)if(wants[id]>counts[id]){
   auto* type=f.get_unit_type(UnitTypes(id));if(!f.has_available_resources_for(owner,type))continue;
   if(f.ut_building(type) && !f.ut_addon(type)){
    bool morph=false;for(auto* parent:parents)if(z && f.unit_can_build(parent,type)){a.action_select(owner,parent);if(a.action_morph_building(owner,type)){morph=true;break;}}if(morph)break;
    if(builder)continue;unit_t* w=nullptr;for(auto* candidate:workers)if(f.unit_can_build(candidate,type) && !f.us_hidden(candidate)){w=candidate;break;}if(!w)continue;
    auto place=[&](xy pos){if(pos.x<type->placement_size.x/2 || pos.y<type->placement_size.y/2 || pos.x>=int(st.game->map_width) || pos.y>=int(st.game->map_height))return false;auto tile=(pos-type->placement_size/2)/32;pos=tile*32+type->placement_size/2;if(!f.can_place_building(w,owner,type,pos,true,true))return false;a.action_select(owner,w);auto order=z?Orders::DroneStartBuild:p?Orders::PlaceProtossBuilding:Orders::PlaceBuilding;if(!a.action_build(owner,a.get_order_type(order),type,{size_t(tile.x),size_t(tile.y)}))return false;side.builder=f.get_unit_id_32(w);side.build_frame=st.current_frame;return true;};
    bool built=false;if(UnitTypes(id)==gas){for(auto* g:ptr(st.visible_units))if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser)){auto d=g->sprite->position-side.center;if(d.x*d.x+d.y*d.y<600*600 && place(g->sprite->position)){built=true;break;}}}else for(unsigned j=0;j<12;++j){if(side.search>=640)side.search=0;if(place(side.center+spiral(side.search++)*32)){built=true;break;}}if(built)break;
   }else for(auto* parent:parents)if(parent->build_queue.empty() && f.unit_can_build(parent,type)){a.action_select(owner,parent);bool ok=false;if(f.ut_addon(type)){auto pos=parent->sprite->position-parent->unit_type->placement_size/2+type->addon_position+type->placement_size/2;if(f.can_place_building(parent,owner,type,pos,true,true)){f.place_building(parent,a.get_order_type(Orders::PlaceAddon),type,parent->sprite->position);ok=true;}}else ok=z?a.action_morph(owner,type):a.action_train(owner,type);if(ok){++counts[id];break;}}
  }
  for(auto* b:parents)if(f.ut_building(b) && !f.unit_is_researching(b) && !f.unit_is_upgrading(b)){
   bool busy=false;catalogue::techs([&](const auto& e){if(!busy && side.tech[unsigned(e.type)] && zerg::parent(b->unit_type->id,e.parent)){a.action_select(owner,b);busy=a.action_research(owner,a.get_tech_type(e.type));}});
   catalogue::upgrades([&](const auto& e){if(!busy && side.upgrades[unsigned(e.type)]>st.upgrade_levels[owner][e.type] && zerg::parent(b->unit_type->id,e.parent)){a.action_select(owner,b);busy=a.action_upgrade(owner,a.get_upgrade_type(e.type));}});
  }
 }
public:
 template<class Load>void load(Load load){load(original_,"scripts/aiscript.bin");load(brood_,"scripts/bwscript.bin");if(original_.size()>128*1024 || brood_.size()>128*1024)throw std::runtime_error("Campaign AI script size");}
 void start(state_functions& f,unsigned id,unsigned owner,xy center){if(owner>=8)throw std::runtime_error("Campaign AI owner bounds");
  if(id==0x63697553u){auto& s=sides_[owner];s.enabled=true;s.suicide=true;s.center=center;attack(f,owner,true);return;} // Suic
  bool expansion=false;unsigned pc=entry(id,expansion);if(!pc)throw std::runtime_error("Missing original campaign AI script");auto& thread=launch(pc,owner,expansion,center,f.st.current_frame);interpret(f,thread);
 }
 void step(state_functions& f){for(auto& t:threads_)if(t.active && f.st.current_frame>=t.wake)interpret(f,t);update_guards(f);defend(f);for(unsigned p=0;p<8;++p){nuclear(f,p);economy(f,p);}}
 template<class Add>void digest(Add add)const{add(defending_owners_);for(auto region:defended_)add(region);for(const auto& g:guards_){add(g.active);add(g.owner);add(g.type);add(g.deaths);add(g.unit.raw_value);add(g.position.x);add(g.position.y);}for(auto id:committed_)add(id);for(const auto& t:threads_){add(t.pc);add(t.wake);add(t.owner);add(t.brood);add(t.active);add(t.center.x);add(t.center.y);add(t.depth);for(auto pc:t.calls)add(pc);add(t.killable);add(t.kill_pending);}for(const auto& s:sides_){add(s.enabled);add(s.suicide);add(s.target_expansion);add(s.nuclear);add(s.nuke_minutes);add(s.nuke_due);add(s.spotter.raw_value);add(s.missile.raw_value);add(s.center.x);add(s.center.y);add(s.search);add(s.build_frame);add(s.builder.raw_value);for(auto n:s.maximum)add(n);for(auto n:s.desired)add(n);for(auto n:s.defense)add(n);for(auto n:s.attack)add(n);for(auto n:s.upgrades)add(n);for(auto n:s.tech)add(n);}}
};
}}
