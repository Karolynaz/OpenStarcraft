// Public-controller smoke playthrough for the two fixed-army BW openings.
// Original maps, units and triggers are unchanged. Fog disabled for test routing.
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==4);unsigned mission=std::atoi(argv[3]);assert(mission==3 || mission==5);const auto& m=campaign::missions[mission];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-bw-route-")+m.key).c_str()));e.set_fog_enabled(false);
 struct Point{int x,y;};std::vector<Point> route=mission==3?std::vector<Point>{{3488,2656},{3376,2528}}:std::vector<Point>{{528,352},{2592,400},{2240,1808},{1344,2448},{320,1840},{352,3888},{2208,3856},{2176,3568}};unsigned goal=0;
 for(unsigned frame=0;frame<100000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%96)continue;UnitView units[1700];unsigned n=e.visible_units(units,1700);auto target=route[goal];bool arrived=false,hostile=false;unsigned army=0;
  for(unsigned i=0;i<n;++i){auto& u=units[i];int dx=u.x-target.x,dy=u.y-target.y,d=dx*dx+dy*dy;if((u.owner==4 || u.owner==5) && mission==5 && d<320*320)hostile=true;if(u.owner==0 && u.completed && !u.worker && u.type<106 && u.type!=35 && u.type!=42){++army;if(d<128*128 && (mission!=3 || u.type==75))arrived=true;if(e.select(u.x,u.y)){if(mission==3 && u.type==75)e.move_selected(target.x,target.y);else e.command(Command::attack,target.x,target.y);}}}
  if(arrived && !hostile && goal+1<route.size()){++goal;printf("%s reached waypoint%u frame%u army%u\n",m.key,goal,e.frame(),army);fflush(stdout);}
  if(frame%4800==0){printf("%s frame%u goal%u army%u winner%d\n",m.key,e.frame(),goal,army,e.snapshot().winner);fflush(stdout);}
 }
 printf("%s route result%d frame%u\n",m.key,e.snapshot().winner,e.frame());fflush(stdout);if(e.snapshot().winner!=0)return 1;SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));puts("Original BW fixed-army mission completed through public orders and original triggers PASS");
}
