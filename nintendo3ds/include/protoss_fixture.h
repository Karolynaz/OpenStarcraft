#pragma once
#include "actions.h"
#include "protoss_catalogue.h"
namespace port3ds {namespace detail {
using namespace bwgame;
inline void setup_protoss(game_load_functions& f,bool lab){
 for(unsigned p=0;p<(lab?1u:2u);++p){
  f.st.current_minerals[p]=10000;f.st.current_gas[p]=10000;
  if(lab){
   unsigned i=0;
   for(const auto& b:protoss::buildings)if(b.type!=UnitTypes::Protoss_Assimilator){
    const xy pos={192+int(i%5)*320,192+int(i/5)*320};++i;
    f.create_initial_unit(f.get_unit_type(b.type),pos,p);
    if(b.type==UnitTypes::Protoss_Shield_Battery){auto* patient=f.create_initial_unit(f.get_unit_type(UnitTypes::Protoss_Zealot),pos+xy(100,0),p);patient->shield_points=1_fp8;}
    if(b.type!=UnitTypes::Protoss_Pylon && b.type!=UnitTypes::Protoss_Nexus)f.create_initial_unit(f.get_unit_type(UnitTypes::Protoss_Pylon),pos+xy(0,128),p);
   }
   f.create_initial_unit(f.get_unit_type(UnitTypes::Resource_Vespene_Geyser),{600,1800},11);
   for(unsigned j=0;j<8;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Protoss_Probe),{256+int(j)*36,1600},p);
   unsigned target=0;for(auto type:{UnitTypes::Terran_Battlecruiser,UnitTypes::Terran_Ghost,UnitTypes::Protoss_Zealot,UnitTypes::Terran_Siege_Tank_Tank_Mode}){
    auto* enemy=f.create_initial_unit(f.get_unit_type(type),{1850,1450+int(target++)*150},1);
    if(f.ut_has_energy(enemy))enemy->energy=fp8::integer(200);f.set_unit_order(enemy,f.get_order_type(Orders::HoldPosition));
   }
  }else{
   for(const auto& t:protoss::techs)f.st.tech_researched[p][t.type]=true;
   f.st.upgrade_levels[p][UpgradeTypes::Carrier_Capacity]=1;
   f.st.upgrade_levels[p][UpgradeTypes::Reaver_Capacity]=1;
   f.create_initial_unit(f.get_unit_type(UnitTypes::Protoss_Nexus),{p?1696:256,1920},p);
   for(unsigned j=0;j<24;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Protoss_Pylon),{int(p?1216:128)+int(j%6)*128,1408+int(j/6)*96},p);
   const UnitTypes types[]={UnitTypes::Protoss_Probe,UnitTypes::Protoss_Zealot,UnitTypes::Protoss_Dragoon,UnitTypes::Protoss_High_Templar,UnitTypes::Protoss_Dark_Templar,UnitTypes::Protoss_Shuttle,UnitTypes::Protoss_Reaver,UnitTypes::Protoss_Observer,UnitTypes::Protoss_Scout,UnitTypes::Protoss_Corsair,UnitTypes::Protoss_Carrier,UnitTypes::Protoss_Arbiter,UnitTypes::Protoss_Archon,UnitTypes::Protoss_Dark_Archon};
   const unsigned amounts[]={4,18,12,6,6,2,4,2,6,4,4,2,4,4};unsigned n=0;
   for(unsigned k=0;k<14;++k)for(unsigned j=0;j<amounts[k];++j,++n){
    const xy pos={int(p?1440:600)+(p?-1:1)*(int(n%10)-5)*44,650+int(n/10)*52};
    auto* u=f.create_unit(f.get_unit_type(types[k]),pos,p);if(!u)throw std::runtime_error("Protoss fixture allocation");f.finish_building_unit(u);
    const auto place=f.find_unit_placement(u,pos,{pos-xy(512,512),pos+xy(512,512)},true);if(!place.first)throw std::runtime_error("Protoss fixture placement");
    f.move_unit(u,place.second);if(!f.place_completed_unit(u))throw std::runtime_error("Protoss fixture completion");f.complete_unit(u);
    if(f.ut_has_energy(u))u->energy=fp8::integer(200);
    if(f.unit_is_carrier(u) || f.unit_is_reaver(u)){
     const unsigned count=f.unit_is_carrier(u)?8:10;
     for(unsigned a=0;a<count;++a){
      auto* fighter=f.create_initial_unit(f.get_unit_type(f.unit_is_carrier(u)?UnitTypes::Protoss_Interceptor:UnitTypes::Protoss_Scarab),u->sprite->position,p);
      fighter->fighter.parent=u;fighter->fighter.is_outside=false;f.hide_unit(fighter);
      if(f.unit_is_carrier(u)){u->carrier.inside_units.push_front(*fighter);++u->carrier.inside_count;}
      else{u->reaver.inside_units.push_front(*fighter);++u->reaver.inside_count;}
     }
    }
   }
  }
 }
}
inline void step_protoss(game_player& player){
 auto& f=player.funcs();auto& st=player.st();action_state as;action_functions a(st,as);
 for(unsigned p=0;p<2;++p){
  unit_t* enemy=nullptr;unit_t* ground=nullptr;
  for(auto* u:ptr(st.player_units[1-p]))if(!f.unit_dead(u) && !f.ut_building(u) && !f.unit_is_fighter(u) && !f.us_hidden(u)){if(!enemy)enemy=u;if(!f.u_flying(u))ground=u;}
  if(!enemy)continue;
  for(auto* u:ptr(st.player_units[p]))if(!f.unit_dead(u) && !f.ut_building(u) && !f.unit_is_fighter(u) && !f.us_hidden(u)){
   a.action_select(p,u);const auto type=u->unit_type->id;const auto order=u->order_type->id;
   if(st.current_frame%120==0){
    if(f.unit_is_carrier(u) || f.unit_is_reaver(u))a.action_train_fighter(p);
    if(type==UnitTypes::Protoss_High_Templar && ground)a.action_order(p,a.get_order_type(Orders::CastPsionicStorm),ground->sprite->position,nullptr,nullptr,false);
    if(type==UnitTypes::Protoss_Arbiter && ground)a.action_order(p,a.get_order_type(Orders::CastStasisField),ground->sprite->position,nullptr,nullptr,false);
    if(type==UnitTypes::Protoss_Corsair && ground)a.action_order(p,a.get_order_type(Orders::CastDisruptionWeb),ground->sprite->position,nullptr,nullptr,false);
    if(type==UnitTypes::Protoss_Dark_Archon)a.action_order(p,a.get_order_type(Orders::CastFeedback),enemy->sprite->position,enemy,nullptr,false);
   }
   if(type==UnitTypes::Protoss_Shuttle){
    if(st.current_frame==12){for(auto* passenger:ptr(st.player_units[p]))if(f.unit_is(passenger,UnitTypes::Protoss_Zealot) && !f.us_hidden(passenger)){a.action_order(p,a.get_order_type(Orders::PickupTransport),passenger->sprite->position,passenger,nullptr,false);break;}}
    else if(st.current_frame>=240 && !f.loaded_units(u).empty())a.action_unload_all(p,false);
   }else if(type==UnitTypes::Protoss_Probe || type==UnitTypes::Protoss_Observer){
    if(st.current_frame==12)a.action_order(p,a.get_order_type(Orders::Move),{1024,1150},nullptr,nullptr,false);
   }else if(order==Orders::PlayerGuard || order==Orders::Guard || order==Orders::Stop || order==u->unit_type->return_to_idle->id || st.current_frame==12){
    a.action_order(p,a.get_order_type(Orders::AttackMove),enemy->sprite->position,nullptr,nullptr,false);
   }
  }
 }
}
}}
