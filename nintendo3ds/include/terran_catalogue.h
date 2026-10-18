#pragma once
#include "bwenums.h"
namespace port3ds { namespace terran {
using namespace bwgame;
struct Product {UnitTypes type,parent;const char* name;};
static constexpr Product buildings[]={
 {UnitTypes::Terran_Command_Center,UnitTypes::Terran_SCV,"Command Center"},
 {UnitTypes::Terran_Supply_Depot,UnitTypes::Terran_SCV,"Supply Depot"},
 {UnitTypes::Terran_Refinery,UnitTypes::Terran_SCV,"Refinery"},
 {UnitTypes::Terran_Barracks,UnitTypes::Terran_SCV,"Barracks"},
 {UnitTypes::Terran_Academy,UnitTypes::Terran_SCV,"Academy"},
 {UnitTypes::Terran_Engineering_Bay,UnitTypes::Terran_SCV,"Engineering Bay"},
 {UnitTypes::Terran_Bunker,UnitTypes::Terran_SCV,"Bunker"},
 {UnitTypes::Terran_Missile_Turret,UnitTypes::Terran_SCV,"Missile Turret"},
 {UnitTypes::Terran_Factory,UnitTypes::Terran_SCV,"Factory"},
 {UnitTypes::Terran_Starport,UnitTypes::Terran_SCV,"Starport"},
 {UnitTypes::Terran_Science_Facility,UnitTypes::Terran_SCV,"Science Facility"},
 {UnitTypes::Terran_Armory,UnitTypes::Terran_SCV,"Armory"},
 {UnitTypes::Terran_Comsat_Station,UnitTypes::Terran_Command_Center,"Comsat Station"},
 {UnitTypes::Terran_Nuclear_Silo,UnitTypes::Terran_Command_Center,"Nuclear Silo"},
 {UnitTypes::Terran_Machine_Shop,UnitTypes::Terran_Factory,"Machine Shop"},
 {UnitTypes::Terran_Control_Tower,UnitTypes::Terran_Starport,"Control Tower"},
 {UnitTypes::Terran_Covert_Ops,UnitTypes::Terran_Science_Facility,"Covert Ops"},
 {UnitTypes::Terran_Physics_Lab,UnitTypes::Terran_Science_Facility,"Physics Lab"}
};
static constexpr Product units[]={
 {UnitTypes::Terran_SCV,UnitTypes::Terran_Command_Center,"SCV"},
 {UnitTypes::Terran_Marine,UnitTypes::Terran_Barracks,"Marine"},
 {UnitTypes::Terran_Firebat,UnitTypes::Terran_Barracks,"Firebat"},
 {UnitTypes::Terran_Medic,UnitTypes::Terran_Barracks,"Medic"},
 {UnitTypes::Terran_Ghost,UnitTypes::Terran_Barracks,"Ghost"},
 {UnitTypes::Terran_Vulture,UnitTypes::Terran_Factory,"Vulture"},
 {UnitTypes::Terran_Siege_Tank_Tank_Mode,UnitTypes::Terran_Factory,"Siege Tank"},
 {UnitTypes::Terran_Goliath,UnitTypes::Terran_Factory,"Goliath"},
 {UnitTypes::Terran_Wraith,UnitTypes::Terran_Starport,"Wraith"},
 {UnitTypes::Terran_Valkyrie,UnitTypes::Terran_Starport,"Valkyrie"},
 {UnitTypes::Terran_Dropship,UnitTypes::Terran_Starport,"Dropship"},
 {UnitTypes::Terran_Science_Vessel,UnitTypes::Terran_Starport,"Science Vessel"},
 {UnitTypes::Terran_Battlecruiser,UnitTypes::Terran_Starport,"Battlecruiser"},
 {UnitTypes::Terran_Nuclear_Missile,UnitTypes::Terran_Nuclear_Silo,"Nuclear Missile"}
};
struct Research {TechTypes type;UnitTypes parent;const char* name;};
static constexpr Research techs[]={
 {TechTypes::Stim_Packs,UnitTypes::Terran_Academy,"Stim Packs"},
 {TechTypes::Restoration,UnitTypes::Terran_Academy,"Restoration"},
 {TechTypes::Optical_Flare,UnitTypes::Terran_Academy,"Optical Flare"},
 {TechTypes::Spider_Mines,UnitTypes::Terran_Machine_Shop,"Spider Mines"},
 {TechTypes::Tank_Siege_Mode,UnitTypes::Terran_Machine_Shop,"Siege Mode"},
 {TechTypes::Cloaking_Field,UnitTypes::Terran_Control_Tower,"Wraith Cloaking"},
 {TechTypes::EMP_Shockwave,UnitTypes::Terran_Science_Facility,"EMP Shockwave"},
 {TechTypes::Irradiate,UnitTypes::Terran_Science_Facility,"Irradiate"},
 {TechTypes::Lockdown,UnitTypes::Terran_Covert_Ops,"Lockdown"},
 {TechTypes::Personnel_Cloaking,UnitTypes::Terran_Covert_Ops,"Ghost Cloaking"},
 {TechTypes::Yamato_Gun,UnitTypes::Terran_Physics_Lab,"Yamato Gun"}
};
struct Upgrade {UpgradeTypes type;UnitTypes parent;const char* name;};
static constexpr Upgrade upgrades[]={
 {UpgradeTypes::Terran_Infantry_Weapons,UnitTypes::Terran_Engineering_Bay,"Infantry Weapons"},
 {UpgradeTypes::Terran_Infantry_Armor,UnitTypes::Terran_Engineering_Bay,"Infantry Armor"},
 {UpgradeTypes::Terran_Vehicle_Weapons,UnitTypes::Terran_Armory,"Vehicle Weapons"},
 {UpgradeTypes::Terran_Vehicle_Plating,UnitTypes::Terran_Armory,"Vehicle Armor"},
 {UpgradeTypes::Terran_Ship_Weapons,UnitTypes::Terran_Armory,"Ship Weapons"},
 {UpgradeTypes::Terran_Ship_Plating,UnitTypes::Terran_Armory,"Ship Armor"},
 {UpgradeTypes::U_238_Shells,UnitTypes::Terran_Academy,"U-238 Shells"},
 {UpgradeTypes::Caduceus_Reactor,UnitTypes::Terran_Academy,"Caduceus Reactor"},
 {UpgradeTypes::Ion_Thrusters,UnitTypes::Terran_Machine_Shop,"Ion Thrusters"},
 {UpgradeTypes::Charon_Boosters,UnitTypes::Terran_Machine_Shop,"Charon Boosters"},
 {UpgradeTypes::Apollo_Reactor,UnitTypes::Terran_Control_Tower,"Apollo Reactor"},
 {UpgradeTypes::Titan_Reactor,UnitTypes::Terran_Science_Facility,"Titan Reactor"},
 {UpgradeTypes::Ocular_Implants,UnitTypes::Terran_Covert_Ops,"Ocular Implants"},
 {UpgradeTypes::Moebius_Reactor,UnitTypes::Terran_Covert_Ops,"Moebius Reactor"},
 {UpgradeTypes::Colossus_Reactor,UnitTypes::Terran_Physics_Lab,"Colossus Reactor"}
};
inline const char* name(UnitTypes id) {
 for(const auto& x:buildings) if(x.type==id) return x.name;
 for(const auto& x:units) if(x.type==id) return x.name;
 if(id==UnitTypes::Terran_Siege_Tank_Siege_Mode) return "Tank (Sieged)";
 if(id==UnitTypes::Terran_Vulture_Spider_Mine) return "Spider Mine";
 return "Unit";
}
} }
