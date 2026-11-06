// Original SC Zerg9: paid colony, Overlord transport/speed, a real 600-second
// harvest, physical Khaydarin Crystal pickup and transport home to the beacon.
// Fog-off navigation only; original map, AI, triggers, funds, stats and tech
// availability remain unchanged. Controls are public Engine actions, including
// rectangle selection so stacked Overlords cannot hide the loaded transport.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <queue>
#include <functional>
#include <limits>
using namespace port3ds;
struct Point{int x,y;};
static std::vector<Point> flight(const UnitView* units,unsigned count,int width,int height,Point from,Point to){unsigned w=(width+63)/64,h=(height+63)/64;std::vector<int> costs(w*h,1),dist(w*h,1000000000),prev(w*h,-1);for(unsigned i=0;i<count;++i){const auto& u=units[i];if(!u.owner || u.owner>=8 || u.hp<=0)continue;unsigned range=0;switch(u.type){case 0:case 1:case 3:case 8:case 12:case 38:case 43:case 47:case 53:case 66:case 70:case 124:case 125:case 144:case 162:range=512;break;default:break;}if(!range)continue;for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){auto dx=int(x*64+32)-u.x,dy=int(y*64+32)-u.y;auto d=dx*dx+dy*dy;if(d<int(range*range))costs[y*w+x]+=int((range*range-d)/1024)+40;}}auto cell=[&](Point p){return std::min(h-1,unsigned(p.y/64))*w+std::min(w-1,unsigned(p.x/64));};unsigned a=cell(from),b=cell(to);using Node=std::pair<int,unsigned>;std::priority_queue<Node,std::vector<Node>,std::greater<Node>> open;dist[a]=0;open.push({0,a});while(!open.empty()){auto at=open.top();open.pop();auto i=at.second;if(at.first!=dist[i])continue;if(i==b)break;int x=i%w,y=i/w;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){if(!dx && !dy)continue;int nx=x+dx,ny=y+dy;if(nx<0 || ny<0 || nx>=int(w) || ny>=int(h))continue;unsigned j=ny*w+nx;int d=at.first+costs[j]*(dx && dy?14:10);if(d<dist[j]){dist[j]=d;prev[j]=i;open.push({d,j});}}}assert(prev[b]>=0);std::vector<Point> route;for(int i=b;i!=int(a);i=prev[i])route.push_back({int((i%w)*64+32),int((i/w)*64+32)});std::reverse(route.begin(),route.end());std::vector<Point> turns;for(unsigned k=0;k<route.size();++k){if(k>0 && k+1<route.size()){auto a=route[k-1],b=route[k],c=route[k+1];if((b.x-a.x)*(c.y-b.y)==(b.y-a.y)*(c.x-b.x))continue;}turns.push_back(route[k]);}return turns;}

int main(int argc,char**argv){assert(argc==3 || argc==4);if(argc==4){
 Engine e;assert(e.load(argv[1],argv[2],Scenario::zerg_campaign_9));assert(e.start_recording("/tmp/sc3ds-zerg09-defeat-journal"));
 // Original attacks overrun an undefended colony; no surrender/trigger injection.
 for(unsigned i=0;i<160000 && e.snapshot().winner<0;++i)assert(e.tick());
 assert(e.snapshot().winner==1);printf("Zerg9 original colony-loss frame%u\n",e.frame());
 SaveView view;assert(e.save_game("/tmp/sc3ds-zerg09-defeat.sav",view));Engine replay;SaveView restored;
 assert(replay.load_save(argv[1],"/tmp/sc3ds-zerg09-defeat.sav","/tmp/sc3ds-zerg09-defeat-replay","/tmp/sc3ds-zerg09-defeat-map",restored));
 assert(e.state_digest()==replay.state_digest());puts("Original Zerg9 colony loss and exact replay PASS");return 0;
}
const auto& m=campaign::missions[campaign::key_index("zerg09")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const unsigned worker=41,base=131,gas=149;
 unsigned courier=0,phase=0,route_stage=0,harvest_frame=0,crystal_frame=0;bool active_saved=false;bool returning_pickup=false;std::vector<Point>path;unsigned build_pending=0,build_at=0,build_count=0;bool attack=false;unsigned voices=0;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<48000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}SoundEvent events[256];auto sounds=e.drain_sounds(events,256);for(unsigned j=0;j<sounds;++j)voices+=events[j].id>=1200;
if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner>0 && u.owner<8 && u.type<173 && u.type!=128 && u.type!=129)enemies.push_back(u);}
  if(!home_x)continue;

