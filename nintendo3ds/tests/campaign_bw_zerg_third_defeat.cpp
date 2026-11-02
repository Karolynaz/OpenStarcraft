// Original mission and native Fenix-death failure, induced by public movement.
#include "engine.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int c,char**v){assert(c==3);Engine e;assert(e.load(v[1],v[2],Scenario::bw_zerg_campaign_3));e.set_fog_enabled(false);bool found=false;
for(unsigned f=0;f<30000 && e.snapshot().winner<0;++f){assert(e.tick());SoundEvent s[256];e.drain_sounds(s,256);if(f%120)continue;UnitView u[1700];unsigned n=e.visible_units(u,1700);for(unsigned i=0;i<n;++i)if(u[i].owner==0 && u[i].type==78 && e.select(u[i].x,u[i].y)){found=true;e.move_selected(3584,3696);}}
printf("Fenix loss winner%d frame%u\n",e.snapshot().winner,e.frame());assert(found && e.count_units(0,78)==0 && e.snapshot().winner==1);}
