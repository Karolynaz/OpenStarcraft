#pragma once
#include "engine.h"
#include "protoss_catalogue.h"
#include <vector>
#include <functional>
#include <cstdio>
#include <stdexcept>
inline void protoss_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* why){if(!ok){std::fprintf(stderr,"Protoss: %s / %s\n",why,e.command_status());throw std::runtime_error(why);}};
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"tick");if(observe)observe(e);}};
 auto units=[&](){std::vector<UnitView> a(512);a.resize(e.visible_units(a.data(),a.size()));return a;};
 auto locate=[&](UnitTypes type,unsigned owner=0){for(auto u:units())if(u.type==unsigned(type) && u.owner==owner && u.completed)return u;throw std::runtime_error("Missing Protoss test unit");};
 auto select=[&](UnitTypes type){auto u=locate(type);return e.select(u.x,u.y);};
 require(e.load(data,arena,Scenario::protoss_lab),"load lab");tick(24);
 auto patient=locate(UnitTypes::Protoss_Zealot),battery=locate(UnitTypes::Protoss_Shield_Battery);
 require(patient.shields<patient.max_shields,"prepared shield patient");
 require(e.select(patient.x,patient.y) && e.activate(battery.x,battery.y),"contextual battery recharge");tick(500);
 require(locate(UnitTypes::Protoss_Zealot).shields==patient.max_shields,"battery restores shields");
 for(const auto& r:protoss::units){
  require(select(r.parent),"select producer");const auto before=e.count_units(0,unsigned(r.type));
  require(e.perform(option_code(OptionKind::train,unsigned(r.type))),"native production accepted");
  for(unsigned n=0;n<5000 && e.count_units(0,unsigned(r.type))==before;++n)tick(1);
  require(e.count_units(0,unsigned(r.type))>before,"production completed");std::printf("Protoss produced %s\n",r.name);
 }
 for(const auto& r:protoss::techs){
  require(select(r.parent),"select research building");const auto before=e.snapshot();
  require(!e.researched(unsigned(r.type)) && e.perform(option_code(OptionKind::research,unsigned(r.type))),"research starts without grants");
  require(e.perform(option_code(OptionKind::cancel)),"cancel research");
  require(e.snapshot().minerals==before.minerals && e.snapshot().gas==before.gas,"research refund");
  require(e.perform(option_code(OptionKind::research,unsigned(r.type))),"restart research");
  for(unsigned n=0;n<6000 && !e.researched(unsigned(r.type));++n)tick(1);
  require(e.researched(unsigned(r.type)),"research completes");std::printf("Protoss researched %s\n",r.name);
 }
 for(unsigned i=0;i<19;++i){const auto& r=protoss::upgrades[i];const unsigned max=i<5?3:1;
  for(unsigned level=1;level<=max;++level){
   require(select(r.parent),"select upgrade building");require(e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"upgrade accepted");
   for(unsigned n=0;n<8000 && e.upgrade_level(unsigned(r.type))<level;++n)tick(1);
   require(e.upgrade_level(unsigned(r.type))==level,"upgrade completes");
  }
  require(!e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"maximum upgrade rejected");std::printf("Protoss upgraded %s to %u\n",r.name,max);
 }
 auto passenger=locate(UnitTypes::Protoss_Zealot);require(select(UnitTypes::Protoss_Shuttle),"select Shuttle");
 require(e.perform(option_code(OptionKind::order,unsigned(Orders::PickupTransport)),passenger.x,passenger.y),"Shuttle load order");
 for(unsigned n=0;n<2400 && !locate(UnitTypes::Protoss_Shuttle).cargo;++n)tick(1);
 require(locate(UnitTypes::Protoss_Shuttle).cargo==1,"Shuttle carries Zealot");
 require(e.perform(option_code(OptionKind::unload)),"Shuttle unload");tick(240);require(!locate(UnitTypes::Protoss_Shuttle).cargo,"Shuttle unload completes");
 for(auto type:{UnitTypes::Protoss_Carrier,UnitTypes::Protoss_Reaver}){
  const auto fighter=type==UnitTypes::Protoss_Carrier?UnitTypes::Protoss_Interceptor:UnitTypes::Protoss_Scarab;const unsigned capacity=type==UnitTypes::Protoss_Carrier?8:10;
  require(select(type),"select fighter producer");
  for(unsigned i=locate(type).fighters;i<capacity;++i){require(e.perform(option_code(OptionKind::train,unsigned(fighter))),"train fighter to upgraded capacity");for(unsigned n=0;n<2000 && locate(type).fighters<=i;++n)tick(1);require(locate(type).fighters==i+1,"fighter joins native parent");}
  require(!e.perform(option_code(OptionKind::train,unsigned(fighter))),"native fighter capacity enforced");
 }
 for(auto type:{UnitTypes::Protoss_High_Templar,UnitTypes::Protoss_Dark_Templar}){
  require(select(UnitTypes::Protoss_Gateway),"select gateway");auto before=e.count_units(0,unsigned(type));
  require(e.perform(option_code(OptionKind::train,unsigned(type))),"produce merge partner");
  for(unsigned n=0;n<5000 && e.count_units(0,unsigned(type))==before;++n)tick(1);
  require(e.count_units(0,unsigned(type))>before,"merge partner completed");
  unsigned partner=0;for(auto unit:units())if(unit.type==unsigned(type) && unit.owner==0){require(e.select(unit.x,unit.y) && e.move_selected(1000+int(partner++)*64,1200),"move Templar into clear merge area");}
  tick(1200);
  require(select(type) && e.perform(option_code(OptionKind::merge,type==UnitTypes::Protoss_Dark_Templar)),"native Templar merge");tick(1200);

  require(e.count_units(0,unsigned(type==UnitTypes::Protoss_Dark_Templar?UnitTypes::Protoss_Dark_Archon:UnitTypes::Protoss_Archon))==1,"Archon transformation");
 }
 require(select(UnitTypes::Protoss_Gateway) && e.perform(option_code(OptionKind::train,unsigned(UnitTypes::Protoss_High_Templar))),"new spell caster");tick(1000);
 auto cast=[&](UnitTypes type,Orders order,int x,int y){tick(5000);require(select(type),"select caster");
  // Targeted enemy spells require native sight; the observer drawing both
  // armies does not disable combat visibility/detection rules.
  if(order==Orders::CastFeedback || order==Orders::CastMindControl){require(e.move_selected(x-280,y),"reveal spell target");tick(1000);require(select(type),"reselect caster");}
  const auto energy=e.snapshot().selected_energy;
  require(e.perform(option_code(OptionKind::order,unsigned(order)),x,y),"spell order accepted");bool spent=false;
  for(unsigned n=0;n<2400;++n){tick(1);if(e.snapshot().selected_energy+20<energy){spent=true;break;}}
  if(!spent){std::fprintf(stderr,"Protoss spell did not fire: %u initial energy %d selected %s\n",unsigned(order),energy,e.snapshot().selected_name);for(auto v:units())if(v.type==unsigned(type) || v.type==unsigned(UnitTypes::Terran_Ghost))std::fprintf(stderr,"Caster/target type %u owner %u pos %d,%d energy %d order %u\n",v.type,v.owner,v.x,v.y,v.energy,v.order);}require(spent,"native spell spends energy");tick(160);
 };
 auto dragoon=locate(UnitTypes::Protoss_Dragoon);auto before=e.count_units(0,unsigned(UnitTypes::Protoss_Dragoon));
 cast(UnitTypes::Protoss_High_Templar,Orders::CastHallucination,dragoon.x,dragoon.y);
 require(e.count_units(0,unsigned(UnitTypes::Protoss_Dragoon))>=before+2,"two hallucinations created");
 cast(UnitTypes::Protoss_High_Templar,Orders::CastPsionicStorm,1700,1800);
 auto ghost=locate(UnitTypes::Terran_Ghost,1);cast(UnitTypes::Protoss_Dark_Archon,Orders::CastFeedback,ghost.x,ghost.y);
 require(!e.count_units(1,unsigned(UnitTypes::Terran_Ghost)),"Feedback damages energized target");
 auto target=locate(UnitTypes::Terran_Battlecruiser,1);cast(UnitTypes::Protoss_Dark_Archon,Orders::CastMindControl,target.x,target.y);
 require(e.count_units(0,unsigned(UnitTypes::Terran_Battlecruiser))==1,"Mind Control transfers ownership");
 cast(UnitTypes::Protoss_Dark_Archon,Orders::CastMaelstrom,1850,1750);
 cast(UnitTypes::Protoss_Arbiter,Orders::CastStasisField,1800,1900);
 cast(UnitTypes::Protoss_Corsair,Orders::CastDisruptionWeb,1700,1900);
 cast(UnitTypes::Protoss_Arbiter,Orders::CastRecall,256,1600);
 std::puts("Protoss production / seven techs / nineteen upgrades / fighters / merges / spells / shield battery PASS");
}
inline void protoss_battle_test(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}){
 using namespace port3ds;using namespace bwgame;Engine e;if(!e.load(data,arena,Scenario::protoss_battle))throw std::runtime_error(e.error());
 auto initial=e.snapshot();if(initial.supply!=200 || !initial.observer)throw std::runtime_error("Protoss 200 supply fixture");
 for(unsigned p=0;p<2;++p){for(const auto& r:protoss::units)if(!e.count_units(p,unsigned(r.type)))throw std::runtime_error("Protoss roster missing product");if(!e.count_units(p,unsigned(UnitTypes::Protoss_Archon)) || !e.count_units(p,unsigned(UnitTypes::Protoss_Dark_Archon)))throw std::runtime_error("Protoss merge roster missing");}
 unsigned peak=initial.units;for(unsigned n=0;n<4321;++n){if(!e.tick())throw std::runtime_error(e.error());if(observe)observe(e);auto s=e.snapshot();peak=std::max(peak,s.units);if(s.winner>=0)break;}
 auto s=e.snapshot();if(s.winner<0 || !s.end_reason || !s.players[0].lost || !s.players[1].lost)throw std::runtime_error("Protoss bounded real combat");
 std::printf("Protoss stress PASS: peak=%u frame=%u winner=%d scores=%u/%u losses=%u/%u\n",peak,s.frame,s.winner,s.score[0],s.score[1],s.players[0].lost,s.players[1].lost);
}