std::vector<UnitView>lords;UnitView ferry{};bool has_ferry=false;for(const auto&u:parents)if(u.type==42){lords.push_back(u);if(!has_ferry || u.cargo>ferry.cargo){ferry=u;has_ferry=true;}}
if((phase==3 || phase==4) && !lords.empty()){ferry=lords.front();for(const auto&u:lords)if(abs(u.x-1792)+abs(u.y-2912)<abs(ferry.x-1792)+abs(ferry.y-2912))ferry=u;has_ferry=true;}
if(!phase && has_ferry && lords.size()>=4 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs)) && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace)) && workers.size()>8){
 if(!courier)courier=e.own_unit_at(workers.back().x,workers.back().y);
 for(const auto&w:workers)if(e.own_unit_at(w.x,w.y)==courier && e.select(w.x,w.y))e.command(Command::context,ferry.x,ferry.y);
 if(ferry.cargo){path=flight(list,n,state.width*32,state.height*32,{ferry.x,ferry.y},{1984,2848});route_stage=0;phase=1;printf("DELIVERY f%u cargo%u lords%zu\n",e.frame(),ferry.cargo,lords.size());}
}
if((phase==1 || phase==5) && has_ferry && !path.empty()){
 auto goal=path[route_stage];int dx=ferry.x-goal.x,dy=ferry.y-goal.y;
 if(dx*dx+dy*dy<64*64 && route_stage+1<path.size())++route_stage;
 else if(dx*dx+dy*dy<64*64 && route_stage+1==path.size()){e.select_area(ferry.x-96,ferry.y-96,192,192);bool unloaded=e.perform(option_code(OptionKind::unload));if(unloaded)phase=phase==1?2:6;printf("DROP f%u phase%u\n",e.frame(),phase);}
 if(phase==1 || phase==5)for(const auto&u:lords)if(e.select_area(u.x,u.y,0,0))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),path[route_stage].x,path[route_stage].y);
}
if(phase==2 && has_ferry && ferry.cargo && frame%240==0){e.select_area(ferry.x-96,ferry.y-96,192,192);e.perform(option_code(OptionKind::unload));}
if(phase==2)for(const auto&w:workers)if(w.x>1500 && w.y>2400 && e.select_workers(w.x-200,w.y-120))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),1984,2896);
if(phase==2 && state.countdown_active){phase=3;harvest_frame=e.frame();printf("HARVEST f%u countdown%u\n",e.frame(),state.countdown_seconds);}
if(phase==3 && state.countdown_seconds>80)for(const auto&u:lords)if(e.select_area(u.x,u.y,0,0))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),528,480);
if(phase==3 && state.countdown_seconds<=80 && has_ferry){if(!returning_pickup){path=flight(list,n,state.width*32,state.height*32,{ferry.x,ferry.y},{1792,2912});route_stage=0;returning_pickup=true;}auto goal=path[route_stage];int dx=ferry.x-goal.x,dy=ferry.y-goal.y;if(dx*dx+dy*dy<64*64 && route_stage+1<path.size())++route_stage;for(const auto&u:lords)if(e.select_area(u.x,u.y,0,0))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),path[route_stage].x,path[route_stage].y);}
if(phase==3 && state.countdown_seconds==0)for(const auto&w:workers)if(w.x>1500 && w.y>2400 && e.select_workers(w.x-200,w.y-120)){e.activate(1792,2912);if(w.carried_powerup==219){phase=4;crystal_frame=e.frame();e.store_group(3);courier=e.own_unit_at(w.x,w.y);printf("CRYSTAL f%u\n",e.frame());break;}}
if(phase==4 && !active_saved){
 SaveView view;view.camera_x=1592;view.camera_y=2792;
 assert(e.save_game("/tmp/sc3ds-zerg09-active.sav",view));Engine replay;SaveView restored;
 assert(replay.load_save(argv[1],"/tmp/sc3ds-zerg09-active.sav","/tmp/sc3ds-zerg09-active-replay","/tmp/sc3ds-zerg09-active-map",restored));
 assert(e.state_digest()==replay.state_digest());active_saved=true;puts("Zerg9 active carried-crystal replay PASS");
}
if(phase==4 && has_ferry){if(frame%240==0)for(const auto&w:workers)if(w.carried_powerup==219 && e.select_area(ferry.x,ferry.y,0,0))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::PickupTransport)),w.x,w.y);if(ferry.cargo){path=flight(list,n,state.width*32,state.height*32,{ferry.x,ferry.y},{528,480});route_stage=0;phase=5;printf("RETURN f%u\n",e.frame());}}
if(phase==6 && e.recall_group(3))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),528,480);
// Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,38)+e.count_units(0,37)+e.count_units(0,43);
  attack=true;
  
  if(attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}if(distance>900*900)target=nullptr;
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==43 || list[i].type==38))if(target && e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(!w.carried_powerup && e.own_unit_at(w.x,w.y)!=courier && (w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop))){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}if(distance>900*900)target=nullptr;
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  // Assign three workers to the paid refinery once it has completed.
  if(frame%480==0 && e.count_units(0,gas) && workers.size()>=7){UnitView* refinery=nullptr;for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==gas && list[i].completed)refinery=&list[i];if(refinery)for(unsigned i=0;i<3;++i)if(!workers[i].carried_powerup && workers[i].x<1500 && workers[i].y<1600 && e.own_unit_at(workers[i].x,workers[i].y)!=courier && e.select(workers[i].x,workers[i].y))e.command(Command::context,refinery->x,refinery->y);}
  if(build_pending && (e.count_units(0,build_pending)>build_count || frame>build_at+1500))build_pending=0;
  unsigned want=0;int anchor_x=home_x,anchor_y=home_y;
  if(!want && !e.count_units(0,142))want=142;
  if(!want && !e.count_units(0,gas))want=gas;
  if(!want && !e.count_units(0,135))want=135;
  if(!want && e.count_units(0,131,false)+e.count_units(0,132,false)<3)want=131;
  if(!want && !e.count_units(0,139,false))want=139;
  if(!build_pending && want && !workers.empty()){
   bool placed=false;auto w=workers.front();for(const auto&v:workers)if(!v.carried_powerup && v.x<1500 && v.y<1600 && e.own_unit_at(v.x,v.y)!=courier)w=v;if(!w.carried_powerup && w.x<1500 && w.y<1600 && e.own_unit_at(w.x,w.y)!=courier && e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=anchor_x+x*32,py=anchor_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;build_count=e.count_units(0,want);}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if( p.type==35){unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<18?worker:e.count_units(0,135)?38:37;if(next==38 && e.count_units(0,38)>=48)continue;e.perform(option_code(OptionKind::morph,next));}
   else if( p.type==131 && !e.count_units(0,132,false))e.perform(option_code(OptionKind::morph,132));
  }
  for(const auto&u:parents)if(e.select(u.x,u.y)){if(u.type==132){for(auto up:{bwgame::UpgradeTypes::Ventral_Sacs,bwgame::UpgradeTypes::Pneumatized_Carapace})if(!e.upgrade_level(unsigned(up)))e.perform(option_code(OptionKind::upgrade,unsigned(up)));}if(u.type==135){for(auto up:{bwgame::UpgradeTypes::Muscular_Augments,bwgame::UpgradeTypes::Grooved_Spines})if(!e.upgrade_level(unsigned(up)))e.perform(option_code(OptionKind::upgrade,unsigned(up)));}}
 if(frame%4800==0){printf("Zerg9 frame%u phase%u countdown%u workers%u hydras%u minerals%d gas%d\n",e.frame(),phase,state.countdown_seconds,e.count_units(0,worker),e.count_units(0,38),state.minerals,state.gas);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u next=%s\n",m.key,s.winner,e.frame(),s.next_scenario.c_str());fflush(stdout);
 assert(s.winner==0 && phase==6 && active_saved && voices>0);
 assert(harvest_frame && crystal_frame>=harvest_frame+24*599);
 assert(e.upgrade_level(unsigned(bwgame::UpgradeTypes::Ventral_Sacs))==1);
 assert(e.upgrade_level(unsigned(bwgame::UpgradeTypes::Pneumatized_Carapace))==1);
 assert(e.count_units(0,131)+e.count_units(0,132)+e.count_units(0,133)>0);
 // The authored Zerg9 CHK has no SetNextScenario action; normal episode order applies.
 assert(s.next_scenario.empty());
 SaveView view;view.camera_x=328;view.camera_y=360;assert(e.save_game("/tmp/sc3ds-zerg09-completed.sav",view));Engine replay;SaveView restored;
 assert(replay.load_save(argv[1],"/tmp/sc3ds-zerg09-completed.sav","/tmp/sc3ds-zerg09-replay","/tmp/sc3ds-zerg09-map",restored));
 assert(e.state_digest()==replay.state_digest());
 puts("The Invasion of Aiur: paid economy/transport, original 600-second harvest, physical crystal pickup/return, original victory and exact completed replay PASS");
}
