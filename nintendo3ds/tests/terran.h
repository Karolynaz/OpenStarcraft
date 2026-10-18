#pragma once
#include "engine.h"
#include "terran_catalogue.h"
#include <vector>
#include <functional>
#include <stdexcept>
#include <cstdio>
inline void terran_tests(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}) {
 using namespace port3ds;using namespace bwgame;Engine e;
 auto require=[&](bool ok,const char* msg){if(!ok){std::fprintf(stderr,"Terran: %s (%s)\n",msg,e.command_status());throw std::runtime_error(msg);}};
 auto tick=[&](unsigned n){while(n--){require(e.tick(),"tick");if(observe)observe(e);}};
 auto units=[&](){std::vector<UnitView> v(512);v.resize(e.visible_units(v.data(),v.size()));return v;};
 require(e.load(data,arena,Scenario::terran_lab),"load full Terran lab");tick(24);
 auto select=[&](UnitTypes type){for(auto u:units())if(u.owner==0 && u.type==unsigned(type) && u.completed && e.select(u.x,u.y))return true;return false;};
 // Every listed product must be offered by a completed legal parent and pay
 // native costs/supply. Tank and Goliath turrets are not separate products.
 for(const auto& r:terran::units) {
  bool started=false;const auto before=e.count_units(0,unsigned(r.type));
  for(auto u:units())if(u.owner==0 && u.type==unsigned(r.parent)) {
   require(e.select(u.x,u.y),"select producer");
   if(e.perform(option_code(OptionKind::train,unsigned(r.type)))){started=true;break;}
  }
  if(!started)std::fprintf(stderr,"Failed product: %s\n",r.name);
  require(started,"all fourteen products accepted");
  bool complete=false;for(unsigned n=0;n<5000;++n){tick(1);if(e.count_units(0,unsigned(r.type))>before){complete=true;break;}}
  require(complete,"normal production completes");std::printf("Produced %s\n",r.name);
 }
 for(const auto& r:terran::techs) {
  require(select(r.parent),"select research building");const auto before=e.snapshot();
  require(!e.researched(unsigned(r.type)),"lab begins without research grants");
  require(e.perform(option_code(OptionKind::research,unsigned(r.type))),"research accepted");
  const auto paid=e.snapshot();require(paid.minerals<before.minerals || paid.gas<before.gas,"research pays real resources");
  require(e.perform(option_code(OptionKind::cancel)),"cancel research");
  require(e.snapshot().minerals==before.minerals && e.snapshot().gas==before.gas,"research cancellation refund");
  require(e.perform(option_code(OptionKind::research,unsigned(r.type))),"restart research");
  for(unsigned n=0;n<6000 && !e.researched(unsigned(r.type));++n)tick(1);
  require(e.researched(unsigned(r.type)),"native research completion");std::printf("Researched %s\n",r.name);
 }
 for(const auto& r:terran::upgrades) {
  const unsigned levels=unsigned(r.type)<=unsigned(UpgradeTypes::Terran_Ship_Weapons)?3:1;
  for(unsigned level=1;level<=levels;++level){
   require(select(r.parent),"select upgrade building");require(e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"upgrade accepted");
   for(unsigned n=0;n<8000 && e.upgrade_level(unsigned(r.type))<level;++n)tick(1);
   require(e.upgrade_level(unsigned(r.type))==level,"native upgrade completion");
  }
  require(!e.perform(option_code(OptionKind::upgrade,unsigned(r.type))),"reject completed max upgrade");std::printf("Upgraded %s to %u\n",r.name,levels);
 }
 require(select(UnitTypes::Terran_Siege_Tank_Tank_Mode),"select tank");
 require(e.perform(option_code(OptionKind::siege)),"siege");tick(120);
 require(e.count_units(0,unsigned(UnitTypes::Terran_Siege_Tank_Siege_Mode))==1,"tank sieges");
 require(e.perform(option_code(OptionKind::siege)),"unsiege");tick(120);
 require(e.count_units(0,unsigned(UnitTypes::Terran_Siege_Tank_Tank_Mode))==1,"tank unsieges");
 require(select(UnitTypes::Terran_Marine) && e.perform(option_code(OptionKind::stim)),"researched stim");tick(24);
 require(select(UnitTypes::Terran_Wraith) && e.perform(option_code(OptionKind::cloak)),"researched Wraith cloak");tick(120);
 require(e.perform(option_code(OptionKind::cloak)),"decloak");tick(120);
 require(select(UnitTypes::Terran_Ghost) && e.perform(option_code(OptionKind::cloak)),"researched Ghost cloak");tick(120);
 require(e.perform(option_code(OptionKind::cloak)),"Ghost decloak");tick(120);
 auto locate=[&](UnitTypes type){for(auto u:units())if(u.owner==0 && u.type==unsigned(type))return u;throw std::runtime_error("Missing ability test unit");};
 auto cast=[&](UnitTypes type,Orders order,int x,int y){
  tick(4000);require(select(type),"select spell caster");const auto energy=e.snapshot().selected_energy;
  require(e.perform(option_code(OptionKind::order,unsigned(order)),x,y),"targeted ability accepted");
  bool spent=false;for(unsigned n=0;n<1600;++n){tick(1);if(e.snapshot().selected_energy+20<energy){spent=true;break;}}
  if(!spent)std::fprintf(stderr,"Spell failed: %u\n",unsigned(order));require(spent,"native spell spends energy at valid target");tick(160);
 };
 auto marine=locate(UnitTypes::Terran_Marine);
 require(select(UnitTypes::Terran_Dropship),"select dropship");
 require(e.perform(option_code(OptionKind::order,unsigned(Orders::PickupTransport)),marine.x,marine.y),"load marine");
 for(unsigned n=0;n<2000 && !locate(UnitTypes::Terran_Dropship).cargo;++n)tick(1);
 require(locate(UnitTypes::Terran_Dropship).cargo==1,"native cargo loaded");
 require(e.perform(option_code(OptionKind::unload)),"unload marine");tick(240);
 require(!locate(UnitTypes::Terran_Dropship).cargo,"native cargo unloaded");
 auto bunker=locate(UnitTypes::Terran_Bunker);marine=locate(UnitTypes::Terran_Marine);
 require(e.select(marine.x,marine.y) && e.activate(bunker.x,bunker.y),"contextual bunker entry");
 for(unsigned n=0;n<2000 && !locate(UnitTypes::Terran_Bunker).cargo;++n)tick(1);
 require(locate(UnitTypes::Terran_Bunker).cargo==1,"bunker holds infantry");
 require(select(UnitTypes::Terran_Bunker) && e.perform(option_code(OptionKind::unload)),"unload bunker");tick(240);
 require(!locate(UnitTypes::Terran_Bunker).cargo,"bunker releases infantry");
 require(select(UnitTypes::Terran_Marine) && e.perform(option_code(OptionKind::stim)),"wound marine with stim");
 marine=locate(UnitTypes::Terran_Marine);const int injured_hp=marine.hp;
 require(select(UnitTypes::Terran_Medic) && e.perform(option_code(OptionKind::order,unsigned(Orders::MedicHeal)),marine.x,marine.y),"Medic heal order");tick(1200);
 require(locate(UnitTypes::Terran_Marine).hp>injured_hp,"Medic restores organic health");
 require(select(UnitTypes::Terran_Barracks),"select liftoff building");
 require(e.perform(option_code(OptionKind::liftoff)),"native liftoff");tick(240);
 require(locate(UnitTypes::Terran_Barracks).flying,"building is airborne");
 require(e.perform(option_code(OptionKind::land),1400,1700),"native land order");
 for(unsigned n=0;n<3000 && locate(UnitTypes::Terran_Barracks).flying;++n)tick(1);
 require(!locate(UnitTypes::Terran_Barracks).flying,"building lands");
 auto wraith=locate(UnitTypes::Terran_Wraith);
 cast(UnitTypes::Terran_Ghost,Orders::CastLockdown,wraith.x,wraith.y);
 cast(UnitTypes::Terran_Medic,Orders::CastOpticalFlare,wraith.x,wraith.y);
 cast(UnitTypes::Terran_Medic,Orders::CastRestoration,wraith.x,wraith.y);
 auto cc=locate(UnitTypes::Terran_Command_Center);
 cast(UnitTypes::Terran_Science_Vessel,Orders::CastDefensiveMatrix,wraith.x,wraith.y);
 cast(UnitTypes::Terran_Science_Vessel,Orders::CastEMPShockwave,cc.x,cc.y);
 marine=locate(UnitTypes::Terran_Marine);
 cast(UnitTypes::Terran_Science_Vessel,Orders::CastIrradiate,marine.x,marine.y);
 cast(UnitTypes::Terran_Battlecruiser,Orders::FireYamatoGun,cc.x,cc.y);
 cast(UnitTypes::Terran_Comsat_Station,Orders::CastScannerSweep,1900,1900);
 require(select(UnitTypes::Terran_Vulture),"select mine layer");auto vulture=locate(UnitTypes::Terran_Vulture);
 require(e.perform(option_code(OptionKind::order,unsigned(Orders::PlaceMine)),vulture.x,vulture.y),"lay mine");tick(240);
 require(e.count_units(0,unsigned(UnitTypes::Terran_Vulture_Spider_Mine))>0,"native mine placed");
 require(select(UnitTypes::Terran_Ghost),"select nuclear spotter");
 require(e.count_units(0,unsigned(UnitTypes::Terran_Nuclear_Missile))==1,"silo has manufactured missile");
 require(e.perform(option_code(OptionKind::order,unsigned(Orders::NukePaint)),1800,1800),"nuclear strike order");
 for(unsigned n=0;n<2400 && e.count_units(0,unsigned(UnitTypes::Terran_Nuclear_Missile));++n)tick(1);
 require(!e.count_units(0,unsigned(UnitTypes::Terran_Nuclear_Missile)),"manufactured missile launches and detonates");
 std::puts("Full Terran production / research / all upgrade levels / cancellation / targeted spells / transport / flight / nuclear strike PASS");
}
inline void terran_battle_test(const char* data,const char* arena,const std::function<void(port3ds::Engine&)>& observe={}) {
 using namespace port3ds;using namespace bwgame;Engine e;
 if(!e.load(data,arena,Scenario::terran_battle))throw std::runtime_error(e.error());
 auto initial=e.snapshot();unsigned total=0;
 for(unsigned p=0;p<2;++p)for(unsigned k=0;k<13;++k){auto n=e.count_units(p,unsigned(terran::units[k].type));if(!n)throw std::runtime_error("Stress roster missing unit");total+=n;}
 if(total!=228 || !initial.observer || initial.players[0].army<100)throw std::runtime_error("Stress size/observer setup");
 unsigned peak=0,frame=0;for(;frame<4321;++frame){
  if(!e.tick())throw std::runtime_error(e.error());if(observe)observe(e);
  auto s=e.snapshot();peak=std::max(peak,s.units);if(s.winner>=0)break;
 }
 auto s=e.snapshot();if(s.winner<0 || frame>4320 || !s.end_reason || !s.players[0].lost || !s.players[1].lost)throw std::runtime_error("Bounded stress result / real combat");
 std::printf("Terran stress PASS: initial=228 peak=%u frame=%u winner=%d reason=%u scores=%u/%u losses=%u/%u\n",peak,s.frame,s.winner,s.end_reason,s.score[0],s.score[1],s.players[0].lost,s.players[1].lost);
 // The wall-clock cap also terminates a slow simulation in three real minutes.
 if(!e.load(data,arena,Scenario::terran_battle))throw std::runtime_error(e.error());e.advance_wall_time(180000000);
 if(e.snapshot().winner<0 || e.snapshot().end_reason!=3)throw std::runtime_error("Real-time battle cap");
 std::puts("180-second real-time cap / symmetric score tie PASS");
}
