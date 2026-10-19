#pragma once
#include "engine.h"
#include "zerg_catalogue.h"
#include <vector>
#include <string>
#include <functional>
#include <cstdio>
#include <stdexcept>
inline void zerg_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* why){if(!ok){std::fprintf(stderr,"Zerg: %s / %s\n",why,e.command_status());throw std::runtime_error(why);}};
 unsigned broodling_peak=0;bool track_broodlings=false;
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"tick");if(observe)observe(e);if(track_broodlings)broodling_peak=std::max(broodling_peak,e.count_units(0,unsigned(UnitTypes::Zerg_Broodling)));}};
 auto units=[&](){std::vector<UnitView> a(512);a.resize(e.visible_units(a.data(),a.size()));return a;};
 auto locate=[&](UnitTypes type,unsigned owner=0){for(auto u:units())if(u.type==unsigned(type) && u.owner==owner && u.completed)return u;std::fprintf(stderr,"Missing Zerg unit %u count=%u all=%u\n",unsigned(type),e.count_units(owner,unsigned(type)),e.count_units(owner,unsigned(type),false));throw std::runtime_error("Missing Zerg test unit");};
 auto selected=[&](){for(auto u:units())if(u.selected)return u;throw std::runtime_error("Selected Zerg unit missing");};
 auto select=[&](UnitTypes type){auto u=locate(type);return e.select(u.x,u.y);};
 require(e.load(data,arena,Scenario::zerg_lab),"load lab");tick(500);
 require(select(UnitTypes::Zerg_Hatchery) && e.perform(option_code(OptionKind::larvae)),"select native connected larva");
 require(e.snapshot().selected_name==std::string("Larva"),"Larva selection reflected in UI");
 MenuOption options[8];require(e.options(OptionPage::primary,options,8)==8,"first eight Larva morphs");
 require(e.options(OptionPage::secondary,options,8)>=1 && (options[0].code&65535)==unsigned(UnitTypes::Zerg_Ultralisk),"ninth Larva product available on L+Y");
 // Native research is needed before Hydralisk -> Lurker morph.
 require(select(UnitTypes::Zerg_Hydralisk_Den) && e.perform(option_code(OptionKind::research,unsigned(TechTypes::Lurker_Aspect))),"Lurker Aspect accepted");
 for(unsigned i=0;i<5000 && !e.researched(unsigned(TechTypes::Lurker_Aspect));++i)tick(1);require(e.researched(unsigned(TechTypes::Lurker_Aspect)),"Lurker Aspect complete");
 for(const auto& r:zerg::units){
  if(r.type==UnitTypes::Zerg_Devourer){require(select(UnitTypes::Zerg_Larva) && e.perform(option_code(OptionKind::morph,unsigned(UnitTypes::Zerg_Mutalisk))),"replace consumed Mutalisk");tick(2000);}
  require(select(r.parent),"select morph producer");auto before=e.count_units(0,unsigned(r.type));auto money=e.snapshot();
  const auto kind=r.parent==UnitTypes::Zerg_Infested_Command_Center?OptionKind::train:OptionKind::morph;
  require(e.perform(option_code(kind,unsigned(r.type))),"native production accepted");
  for(unsigned n=0;n<5000 && e.count_units(0,unsigned(r.type))==before;++n)tick(1);
  const unsigned expected=(r.type==UnitTypes::Zerg_Zergling || r.type==UnitTypes::Zerg_Scourge)?2:1;
  require(e.count_units(0,unsigned(r.type))==before+expected,"native production/pair completed");require(e.snapshot().minerals<money.minerals,"native production charges resources");std::printf("Zerg produced %s\n",r.name);
 }
 for(const auto& r:zerg::techs){if(e.researched(unsigned(r.type)))continue;
  require(select(r.parent),"research producer");const auto before=e.snapshot();
  require(e.perform(option_code(OptionKind::research,unsigned(r.type))),"research accepted");tick(12);
  require(e.perform(option_code(OptionKind::cancel)),"research cancel");require(e.snapshot().minerals==before.minerals && e.snapshot().gas==before.gas,"research refund");
  require(e.perform(option_code(OptionKind::research,unsigned(r.type))),"research restart");for(unsigned n=0;n<5000 && !e.researched(unsigned(r.type));++n)tick(1);require(e.researched(unsigned(r.type)),"research complete");
 }
 for(unsigned i=0;i<sizeof(zerg::upgrades)/sizeof(*zerg::upgrades);++i){const auto& r=zerg::upgrades[i];require(select(r.parent),"upgrade producer");const unsigned levels=i<5?3:1;
  for(unsigned l=1;l<=levels;++l){require(e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"upgrade accepted");for(unsigned n=0;n<7000 && e.upgrade_level(unsigned(r.type))<l;++n)tick(1);require(e.upgrade_level(unsigned(r.type))==l,"upgrade complete");}
  require(!e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"maximum upgrade rejected");std::printf("Zerg upgraded %s\n",r.name);
 }
 // Cancel a normal Egg and a Lurker Egg through the public command menu.
 require(select(UnitTypes::Zerg_Larva),"Larva for cancellation");const auto cancel_resources=e.snapshot();const auto drone_before=e.count_units(0,unsigned(UnitTypes::Zerg_Drone));
 require(e.perform(option_code(OptionKind::morph,unsigned(UnitTypes::Zerg_Drone))),"Drone Egg morph");tick(12);
 require(e.perform(option_code(OptionKind::cancel)),"cancel Egg");tick(24);require(e.snapshot().minerals==cancel_resources.minerals && e.snapshot().gas==cancel_resources.gas && e.count_units(0,unsigned(UnitTypes::Zerg_Drone))==drone_before,"Egg refund without completed product");
 require(select(UnitTypes::Zerg_Larva) && e.perform(option_code(OptionKind::morph,unsigned(UnitTypes::Zerg_Hydralisk))),"Hydralisk for cancel test");tick(2000);
 require(select(UnitTypes::Zerg_Hydralisk),"select Hydralisk for Lurker cancel");const auto hydras=e.count_units(0,unsigned(UnitTypes::Zerg_Hydralisk));const auto lurkers=e.count_units(0,unsigned(UnitTypes::Zerg_Lurker));const auto lurker_money=e.snapshot();
 require(e.perform(option_code(OptionKind::morph,unsigned(UnitTypes::Zerg_Lurker))),"Lurker morph accepted");tick(30);require(e.perform(option_code(OptionKind::cancel)),"cancel Lurker Egg");tick(160);
 require(e.count_units(0,unsigned(UnitTypes::Zerg_Hydralisk))==hydras && e.count_units(0,unsigned(UnitTypes::Zerg_Lurker))==lurkers && e.snapshot().minerals==lurker_money.minerals && e.snapshot().gas==lurker_money.gas,"Lurker cancellation restores Hydralisk and costs");
 // Zergling and Lurker use the actual Burrowing orders.
 for(auto type:{UnitTypes::Zerg_Zergling,UnitTypes::Zerg_Lurker}){
  require(select(type) && e.perform(option_code(OptionKind::burrow)),"native burrow");tick(100);require(selected().burrowed,"burrowed native state");
  require(e.perform(option_code(OptionKind::burrow)),"native unburrow");tick(100);require(!selected().burrowed,"unburrowed");
 }
 auto passenger=locate(UnitTypes::Zerg_Drone);require(select(UnitTypes::Zerg_Overlord) && e.perform(option_code(OptionKind::order,unsigned(Orders::PickupTransport)),passenger.x,passenger.y),"Overlord pickup");tick(1800);require(selected().cargo==1,"Ventral Sacs passenger");require(e.perform(option_code(OptionKind::unload)),"Overlord unload");tick(300);require(selected().cargo==0,"Overlord empty");
 auto cast=[&](UnitTypes type,Orders order,int x,int y,bool targeted){
  tick(5000);require(select(type),"select Zerg caster");if(targeted){require(e.move_selected(x-230,y),"reveal native spell target");tick(700);require(select(type),"reselect Zerg caster");}
  const auto energy=e.snapshot().selected_energy;require(e.perform(option_code(OptionKind::order,unsigned(order)),x,y),"spell order accepted");bool spent=false;
  for(unsigned n=0;n<2400;++n){tick(1);if(e.snapshot().selected_energy+15<energy){spent=true;break;}}require(spent,"native spell energy spent");tick(100);
 };
 auto cc=locate(UnitTypes::Terran_Command_Center,1);require(select(UnitTypes::Zerg_Queen) && e.perform(option_code(OptionKind::order,unsigned(Orders::CastInfestation)),cc.x,cc.y),"native infestation order");
 for(unsigned i=0;i<3000 && e.count_units(1,unsigned(UnitTypes::Terran_Command_Center));++i)tick(1);require(!e.count_units(1,unsigned(UnitTypes::Terran_Command_Center)) && e.count_units(0,unsigned(UnitTypes::Zerg_Infested_Command_Center))==2,"native command center infestation");
 auto marine=locate(UnitTypes::Terran_Marine,1);cast(UnitTypes::Zerg_Queen,Orders::CastParasite,marine.x,marine.y,true);
 track_broodlings=true;cast(UnitTypes::Zerg_Queen,Orders::CastSpawnBroodlings,marine.x,marine.y,true);
 require(e.count_units(1,unsigned(UnitTypes::Terran_Marine))==0 && broodling_peak>=2,"Broodlings replace biological target");
 cast(UnitTypes::Zerg_Queen,Orders::CastEnsnare,1800,1650,false);
 cast(UnitTypes::Zerg_Defiler,Orders::CastDarkSwarm,1700,1800,false);require(e.count_units(11,unsigned(UnitTypes::Spell_Dark_Swarm)) || e.count_units(0,unsigned(UnitTypes::Spell_Dark_Swarm)),"native Dark Swarm object");
 cast(UnitTypes::Zerg_Defiler,Orders::CastPlague,1850,1550,false);
 passenger=locate(UnitTypes::Zerg_Drone);const auto drones=e.count_units(0,unsigned(UnitTypes::Zerg_Drone));
 require(select(UnitTypes::Zerg_Defiler) && e.perform(option_code(OptionKind::order,unsigned(Orders::CastConsume)),passenger.x,passenger.y),"Consume order");
 for(unsigned i=0;i<2500 && e.count_units(0,unsigned(UnitTypes::Zerg_Drone))==drones;++i)tick(1);require(e.count_units(0,unsigned(UnitTypes::Zerg_Drone))==drones-1,"Consume sacrifices own biological unit");
 std::puts("Zerg production / pairs / research / all upgrade levels / burrow / transport / spells / infestation PASS");
}
inline void zerg_battle_test(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;if(!e.load(data,arena,Scenario::zerg_battle))throw std::runtime_error(e.error());auto initial=e.snapshot();
 if(initial.supply!=200 || initial.supply_max!=200 || initial.players[1].supply_max!=200 || !initial.observer)throw std::runtime_error("Zerg 200 supply fixture");
 unsigned peak=initial.units;for(unsigned i=0;i<4321;++i){if(!e.tick())throw std::runtime_error(e.error());if(observe)observe(e);auto s=e.snapshot();peak=std::max(peak,s.units);if(s.winner>=0)break;}
 auto s=e.snapshot();if(s.winner<0 || !s.end_reason || !s.players[0].lost || !s.players[1].lost)throw std::runtime_error("Zerg bounded battle");
 std::printf("Zerg stress PASS: peak=%u frame=%u winner=%d scores=%u/%u losses=%u/%u\n",peak,s.frame,s.winner,s.score[0],s.score[1],s.players[0].lost,s.players[1].lost);
}
