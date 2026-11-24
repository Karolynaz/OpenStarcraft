// Original owned BW Protoss4. Paid colony, authored restricted tech,
// eight-Shuttle decoy/transport fleet, Kerrigan ground approach to Uraj.
// Fog off only for repeatable navigation; no altered map/funds/AI/triggers.
#include "engine.h"
#include "bwenums.h"
#include <cstdio>
#include <cassert>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <limits>
using namespace port3ds;
struct Point{int x,y;};
static std::vector<Point> flight(const UnitView* units,unsigned count,int width,int height,Point from,Point to){unsigned w=(width+63)/64,h=(height+63)/64;std::vector<int> costs(w*h,1),dist(w*h,1000000000),prev(w*h,-1);for(unsigned i=0;i<count;++i){const auto& u=units[i];if(!u.owner || u.owner>=8 || u.hp<=0)continue;unsigned range=0;switch(u.type){case 0:case 1:case 3:case 8:case 12:case 38:case 43:case 47:case 53:case 66:case 70:case 124:case 125:case 144:case 162:range=352;break;default:break;}if(!range)continue;for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){auto dx=int(x*64+32)-u.x,dy=int(y*64+32)-u.y;auto d=dx*dx+dy*dy;if(d<int(range*range))costs[y*w+x]+=int((range*range-d)/1024)+40;}}auto cell=[&](Point p){return std::min(h-1,unsigned(p.y/64))*w+std::min(w-1,unsigned(p.x/64));};unsigned a=cell(from),b=cell(to);using Node=std::pair<int,unsigned>;std::priority_queue<Node,std::vector<Node>,std::greater<Node>> open;dist[a]=0;open.push({0,a});while(!open.empty()){auto at=open.top();open.pop();auto i=at.second;if(at.first!=dist[i])continue;if(i==b)break;int x=i%w,y=i/w;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){if(!dx && !dy)continue;int nx=x+dx,ny=y+dy;if(nx<0 || ny<0 || nx>=int(w) || ny>=int(h))continue;unsigned j=ny*w+nx;int d=at.first+costs[j]*(dx && dy?14:10);if(d<dist[j]){dist[j]=d;prev[j]=i;open.push({d,j});}}}assert(prev[b]>=0);std::vector<Point> route;for(int i=b;i!=int(a);i=prev[i])route.push_back({int((i%w)*64+32),int((i/w)*64+32)});std::reverse(route.begin(),route.end());std::vector<Point> turns;for(unsigned k=0;k<route.size();++k){if(k>0 && k+1<route.size()){auto a=route[k-1],b=route[k],c=route[k+1];if((b.x-a.x)*(c.y-b.y)==(b.y-a.y)*(c.x-b.x))continue;}turns.push_back(route[k]);}return turns;}

