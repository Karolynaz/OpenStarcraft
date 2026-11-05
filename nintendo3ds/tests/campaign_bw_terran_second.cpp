// Original owned shipyards, normal public control only; no map/actor edits.
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include "bwgame.h"
#include <fstream>
#include "bwenums.h"
using namespace port3ds;using namespace bwgame;
struct Point{int x,y;};
int main(int n,char**v){assert(n==3 || (n==4 && std::string(v[3])=="defeat"));const bool defeat=n==4;Engine e;assert(e.load(v[1],v[2],Scenario::bw_terran_campaign_2));assert(e.start_recording("/tmp/sc3ds-bwt2-platform-local.journal"));e.set_fog_enabled(false);
 bwgame::game_player navigation(bwgame::data_loading::data_files_directory(v[1]));std::ifstream nav_input(v[2],std::ios::binary);bwgame::a_vector<uint8_t> nav_bytes((std::istreambuf_iterator<char>(nav_input)),{});bwgame::game_load_functions nav_loader(navigation.st());nav_loader.load_map_data(nav_bytes.data(),nav_bytes.size(),[&]{for(auto& p:navigation.st().players)p.controller=bwgame::player_t::controller_occupied;},false);auto& nav=navigation.funcs();
 std::vector<Point> route{{4400,3328},{4688,2624},{3824,3104},{4400,1920},{3952,1632},{5072,1024},{5392,1184},{2512,1216},{2384,640},{2000,448},{1136,608},{752,992},{1616,1920},{880,2400},{1552,2656},{1744,3008},{1680,3232},{432,2624}};std::set<unsigned> cloaked;std::map<unsigned,unsigned> busy;std::map<std::pair<int,int>,unsigned> locked;std::map<unsigned,unsigned> born;unsigned hold=0;unsigned goal=0,previous=0,captured_at=0;
 for(unsigned frame=0;frame<90000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}SoundEvent events[256];e.drain_sounds(events,256);if(frame%12 || (frame%48 && !e.count_units(0,12)))continue;UnitView units[1700];auto count=e.visible_units(units,1700);auto ships=e.count_units(0,12)+e.count_units(2,12);UnitView* civilian=nullptr;for(unsigned i=0;i<count;++i)if(units[i].owner==0 && units[i].type==15)civilian=&units[i];
  if(ships>previous){previous=ships;captured_at=frame;printf("capture%u f%u waypoint%u\n",ships,e.frame(),goal);fflush(stdout);if(goal+1<route.size())++goal;}
  
  if(e.count_units(0,12)){
   const UnitView* target=nullptr;int near=99999999;int x=0,y=0,k=0;for(unsigned j=0;j<count;++j)if(units[j].owner==0 && units[j].type==12){x+=units[j].x;y+=units[j].y;++k;}if(k){x/=k;y/=k;}
   for(unsigned j=0;j<count;++j)if(units[j].owner==6 && units[j].type==12){int dx=units[j].x-x,dy=units[j].y-y,d=dx*dx+dy*dy;if(units[j].hp<280)d-=10000000;else d+=units[j].hp*100;if(d<near){near=d;target=&units[j];}}
   if(defeat){if(target && frame%48==0){e.select_area(0,0,e.snapshot().width*32,e.snapshot().height*32);e.perform(option_code(OptionKind::order,unsigned(Orders::Move)),target->x,target->y);}continue;}
   if(target){std::map<std::pair<int,int>,int> damage;
    for(unsigned j=0;j<count;++j){const auto& u=units[j];if(u.owner || u.type!=12)continue;auto id=e.own_unit_at(u.x,u.y);if(u.order==unsigned(Orders::FireYamatoGun) || !e.select(u.x,u.y))continue;auto info=e.selection_info();const UnitView* victim=nullptr;int score=99999999;
     for(unsigned k=0;k<count;++k){const auto& q=units[k];if(q.owner!=6 || q.type!=12)continue;int dx=q.x-u.x,dy=q.y-u.y,d=dx*dx+dy*dy;int remaining=q.hp-damage[{q.x,q.y}];if(remaining<=0)d+=10000000;else if(remaining<270)d-=100000;if(d<score){score=d;victim=&q;}}
     if(!victim)continue;
     if(info.type==12 && info.energy>=150 && e.perform(option_code(OptionKind::order,unsigned(Orders::FireYamatoGun)),victim->x,victim->y)){damage[{victim->x,victim->y}]+=260;busy[id]=frame+96;printf("CAST f%u id%u hp%d energy%d at%d,%d target%d,%d hp%d\n",e.frame(),id,info.hp,info.energy,u.x,u.y,victim->x,victim->y,victim->hp);}
     else if(frame>=busy[id] && frame%60==0){if(u.hp<180){int dx=u.x-target->x,dy=u.y-target->y;e.move_selected(std::max(64,std::min(int(e.snapshot().width*32)-64,u.x+dx*3)),std::max(64,std::min(int(e.snapshot().height*32)-64,u.y+dy*3)));}else e.command(Command::context,target->x,target->y);}
    }
   }
   if(frame%1200==0){printf("FINAL f%u own%u enemy%u target%d,%d\n",e.frame(),e.count_units(0,12),e.count_units(6,12),target?target->x:0,target?target->y:0);fflush(stdout);}continue;
  }
  Point target=route[goal];
  const UnitView* beacon=nullptr;int nearest=0x7fffffff;
  if(civilian)for(unsigned j=0;j<count;++j)if(units[j].owner==0 && units[j].type==195 && nav.is_reachable({civilian->x,civilian->y},{units[j].x,units[j].y})){int dx=units[j].x-civilian->x,dy=units[j].y-civilian->y,d=dx*dx+dy*dy;if(d<nearest){nearest=d;beacon=&units[j];}}
  if(beacon)target={beacon->x,beacon->y};
  const UnitView* threat=nullptr;int best=0x7fffffff;for(unsigned j=0;j<count;++j){const auto& q=units[j];if(q.owner!=6 || q.hp<=0 || q.type==101 || q.type==195 || (q.disabled && q.type>=203))continue;int dx=q.x-target.x,dy=q.y-target.y;bool close=dx*dx+dy*dy<600*600;for(unsigned k=0;k<count && !close;++k)if(units[k].owner==0 && units[k].type<106 && units[k].type!=15 && units[k].type!=101 && units[k].type!=11){dx=q.x-units[k].x;dy=q.y-units[k].y;close=dx*dx+dy*dy<384*384;}if(!close)continue;dx=q.x-target.x;dy=q.y-target.y;int score=dx*dx+dy*dy;if(q.type==9)score-=100000;if(score<best){best=score;threat=&q;}}
  bool cleared=!threat;
  for(unsigned i=0;i<count;++i){const auto& u=units[i];if(u.owner || !u.completed || u.type==101 || u.type>=106 || u.type==11)continue;unsigned id=e.own_unit_at(u.x,u.y);if(frame<busy[id] || !e.select(u.x,u.y))continue;Point target={u.x,u.y};int local_distance=0x7fffffff;for(unsigned j=0;j<count;++j)if(units[j].owner==0 && units[j].type==195 && nav.is_reachable({u.x,u.y},{units[j].x,units[j].y})){int dx=units[j].x-u.x,dy=units[j].y-u.y,d=dx*dx+dy*dy;if(d<local_distance){local_distance=d;target={units[j].x,units[j].y};}}
   if(u.type==15){if(!born.count(id))born[id]=frame;const UnitView* local=nullptr;int nearest=99999999;bool danger=false;unsigned guards=0;for(unsigned j=0;j<count;++j){const auto& q=units[j];int dx=q.x-u.x,dy=q.y-u.y,d=dx*dx+dy*dy;if(q.owner==0 && (q.type==0 || q.type==1 || q.type==3 || q.type==5) && d<800*800)++guards;if(q.owner==6 && (q.type==0 || q.type==1 || q.type==3 || q.type==5 || q.type==30 || q.type==32) && frame>=locked[{q.x,q.y}] && d<550*550)danger=true;if(q.owner==0 && q.type==195 && nav.is_reachable({u.x,u.y},{q.x,q.y}) && d<nearest){nearest=d;local=&q;}}if(danger && guards && frame<born[id]+2400)e.command(Command::stop,u.x,u.y);else if(local)e.move_selected(local->x,local->y);continue;}
   if(u.type==0 && u.hp>30 && frame%480==0)e.perform(option_code(OptionKind::stim));
   if(u.type==1 && !cloaked.count(id)){if(e.perform(option_code(OptionKind::cloak)))cloaked.insert(id);}
   if(u.type==1 && u.energy>=100){const UnitView* lock=nullptr;int best=520*520;for(unsigned j=0;j<count;++j)if(units[j].owner==6 && frame>=locked[{units[j].x,units[j].y}] && (units[j].type==3 || units[j].type==5 || units[j].type==30 || units[j].type==9)){int dx=u.x-units[j].x,dy=u.y-units[j].y,d=dx*dx+dy*dy;if(d<best){best=d;lock=&units[j];}}if(lock && e.perform(option_code(OptionKind::order,unsigned(Orders::CastLockdown)),lock->x,lock->y)){busy[id]=frame+180;locked[{lock->x,lock->y}]=frame+900;continue;}}
   if((u.type==5 || u.type==30) && frame%120==0){int closest=99999999;for(unsigned j=0;j<count;++j)if(units[j].owner==6 && !units[j].flying && units[j].type<176 && units[j].type!=101){int dx=u.x-units[j].x,dy=u.y-units[j].y;closest=std::min(closest,dx*dx+dy*dy);}if((u.type==5 && closest<380*380) || (u.type==30 && closest>420*420)){if(e.perform(option_code(OptionKind::siege))){busy[id]=frame+120;continue;}}}
   if(u.type==34 && u.energy>=75 && e.researched(unsigned(TechTypes::Optical_Flare))){const UnitView* detector=nullptr;for(unsigned j=0;j<count;++j)if(units[j].owner==6 && units[j].type==9 && frame>=locked[{units[j].x,units[j].y}]){int dx=u.x-units[j].x,dy=u.y-units[j].y;if(dx*dx+dy*dy<500*500){detector=&units[j];break;}}if(detector && e.perform(option_code(OptionKind::order,unsigned(Orders::CastOpticalFlare)),detector->x,detector->y)){busy[id]=frame+120;locked[{detector->x,detector->y}]=frame+10000;continue;}}
   if(u.type==12 && threat && threat->type==12 && u.energy>=150 && e.perform(option_code(OptionKind::order,unsigned(Orders::FireYamatoGun)),threat->x,threat->y)){busy[id]=frame+120;continue;}if(u.type==34){const UnitView* wounded=nullptr;int distance=0x7fffffff;for(unsigned j=0;j<count;++j)if(units[j].owner==0 && (units[j].type==0 || units[j].type==1 || units[j].type==15) && units[j].hp<units[j].max_hp){int dx=u.x-units[j].x,dy=u.y-units[j].y;if(dx*dx+dy*dy<distance){distance=dx*dx+dy*dy;wounded=&units[j];}}if(wounded)e.perform(option_code(OptionKind::order,unsigned(Orders::MedicHeal)),wounded->x,wounded->y);else if(frame%240==0)e.move_selected(target.x,target.y);continue;}
   if(frame%120==0){const UnitView* enemy=nullptr;int best=850*850;for(unsigned j=0;j<count;++j)if(units[j].owner==6 && units[j].type<176 && units[j].type!=101 && units[j].type!=11 && nav.is_reachable({u.x,u.y},{units[j].x,units[j].y})){int dx=u.x-units[j].x,dy=u.y-units[j].y,d=dx*dx+dy*dy;if(d<best){best=d;enemy=&units[j];}}e.command(enemy?Command::context:Command::attack,enemy?enemy->x:target.x,enemy?enemy->y:target.y);}
  }
  if(frame%1200==0){for(unsigned i=0;i<count;++i)if(units[i].owner==0 && units[i].type<106 && units[i].type!=101)printf("own%u at%d,%d hp%d order%u\n",units[i].type,units[i].x,units[i].y,units[i].hp,units[i].order);printf("beacon%d,%d threat%u %d,%d ",target.x,target.y,threat?threat->type:999,threat?threat->x:0,threat?threat->y:0);printf("BWT2 f%u ships%u goal%u pilot%d,%d ownMarines%u winner%d\n",e.frame(),ships,goal,civilian?civilian->x:0,civilian?civilian->y:0,e.count_units(0,0),e.snapshot().winner);fflush(stdout);}
 }
 printf("BWT2 result%d frame%u captured%u\n",e.snapshot().winner,e.frame(),previous);if(defeat){assert(previous>=3 && e.snapshot().winner==1 && e.count_units(0,12)==0);puts("Original shipyard all-warships-lost defeat through public commands PASS");return 0;}assert(e.snapshot().winner==0 && previous>=3 && e.count_units(6,12)==0 && e.snapshot().next_scenario.empty());SaveView view;assert(e.save_game("/tmp/sc3ds-bwt2-completed.sav",view));Engine replay;SaveView restored;assert(replay.load_save(v[1],"/tmp/sc3ds-bwt2-completed.sav","/tmp/sc3ds-bwt2-replay","/tmp/sc3ds-bwt2-map",restored));assert(e.state_digest()==replay.state_digest());puts("Original shipyard capture, native battlecruiser strike and exact completed save replay PASS");
}
