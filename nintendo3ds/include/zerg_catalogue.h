#pragma once
#include "terran_catalogue.h"
namespace port3ds {namespace zerg {
using namespace bwgame;using terran::Product;using terran::Research;using terran::Upgrade;
static constexpr Product buildings[]={
 {UnitTypes::Zerg_Hatchery,UnitTypes::Zerg_Drone,"Hatchery"},
 {UnitTypes::Zerg_Extractor,UnitTypes::Zerg_Drone,"Extractor"},
 {UnitTypes::Zerg_Spawning_Pool,UnitTypes::Zerg_Drone,"Spawning Pool"},
 {UnitTypes::Zerg_Evolution_Chamber,UnitTypes::Zerg_Drone,"Evolution Chamber"},
 {UnitTypes::Zerg_Hydralisk_Den,UnitTypes::Zerg_Drone,"Hydralisk Den"},
 {UnitTypes::Zerg_Creep_Colony,UnitTypes::Zerg_Drone,"Creep Colony"},
 {UnitTypes::Zerg_Spire,UnitTypes::Zerg_Drone,"Spire"},
 {UnitTypes::Zerg_Queens_Nest,UnitTypes::Zerg_Drone,"Queen's Nest"},
 {UnitTypes::Zerg_Defiler_Mound,UnitTypes::Zerg_Drone,"Defiler Mound"},
 {UnitTypes::Zerg_Ultralisk_Cavern,UnitTypes::Zerg_Drone,"Ultralisk Cavern"},
 {UnitTypes::Zerg_Nydus_Canal,UnitTypes::Zerg_Drone,"Nydus Canal"},
 {UnitTypes::Zerg_Lair,UnitTypes::Zerg_Hatchery,"Morph Lair"},
 {UnitTypes::Zerg_Hive,UnitTypes::Zerg_Lair,"Morph Hive"},
 {UnitTypes::Zerg_Greater_Spire,UnitTypes::Zerg_Spire,"Morph Greater Spire"},
 {UnitTypes::Zerg_Sunken_Colony,UnitTypes::Zerg_Creep_Colony,"Morph Sunken Colony"},
 {UnitTypes::Zerg_Spore_Colony,UnitTypes::Zerg_Creep_Colony,"Morph Spore Colony"}
};
static constexpr Product units[]={
 {UnitTypes::Zerg_Drone,UnitTypes::Zerg_Larva,"Drone"},
 {UnitTypes::Zerg_Overlord,UnitTypes::Zerg_Larva,"Overlord"},
 {UnitTypes::Zerg_Zergling,UnitTypes::Zerg_Larva,"Zergling pair"},
 {UnitTypes::Zerg_Hydralisk,UnitTypes::Zerg_Larva,"Hydralisk"},
 {UnitTypes::Zerg_Mutalisk,UnitTypes::Zerg_Larva,"Mutalisk"},
 {UnitTypes::Zerg_Scourge,UnitTypes::Zerg_Larva,"Scourge pair"},
 {UnitTypes::Zerg_Queen,UnitTypes::Zerg_Larva,"Queen"},
 {UnitTypes::Zerg_Defiler,UnitTypes::Zerg_Larva,"Defiler"},
 {UnitTypes::Zerg_Ultralisk,UnitTypes::Zerg_Larva,"Ultralisk"},
 {UnitTypes::Zerg_Guardian,UnitTypes::Zerg_Mutalisk,"Morph Guardian"},
 {UnitTypes::Zerg_Devourer,UnitTypes::Zerg_Mutalisk,"Morph Devourer"},
 {UnitTypes::Zerg_Lurker,UnitTypes::Zerg_Hydralisk,"Morph Lurker"},
 {UnitTypes::Zerg_Infested_Terran,UnitTypes::Zerg_Infested_Command_Center,"Infested Terran"}
};
static constexpr Research techs[]={
 {TechTypes::Burrowing,UnitTypes::Zerg_Hatchery,"Burrowing"},
 {TechTypes::Burrowing,UnitTypes::Zerg_Lair,"Burrowing"},
 {TechTypes::Burrowing,UnitTypes::Zerg_Hive,"Burrowing"},
 {TechTypes::Spawn_Broodlings,UnitTypes::Zerg_Queens_Nest,"Spawn Broodlings"},
 {TechTypes::Ensnare,UnitTypes::Zerg_Queens_Nest,"Ensnare"},
 {TechTypes::Plague,UnitTypes::Zerg_Defiler_Mound,"Plague"},
 {TechTypes::Consume,UnitTypes::Zerg_Defiler_Mound,"Consume"},
 {TechTypes::Lurker_Aspect,UnitTypes::Zerg_Hydralisk_Den,"Lurker Aspect"}
};
static constexpr Upgrade upgrades[]={
 {UpgradeTypes::Zerg_Melee_Attacks,UnitTypes::Zerg_Evolution_Chamber,"Melee Attacks"},
 {UpgradeTypes::Zerg_Missile_Attacks,UnitTypes::Zerg_Evolution_Chamber,"Missile Attacks"},
 {UpgradeTypes::Zerg_Carapace,UnitTypes::Zerg_Evolution_Chamber,"Carapace"},
 {UpgradeTypes::Zerg_Flyer_Attacks,UnitTypes::Zerg_Spire,"Flyer Attacks"},
 {UpgradeTypes::Zerg_Flyer_Carapace,UnitTypes::Zerg_Spire,"Flyer Carapace"},
 {UpgradeTypes::Ventral_Sacs,UnitTypes::Zerg_Lair,"Ventral Sacs"},
 {UpgradeTypes::Antennae,UnitTypes::Zerg_Lair,"Antennae"},
 {UpgradeTypes::Pneumatized_Carapace,UnitTypes::Zerg_Lair,"Overlord Speed"},
 {UpgradeTypes::Metabolic_Boost,UnitTypes::Zerg_Spawning_Pool,"Metabolic Boost"},
 {UpgradeTypes::Adrenal_Glands,UnitTypes::Zerg_Spawning_Pool,"Adrenal Glands"},
 {UpgradeTypes::Muscular_Augments,UnitTypes::Zerg_Hydralisk_Den,"Muscular Augments"},
 {UpgradeTypes::Grooved_Spines,UnitTypes::Zerg_Hydralisk_Den,"Grooved Spines"},
 {UpgradeTypes::Gamete_Meiosis,UnitTypes::Zerg_Queens_Nest,"Gamete Meiosis"},
 {UpgradeTypes::Metasynaptic_Node,UnitTypes::Zerg_Defiler_Mound,"Metasynaptic Node"},
 {UpgradeTypes::Chitinous_Plating,UnitTypes::Zerg_Ultralisk_Cavern,"Chitinous Plating"},
 {UpgradeTypes::Anabolic_Synthesis,UnitTypes::Zerg_Ultralisk_Cavern,"Anabolic Synthesis"}
};
inline bool parent(UnitTypes actual,UnitTypes expected){return actual==expected || (actual==UnitTypes::Zerg_Hive && expected==UnitTypes::Zerg_Lair) || (actual==UnitTypes::Zerg_Greater_Spire && expected==UnitTypes::Zerg_Spire);}
inline const char* name(UnitTypes id){if(id==UnitTypes::Zerg_Zergling)return "Zergling";if(id==UnitTypes::Zerg_Scourge)return "Scourge";for(const auto& p:buildings)if(p.type==id)return (p.name[0]=='M' && p.name[1]=='o')?p.name+6:p.name;for(const auto& p:units)if(p.type==id)return (p.name[0]=='M' && p.name[1]=='o')?p.name+6:p.name;if(id==UnitTypes::Zerg_Larva)return "Larva";if(id==UnitTypes::Zerg_Egg)return "Egg";if(id==UnitTypes::Zerg_Cocoon)return "Cocoon";if(id==UnitTypes::Zerg_Lurker_Egg)return "Lurker Egg";if(id==UnitTypes::Zerg_Broodling)return "Broodling";if(id==UnitTypes::Zerg_Infested_Command_Center)return "Infested Command Center";return nullptr;}
}}
