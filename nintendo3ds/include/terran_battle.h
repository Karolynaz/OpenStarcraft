#pragma once
#include "actions.h"
#include "race_catalogue.h"
#include "protoss_fixture.h"
#include "zerg_fixture.h"
namespace port3ds {namespace detail {
using namespace bwgame;
// A prepared, symmetric performance fixture, not the ordinary economy mode.
class TerranBattle {
 bool enabled_=false;unsigned race_=1;int winner_=-1;unsigned reason_=0;
 unsigned initial_[2]{},score_[2]{},lost_[2]{},count_[2]{};
 unsigned initial_count_[2]{},orders_=0;bool nuclear_ordered_[2]{};uint64_t elapsed_=0;
 static unsigned value(const unit_t* u) {return std::max(1,u->unit_type->mineral_cost+u->unit_type->gas_cost);}
 static bool active(state_functions& f,const unit_t* u) {
  return !f.unit_dead(u) && !f.ut_building(u) && !f.ut_turret(u) && !f.unit_is_fighter(u) &&
   !f.unit_is(u,UnitTypes::Zerg_Larva) && !f.unit_is(u,UnitTypes::Zerg_Egg) && !f.unit_is(u,UnitTypes::Zerg_Cocoon) && !f.unit_is(u,UnitTypes::Zerg_Lurker_Egg) && !f.unit_is(u,UnitTypes::Zerg_Overlord) && !f.unit_is(u,UnitTypes::Terran_Nuclear_Missile) && !f.unit_is(u,UnitTypes::Terran_Vulture_Spider_Mine);
 }
 void evaluate(state_functions& f) {
  score_[0]=score_[1]=count_[0]=count_[1]=0;
  for(unsigned p=0;p<2;++p) for(auto* u:ptr(f.st.player_units[p])) if(active(f,u)) {score_[p]+=value(u);++count_[p];}
  for(unsigned p=0;p<2;++p) lost_[p]=initial_count_[p]>count_[p]?initial_count_[p]-count_[p]:0;
  if(!count_[0] || !count_[1]) reason_=1;
  else if(f.st.current_frame>=24*30 && (score_[0]*100<=initial_[0]*35 || score_[1]*100<=initial_[1]*35)) reason_=2;
  else if(elapsed_>=180000000 || f.st.current_frame>=24*180) reason_=3;
  if(reason_) winner_=score_[0]==score_[1]?2:score_[0]>score_[1]?0:1;
 }
public:
 static constexpr unsigned limit_seconds=180;
 void reset(bool on,unsigned race=1) { *this=TerranBattle{};enabled_=on;race_=race; }
 bool enabled()const{return enabled_;}int winner()const{return winner_;}
 unsigned score(unsigned p)const{return score_[p];}unsigned lost(unsigned p)const{return lost_[p];}
 unsigned count(unsigned p)const{return count_[p];}unsigned reason()const{return reason_;}
 unsigned seconds()const{return elapsed_/1000000;}
 void elapsed(state_functions& f,uint64_t us) {if(enabled_ && winner_<0) {elapsed_+=us;evaluate(f);}}
 void setup(game_load_functions& f,bool lab) {
  if(!lab && !enabled_)return;
  if(race_!=1){if(race_==2)setup_protoss(f,lab);else setup_zerg(f,lab);if(!lab)for(unsigned p=0;p<2;++p)for(auto* u:ptr(f.st.player_units[p]))if(active(f,u)){initial_[p]+=value(u);++initial_count_[p];}evaluate(f);return;}
  for(unsigned p=0;p<(lab?1u:2u);++p) {
   // Prepared lab funds are explicit. Normal Player vs AI still starts at 50/0.
   if(lab) {f.st.current_minerals[p]=10000;f.st.current_gas[p]=10000;}
   else for(const auto& t:terran::techs)f.st.tech_researched[p][t.type]=true;
   unsigned i=0;
   if(lab) {
    for(const auto& b:terran::buildings) if(b.parent==UnitTypes::Terran_SCV && b.type!=UnitTypes::Terran_Refinery) {
     const xy pos={256+int(i%5)*320,256+int(i/5)*320};
     auto* parent=f.create_initial_unit(f.get_unit_type(b.type),pos,p);++i;
     // One legal addon per parent; duplicate parents for mutually exclusive addons.
     for(const auto& ad:terran::buildings) if(ad.parent==b.type) {
      auto* use=parent;
      if(parent->building.addon) {use=f.create_initial_unit(f.get_unit_type(b.type),{256+int(i%5)*320,256+int(i/5)*320},p);++i;}
      const auto* type=f.get_unit_type(ad.type);
      const auto ap=use->sprite->position-use->unit_type->placement_size/2+type->addon_position+type->placement_size/2;
      auto* addon=f.create_initial_unit(type,ap,p);f.connect_addon(use,addon);
     }
    }
    for(unsigned n=0;n<6;++n)f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Supply_Depot),{256+int(n)*220,1400},p);
    f.create_initial_unit(f.get_unit_type(UnitTypes::Resource_Vespene_Geyser),{600,1800},11);
    for(unsigned n=0;n<8;++n) f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_SCV),{256+int(n)*36,1600},p);
   } else {
    const unsigned amounts[]={4,24,12,8,8,12,12,10,8,4,4,4,4};
    auto spawn=[&](const unit_type_t* type,xy pos) {
     auto* u=f.create_unit(type,pos,p);if(!u)throw std::runtime_error("Stress unit allocation failed");
     f.finish_building_unit(u);
     const auto place=f.find_unit_placement(u,pos,{pos-xy(512,512),pos+xy(512,512)},true);
     if(!place.first)throw std::runtime_error("No terrain space for stress army");
     f.move_unit(u,place.second);if(!f.place_completed_unit(u))throw std::runtime_error("Stress placement failed");f.complete_unit(u);return u;
    };
    // Native supply providers and a prepared silo: 192 army supply plus
    // the missile's eight supply, within the standard 200 limit.
    const xy base={p?1696:256,1920};
    auto* cc=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Command_Center),base,p);
    const auto* silo_type=f.get_unit_type(UnitTypes::Terran_Nuclear_Silo);
    auto* silo=f.create_initial_unit(silo_type,base-cc->unit_type->placement_size/2+silo_type->addon_position+silo_type->placement_size/2,p);
    f.connect_addon(cc,silo);
    for(unsigned j=0;j<24;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Supply_Depot),{int(p?1216:128)+int(j%6)*128,1408+int(j/6)*96},p);
    auto* missile=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Nuclear_Missile),silo->sprite->position,p);
    f.hide_unit(missile);missile->connected_unit=silo;silo->building.silo.nuke=missile;silo->building.silo.ready=true;
    unsigned n=0;
    for(unsigned k=0;k<13;++k) for(unsigned j=0;j<amounts[k];++j,++n) {
     const xy pos={int(p?1440:600)+(p?-1:1)*(int(n%12)-6)*40,650+int(n/12)*48};
     auto* u=spawn(f.get_unit_type(terran::units[k].type),pos);
     if(f.ut_has_energy(u)) u->energy=fp8::integer(200);
    }
   }
  }
  if(!lab) for(unsigned p=0;p<2;++p)for(auto* u:ptr(f.st.player_units[p]))if(active(f,u)) {initial_[p]+=value(u);++initial_count_[p];}
  evaluate(f);
 }
 void step(game_player& player) {
  if(!enabled_ || winner_>=0)return;
  auto& f=player.funcs();auto& st=player.st();
  if(st.current_frame%12)return;
  evaluate(f);if(winner_>=0)return;
  if(race_!=1){if(race_==2)step_protoss(player);else step_zerg(player);return;}
  action_state as;action_functions a(st,as);
  for(unsigned p=0;p<2;++p) {
   // Re-use fixed arrays: no all-pairs full army search every simulation frame.
   unit_t* enemy=nullptr;unit_t* biological=nullptr;unit_t* mechanical=nullptr;unit_t* friendly=nullptr;unit_t* wounded=nullptr;
   for(auto* u:ptr(st.player_units[1-p]))if(active(f,u)) {
    if(!enemy)enemy=u;
    if(f.ut_mechanical(u) && !mechanical)mechanical=u;
    if(f.ut_organic(u) && !biological)biological=u;
   }
   for(auto* u:ptr(st.player_units[p]))if(active(f,u) && !f.us_hidden(u)) {
    if(!friendly || f.unit_is(u,UnitTypes::Terran_Battlecruiser))friendly=u;
    if(f.ut_organic(u) && u->hp<u->unit_type->hitpoints)wounded=u;
   }
   if(!enemy)continue;
   for(auto* u:ptr(st.player_units[p]))if(active(f,u) && !f.us_hidden(u)) {
    a.action_select(p,u);const auto type=u->unit_type->id;
    if(type==UnitTypes::Terran_Ghost) {
     const auto order=u->order_type->id;
     if(order==Orders::NukePaint || order==Orders::NukeUnit || order==Orders::CastNuclearStrike || order==Orders::NukeTrack)continue;
     if(!nuclear_ordered_[p]) {a.action_cloak(p);a.action_order(p,a.get_order_type(Orders::NukePaint),{1024,1000},nullptr,nullptr,false);nuclear_ordered_[p]=true;continue;}
    }
    if(type==UnitTypes::Terran_Dropship) {
     if(st.current_frame==12) {for(auto* passenger:ptr(st.player_units[p]))if(f.unit_is(passenger,UnitTypes::Terran_Marine) && !f.us_hidden(passenger)){a.action_order(p,a.get_order_type(Orders::PickupTransport),passenger->sprite->position,passenger,nullptr,false);break;}continue;}
     if(st.current_frame>=240 && !f.loaded_units(u).empty()){a.action_unload_all(p,false);continue;}
    }
    const auto d=enemy->sprite->position-u->sprite->position;const int distance=std::max(std::abs(d.x),std::abs(d.y));
    if(type==UnitTypes::Terran_Siege_Tank_Tank_Mode && distance<350) {a.action_siege(p,false);continue;}
    if(type==UnitTypes::Terran_Siege_Tank_Siege_Mode) {if(distance>430)a.action_unsiege(p,false);continue;}
    if(st.current_frame%120==0) {
     if(type==UnitTypes::Terran_Marine || type==UnitTypes::Terran_Firebat) {if(u->hp>fp8::integer(25) && !u->stim_timer)a.action_stim_pack(p);}
     if(type==UnitTypes::Terran_Ghost && mechanical)a.action_order(p,a.get_order_type(Orders::CastLockdown),mechanical->sprite->position,mechanical,nullptr,false);
     if(type==UnitTypes::Terran_Science_Vessel && friendly)a.action_order(p,a.get_order_type(Orders::CastDefensiveMatrix),friendly->sprite->position,friendly,nullptr,false);
     if(type==UnitTypes::Terran_Battlecruiser && mechanical)a.action_order(p,a.get_order_type(Orders::FireYamatoGun),mechanical->sprite->position,mechanical,nullptr,false);
     if(type==UnitTypes::Terran_Vulture && st.current_frame<=240)a.action_order(p,a.get_order_type(Orders::PlaceMine),u->sprite->position,nullptr,nullptr,false);
    }
    const auto order=u->order_type->id;
    const bool idle=order==Orders::PlayerGuard || order==Orders::Guard || order==Orders::Stop || order==Orders::Nothing || order==u->unit_type->return_to_idle->id;
    if(type==UnitTypes::Terran_Medic && wounded) {if(idle)a.action_order(p,a.get_order_type(Orders::MedicHeal),wounded->sprite->position,wounded,nullptr,false);continue;}
    if(type==UnitTypes::Terran_Science_Vessel || type==UnitTypes::Terran_Dropship || type==UnitTypes::Terran_SCV) {
     if(idle && friendly && friendly!=u)a.action_default_order(p,friendly->sprite->position,friendly,nullptr,false);
    } else if(idle || st.current_frame==12) {a.action_order(p,a.get_order_type(Orders::AttackMove),enemy->sprite->position,nullptr,nullptr,false);++orders_;}
   }
  }
 }
};
} }
