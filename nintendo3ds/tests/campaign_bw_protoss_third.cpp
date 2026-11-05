// Original Brood War Protoss 3: paid northwest expansion and a Corsair-escorted
// sixteen Dark Templar Shuttle landing destroys both original Cerebrates.
// Uses public controls and native restrictions, resources and placement only.
// Fog is disabled for repeatable test navigation; no map/triggers/unit stats edited.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <limits>
#include <set>
#include <map>
#include <queue>
using namespace port3ds;
struct Point{int x,y;};
static std::vector<Point> flight(const UnitView* units,unsigned count,int width,int height,Point from,Point to){unsigned w=(width+63)/64,h=(height+63)/64;std::vector<int> costs(w*h,1),dist(w*h,1000000000),prev(w*h,-1);for(unsigned i=0;i<count;++i){const auto& u=units[i];if(!u.owner || u.owner>=8 || u.hp<=0)continue;unsigned range=0;switch(u.type){case 0:case 1:case 3:case 8:case 12:case 38:case 43:case 47:case 53:case 66:case 70:case 124:case 144:case 162:range=272;break;default:break;}if(!range)continue;for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){auto dx=int(x*64+32)-u.x,dy=int(y*64+32)-u.y;auto d=dx*dx+dy*dy;if(d<int(range*range))costs[y*w+x]+=int((range*range-d)/1024)+40;}}auto cell=[&](Point p){return std::min(h-1,unsigned(p.y/64))*w+std::min(w-1,unsigned(p.x/64));};unsigned a=cell(from),b=cell(to);using Node=std::pair<int,unsigned>;std::priority_queue<Node,std::vector<Node>,std::greater<Node>> open;dist[a]=0;open.push({0,a});while(!open.empty()){auto at=open.top();open.pop();auto i=at.second;if(at.first!=dist[i])continue;if(i==b)break;int x=i%w,y=i/w;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){if(!dx && !dy)continue;int nx=x+dx,ny=y+dy;if(nx<0 || ny<0 || nx>=int(w) || ny>=int(h))continue;unsigned j=ny*w+nx;int d=at.first+costs[j]*(dx && dy?14:10);if(d<dist[j]){dist[j]=d;prev[j]=i;open.push({d,j});}}}assert(prev[b]>=0);std::map<unsigned,unsigned> cast_busy;std::map<std::pair<int,int>,unsigned> webbed;std::vector<Point> route;for(int i=b;i!=int(a);i=prev[i])route.push_back({int((i%w)*64+32),int((i/w)*64+32)});std::reverse(route.begin(),route.end());std::vector<Point> turns;for(unsigned k=0;k<route.size();++k){if(k>0 && k+1<route.size()){auto a=route[k-1],b=route[k],c=route[k+1];if((b.x-a.x)*(c.y-b.y)==(b.y-a.y)*(c.x-b.x))continue;}turns.push_back(route[k]);}return turns;}

