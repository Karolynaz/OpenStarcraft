#pragma once
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
 {"protoss02","Into the Flames",24,2,false}
};
constexpr unsigned count=sizeof(missions)/sizeof(missions[0]);
inline unsigned index(unsigned scenario){for(unsigned i=0;i<count;++i)if(missions[i].scenario==scenario)return i;return ~0u;}
inline bool scenario(unsigned value){return index(value)<count;}
inline unsigned race(unsigned value){auto i=index(value);return i<count?missions[i].race:1;}
inline unsigned adjacent(unsigned current,int step){if(current>=count)return 0;const auto& m=missions[current];for(unsigned n=1;n<=count;++n){unsigned next=(current+count+(step>0?n:count-n))%count;if(missions[next].brood==m.brood && missions[next].race==m.race)return next;}return current;}
inline unsigned opening(bool brood,unsigned race){for(unsigned i=0;i<count;++i)if(missions[i].brood==brood && missions[i].race==race)return i;return 0;}
}}
