#pragma once
// Bounded melee controller. Every purchase and order uses the native engine;
// no resource grants, instant buildings, free research or campaign triggers.
#include "actions.h"
#include "race_catalogue.h"
namespace port3ds { namespace detail {
using namespace bwgame;
class BattleAI {
 struct Side {
  bool enabled=false,building=false;
  unit_id_32 builder;UnitTypes plan=UnitTypes::Terran_Supply_Depot;
  unsigned search=24,trained=0,built=0,waves=0,lost=0,researches=0;
  unsigned build_frame=0,attack_frame=0;
  static_vector<unit_id_32,128> previous;
 };
 std::array<Side,3> sides_{};
 std::array<bool,3> active_{{true,true,false}};bool custom_=false,gas_=false;
 int winner_=-1;
 static bool idle(const unit_t* u){auto o=u->order_type->id;return o==Orders::PlayerGuard || o==Orders::Guard || o==Orders::Stop || o==Orders::Nothing || o==u->unit_type->return_to_idle->id;}
 static xy spiral(unsigned n){unsigned r=1;while(n>=8*r){n-=8*r;++r;}const int k=n%(2*r),d=r;switch(n/(2*r)){case 0:return {d,-d+k};case 1:return {d-k,d};case 2:return {-d,d-k};default:return {-d+k,-d};}}
 void update(game_player& player,unsigned owner){
  auto& side=sides_[owner];if(!side.enabled)return;
  auto& f=player.funcs();auto& st=player.st();action_state state;action_functions a(st,state);
  const auto race=st.players[owner].race;const bool z=race==race_t::zerg,p=race==race_t::protoss;
  const auto base_type=z?UnitTypes::Zerg_Hatchery:p?UnitTypes::Protoss_Nexus:UnitTypes::Terran_Command_Center;
  const auto worker_type=z?UnitTypes::Zerg_Drone:p?UnitTypes::Protoss_Probe:UnitTypes::Terran_SCV;
  const auto supply_type=z?UnitTypes::Zerg_Overlord:p?UnitTypes::Protoss_Pylon:UnitTypes::Terran_Supply_Depot;
  const auto gas_type=z?UnitTypes::Zerg_Extractor:p?UnitTypes::Protoss_Assimilator:UnitTypes::Terran_Refinery;
  const auto place_order=z?Orders::DroneStartBuild:p?Orders::PlaceProtossBuilding:Orders::PlaceBuilding;
  std::array<unsigned,228> counts{},pending{};std::array<unit_t*,228> structures{};
  static_vector<unit_t*,128> workers,army,larvae,production;
  unit_t* base=nullptr;unit_t* refinery=nullptr;unit_t* unfinished=nullptr;unsigned live=0;
  for(auto* u:ptr(st.player_units[owner])){
   if(f.unit_dying(u))continue;++live;const unsigned id=unsigned(u->unit_type->id);++counts[id];
   for(const auto* t:u->build_queue)++pending[unsigned(t->id)];
   if(f.ut_building(u)){if(f.u_completed(u)){structures[id]=u;production.push_back(u);}else unfinished=u;}
   if(f.unit_is(u,base_type) || (z && (f.unit_is(u,UnitTypes::Zerg_Lair) || f.unit_is(u,UnitTypes::Zerg_Hive))))base=u;
   if(f.unit_is(u,gas_type) && f.u_completed(u))refinery=u;
   if(f.unit_is(u,worker_type) && f.u_completed(u))workers.push_back(u);
   else if(f.unit_is(u,UnitTypes::Zerg_Larva) && u->build_queue.empty())larvae.push_back(u);
   else if(f.u_completed(u) && !f.ut_building(u) && !f.ut_turret(u) && (f.unit_or_subunit_ground_weapon(u) || f.unit_or_subunit_air_weapon(u) || f.unit_is(u,UnitTypes::Terran_Medic)))army.push_back(u);
  }
  auto has=[&](UnitTypes id){return counts[unsigned(id)]!=0;};
  auto complete=[&](UnitTypes id){return structures[unsigned(id)]!=nullptr;};
  auto* builder=f.get_unit(side.builder);
  if(side.building && (!builder || f.unit_dying(builder) || (z && f.ut_building(builder)) || (st.current_frame>side.build_frame+96 && builder->order_type->id!=place_order && builder->order_type->id!=Orders::ConstructingBuilding && builder->order_type->id!=Orders::DroneLand))){side.building=false;side.builder={};side.search=z?0:24;}
  // Native SCVs can resume a foundation whose original builder was killed.
  if(!p && !z && unfinished && !side.building && (!unfinished->connected_unit || f.unit_dying(unfinished->connected_unit)))for(auto* w:workers){a.action_select(owner,w);if(a.action_default_order(owner,unfinished->sprite->position,unfinished,nullptr,false)){side.builder=f.get_unit_id_32(w);side.building=true;side.build_frame=st.current_frame;}break;}
  auto gas_order=[](const unit_t* w){return w->order_type->id==Orders::HarvestGas || w->order_type->id==Orders::ReturnGas || w->order_type->id==Orders::MoveToGas || w->order_type->id==Orders::WaitForGas;};
  unsigned gas_workers=0;for(auto* w:workers)if(gas_order(w))++gas_workers;
  for(auto* w:workers){
   if(side.building && f.get_unit_id_32(w)==side.builder)continue;
   // Keep stable gas assignments while workers are inside the refinery or
   // returning cargo, and grow to three only after an eight-worker economy.
   if(gas_order(w))continue;
   const bool assign_gas=gas_ && refinery && gas_workers<(workers.size()>=9?3u:1u);
   if(!idle(w) && !assign_gas)continue;
   unit_t* target=nullptr;
   if(assign_gas){target=refinery;++gas_workers;}
   else {int distance=0x7fffffff;for(auto* m:ptr(st.visible_units))if(f.unit_is_mineral_field(m) && !f.unit_dying(m) && m->building.resource.resource_count>0){auto d=m->sprite->position-w->sprite->position;int score=d.x*d.x+d.y*d.y;if(score<distance){distance=score;target=m;}}}
   if(target){a.action_select(owner,w);a.action_default_order(owner,target->sprite->position,target,nullptr,false);}
  }
  const unsigned r=unsigned(race);const int free=(st.supply_available[owner][r]-st.supply_used[owner][r]).integer_part();
  bool need=false;UnitTypes plan=supply_type;
  auto want=[&](UnitTypes type,bool condition=true){if(!need && condition && !has(type)){need=true;plan=type;}};
  if(!z && (counts[unsigned(supply_type)]==0 || (free<=5 && st.supply_available[owner][r]<fp1::integer(120)))){need=true;plan=supply_type;}
  if(z){
   want(UnitTypes::Zerg_Spawning_Pool,workers.size()>=6);
   want(gas_type,gas_ && complete(UnitTypes::Zerg_Spawning_Pool));
   want(UnitTypes::Zerg_Hydralisk_Den,refinery);
   want(UnitTypes::Zerg_Evolution_Chamber,army.size()>=4 && refinery);
   want(UnitTypes::Zerg_Spire,complete(UnitTypes::Zerg_Lair));
   want(UnitTypes::Zerg_Queens_Nest,complete(UnitTypes::Zerg_Spire) && army.size()>=8);
   want(UnitTypes::Zerg_Ultralisk_Cavern,complete(UnitTypes::Zerg_Hive));
  }else if(p){
   want(UnitTypes::Protoss_Gateway,complete(UnitTypes::Protoss_Pylon));
   want(gas_type,gas_ && complete(UnitTypes::Protoss_Gateway));
   want(UnitTypes::Protoss_Cybernetics_Core,refinery);
   want(UnitTypes::Protoss_Forge,complete(UnitTypes::Protoss_Cybernetics_Core) && army.size()>=4);
   want(UnitTypes::Protoss_Citadel_of_Adun,complete(UnitTypes::Protoss_Forge));
   want(UnitTypes::Protoss_Stargate,complete(UnitTypes::Protoss_Citadel_of_Adun));
   want(UnitTypes::Protoss_Robotics_Facility,complete(UnitTypes::Protoss_Stargate));
   want(UnitTypes::Protoss_Observatory,complete(UnitTypes::Protoss_Robotics_Facility));
  }else{
   want(UnitTypes::Terran_Barracks,complete(UnitTypes::Terran_Supply_Depot));
   want(gas_type,gas_ && complete(UnitTypes::Terran_Barracks));
   want(UnitTypes::Terran_Academy,refinery && army.size()>=4);
   want(UnitTypes::Terran_Engineering_Bay,complete(UnitTypes::Terran_Academy));
   want(UnitTypes::Terran_Factory,complete(UnitTypes::Terran_Engineering_Bay));
   want(UnitTypes::Terran_Armory,complete(UnitTypes::Terran_Factory));
   want(UnitTypes::Terran_Starport,complete(UnitTypes::Terran_Armory));
  }
  const auto* type=f.get_unit_type(plan);
  if(base && f.u_completed(base) && need && !side.building && !unfinished && live<unit_budget && f.has_available_resources_for(owner,type)){
   unit_t* worker=nullptr;for(auto* w:workers)if(f.unit_can_build(w,type) && w->order_type->id!=Orders::HarvestGas){worker=w;break;}
   if(worker){if(side.plan!=plan){side.plan=plan;side.search=z?0:24;}
    auto place=[&](xy center){if(center.x<type->placement_size.x/2 || center.y<type->placement_size.y/2)return false;const auto tile=(center-type->placement_size/2)/32,pos=tile*32+type->placement_size/2;
     if(!f.can_place_building(worker,owner,type,pos,true,true))return false;
     if(plan==UnitTypes::Terran_Factory || plan==UnitTypes::Terran_Starport){const auto* addon=f.get_unit_type(plan==UnitTypes::Terran_Factory?UnitTypes::Terran_Machine_Shop:UnitTypes::Terran_Control_Tower);const auto addon_pos=pos-type->placement_size/2+addon->addon_position+addon->placement_size/2;if(!f.can_place_building(worker,owner,addon,addon_pos,true,true))return false;}
     if(!z)for(auto* b:ptr(st.player_units[owner]))if(f.ut_building(b) && !f.unit_dying(b)){auto d=pos-b->sprite->position;if(std::abs(d.x)<(type->placement_size.x+b->unit_type->placement_size.x)/2+16 && std::abs(d.y)<(type->placement_size.y+b->unit_type->placement_size.y)/2+16)return false;}
     a.action_select(owner,worker);if(!a.action_build(owner,a.get_order_type(place_order),type,{size_t(tile.x),size_t(tile.y)}))return false;side.builder=f.get_unit_id_32(worker);side.building=true;side.build_frame=st.current_frame;++side.built;++live;return true;};
    if(plan==gas_type){unit_t* geyser=nullptr;int best=512*512;for(auto* g:ptr(st.visible_units))if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !f.unit_dying(g)){auto d=g->sprite->position-base->sprite->position;int distance=d.x*d.x+d.y*d.y;if(distance<best){best=distance;geyser=g;}}if(geyser)place(geyser->sprite->position);}
    else for(unsigned n=0;n<8;++n){if(side.search>=(z?224u:960u))side.search=z?0:24;if(place(base->sprite->position+spiral(side.search++)*32))break;}
   }
  }
  // Technology purchases reserve their actual cost before optional army units.
  const bool reserve=need && (st.current_minerals[owner]<type->mineral_cost+50 || st.current_gas[owner]<type->gas_cost);
  auto train=[&](unit_t* parent,UnitTypes next,bool morph=false){if(!parent || live>=unit_budget)return false;a.action_select(owner,parent);bool ok=morph?a.action_morph(owner,a.get_unit_type(next)):a.action_train(owner,a.get_unit_type(next));if(ok){++side.trained;++pending[unsigned(next)];++live;}return ok;};
  if(z){
   for(auto* larva:larvae){
    if(free<=5 && !pending[unsigned(supply_type)] && st.supply_available[owner][r]<fp1::integer(120)){if(train(larva,supply_type,true))continue;}
    if(workers.size()+pending[unsigned(worker_type)]<10){train(larva,worker_type,true);continue;}
    if(reserve || army.size()+pending[unsigned(UnitTypes::Zerg_Hydralisk)]+pending[unsigned(UnitTypes::Zerg_Zergling)]*2>=24 || !complete(UnitTypes::Zerg_Spawning_Pool))continue;
    UnitTypes next=UnitTypes::Zerg_Zergling;
    if(complete(UnitTypes::Zerg_Hydralisk_Den) && st.current_gas[owner]>=25)next=UnitTypes::Zerg_Hydralisk;
    if(complete(UnitTypes::Zerg_Spire) && st.current_gas[owner]>=100 && counts[unsigned(UnitTypes::Zerg_Mutalisk)]<6)next=UnitTypes::Zerg_Mutalisk;
    if(complete(UnitTypes::Zerg_Ultralisk_Cavern) && st.current_gas[owner]>=200 && counts[unsigned(UnitTypes::Zerg_Ultralisk)]<2)next=UnitTypes::Zerg_Ultralisk;
    train(larva,next,true);
   }
   if(!reserve && army.size()>=4 && base && f.u_completed(base)){
    a.action_select(owner,base);
    if(f.unit_is(base,UnitTypes::Zerg_Hatchery) && complete(UnitTypes::Zerg_Hydralisk_Den))a.action_morph_building(owner,a.get_unit_type(UnitTypes::Zerg_Lair));
    else if(f.unit_is(base,UnitTypes::Zerg_Lair) && complete(UnitTypes::Zerg_Queens_Nest))a.action_morph_building(owner,a.get_unit_type(UnitTypes::Zerg_Hive));
   }
  }else{
   if(base && f.u_completed(base) && base->build_queue.empty() && workers.size()<10)train(base,worker_type);
   unsigned queued=0;for(auto* b:production)queued+=b->build_queue.size();
   for(auto* b:production)if(b->build_queue.empty() && !reserve && army.size()+queued<24){
    UnitTypes next=p?UnitTypes::Protoss_Zealot:UnitTypes::Terran_Marine;bool produces=true;
    if(p){if(f.unit_is(b,UnitTypes::Protoss_Gateway)){if(counts[65]+counts[66]+pending[65]+pending[66]>=16)continue;if(complete(UnitTypes::Protoss_Cybernetics_Core) && st.current_gas[owner]>=50 && counts[unsigned(UnitTypes::Protoss_Dragoon)]<(army.size()+queued)/2+1)next=UnitTypes::Protoss_Dragoon;}
     else if(f.unit_is(b,UnitTypes::Protoss_Stargate)){if(counts[70]+pending[70]>=6)continue;next=UnitTypes::Protoss_Scout;}
     else if(f.unit_is(b,UnitTypes::Protoss_Robotics_Facility) && complete(UnitTypes::Protoss_Observatory) && !has(UnitTypes::Protoss_Observer))next=UnitTypes::Protoss_Observer;
     else produces=false;
    }else{
     if(f.unit_is(b,UnitTypes::Terran_Barracks)){if(counts[0]+counts[32]+counts[34]+pending[0]+pending[32]+pending[34]>=12)continue;if(complete(UnitTypes::Terran_Academy) && st.current_gas[owner]>=25){if(counts[unsigned(UnitTypes::Terran_Medic)]<2)next=UnitTypes::Terran_Medic;else if(counts[unsigned(UnitTypes::Terran_Firebat)]<2)next=UnitTypes::Terran_Firebat;}}
     else if(f.unit_is(b,UnitTypes::Terran_Factory)){
      if(counts[2]+counts[3]+counts[5]+counts[30]+pending[2]+pending[3]+pending[5]>=8)continue;
      if(!b->building.addon){a.action_select(owner,b);const auto* addon=a.get_unit_type(UnitTypes::Terran_Machine_Shop);const auto pos=b->sprite->position-b->unit_type->placement_size/2+addon->addon_position+addon->placement_size/2;
       if(f.unit_can_build(b,addon) && f.has_available_resources_for(owner,addon) && f.can_place_building(b,owner,addon,pos,true,true))f.place_building(b,a.get_order_type(Orders::PlaceAddon),addon,b->sprite->position);produces=false;}
      else next=st.current_gas[owner]>=100 && counts[unsigned(UnitTypes::Terran_Siege_Tank_Tank_Mode)]<3?UnitTypes::Terran_Siege_Tank_Tank_Mode:complete(UnitTypes::Terran_Armory)?UnitTypes::Terran_Goliath:UnitTypes::Terran_Vulture;
     }else if(f.unit_is(b,UnitTypes::Terran_Starport)){if(counts[8]+pending[8]>=4)continue;next=UnitTypes::Terran_Wraith;}else produces=false;
    }
    if(produces && train(b,next))++queued;
   }
  }
  // Research is paid and runs for native durations; one level at a time.
  if(!reserve && army.size()>=4)for(auto* b:production){if(f.unit_is_researching(b) || f.unit_is_upgrading(b) || !b->build_queue.empty())continue;a.action_select(owner,b);
   bool busy=false;
   catalogue::upgrades([&](const terran::Upgrade& entry){if(!busy && entry.parent==b->unit_type->id && st.upgrade_levels[owner][entry.type]<1)if(a.action_upgrade(owner,a.get_upgrade_type(entry.type))){busy=true;++side.researches;}});
   if(!busy && f.unit_is(b,UnitTypes::Terran_Machine_Shop) && !st.tech_researched[owner][TechTypes::Tank_Siege_Mode])if(a.action_research(owner,a.get_tech_type(TechTypes::Tank_Siege_Mode)))++side.researches;
  }
  // React to a nearby visible threat, otherwise send waves toward an enemy
  // start location. Do not inspect unseen enemy units for target selection.
  unit_t* threat=nullptr;int nearest=480*480;
  if(base)for(auto* e:ptr(st.visible_units))if(e->owner<3 && e->owner!=owner && active_[e->owner] && !f.unit_dying(e) && (e->sprite->visibility_flags&(1u<<owner)) && !f.unit_is_undetected(e,owner)){auto d=e->sprite->position-base->sprite->position;int distance=d.x*d.x+d.y*d.y;if(distance<nearest){nearest=distance;threat=e;}}
  if((threat || st.current_frame>=side.attack_frame) && (threat || army.size()>=6)){
   unsigned enemy=owner;for(unsigned p2=1;p2<=3;++p2){const unsigned candidate=(owner+p2)%3;if(candidate!=owner && active_[candidate]){bool buildings=false;for(auto* u:ptr(st.player_units[candidate]))if(!f.unit_dying(u) && f.ut_building(u)){buildings=true;break;}if(buildings){enemy=candidate;break;}}}
   const xy target=threat?threat->sprite->position:st.game->start_locations[enemy];unsigned sent=0;
   if(enemy!=owner || threat)for(auto* u:army){
    const bool tank=f.unit_is(u,UnitTypes::Terran_Siege_Tank_Tank_Mode),sieged=f.unit_is(u,UnitTypes::Terran_Siege_Tank_Siege_Mode);
    if(st.tech_researched[owner][TechTypes::Tank_Siege_Mode] && (tank || sieged)){auto d=target-u->sprite->position;int distance=d.x*d.x+d.y*d.y;a.action_select(owner,u);if(tank && distance<350*350 && distance>80*80){a.action_siege(owner,false);continue;}if(sieged && distance>400*400){a.action_unsiege(owner,false);continue;}}
    if(!threat && !idle(u))continue;a.action_select(owner,u);if(a.action_order(owner,a.get_order_type(Orders::AttackMove),target,nullptr,nullptr,false))++sent;
   }
   if(sent>=4)++side.waves;side.attack_frame=st.current_frame+240;
  }
 }
public:
 static constexpr unsigned unit_budget=ai_unit_budget;
 void reset(bool red,bool blue,bool gas=false,bool=false,bool=false){custom_=false;active_={{true,true,false}};gas_=gas;sides_={};sides_[0].enabled=red;sides_[1].enabled=blue;winner_=-1;}
 void configure(const MatchSetup& setup){custom_=true;gas_=true;sides_={};winner_=-1;for(unsigned p=0;p<3;++p){active_[p]=setup.races[p]>=0;sides_[p].enabled=p!=0 && active_[p];}}
 bool enabled(unsigned p)const{return p<3 && sides_[p].enabled;}
 bool observer()const{return enabled(0) && enabled(1);}
 int winner()const{return winner_;}
 bool surrender(){if(enabled(0) || winner_>=0)return false;winner_=active_[1]?1:active_[2]?2:3;return true;}
 unsigned trained(unsigned p)const{return sides_[p].trained;}unsigned built(unsigned p)const{return sides_[p].built;}unsigned waves(unsigned p)const{return sides_[p].waves;}unsigned lost(unsigned p)const{return sides_[p].lost;}
 void step(game_player& player){
  if(winner_>=0 || (!enabled(0) && !enabled(1) && !enabled(2)) || player.st().current_frame%12)return;
  auto& f=player.funcs();bool buildings[3]{};
  for(unsigned p=0;p<3;++p){auto& side=sides_[p];for(auto id:side.previous){auto* u=f.get_unit(id);if(!u || f.unit_dying(u))++side.lost;}side.previous.clear();
   for(auto* u:ptr(player.st().player_units[p]))if(!f.unit_dying(u)){if(side.previous.size()<128)side.previous.push_back(f.get_unit_id_32(u));if(f.ut_building(u))buildings[p]=true;}
  }
  if(custom_){if(!buildings[0]){winner_=buildings[1]?1:buildings[2]?2:3;return;}if((active_[1] || active_[2]) && !((active_[1] && buildings[1]) || (active_[2] && buildings[2]))){winner_=0;return;}}
  else if(!buildings[0] || !buildings[1]){winner_=buildings[0]?0:buildings[1]?1:2;return;}
  for(unsigned p=0;p<3;++p)if(active_[p] && buildings[p])update(player,p);
 }
};
} }