int main(int argc,char**argv){assert(argc==3);const auto& m=campaign::missions[campaign::key_index("expprotoss03")];Engine e;assert(e.load(argv[1],argv[2],Scenario(m.scenario)));assert(e.start_recording((std::string("/tmp/sc3ds-campaign-playthrough-")+m.key).c_str()));e.set_fog_enabled(false);
 const bool z=false;const unsigned worker=z?41:64,base=z?131:154,supply=z?42:156,gas=z?149:157;
 std::map<unsigned,unsigned> cast_busy;std::map<std::pair<int,int>,unsigned> webbed;std::vector<Point> route;bool landed=false;bool convoy_live=false;unsigned carrier_id=0;bool launch=false;unsigned dt_made=2;std::map<unsigned,unsigned> brain_assignment;unsigned assigned=0;unsigned pioneer=0;bool expanded=false;unsigned zealot_made=0;unsigned flight_stage=0;unsigned build_pending=0,build_at=0,build_count=0;bool attack=false;std::set<unsigned>gas_ids;std::map<unsigned,unsigned>seen;int home_x=0,home_y=0;
 for(unsigned frame=0;frame<110000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}if(frame%48)continue;
  UnitView list[1700];unsigned n=e.visible_units(list,1700);auto state=e.snapshot();std::vector<UnitView> workers,parents;std::vector<UnitView> minerals;std::vector<UnitView> enemies;
  for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.type==worker && u.completed)workers.push_back(u);if(u.type==base && u.x>1600){home_x=u.x;home_y=u.y;}if(u.completed)parents.push_back(u);}if(u.mineral)minerals.push_back(u);if(u.owner==3 || u.owner==4)enemies.push_back(u);}
  if(!home_x)continue;
  
  const UnitView* brain=nullptr;for(const auto& u:enemies)if(u.type==151 && (!brain || u.x<brain->x))brain=&u;
  const UnitView* shuttle=nullptr;for(const auto& p:parents)if(p.type==69 && (!shuttle || p.cargo>shuttle->cargo))shuttle=&p;
  if(convoy_live && shuttle && !shuttle->cargo && shuttle->x<1000 && shuttle->y>1700)landed=true;
  if(!shuttle || !shuttle->cargo)convoy_live=false;
  // Clear and establish the connected northwest mineral expansion normally.
  bool nw_threat=false;for(const auto& u:enemies)if(u.x<1500 && u.y<1100 && u.type<200 && u.type!=42 && u.type!=144)nw_threat=true;
  for(const auto& p:parents)if(p.type==65 && e.select(p.x,p.y)){
   const UnitView* target=nullptr;int best=99999999;for(const auto& u:enemies)if(u.x<1500 && u.y<1100 && u.type!=42){int d=abs(u.x-p.x)+abs(u.y-p.y);if(d<best){best=d;target=&u;}}
   if(frame%240==0)e.command(Command::attack,target?target->x:864,target?target->y:736);
  }
  unsigned bases=e.count_units(0,154,false);expanded=bases>1;
  if(zealot_made>=6 && !expanded && !nw_threat && !workers.empty()){
   if(!pioneer)pioneer=e.own_unit_at(workers.back().x,workers.back().y);
   for(const auto& w:workers)if(e.own_unit_at(w.x,w.y)==pioneer && e.select(w.x,w.y)){
    bool placed=false;for(int y=880;y>=224 && !placed;y-=32)for(int x=960;x<=1408 && !placed;x+=32)if(e.placement_unit(154,x,y).valid)placed=e.perform(option_code(OptionKind::build,154),x,y);
    if(!placed)e.move_selected(1120,800);
   }
  }
  if(expanded)pioneer=0;
  // Three paid Shuttles carry four Reavers and four Dark Templar.
  std::vector<UnitView> transports;for(const auto& p:parents)if(p.type==69)transports.push_back(p);
  unsigned cargo=0;for(const auto& p:transports)cargo+=p.cargo;
  if(!launch && cargo>=16 && transports.size()>=4 && e.count_units(0,60)>=8 && e.upgrade_level(unsigned(bwgame::UpgradeTypes::Gravitic_Drive))){
   bool together=true;for(const auto& p:transports){int dx=p.x-2400,dy=p.y-1550;if(dx*dx+dy*dy>80*80){together=false;if(e.select(p.x,p.y))e.move_selected(2400,1550);}}
   if(together && e.select_similar(transports[0].x,transports[0].y,2200,1430)>=4){assert(e.store_group(0));launch=convoy_live=true;route=flight(list,n,3072,3072,{2400,1550},{640,2336});flight_stage=0;printf("HEAVY DROP f%u cargo%u route%zu\n",e.frame(),cargo,route.size());}
  }
  if(launch){if(cargo==0){landed=true;convoy_live=false;}else{auto goal=flight_stage<route.size()?route[flight_stage]:Point{640,2336};if(e.recall_group(0))e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Move)),goal.x,goal.y);auto p=transports.empty()?Point{0,0}:Point{transports[0].x,transports[0].y};int dx=p.x-goal.x,dy=p.y-goal.y;if(dx*dx+dy*dy<64*64 && flight_stage<route.size())++flight_stage;dx=p.x-640;dy=p.y-2336;if(dx*dx+dy*dy<96*96 && e.recall_group(0))e.perform(option_code(OptionKind::unload));}}
  else {std::set<unsigned> reserved;for(unsigned j=0;j<transports.size();++j){const auto& p=transports[j];unsigned cap=4;if(p.cargo>=cap)continue;for(const auto& u:parents){if(u.x<1600 || (u.type!=61))continue;auto id=e.own_unit_at(u.x,u.y);if(reserved.count(id))continue;if(e.select(p.x,p.y)){e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::PickupTransport)),u.x,u.y);reserved.insert(id);}break;}}}
  if(frame%48==0 && brain)for(const auto& p:parents)if((p.type==61 || p.type==83) && p.x<1600 && p.y>1700 && e.select(p.x,p.y)){
   const UnitView* threat=nullptr;int best=600*600;for(const auto& u:enemies)if(u.type==146 || u.type==38 || u.type==37 || u.type==144){int dx=u.x-p.x,dy=u.y-p.y,d=dx*dx+dy*dy;if(d<best){best=d;threat=&u;}}
   if(p.type==83){if(p.fighters<5)e.perform(option_code(OptionKind::train,85));if(threat)e.command(Command::context,threat->x,threat->y);else if(brain)e.command(Command::context,brain->x,brain->y);}
   else {auto id=e.own_unit_at(p.x,p.y);if(!brain_assignment.count(id))brain_assignment[id]=assigned++%2;const UnitView* target=brain;for(const auto& u:enemies)if(u.type==151 && ((brain_assignment[id]==0 && u.x<640) || (brain_assignment[id]==1 && u.x>=640)))target=&u;e.command(Command::context,target->x,target->y);}
  }
  if(frame%120==0)for(const auto& p:parents)if(p.type==60 && frame>=cast_busy[e.own_unit_at(p.x,p.y)] && e.select(p.x,p.y)){
    if(!convoy_live && !landed){if(abs(p.x-home_x)+abs(p.y-home_y)>400)e.move_selected(home_x,home_y-100);else{const UnitView* danger=nullptr;for(const auto& u:enemies)if(u.flying && abs(u.x-home_x)+abs(u.y-home_y)<700){danger=&u;break;}if(danger)e.command(Command::context,danger->x,danger->y);}continue;}
    const UnitView* spore=nullptr;int dd=360*360;for(const auto& u:enemies)if(u.type==144 && frame>webbed[{u.x,u.y}]){int dx=u.x-(shuttle?shuttle->x:p.x),dy=u.y-(shuttle?shuttle->y:p.y),d=dx*dx+dy*dy;if(d<dd){dd=d;spore=&u;}}
    if(spore && p.energy>=125 && e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::CastDisruptionWeb)),spore->x,spore->y)){webbed[{spore->x,spore->y}]=frame+550;cast_busy[e.own_unit_at(p.x,p.y)]=frame+180;printf("WEB f%u at%d,%d energy%d\n",e.frame(),spore->x,spore->y,p.energy);continue;}
    const UnitView* target=nullptr;int best=99999999;for(const auto& u:enemies)if(u.flying){int dx=u.x-p.x,dy=u.y-p.y,d=dx*dx+dy*dy;if(d<400*400){if(d<best){best=d;target=&u;}}}
    const Point escort=landed?Point{432,2400}:flight_stage<route.size()?route[flight_stage]:Point{432,2400};
    if(target && (landed || (shuttle && abs(target->x-shuttle->x)+abs(target->y-shuttle->y)<400)))e.command(Command::context,target->x,target->y);else e.move_selected(escort.x,escort.y);
  }
  // Native guards defend the colony until a paid army has assembled.
  unsigned army=e.count_units(0,z?38:65)+e.count_units(0,z?37:66)+e.count_units(0,z?43:70);
  if(army>=45 || (z && e.count_units(0,43)>=20) || (!z && e.count_units(0,70)>=20))attack=true;
  if(army<2)attack=false;
  if(false && attack && frame%240==0 && !enemies.empty()){
   const UnitView* target=&enemies.front();int distance=std::numeric_limits<int>::max();for(const auto& u:enemies){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(!z)for(const auto& u:enemies)if(u.type>=106 && u.type<202){int dx=u.x-home_x,dy=u.y-home_y,d=dx*dx+dy*dy;if(target->type<106 || d<distance){distance=d;target=&u;}}
   for(unsigned i=0;i<n;++i)if(list[i].owner==0 && (list[i].type==(z?37u:65u) || list[i].type==(z?38u:66u) || list[i].type==(z?43u:70u)))if(e.select(list[i].x,list[i].y))e.command(Command::attack,target->x,target->y);
  }
  for(const auto& w:workers)if(e.own_unit_at(w.x,w.y)!=pioneer && !gas_ids.count(e.own_unit_at(w.x,w.y)) && (w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop))){
   const UnitView* target=nullptr;int distance=std::numeric_limits<int>::max();for(const auto& u:minerals){int dx=u.x-w.x,dy=u.y-w.y,d=dx*dx+dy*dy;if(d<distance){distance=d;target=&u;}}
   if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);
  }
  for(const auto& w:workers){auto id=e.own_unit_at(w.x,w.y);if(gas_ids.count(id))seen[id]=frame;}
  for(auto it=gas_ids.begin();it!=gas_ids.end();)if(frame>seen[*it]+240){seen.erase(*it);it=gas_ids.erase(it);}else ++it;
  if(workers.size()>10)for(const auto& r:parents)if(r.type==gas){unsigned assigned=0;for(const auto& w:workers)if(gas_ids.count(e.own_unit_at(w.x,w.y)) && abs(w.x-r.x)+abs(w.y-r.y)<400)++assigned;for(const auto& w:workers){unsigned id=e.own_unit_at(w.x,w.y);if(id==pioneer || gas_ids.count(id) || assigned>=3 || abs(w.x-r.x)+abs(w.y-r.y)>700)continue;if(e.select(w.x,w.y) && e.command(Command::context,r.x+40,r.y+16)){gas_ids.insert(id);seen[id]=frame;++assigned;}}}
  if(build_pending && (e.count_units(0,build_pending)>build_count || frame>build_at+1500))build_pending=0;
  unsigned want=0;int anchor_x=home_x,anchor_y=home_y;
  if(!z)for(const auto& p:parents)if(p.type==160 && p.disabled){want=supply;anchor_x=p.x;anchor_y=p.y;break;}
  if(!want && state.supply_max-state.supply<5 && !z)want=supply;
  if(!want && !e.count_units(0,z?142:160))want=z?142:160;
  if(!want && !e.count_units(0,gas))want=gas;
  if(!want && z && !e.count_units(0,135))want=135;
  
  
  if(!want && !e.count_units(0,166,false))want=166;
  if(!want && !e.count_units(0,164))want=164;
  if(!want && expanded && e.count_units(0,gas,false)<2)want=gas;
  if(!want && zealot_made>=6 && !z && !e.count_units(0,155,false))want=155;
  if(!want && e.count_units(0,155) && !e.count_units(0,171,false))want=171;
  if(!want && expanded && e.count_units(0,167,false)<2)want=167;
  if(!want && z && e.count_units(0,131,false)<2)want=131;
  if(!want && z && e.count_units(0,132) && !e.count_units(0,141))want=141;
  if(!build_pending && want && !workers.empty()){
   bool placed=false;auto w=workers.back();for(const auto& q:workers)if(e.own_unit_at(q.x,q.y)!=pioneer && !gas_ids.count(e.own_unit_at(q.x,q.y)) && q.x>1600){w=q;break;}if(e.select(w.x,w.y)){
    if(want==gas){for(unsigned i=0;i<n && !placed;++i)if(list[i].geyser && true && e.placement_unit(want,list[i].x,list[i].y).valid)placed=e.perform(option_code(OptionKind::build,want),list[i].x,list[i].y);}
    else for(int ring=2;ring<11 && !placed;++ring)for(int y=-ring;y<=ring && !placed;++y)for(int x=-ring;x<=ring && !placed;++x){if(std::abs(x)!=ring && std::abs(y)!=ring)continue;int px=anchor_x+x*32,py=anchor_y+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}
   }if(placed){build_pending=want;build_at=frame;build_count=e.count_units(0,want);}
  }
  for(const auto& p:parents){if(!e.select(p.x,p.y))continue;if(z && p.type==35){unsigned next=state.supply_max-state.supply<5?42:e.count_units(0,worker)<18?worker:e.count_units(0,141)?43:e.count_units(0,135)?38:37;e.perform(option_code(OptionKind::morph,next));}
   else if(z && p.type==131 && !e.count_units(0,132,false))e.perform(option_code(OptionKind::morph,132));
   else if(!z && p.type==base && e.count_units(0,worker)<(expanded?32:16) && !e.selection_info().queue_count)e.perform(option_code(OptionKind::train,worker));
   else if(!z && p.type==155 && e.count_units(0,69,false)<4 && !e.selection_info().queue_count)e.perform(option_code(OptionKind::train,69));
   else if(p.type==167 && e.count_units(0,60)<10 && !e.selection_info().queue_count)e.perform(option_code(OptionKind::train,60));
   
   else if(p.type==83 && p.fighters<5)e.perform(option_code(OptionKind::train,85));
   else if(p.type==171)e.perform(option_code(OptionKind::upgrade,unsigned(bwgame::UpgradeTypes::Gravitic_Drive)));
   else if(p.type==160 && zealot_made<6 && !e.selection_info().queue_count){if(e.perform(option_code(OptionKind::train,65)))++zealot_made;}else if(p.type==160 && zealot_made>=6 && dt_made<16 && !e.selection_info().queue_count){if(e.perform(option_code(OptionKind::train,61)))++dt_made;}
  }
  if(frame%4800==0){for(const auto& p:parents)if(p.type==61 || p.type==69 || p.type==83)printf("t%u at%d,%d hp%d cargo%u o%u\n",p.type,p.x,p.y,p.hp,p.cargo,p.order);if(brain)printf("brain%d,%d hp%d ",brain->x,brain->y,brain->hp);if(!z)printf("Corsairs%u Stargates%u enemies%zu ",e.count_units(0,60),e.count_units(0,167),enemies.size());printf("%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",m.key,e.frame(),state.minerals,state.gas,e.count_units(0,worker),army,e.count_units(0,z?142:160),e.count_units(0,z?135:164),build_pending,attack);fflush(stdout);}
 }
 auto s=e.snapshot();printf("%s result%d frame%u\n",m.key,s.winner,e.frame());fflush(stdout);assert(s.winner==0 && s.next_scenario.empty() && !e.count_units(3,151) && !e.count_units(4,151));SaveView view;assert(e.save_game((std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(std::string("/tmp/sc3ds-")+m.key+"-completed.sav").c_str(),(std::string("/tmp/sc3ds-third-replay-")+m.key).c_str(),(std::string("/tmp/sc3ds-third-map-")+m.key).c_str(),restored));assert(e.state_digest()==replay.state_digest());puts("Original third mission paid economy/combat/victory + completed save replay PASS");
}
