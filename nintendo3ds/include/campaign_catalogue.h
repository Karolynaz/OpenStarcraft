#pragma once
#include <string>
namespace port3ds { namespace campaign {
struct Mission {const char* key;const char* title;unsigned scenario,race;bool brood;};
static constexpr Mission missions[]={
 {"terran01","Wasteland",16,1,false},
 {"zerg01","Among the Ruins",17,0,false},
 {"protoss01","First Strike",18,2,false},
 {"expprotoss01","Escape from Aiur",19,2,true},
 {"expterran01","First Strike",20,1,true},
 {"expzerg01","Vile Disruption",21,0,true},
 {"terran02","Backwater Station",22,1,false},
 {"zerg02","Egression",23,0,false},
 {"protoss02","Into the Flames",24,2,false},
 {"terran03","Desperate Alliance",25,1,false},
 {"zerg03","The New Dominion",26,0,false},
 {"protoss03","Higher Ground",27,2,false},
 {"terran04","The Jacobs Installation",28,1,false},
 {"protoss04","The Hunt for Tassadar",29,2,false},
 {"zerg04","Agents of the Swarm",30,0,false},
 {"terran05","Revolution",31,1,false},
 {"expprotoss02","Dunes of Shakuras",32,2,true},
 {"terran06","Norad II",33,1,false},
 {"terran08","The Trump Card",34,1,false},
 {"terran09","The Big Push",53,1,false},
 {"terran11","New Gettysburg",54,1,false},
 {"zerg05","The Amerigo",35,0,false},
 {"zerg06","The Dark Templar",36,0,false},
 {"zerg07","The Culling",37,0,false},
 {"zerg08","Eye for an Eye",38,0,false},
 {"expzerg02","Reign of Fire",39,0,true},
 {"expzerg03","The Kel-Morian Combine",40,0,true},
 {"expterran02","The Dylarian Shipyards",41,1,true},
 {"expprotoss03","Legacy of the Xel\'Naga",42,2,true},
 {"expterran03","Ruins of Tarsonis",43,1,true},
 {"expprotoss04","The Quest for Uraj",44,2,true},
 {"expterran04","Assault on Korhal",45,1,true},
 {"expterran06","Emperor's Flight",57,1,true},
 {"zerg09","The Invasion of Aiur",46,0,false},
 {"protoss05","Choosing Sides",47,2,false},
 {"protoss06","Into the Darkness",48,2,false},
 {"protoss07","Homeland",49,2,false},
 {"protoss08","The Trial of Tassadar",50,2,false},
 {"protoss09","Shadow Hunters",51,2,false},
 {"protoss10","Eye of the Storm",52,2,false},
 {"terran12","The Hammer Falls",55,1,false},
 {"zerg10","Full Circle",56,0,false}
};
constexpr unsigned count=sizeof(missions)/sizeof(missions[0]);
inline unsigned index(unsigned scenario){for(unsigned i=0;i<count;++i)if(missions[i].scenario==scenario)return i;return ~0u;}
inline bool scenario(unsigned value){return index(value)<count;}
// Author-selected branch names are local SCX basenames, never file paths.
inline unsigned key_episode(const std::string& key){unsigned prefix=key.compare(0,3,"exp")==0?3:0;auto stem=key.substr(prefix);unsigned race=stem.compare(0,4,"zerg")==0?0:stem.compare(0,6,"terran")==0?1:stem.compare(0,7,"protoss")==0?2:~0u;if(race==~0u)return prefix && stem=="bonus"?3:~0u;unsigned n=race==0?4:race==1?6:7;if(stem.size()<n+2 || stem.size()>n+3 || stem[n]<'0' || stem[n]>'9' || stem[n+1]<'0' || stem[n+1]>'9' || (stem.size()==n+3 && (stem[n+2]<'a' || stem[n+2]>'f')))return ~0u;return (prefix?3:0)+race;}
inline std::string next_key(unsigned current,const std::string& filename){const auto i=index(current);if(i>=count || filename.size()<5 || filename.size()>31)return {};std::string stem;for(unsigned char c:filename){if(c>='A' && c<='Z')c+=32;if(!((c>='a' && c<='z') || (c>='0' && c<='9') || c=='.'))return {};stem+=char(c);}if(stem.substr(stem.size()-4)!=".scx")return {};stem.resize(stem.size()-4);if(missions[i].brood)stem="exp"+stem;return key_episode(stem)==unsigned(missions[i].brood?3:0)+missions[i].race?stem:std::string{};}
inline unsigned key_index(const std::string& key){for(unsigned i=0;i<count;++i)if(key==missions[i].key)return i;return ~0u;}
inline unsigned race(unsigned value){auto i=index(value);return i<count?missions[i].race:1;}
inline unsigned adjacent(unsigned current,int step){if(current>=count)return 0;const auto& m=missions[current];for(unsigned n=1;n<=count;++n){unsigned next=(current+count+(step>0?n:count-n))%count;if(missions[next].brood==m.brood && missions[next].race==m.race)return next;}return current;}
inline unsigned opening(bool brood,unsigned race){for(unsigned i=0;i<count;++i)if(missions[i].brood==brood && missions[i].race==race)return i;return 0;}
}}
