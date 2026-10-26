// Public-controller smoke playthrough for the two fixed-army BW openings.
// Original maps, units and triggers are unchanged. Fog disabled for test routing.
#include "engine.h"
#include "bwenums.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace port3ds;
static bool controlled(unsigned mission,const UnitView& u){return u.owner==0 && u.completed && !u.worker && u.type<106 && u.type!=35 && u.type!=42;}
int main(int argc,char**argv){assert(argc==4);unsigned mission=std::atoi(argv[3]);assert(mission==3 || mission==5);const auto& m=campaign::missions[mission];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-bw-route-")+m.key).c_str()));e.set_fog_enabled(false);
 struct Point{int x,y;};std::vector<Point> route=mission==3?std::vector<Point>{{2688,300},{2840,940},{1450,2400},{540,2500},{2370,2840},{3488,2656},{3376,2528}}:std::vector<Point>{{528,352},{2592,400},{2240,1808},{1344,2448},{320,1840},{1152,3008},{352,3888},{1344,3616},{2208,3856},{2176,3568}};unsigned goal=0,secured_at=0;const unsigned hive_target[]={1,2,3,4,5,5,6,6,7,7};
 for(unsigned frame=0;frame<100000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%96)continue;UnitView units[1700];unsigned n=e.visible_units(units,1700);auto target=route[goal];bool arrived=false,hostile=false;unsigned army=0;
  std::vector<int> xs,ys;unsigned near=0;for(unsigned i=0;i<n;++i){auto& u=units[i];if(controlled(mission,u)){xs.push_back(u.x);ys.push_back(u.y);}}
  if(!xs.empty()){std::sort(xs.begin(),xs.end());std::sort(ys.begin(),ys.end());}
  int centerx=xs.empty()?0:xs[xs.size()/2],centery=ys.empty()?0:ys[ys.size()/2];
  for(unsigned i=0;i<n;++i){auto& u=units[i];int dx=u.x-target.x,dy=u.y-target.y,d=dx*dx+dy*dy;if((u.owner==4 || u.owner==5) && mission==5 && d<320*320)hostile=true;if(controlled(mission,u)){++army;if(d<256*256)++near;if(d<128*128 && (mission!=3 || u.type==75))arrived=true;
   if(frame%480==0 && u.type==75){printf("Zeratul f%u xy%d,%d hp%d sh%d center%d,%d goal%u\n",e.frame(),u.x,u.y,u.hp,u.shields,centerx,centery,goal);fflush(stdout);}
   if(e.select(u.x,u.y)){const auto selected=e.selection_info();if(selected.type==75)assert(std::string(selected.name)=="Zeratul");if(selected.type==53)assert(std::string(selected.name)=="Hunter Killer");
    if(mission==5 && goal==7){if(!u.flying && u.x<1000)e.command(Command::context,384,3712);else e.move_selected(target.x,target.y);}
    else if(mission==3 && u.type==75){int gx=u.x-centerx,gy=u.y-centery;if(gx*gx+gy*gy>192*192)e.move_selected(centerx,centery);else e.command(Command::attack,target.x,target.y);}
    else if(mission==3 && u.type==67 && u.energy>=75){const UnitView* enemy=nullptr;int distance=256*256;for(unsigned k=0;k<n;++k)if(units[k].owner==3 || units[k].owner==4){int ex=units[k].x-u.x,ey=units[k].y-u.y,dd=ex*ex+ey*ey;if(dd<distance && (units[k].type==37 || units[k].type==38 || units[k].type==39)){enemy=&units[k];distance=dd;}}if(enemy)e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastPsionicStorm)),enemy->x,enemy->y);else e.command(Command::attack,target.x,target.y);}
    else if(mission==5 && (u.type==45 || u.type==46)){
     const UnitView* enemy=nullptr;int best=320*320;for(unsigned k=0;k<n;++k)if((units[k].owner==4 || units[k].owner==5) && (units[k].type==39 || units[k].type==38 || units[k].type==43)){int ex=u.x-units[k].x,ey=u.y-units[k].y,dd=ex*ex+ey*ey;if(dd<best){enemy=&units[k];best=dd;}}
     bool cast=false;if(enemy && u.energy>=150)cast=e.perform(option_code(OptionKind::order,unsigned(u.type==45?bwgame::Orders::CastSpawnBroodlings:bwgame::Orders::CastPlague)),enemy->x,enemy->y);
     if(!cast)e.move_selected(centerx,centery);
    }else e.command(Command::attack,target.x,target.y);
   }
  }}
  bool secured=mission==5?e.count_units(0,133)>=hive_target[goal] && ((goal!=5 && goal!=7) || arrived):arrived;if(!secured)secured_at=0;else if(!secured_at)secured_at=frame;
  if(secured && (mission!=5 || frame-secured_at>=(goal==5 || goal==7?0u:1200u)) && near*4>=army*3 && (!hostile || (mission==5 && goal==7)) && goal+1<route.size()){++goal;secured_at=0;if(mission==5 && goal==7){for(unsigned j=0;j<n;++j)if(units[j].owner==0 && units[j].type==53){e.select(units[j].x,units[j].y);break;}SaveView view;view.camera_x=184;view.camera_y=3592;view.cursor_x=200;view.cursor_y=120;view.auto_camera=false;assert(e.save_game("/tmp/sc3ds-expzerg-before-nydus.sav",view));}printf("%s reached waypoint%u frame%u army%u\n",m.key,goal,e.frame(),army);fflush(stdout);}
  if(frame%4800==0){printf("%s frame%u goal%u army%u hives%u winner%d\n",m.key,e.frame(),goal,army,e.count_units(0,133),e.snapshot().winner);fflush(stdout);}
 }
 if(e.snapshot().winner!=0){UnitView units[1700];auto n=e.visible_units(units,1700);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type<106)printf("final owner0 type%u xy%d,%d hp%d order%u\n",units[i].type,units[i].x,units[i].y,units[i].hp,units[i].order);}
 printf("%s route result%d frame%u\n",m.key,e.snapshot().winner,e.frame());fflush(stdout);if(e.snapshot().winner!=0)return 1;SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));puts("Original BW fixed-army mission completed through public orders and original triggers PASS");
}
