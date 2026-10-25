#pragma once
// Bounded interpreter for the original opening-mission AIScript bytecode.
// Scripts remain private assets. Purchases, placement, research and orders use
// native action validation; this adapter never grants resources or free units.
#include "actions.h"
#include "race_catalogue.h"
namespace port3ds { namespace detail {
using namespace bwgame;
class CampaignAI {
 struct Thread {unsigned pc=0,wake=0,owner=0;bool brood=false,active=false;xy center{};};
 struct Side {
  bool enabled=false,suicide=false;xy center{};unsigned search=0,build_frame=0;unit_id_32 builder{};
  std::array<unsigned,228> desired{},defense{},attack{};
  std::array<unsigned,61> upgrades{};std::array<bool,44> tech{};
 };
 std::array<unsigned,1700> committed_{};a_vector<uint8_t> original_,brood_;std::array<Thread,16> threads_{};std::array<Side,8> sides_{};
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
  if(!target)return;
  for(auto* u:ptr(f.st.player_units[owner])){unsigned type=unsigned(u->unit_type->id);if(type>=228 || f.ut_building(u) || f.ut_worker(u) || f.ut_turret(u) || !f.u_completed(u) || f.unit_dying(u) || f.us_hidden(u))continue;
   if(!all && u->index<committed_.size() && committed_[u->index]==f.get_unit_id_32(u).raw_value)continue;
   if(!(f.unit_or_subunit_ground_weapon(u) || f.unit_or_subunit_air_weapon(u)))continue;if(!all && !remaining[type])continue;
   a.action_select(owner,u);if(a.action_order(owner,a.get_order_type(Orders::AttackMove),target->sprite->position,nullptr,nullptr,false)){if(u->index<committed_.size())committed_[u->index]=f.get_unit_id_32(u).raw_value;if(remaining[type])--remaining[type];}
  }
 }
 void interpret(state_functions& f,Thread& t){
  const auto& bytes=t.brood?brood_:original_;auto& side=sides_[t.owner];
  auto byte=[&](){return read(bytes,t.pc++,1);};auto word=[&](){unsigned v=read(bytes,t.pc,2);t.pc+=2;return v;};
  for(unsigned budget=0;budget<64 && t.active && f.st.current_frame>=t.wake;++budget){unsigned op=byte();
   switch(op){
   case 0:t.pc=word();break;
   case 2:t.wake=f.st.current_frame+std::max(1u,word());return;
   case 3:case 4:side.enabled=true;side.center=t.center;break;
   case 6:{unsigned n=byte(),id=word();byte();if(id>=228)throw std::runtime_error("Campaign AI build bounds");side.desired[id]=std::max(side.desired[id],n);break;}
   case 7:{unsigned level=byte(),id=word();byte();if(id>=61)throw std::runtime_error("Campaign AI upgrade bounds");side.upgrades[id]=std::max(side.upgrades[id],level);break;}
   case 8:{unsigned id=word();byte();if(id>=44)throw std::runtime_error("Campaign AI research bounds");side.tech[id]=true;break;}
   case 11:side.attack.fill(0);break;
   case 12:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI attack bounds");side.attack[id]=std::min(128u,side.attack[id]+n);break;}
   case 13:break; // Persistent wave requests are paid by the town economy.
   case 14:attack(f,t.owner,false);break;
   case 16:break; // Captured town adoption already uses all existing owner units.
   case 19:case 20:case 21:case 22:{unsigned n=byte(),id=word();if(id>=228)throw std::runtime_error("Campaign AI defense bounds");side.defense[id]=std::max(side.defense[id],n);break;}
   case 23:case 24:case 25:case 26:byte();word();break; // Native guards engage nearby threats.
   case 34:byte();break;
   case 35:case 56:break;
   case 36:t.active=false;break;
   case 41:{action_state state;action_functions a(f.st,state);for(auto* bunker:ptr(f.st.player_units[t.owner]))if(f.unit_is(bunker,UnitTypes::Terran_Bunker))for(auto* u:ptr(f.st.player_units[t.owner]))if(f.unit_is(u,UnitTypes::Terran_Marine) && !f.us_hidden(u)){auto d=u->sprite->position-bunker->sprite->position;if(d.x*d.x+d.y*d.y<256*256){a.action_select(t.owner,u);a.action_default_order(t.owner,bunker->sprite->position,bunker,nullptr,false);}}break;}
   case 46:word();break; // Original force limits exceed our bounded unit request cap.
   default:throw std::runtime_error("Unsupported opening campaign AI opcode");
   }
  }
 }
 void economy(state_functions& f,unsigned owner){
  auto& side=sides_[owner];if(!side.enabled || f.st.current_frame%24)return;auto& st=f.st;action_state state;action_functions a(st,state);
  const bool z=st.players[owner].race==race_t::zerg,p=st.players[owner].race==race_t::protoss;
  const auto worker=z?UnitTypes::Zerg_Drone:p?UnitTypes::Protoss_Probe:UnitTypes::Terran_SCV;
  const auto supply=z?UnitTypes::Zerg_Overlord:p?UnitTypes::Protoss_Pylon:UnitTypes::Terran_Supply_Depot;
  const auto gas=z?UnitTypes::Zerg_Extractor:p?UnitTypes::Protoss_Assimilator:UnitTypes::Terran_Refinery;
  std::array<unsigned,228> counts{};static_vector<unit_t*,128> workers,parents;unit_t* refinery=nullptr;unsigned live=0;
  for(auto* u:ptr(st.player_units[owner]))if(!f.unit_dying(u) && !f.ut_turret(u)){++live;unsigned id=unsigned(u->unit_type->id);if(id<228 && !(u->index<committed_.size() && committed_[u->index]==f.get_unit_id_32(u).raw_value))++counts[id];for(auto* q:u->build_queue)if(unsigned(q->id)<228)counts[unsigned(q->id)]+=q->id==UnitTypes::Zerg_Zergling?2:1;
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
  bool expansion=false;unsigned pc=entry(id,expansion);if(!pc)throw std::runtime_error("Missing original campaign AI script");for(auto& t:threads_)if(!t.active){t={pc,unsigned(f.st.current_frame),owner,expansion,true,center};return;}throw std::runtime_error("Campaign AI thread capacity");
 }
 void step(state_functions& f){for(auto& t:threads_)if(t.active && f.st.current_frame>=t.wake)interpret(f,t);for(unsigned p=0;p<8;++p)economy(f,p);}
 template<class Add>void digest(Add add)const{for(auto id:committed_)add(id);for(const auto& t:threads_){add(t.pc);add(t.wake);add(t.owner);add(t.brood);add(t.active);add(t.center.x);add(t.center.y);}for(const auto& s:sides_){add(s.enabled);add(s.suicide);add(s.center.x);add(s.center.y);add(s.search);add(s.build_frame);add(s.builder.raw_value);for(auto n:s.desired)add(n);for(auto n:s.defense)add(n);for(auto n:s.attack)add(n);for(auto n:s.upgrades)add(n);for(auto n:s.tech)add(n);}}
};
}}
