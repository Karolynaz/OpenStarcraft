#pragma once
#include "terran_catalogue.h"
namespace port3ds {namespace protoss {
using namespace bwgame;using terran::Product;using terran::Research;using terran::Upgrade;
static constexpr Product buildings[]={
 {UnitTypes::Protoss_Nexus,UnitTypes::Protoss_Probe,"Nexus"},
 {UnitTypes::Protoss_Pylon,UnitTypes::Protoss_Probe,"Pylon"},
 {UnitTypes::Protoss_Assimilator,UnitTypes::Protoss_Probe,"Assimilator"},
 {UnitTypes::Protoss_Gateway,UnitTypes::Protoss_Probe,"Gateway"},
 {UnitTypes::Protoss_Forge,UnitTypes::Protoss_Probe,"Forge"},
 {UnitTypes::Protoss_Photon_Cannon,UnitTypes::Protoss_Probe,"Photon Cannon"},
 {UnitTypes::Protoss_Shield_Battery,UnitTypes::Protoss_Probe,"Shield Battery"},
 {UnitTypes::Protoss_Cybernetics_Core,UnitTypes::Protoss_Probe,"Cybernetics Core"},
 {UnitTypes::Protoss_Robotics_Facility,UnitTypes::Protoss_Probe,"Robotics Facility"},
 {UnitTypes::Protoss_Stargate,UnitTypes::Protoss_Probe,"Stargate"},
 {UnitTypes::Protoss_Citadel_of_Adun,UnitTypes::Protoss_Probe,"Citadel of Adun"},
 {UnitTypes::Protoss_Robotics_Support_Bay,UnitTypes::Protoss_Probe,"Robotics Support Bay"},
 {UnitTypes::Protoss_Fleet_Beacon,UnitTypes::Protoss_Probe,"Fleet Beacon"},
 {UnitTypes::Protoss_Templar_Archives,UnitTypes::Protoss_Probe,"Templar Archives"},
 {UnitTypes::Protoss_Observatory,UnitTypes::Protoss_Probe,"Observatory"},
 {UnitTypes::Protoss_Arbiter_Tribunal,UnitTypes::Protoss_Probe,"Arbiter Tribunal"}
};
static constexpr Product units[]={
 {UnitTypes::Protoss_Probe,UnitTypes::Protoss_Nexus,"Probe"},
 {UnitTypes::Protoss_Zealot,UnitTypes::Protoss_Gateway,"Zealot"},
 {UnitTypes::Protoss_Dragoon,UnitTypes::Protoss_Gateway,"Dragoon"},
 {UnitTypes::Protoss_High_Templar,UnitTypes::Protoss_Gateway,"High Templar"},
 {UnitTypes::Protoss_Dark_Templar,UnitTypes::Protoss_Gateway,"Dark Templar"},
 {UnitTypes::Protoss_Shuttle,UnitTypes::Protoss_Robotics_Facility,"Shuttle"},
 {UnitTypes::Protoss_Reaver,UnitTypes::Protoss_Robotics_Facility,"Reaver"},
 {UnitTypes::Protoss_Observer,UnitTypes::Protoss_Robotics_Facility,"Observer"},
 {UnitTypes::Protoss_Scout,UnitTypes::Protoss_Stargate,"Scout"},
 {UnitTypes::Protoss_Corsair,UnitTypes::Protoss_Stargate,"Corsair"},
 {UnitTypes::Protoss_Carrier,UnitTypes::Protoss_Stargate,"Carrier"},
 {UnitTypes::Protoss_Arbiter,UnitTypes::Protoss_Stargate,"Arbiter"},
 {UnitTypes::Protoss_Interceptor,UnitTypes::Protoss_Carrier,"Interceptor"},
 {UnitTypes::Protoss_Scarab,UnitTypes::Protoss_Reaver,"Scarab"}
};
static constexpr Research techs[]={
 {TechTypes::Psionic_Storm,UnitTypes::Protoss_Templar_Archives,"Psionic Storm"},
 {TechTypes::Hallucination,UnitTypes::Protoss_Templar_Archives,"Hallucination"},
 {TechTypes::Recall,UnitTypes::Protoss_Arbiter_Tribunal,"Recall"},
 {TechTypes::Stasis_Field,UnitTypes::Protoss_Arbiter_Tribunal,"Stasis Field"},
 {TechTypes::Disruption_Web,UnitTypes::Protoss_Fleet_Beacon,"Disruption Web"},
 {TechTypes::Mind_Control,UnitTypes::Protoss_Templar_Archives,"Mind Control"},
 {TechTypes::Maelstrom,UnitTypes::Protoss_Templar_Archives,"Maelstrom"}
};
static constexpr Upgrade upgrades[]={
 {UpgradeTypes::Protoss_Ground_Weapons,UnitTypes::Protoss_Forge,"Ground Weapons"},
 {UpgradeTypes::Protoss_Ground_Armor,UnitTypes::Protoss_Forge,"Ground Armor"},
 {UpgradeTypes::Protoss_Plasma_Shields,UnitTypes::Protoss_Forge,"Plasma Shields"},
 {UpgradeTypes::Protoss_Air_Weapons,UnitTypes::Protoss_Cybernetics_Core,"Air Weapons"},
 {UpgradeTypes::Protoss_Air_Armor,UnitTypes::Protoss_Cybernetics_Core,"Air Armor"},
 {UpgradeTypes::Singularity_Charge,UnitTypes::Protoss_Cybernetics_Core,"Singularity Charge"},
 {UpgradeTypes::Leg_Enhancements,UnitTypes::Protoss_Citadel_of_Adun,"Leg Enhancements"},
 {UpgradeTypes::Scarab_Damage,UnitTypes::Protoss_Robotics_Support_Bay,"Scarab Damage"},
 {UpgradeTypes::Reaver_Capacity,UnitTypes::Protoss_Robotics_Support_Bay,"Reaver Capacity"},
 {UpgradeTypes::Gravitic_Drive,UnitTypes::Protoss_Robotics_Support_Bay,"Gravitic Drive"},
 {UpgradeTypes::Sensor_Array,UnitTypes::Protoss_Observatory,"Sensor Array"},
 {UpgradeTypes::Gravitic_Boosters,UnitTypes::Protoss_Observatory,"Gravitic Boosters"},
 {UpgradeTypes::Khaydarin_Amulet,UnitTypes::Protoss_Templar_Archives,"Khaydarin Amulet"},
 {UpgradeTypes::Apial_Sensors,UnitTypes::Protoss_Fleet_Beacon,"Apial Sensors"},
 {UpgradeTypes::Gravitic_Thrusters,UnitTypes::Protoss_Fleet_Beacon,"Gravitic Thrusters"},
 {UpgradeTypes::Carrier_Capacity,UnitTypes::Protoss_Fleet_Beacon,"Carrier Capacity"},
 {UpgradeTypes::Khaydarin_Core,UnitTypes::Protoss_Arbiter_Tribunal,"Khaydarin Core"},
 {UpgradeTypes::Argus_Jewel,UnitTypes::Protoss_Fleet_Beacon,"Argus Jewel"},
 {UpgradeTypes::Argus_Talisman,UnitTypes::Protoss_Templar_Archives,"Argus Talisman"}
};
inline const char* name(UnitTypes id){for(const auto& p:buildings)if(p.type==id)return p.name;for(const auto& p:units)if(p.type==id)return p.name;if(id==UnitTypes::Protoss_Archon)return "Archon";if(id==UnitTypes::Protoss_Dark_Archon)return "Dark Archon";return nullptr;}
}}
