#pragma once
#include "actions.h"
#include "zerg_catalogue.h"
namespace port3ds {namespace detail {
using namespace bwgame;
inline void setup_zerg(game_load_functions& f,bool lab){
 for(unsigned p=0;p<(lab?1u:2u);++p){
  f.st.current_minerals[p]=20000;f.st.current_gas[p]=20000;
  if(lab){
   unsigned i=0;for(const auto& b:zerg::buildings)if(b.type!=UnitTypes::Zerg_Extractor){
    f.create_initial_unit(f.get_unit_type(b.type),{192+int(i%5)*320,192+int(i/5)*320},p);++i;
   }
   f.create_initial_unit(f.get_unit_type(UnitTypes::Zerg_Infested_Command_Center),{1792,1152},p);
   for(unsigned j=0;j<24;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Zerg_Overlord),{256+int(j%8)*96,1350+int(j/8)*64},p);
   for(unsigned j=0;j<16;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Zerg_Drone),{256+int(j)*36,1600},p);
   f.create_initial_unit(f.get_unit_type(UnitTypes::Resource_Vespene_Geyser),{600,1800},11);
   auto* target=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Command_Center),{1792,1792},1);target->hp=fp8::integer(650);
   for(unsigned j=0;j<3;++j){auto* e=f.create_initial_unit(f.get_unit_type(j==0?UnitTypes::Terran_Marine:UnitTypes::Terran_Siege_Tank_Tank_Mode),{1850,1400+int(j)*120},1);f.set_unit_order(e,f.get_order_type(Orders::HoldPosition));}
  }else{
   for(const auto& t:zerg::techs)f.st.tech_researched[p][t.type]=true;
   f.st.upgrade_levels[p][UpgradeTypes::Ventral_Sacs]=1;
   f.create_initial_unit(f.get_unit_type(UnitTypes::Zerg_Hatchery),{p?1728:256,1920},p);
   for(unsigned j=0;j<25;++j)f.create_initial_unit(f.get_unit_type(UnitTypes::Zerg_Overlord),{int(p?1408:128)+int(j%6)*64,1408+int(j/6)*64},p);
   const UnitTypes types[]={UnitTypes::Zerg_Drone,UnitTypes::Zerg_Zergling,UnitTypes::Zerg_Hydralisk,UnitTypes::Zerg_Mutalisk,UnitTypes::Zerg_Scourge,UnitTypes::Zerg_Queen,UnitTypes::Zerg_Defiler,UnitTypes::Zerg_Ultralisk,UnitTypes::Zerg_Guardian,UnitTypes::Zerg_Devourer,UnitTypes::Zerg_Lurker,UnitTypes::Zerg_Infested_Terran};
   const unsigned amounts[]={4,40,20,16,12,4,4,10,6,6,16,6};unsigned n=0;
   for(unsigned k=0;k<12;++k)for(unsigned j=0;j<amounts[k];++j,++n){
    const xy pos={int(p?1500:550)+(p?-1:1)*(int(n%12)-6)*44,450+int(n/12)*52};
    auto* u=f.create_unit(f.get_unit_type(types[k]),pos,p);if(!u)throw std::runtime_error("Zerg fixture allocation");f.finish_building_unit(u);
    const auto place=f.find_unit_placement(u,pos,{pos-xy(512,512),pos+xy(512,512)},true);if(!place.first)throw std::runtime_error("Zerg fixture placement");
    f.move_unit(u,place.second);if(!f.place_completed_unit(u))throw std::runtime_error("Zerg fixture completion");f.complete_unit(u);
    if(f.ut_has_energy(u))u->energy=fp8::integer(200);
   }
  }
 }
}
inline void step_zerg(game_player& player){
 auto& f=player.funcs();auto& st=player.st();action_state as;action_functions a(st,as);
 for(unsigned p=0;p<2;++p){unit_t* enemy=nullptr;unit_t* ground=nullptr;
  for(auto* u:ptr(st.player_units[1-p]))if(!f.unit_dead(u) && !f.ut_building(u) && !f.us_hidden(u) && !f.unit_is(u,UnitTypes::Zerg_Larva)){if(!enemy)enemy=u;if(!f.u_flying(u))ground=u;}
  if(!enemy)continue;
  for(auto* u:ptr(st.player_units[p]))if(!f.unit_dead(u) && !f.ut_building(u) && !f.us_hidden(u)){
   const auto type=u->unit_type->id;if(type==UnitTypes::Zerg_Larva || type==UnitTypes::Zerg_Egg)continue;a.action_select(p,u);
   if(type==UnitTypes::Zerg_Lurker){if(st.current_frame>=240 && !f.u_burrowed(u))a.action_burrow(p,false);if(f.u_burrowed(u) || st.current_frame>=240)continue;}
   if(st.current_frame%120==0){
    if(type==UnitTypes::Zerg_Queen && ground)a.action_order(p,a.get_order_type(Orders::CastEnsnare),ground->sprite->position,nullptr,nullptr,false);
    if(type==UnitTypes::Zerg_Defiler && ground)a.action_order(p,a.get_order_type(st.current_frame%240?Orders::CastDarkSwarm:Orders::CastPlague),ground->sprite->position,nullptr,nullptr,false);
   }
   if(type==UnitTypes::Zerg_Overlord || type==UnitTypes::Zerg_Drone){if(st.current_frame==12)a.action_order(p,a.get_order_type(Orders::Move),{1024,1250},nullptr,nullptr,false);continue;}
   const auto order=u->order_type->id;
   if(st.current_frame==12 || order==Orders::PlayerGuard || order==Orders::Guard || order==Orders::Stop || order==u->unit_type->return_to_idle->id)a.action_order(p,a.get_order_type(Orders::AttackMove),enemy->sprite->position,nullptr,nullptr,false);
  }
 }
}
}}