static int ferry(Engine&e,const char*root){bool launched=false,unloaded=false;unsigned stage=0;std::vector<Point>path;
for(unsigned f=0;f<30000 && e.snapshot().winner<0;++f){assert(e.tick());if(f%12)continue;UnitView list[1700];unsigned n=e.visible_units(list,1700);UnitView hero{},ship{};bool has_hero=false,has_ship=false;std::vector<UnitView>workers,ships;for(unsigned i=0;i<n;++i)if(!list[i].owner){if(list[i].type==51){hero=list[i];has_hero=true;}if(list[i].type==69){ships.push_back(list[i]);if(!has_ship || list[i].cargo>ship.cargo){ship=list[i];has_ship=true;}}if(list[i].type==64)workers.push_back(list[i]);}
if(!has_ship){printf("LOST SHIP f%u\n",e.frame());break;}
if(!launched){for(unsigned i=0;i<n;++i)if(!list[i].owner && list[i].type==155 && e.count_units(0,69,false)<8 && e.select(list[i].x,list[i].y) && !e.selection_info().queue_count)e.perform(option_code(OptionKind::train,69));if(e.select(ship.x,ship.y)){}if(!e.upgrade_level(unsigned(bwgame::UpgradeTypes::Gravitic_Drive)))for(unsigned i=0;i<n;++i)if(!list[i].owner && list[i].type==171 && e.select(list[i].x,list[i].y))e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Gravitic_Drive)));
if(has_hero && e.select(hero.x,hero.y))e.command(Command::context,ship.x,ship.y);
if(!has_hero && ship.cargo<2 && !workers.empty() && e.select(workers.front().x,workers.front().y))e.command(Command::context,ship.x,ship.y);
if(!has_hero && ships.size()>=8 && ship.cargo>=2 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Gravitic_Drive))){auto s=e.snapshot();path=flight(list,n,s.width*32,s.height*32,{ship.x,ship.y},{2780,3470});launched=true;printf("LAUNCH f%u path%zu\n",e.frame(),path.size());for(auto p:path)printf("via %d,%d\n",p.x,p.y);SaveView active;assert(e.save_game("/tmp/sc3ds-expprotoss04-cargo.sav",active));Engine resumed;SaveView restored;assert(resumed.load_save(root,"/tmp/sc3ds-expprotoss04-cargo.sav","/tmp/sc3ds-expprotoss04-cargo-replay","/tmp/sc3ds-expprotoss04-cargo-map",restored));assert(e.state_digest()==resumed.state_digest());puts("Loaded Kerrigan cargo exact active replay PASS");}}
if(launched && !unloaded){auto p=path[stage];int dx=ship.x-p.x,dy=ship.y-p.y;if(dx*dx+dy*dy<24*24 && stage+1<path.size())++stage;else if(stage+1==path.size() && dx*dx+dy*dy<32*32){if(e.select(ship.x,ship.y))e.perform(option_code(OptionKind::unload));unloaded=true;printf("UNLOAD f%u\n",e.frame());}if(!unloaded){for(const auto&u:ships)if(e.select(u.x,u.y))e.move_selected(path[stage].x,path[stage].y);}if(f%240==0)printf("f%u ship%d,%d %d+%d cargo%u via%u\n",e.frame(),ship.x,ship.y,ship.hp,ship.shields,ship.cargo,stage);}
if(unloaded && has_hero && e.select(hero.x,hero.y))e.activate(2770,3510);
}printf("FERRY result%d f%u\n",e.snapshot().winner,e.frame());if(e.snapshot().winner==0){SaveView v;assert(e.save_game("/tmp/sc3ds-expprotoss04-completed.sav",v));Engine replay;SaveView restored;assert(replay.load_save(root,"/tmp/sc3ds-expprotoss04-completed.sav","/tmp/sc3ds-expprotoss04-replay","/tmp/sc3ds-expprotoss04-replay-map",restored));assert(e.state_digest()==replay.state_digest() && replay.snapshot().winner==0);puts("Original BW Protoss4 paid base + eight-Shuttle Kerrigan/Uraj victory / exact completed replay PASS");return 0;}return 1;}
int main(int argc,char**argv){assert(argc==3 || argc==4);Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_protoss_campaign_4));assert(e.start_recording("/tmp/sc3ds-expprotoss04-session"));e.set_fog_enabled(false);if(argc==4){for(unsigned f=0;f<12000 && e.snapshot().winner<0;++f){assert(e.tick());if(f%48)continue;UnitView list[1700];auto n=e.visible_units(list,1700);for(unsigned i=0;i<n;++i)if(!list[i].owner && list[i].type==51 && e.select(list[i].x,list[i].y))e.command(Command::attack,1408,1696);}assert(e.snapshot().winner==1 && e.count_units(0,51)==0);printf("Original BW Protoss4 Kerrigan-loss defeat f%u PASS\n",e.frame());return 0;}std::set<unsigned>gas_ids;unsigned pending=0,pending_at=0,pending_before=0;unsigned builder=0,builder_at=0,cast_until=0;bool hero_cloaked=false;bool boarding=false;bool launched=false,unloaded=false;unsigned stage=0;std::vector<Point> path;
for(unsigned frame=0;frame<150000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"f%u %s\n",e.frame(),e.error());return 1;}if(frame%48)continue;UnitView list[1700];unsigned n=e.visible_units(list,1700);auto s=e.snapshot();std::vector<UnitView> own,workers,minerals;UnitView ship{},hero{},courier{};bool has_ship=false,has_hero=false,has_courier=false;int hx=1248,hy=800;for(unsigned i=0;i<n;++i){const auto&u=list[i];if(!u.owner && u.completed){own.push_back(u);if(u.type==64)workers.push_back(u);if(u.type==154){hx=u.x;hy=u.y;}if(u.type==69 && !has_ship){ship=u;has_ship=true;}if(u.type==51){hero=u;has_hero=true;}}if(u.mineral && abs(u.x-hx)+abs(u.y-hy)<1400)minerals.push_back(u);}
// Authored Scarabs change how early the starting base is cleared. Begin
// the ferry only when its production and speed-upgrade buildings are ready.
if(frame>=31200 && has_ship && e.count_units(0,unsigned(bwgame::UnitTypes::Protoss_Robotics_Facility)) &&
   e.count_units(0,unsigned(bwgame::UnitTypes::Protoss_Robotics_Support_Bay))){
 assert(ferry(e,argv[1])==0);
 return 0;
}
if(!has_hero && !(has_ship && ship.cargo))continue;
if(!launched && !workers.empty())courier=workers.front();
if(has_hero && !launched && !boarding && frame>=cast_until && (!has_ship || !ship.cargo) && e.select(hero.x,hero.y))e.move_selected(1360,500);
if(has_hero && !launched && !hero_cloaked && e.select(hero.x,hero.y))hero_cloaked=e.perform(option_code(OptionKind::cloak));
if(has_hero && !launched && !boarding && frame%192==0 && hero.energy>=75){const UnitView* target=nullptr;int best=0;for(unsigned i=0;i<n;++i){const auto&v=list[i];if(v.owner!=5 || abs(v.x-hero.x)+abs(v.y-hero.y)>650)continue;int score=v.type==30?100:0;for(unsigned j=0;j<n;++j)if(list[j].owner==5 && list[j].type<106 && abs(list[j].x-v.x)+abs(list[j].y-v.y)<130)++score;if(score>best){best=score;target=&v;}}if(target && best>=3 && e.select(hero.x,hero.y)){bool ok=e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastPsionicStorm)),target->x,target->y);if(ok)cast_until=frame+240;printf("Storm f%u ok%d energy%d target%u %d,%d\n",e.frame(),ok,hero.energy,target->type,target->x,target->y);}}
if(pending && (e.count_units(0,pending)>pending_before || frame>pending_at+2400))pending=0;
unsigned want=0;
if(!e.count_units(0,154,false))want=154;
else if(s.supply_max-s.supply<6)want=156;
else if(!e.count_units(0,166,false))want=166;
else if(e.count_units(0,166) && e.count_units(0,162,false)<5)want=162;
else if(!e.count_units(0,157,false))want=157;
else if(!e.count_units(0,160,false))want=160;
else if(!e.count_units(0,164,false))want=164;
else if(!e.count_units(0,155,false))want=155;
else if(!e.count_units(0,171,false))want=171;
if(want && !pending && !e.count_units(5,106,false))for(const auto&w:workers){if(gas_ids.count(e.own_unit_at(w.x,w.y)) || !e.select(w.x,w.y))continue;bool placed=false;if(want==157){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && abs(list[i].x-hx)+abs(list[i].y-hy)<1200 && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}else for(int r=0;r<14 && !placed;++r)for(int y=-r;y<=r && !placed;++y)for(int x=-r;x<=r && !placed;++x){int ax=want==162?1152:want==156?1152:hx,ay=want==162?992:want==156?800:hy;int px=ax+x*32,py=ay+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}if(placed){builder=e.own_unit_at(w.x,w.y);builder_at=frame;pending=want;pending_at=frame;pending_before=e.count_units(0,want);printf("build%u f%u\n",want,e.frame());break;}}
for(const auto&w:workers){auto id=e.own_unit_at(w.x,w.y);if(id==builder && frame<builder_at+144)continue;if(launched && w.carried_powerup)continue;if(!gas_ids.count(id) && (w.order==3 || w.order==2 || w.order==1)){const UnitView*t=nullptr;int best=0x7fffffff;for(const auto&m:minerals){int dx=w.x-m.x,dy=w.y-m.y,d=dx*dx+dy*dy;if(d<best){best=d;t=&m;}}if(t && e.select(w.x,w.y))e.command(Command::context,t->x,t->y);}}
if(workers.size()>5 && gas_ids.size()<3)for(const auto&g:own)if(g.type==157)for(const auto&w:workers){auto id=e.own_unit_at(w.x,w.y);if(gas_ids.count(id) || gas_ids.size()>=3)continue;if(e.select(w.x,w.y) && e.command(Command::context,g.x+40,g.y+16))gas_ids.insert(id);}
for(const auto&u:own)if(e.select(u.x,u.y)){
 if(u.type==154 && workers.size()<12 && e.selection_info().queue_count<1)e.perform(option_code(OptionKind::train,64));
 if(u.type==155 && !e.count_units(0,69,false) && e.selection_info().queue_count<1)e.perform(option_code(OptionKind::train,69));
 if(u.type==171 && !e.upgrade_level(unsigned(bwgame::UpgradeTypes::Gravitic_Drive)))e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Gravitic_Drive)));
 if(u.type==83 && u.fighters<5 && (s.minerals>450 || !e.count_units(0,154,false)) && e.selection_info().queue_count<1)e.perform(option_code(OptionKind::train,85));
}
if(has_ship && !launched && workers.size()>=6 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Gravitic_Drive))){
 boarding=true; if(has_hero && e.select(hero.x,hero.y))e.command(Command::context,ship.x,ship.y);
 if(!has_hero && ship.cargo<2){for(const auto&w:workers)if(!gas_ids.count(e.own_unit_at(w.x,w.y)) && e.select(w.x,w.y)){e.command(Command::context,ship.x,ship.y);break;}}
 if(!has_hero && ship.cargo>=2){path=flight(list,n,s.width*32,s.height*32,{ship.x,ship.y},{2780,3470});launched=true;stage=0;printf("LAUNCH f%u cargo%u path%zu\n",e.frame(),ship.cargo,path.size());}
}
if(launched && has_ship){printf("FLIGHT f%u xy%d,%d hp%d shield%d cargo%u stage%u/%zu\n",e.frame(),ship.x,ship.y,ship.hp,ship.shields,ship.cargo,stage,path.size());if(!unloaded && !path.empty()){auto p=path[stage];int dx=ship.x-p.x,dy=ship.y-p.y;if(dx*dx+dy*dy<24*24 && stage+1<path.size())++stage;else if(stage+1==path.size() && dx*dx+dy*dy<32*32){if(e.select(ship.x,ship.y))e.perform(option_code(OptionKind::unload));unloaded=true;printf("UNLOAD f%u ship%d,%d cargo%u\n",e.frame(),ship.x,ship.y,ship.cargo);}if(!unloaded && e.select(ship.x,ship.y))e.move_selected(path[stage].x,path[stage].y);}
 for(const auto&u:own)if(u.type==60 && e.select(u.x,u.y))e.command(Command::attack,ship.x,ship.y);
}
if(unloaded){if(has_hero && e.select(hero.x,hero.y)){if(hero.x>2000 || hero.y>2500)e.activate(2770,3510);}
 for(const auto&w:workers)if(w.x>2000 || w.y>2500)if(e.select(w.x,w.y))e.activate(2770,3510);
}
// Clear the actual occupied starting base, including its refinery.
if(frame%96==0)for(const auto&u:own)if(u.type==66 || u.type==83){const UnitView*t=nullptr;int best=0x7fffffff;for(unsigned j=0;j<n;++j){const auto&v=list[j];if(v.owner!=5)continue;int priority=v.type==30?0:v.type==125?1:v.type==0 || v.type==1?2:v.type==7?3:v.type==110?4:5;int dx=v.x-u.x,dy=v.y-u.y,d=priority*10000000+dx*dx+dy*dy;if(d<best){best=d;t=&v;}}if(!t)for(unsigned j=0;j<n;++j)if(list[j].owner==1){int dx=list[j].x-hx,dy=list[j].y-hy,d=dx*dx+dy*dy;if(d<800*800 && d<best){best=d;t=&list[j];}}if(t && e.select(u.x,u.y))e.command(Command::context,t->x,t->y);}
if(frame%2400==0){printf("f%u money%d/%d probes%zu supply%d/%d hero%d,%d hp%d ship%u corsair%u launch%d unload%d stage%u/%zu\n",e.frame(),s.minerals,s.gas,workers.size(),s.supply,s.supply_max,hero.x,hero.y,hero.hp,e.count_units(0,69),e.count_units(0,60),launched,unloaded,stage,path.size());fflush(stdout);}
}
printf("BWP4 result%d f%u\n",e.snapshot().winner,e.frame());fflush(stdout);if(e.snapshot().winner!=0)return 1;SaveView v;assert(e.save_game("/tmp/sc3ds-expprotoss04-session-completed.sav",v));}
