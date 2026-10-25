#pragma once
#include "actions.h"
#include "skirmish.h"
namespace port3ds { namespace detail {
using namespace bwgame;
// Bounded, sight-limited decisions. Orders, cargo, energy and spell effects
// are validated/executed by the original engine, never teleported or granted.
class BattleTactics {
 unit_id_32 scout_{},transport_{};static_vector<unit_id_32,8> cargo_;
 xy area_pos_{};unsigned area_until_=0;
 unsigned unload_tries_=0;
 unsigned scout_next_=0,scout_after_=0,phase_=0,since_=0,retry_=0,cast_after_=0;
 xy drop_{},home_{},goal_{},scout_goal_{};bool island_=false;
 unsigned air_=0,ground_=0,mechanical_=0,observed_at_=0;
 unsigned scouts_=0,casts_=0,loads_=0,drops_=0,caster_drops_=0;
 static int distance(xy a,xy b){auto d=a-b;return d.x*d.x+d.y*d.y;}
 static bool caster(UnitTypes t){return t==UnitTypes::Protoss_High_Templar || t==UnitTypes::Terran_Science_Vessel || t==UnitTypes::Zerg_Queen || t==UnitTypes::Zerg_Defiler;}
public:
 bool island()const{return island_;}unsigned seen_air()const{return air_;}unsigned seen_ground()const{return ground_;}
 unsigned caster_drops()const{return caster_drops_;}
 unsigned scouts()const{return scouts_;}unsigned casts()const{return casts_;}unsigned loads()const{return loads_;}unsigned drops()const{return drops_;}
 bool reserved(unit_id_32 id)const{if(id==scout_ || id==transport_)return true;for(auto c:cargo_)if(c==id)return true;return false;}
 void step(game_player& player,unsigned owner,unit_t* base,const std::array<bool,3>& enemies,AiDifficulty difficulty,xy known_target={}){
  if(!base)return;auto& f=player.funcs();auto& st=player.st();action_state state;action_functions a(st,state);
  auto valid=[&](unit_id_32 id){auto* u=f.get_unit(id);return u && !f.unit_dying(u)?u:nullptr;};
  auto sees=[&](const unit_t* u){return u && u->sprite && !f.unit_dying(u) && (u->sprite->visibility_flags&(1u<<owner)) && !f.unit_is_undetected(u,owner);};
  auto move=[&](unit_t* u,xy pos){a.action_select(owner,u);return a.action_order(owner,a.get_order_type(Orders::Move),f.restrict_pos_to_map_bounds(pos),nullptr,nullptr,false);};
  auto home=base->sprite->position;static_vector<unit_t*,128> own,enemy;unit_t* lead=nullptr;
  unsigned air=0,ground=0,mech=0,workers=0;
  for(auto* u:ptr(st.player_units[owner]))if(!f.unit_dying(u) && f.u_completed(u)){
   if(f.ut_worker(u))++workers;
   if(!f.ut_building(u) && !f.ut_turret(u) && !f.us_hidden(u) && own.size()<128){own.push_back(u);if(!f.ut_worker(u) && !f.u_flying(u) && f.unit_or_subunit_ground_weapon(u) && (!lead || distance(u->sprite->position,home)>distance(lead->sprite->position,home)))lead=u;}
  }
  for(auto* e:ptr(st.visible_units))if(e->owner<3 && enemies[e->owner] && sees(e)){
   if(enemy.size()<128)enemy.push_back(e);
   if(!f.ut_building(e)){if(f.u_flying(e))++air;else ++ground;if(f.ut_mechanical(e))++mech;}
  }
  if(st.current_frame>observed_at_+24*90){air_=ground_=mechanical_=0;observed_at_=st.current_frame;}
  air_=std::max(air_,air);ground_=std::max(ground_,ground);mechanical_=std::max(mechanical_,mech);
  island_=false;xy island_goal{};
  for(unsigned n=1;n<=8;++n){auto pos=st.game->start_locations[(owner+n)%8];if(pos!=xy{} && !f.is_reachable(base,pos)){island_=true;island_goal=pos;break;}}
  if(known_target!=xy{} && !f.is_reachable(base,known_target)){island_=true;island_goal=known_target;}
  // One scout, with an explicit role so harvesting/build/attack cannot steal it.
  auto* scout=valid(scout_);
  if(!scout){scout_={};if(st.current_frame>=scout_after_ && st.current_frame>=24*(difficulty==AiDifficulty::medium?45u:90u) && workers>=6){
    for(auto* u:own)if(f.u_flying(u) && !caster(u->unit_type->id) && !(f.get_unit_id_32(u)==transport_)){scout=u;break;}
    if(!scout && !island_)for(auto* u:own)if(f.ut_worker(u) && u->order_type->id!=Orders::ConstructingBuilding && u->order_type->id!=Orders::PlaceBuilding && u->order_type->id!=Orders::PlaceProtossBuilding && u->order_type->id!=Orders::DroneStartBuild && u->order_type->id!=Orders::HarvestGas && u->order_type->id!=Orders::ReturnGas){scout=u;break;}
    if(scout){scout_=f.get_unit_id_32(scout);++scouts_;scout_goal_=home;}
   }}
  if(scout){bool danger=scout->hp<scout->unit_type->hitpoints/3;for(auto* e:enemy)if(distance(e->sprite->position,scout->sprite->position)<160*160 && f.unit_target_weapon(e,scout))danger=true;
   if(danger){move(scout,home);scout_={};scout_after_=st.current_frame+24*60;}
   else {if(distance(scout->sprite->position,scout_goal_)<96*96 || st.current_frame%240==0){
     for(unsigned n=0;n<8;++n){unsigned index=scout_next_++%8;auto pos=st.game->start_locations[index];if(pos==xy{} || distance(pos,home)<256*256 || (!f.u_flying(scout) && !f.is_reachable(scout,pos)))continue;scout_goal_=pos;break;}
    }if(distance(scout->order_target.pos,scout_goal_)>64*64)move(scout,scout_goal_);}
  }
  // Airlift state machine: gather -> load -> fly -> unload/reposition -> return.
  auto* transport=valid(transport_);
  if(!transport){transport_={};phase_=0;cargo_.clear();}
  if(!phase_ && st.current_frame>=retry_ && lead && st.current_frame>=ai_profile(difficulty).first_attack){
   xy target=island_goal;bool found=island_;
   if(!found && difficulty==AiDifficulty::medium)for(auto* e:enemy)if(f.ut_building(e) && distance(e->sprite->position,home)>800*800){target=e->sprite->position;found=true;break;}
   if(found)for(auto* u:own)if(f.u_flying(u) && f.unit_provides_space(u)){
    transport=u;transport_=f.get_unit_id_32(u);if(scout_==transport_)scout_={};home_=home+xy{0,128};goal_=target;drop_=target;phase_=1;since_=st.current_frame;unload_tries_=0;cargo_.clear();move(u,home_);break;
   }
  }
  if(transport && phase_){
   if(phase_==1){
    unsigned loaded=0;for(auto* u:f.loaded_units(transport))++loaded;
    if(loaded && (f.unit_space_left(transport)==0 || st.current_frame-since_>240)){
     // Pick ground outside the base footprint, on the target island. Native
     // unload still checks collision; blocked sites get bounded retries.
     for(unsigned n=0;n<16;++n){xy delta{int(n%4)*96-144,int(n/4)*96-144};auto pos=f.restrict_pos_to_map_bounds(goal_+delta);if(f.is_walkable(pos) && f.is_reachable(pos,goal_)){drop_=pos;break;}}
     for(auto it=cargo_.begin();it!=cargo_.end();){auto* u=valid(*it);if(!u || !f.u_loaded(u)){if(u){a.action_select(owner,u);a.action_order(owner,a.get_order_type(Orders::Stop),u->sprite->position,nullptr,nullptr,false);}it=cargo_.erase(it);}else ++it;}
     phase_=2;since_=st.current_frame;move(transport,drop_);
    }else if(st.current_frame-since_>24*30){phase_=4;since_=st.current_frame;move(transport,home_);}
    else if(distance(transport->sprite->position,home_)<160*160){
     if(cargo_.size()<8 && f.unit_space_left(transport)){
      bool has_caster=false;for(auto id:cargo_)if(auto* u=valid(id))if(caster(u->unit_type->id))has_caster=true;
      unit_t* candidate=nullptr;int best=0x7fffffff;
      for(auto* u:own){const auto id=f.get_unit_id_32(u);
       if(f.ut_worker(u) || f.u_flying(u) || f.u_loaded(u) || f.unit_is_sieged_tank(u) || reserved(id) || !u->unit_type->space_required || (!f.unit_or_subunit_ground_weapon(u) && !caster(u->unit_type->id)) || !f.unit_can_load_target(transport,u) || !f.is_reachable(u,home_) || distance(u->sprite->position,home_)>384*384)continue;
       int score=distance(u->sprite->position,home_);if(!has_caster && caster(u->unit_type->id))score-=512*512;if(score<best){best=score;candidate=u;}
      }
      if(candidate){a.action_select(owner,candidate);if(a.action_default_order(owner,transport->sprite->position,transport,nullptr,false)){cargo_.push_back(f.get_unit_id_32(candidate));++loads_;}}
     }
    }
   }else if(phase_==2){
    if(distance(transport->sprite->position,drop_)<64*64){phase_=3;since_=st.current_frame;a.action_select(owner,transport);a.action_unload_all(owner,false);}
    else if(st.current_frame-since_>24*180){phase_=4;since_=st.current_frame;move(transport,home_);}
   }else if(phase_==3){
    for(auto it=cargo_.begin();it!=cargo_.end();){auto* u=valid(*it);if(!u){it=cargo_.erase(it);continue;}if(!f.u_loaded(u) && distance(u->sprite->position,home_)>384*384){a.action_select(owner,u);a.action_order(owner,a.get_order_type(Orders::AttackMove),goal_,nullptr,nullptr,false);it=cargo_.erase(it);++drops_;if(caster(u->unit_type->id))++caster_drops_;}else ++it;}
    if(f.loaded_units(transport).empty()){phase_=4;since_=st.current_frame;move(transport,home_);}
    else if(st.current_frame-since_>96){if(++unload_tries_>=12){phase_=4;since_=st.current_frame;move(transport,home_);}else {drop_=f.restrict_pos_to_map_bounds(drop_+xy{int((st.current_frame/96)%3)*64-64,64});move(transport,drop_);phase_=2;since_=st.current_frame;}}
   }else if(phase_==4 && distance(transport->sprite->position,home_)<96*96){a.action_select(owner,transport);a.action_unload_all(owner,false);if(f.loaded_units(transport).empty()){phase_=0;transport_={};cargo_.clear();retry_=st.current_frame+24*20;}}
  }
  // One new spell every second per side. Caster orders cannot be overwritten
  // while native cast animation/pathing runs. Area spells avoid friendly blobs.
  for(auto* u:own){const auto id=f.get_unit_id_32(u);if(reserved(id) || f.ut_worker(u) || f.unit_is_disabled(u))continue;
   unit_t* near=nullptr;int best=320*320;for(auto* e:enemy){int d=distance(u->sprite->position,e->sprite->position);if(d<best){best=d;near=e;}}
   if(near && !u->stim_timer && u->hp>u->unit_type->hitpoints/2 && (f.unit_is(u,UnitTypes::Terran_Marine) || f.unit_is(u,UnitTypes::Terran_Firebat)) && st.tech_researched[owner][TechTypes::Stim_Packs]){a.action_select(owner,u);if(a.action_stim_pack(owner))++casts_;}
   if(f.unit_is(u,UnitTypes::Terran_Wraith) && st.tech_researched[owner][TechTypes::Cloaking_Field]){
    a.action_select(owner,u);if(near && !f.u_requires_detector(u) && u->energy>=fp8::integer(50)){a.action_cloak(owner);++casts_;}
    else if(!near && f.u_requires_detector(u))a.action_decloak(owner);
   }
   if(!caster(u->unit_type->id))continue;
   if(u->order_type->tech_type!=TechTypes::None)continue;
   bool cast=false;
   auto spell=[&](Orders order,unit_t* target,xy pos){if(st.current_frame<cast_after_)return false;const bool area=order==Orders::CastPsionicStorm || order==Orders::CastEMPShockwave || order==Orders::CastEnsnare || order==Orders::CastPlague || order==Orders::CastDarkSwarm;
    if(area && st.current_frame<area_until_ && distance(area_pos_,pos)<96*96)return false;
    auto* type=a.get_order_type(order);auto* tech=a.get_tech_type(type->tech_type);if(!f.unit_can_use_tech(u,tech,owner) || u->energy<fp8::integer(tech->energy_cost))return false;
    if(target){if(f.u_invincible(target))return false;
     if(type->targets_enemies && type->weapon!=WeaponTypes::None){if(!f.weapon_can_target_unit(f.get_weapon_type(type->weapon),target,u))return false;}
     else if(!f.unit_tech_target_valid(u,tech,target))return false;
    }
    a.action_select(owner,u);if(!a.action_order(owner,type,pos,target,nullptr,false))return false;++casts_;cast_after_=st.current_frame+24;if(area){area_pos_=pos;area_until_=st.current_frame+96;}cast=true;return true;};
   auto cluster=[&](unit_t* e,unsigned radius,bool organic=false){unsigned hostile=0,friendly=0;for(auto* n:enemy)if(!f.ut_building(n) && (!organic || f.ut_organic(n)) && distance(n->sprite->position,e->sprite->position)<int(radius*radius))++hostile;
    for(auto* n:own)if(distance(n->sprite->position,e->sprite->position)<int(radius*radius))++friendly;return hostile>=3 && friendly<=1;};
   if(f.unit_is(u,UnitTypes::Protoss_High_Templar)){for(auto* e:enemy)if(distance(u->sprite->position,e->sprite->position)<288*288 && cluster(e,72) && spell(Orders::CastPsionicStorm,nullptr,e->sprite->position))break;}
   else if(f.unit_is(u,UnitTypes::Terran_Science_Vessel)){
    for(auto* e:enemy)if(distance(u->sprite->position,e->sprite->position)<288*288){if(e->shield_points>=fp8::integer(60) && cluster(e,80) && spell(Orders::CastEMPShockwave,nullptr,e->sprite->position))break;
     if(f.ut_organic(e) && !e->irradiate_timer && e->hp>=fp8::integer(80) && spell(Orders::CastIrradiate,e,e->sprite->position))break;}
    if(!cast)for(auto* n:own)if(!f.ut_worker(n) && !f.u_flying(n) && n->hp<n->unit_type->hitpoints*3/4 && !n->defensive_matrix_timer && distance(n->sprite->position,u->sprite->position)<288*288 && n->order_target.unit && spell(Orders::CastDefensiveMatrix,n,n->sprite->position))break;
   }else if(f.unit_is(u,UnitTypes::Zerg_Queen)){for(auto* e:enemy)if(distance(u->sprite->position,e->sprite->position)<288*288){if(!e->ensnare_timer && cluster(e,96) && spell(Orders::CastEnsnare,nullptr,e->sprite->position))break;
     if(!f.ut_building(e) && !f.u_flying(e) && f.ut_organic(e) && e->unit_type->gas_cost>=50 && spell(Orders::CastSpawnBroodlings,e,e->sprite->position))break;}
   }else if(f.unit_is(u,UnitTypes::Zerg_Defiler)){for(auto* e:enemy)if(distance(u->sprite->position,e->sprite->position)<288*288 && !e->plague_timer && cluster(e,80) && spell(Orders::CastPlague,nullptr,e->sprite->position))break;
    if(!cast && near && lead && distance(lead->sprite->position,near->sprite->position)<160*160)spell(Orders::CastDarkSwarm,nullptr,lead->sprite->position);
    if(!cast && u->energy<fp8::integer(100) && near)for(auto* n:own)if(f.unit_is(n,UnitTypes::Zerg_Zergling) && distance(n->sprite->position,u->sprite->position)<96*96 && spell(Orders::CastConsume,n,n->sprite->position))break;
   }
   if(!cast && lead && distance(u->sprite->position,lead->sprite->position)>128*128 && st.current_frame%120==0)move(u,lead->sprite->position);
  }
 }
};
} }
