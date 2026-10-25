#pragma once
namespace port3ds { namespace campaign {
struct Mission {const char* key;const char* title;unsigned scenario,race;bool brood;};
static constexpr Mission missions[]={
 {"terran01","Wasteland",16,1,false},
 {"zerg01","Among the Ruins",17,0,false},
 {"protoss01","First Strike",18,2,false},
 {"expprotoss01","Escape from Aiur",19,2,true},
 {"expterran01","First Strike",20,1,true},
 {"expzerg01","Vile Disruption",21,0,true}
};
constexpr unsigned count=sizeof(missions)/sizeof(missions[0]);
inline unsigned index(unsigned scenario){for(unsigned i=0;i<count;++i)if(missions[i].scenario==scenario)return i;return ~0u;}
inline bool scenario(unsigned value){return index(value)<count;}
inline unsigned race(unsigned value){auto i=index(value);return i<count?missions[i].race:1;}
inline unsigned opening(bool brood,unsigned race){for(unsigned i=0;i<count;++i)if(missions[i].brood==brood && missions[i].race==race)return i;return 0;}
}}
