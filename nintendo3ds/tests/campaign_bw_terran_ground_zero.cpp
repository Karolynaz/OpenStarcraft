// Original Ground Zero route using the scripted reinforcements and public orders.
#include "engine.h"
#include "bwenums.h"
#include "sound_ids.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <queue>
#include <set>
#include <vector>
using namespace port3ds;
using namespace bwgame;

constexpr unsigned unit(UnitTypes type) { return unsigned(type); }
constexpr unsigned dropship = unit(UnitTypes::Terran_Dropship);
constexpr unsigned tank = unit(UnitTypes::Terran_Siege_Tank_Tank_Mode);
constexpr unsigned sieged_tank = unit(UnitTypes::Terran_Siege_Tank_Siege_Mode);
constexpr unsigned wraith = unit(UnitTypes::Terran_Wraith);
constexpr unsigned command_center = unit(UnitTypes::Terran_Command_Center);

struct Point { int x, y; };
int d2(Point a, Point b){int dx=a.x-b.x,dy=a.y-b.y;return dx*dx+dy*dy;}
int d1(Point a,Point b){return std::abs(a.x-b.x)+std::abs(a.y-b.y);}
Point pos(const UnitView& u){return {u.x,u.y};}
bool pick(Engine& e,const UnitView& u){return e.select(u.x,u.y)&&e.selection_info().own&&e.selection_info().type==u.type;}
unsigned order(Orders o){return option_code(OptionKind::order,unsigned(o));}
std::vector<Point> route(UnitView* units,unsigned n, Point a, Point b) {
    constexpr int width=192,height=128,step=32;
    std::vector<int> cost(width*height,1),dist(width*height,1000000000),previous(width*height,-1);
    for(unsigned i=0;i<n;++i){
        const auto& u=units[i];
        if((u.owner!=1&&u.owner!=6&&u.owner!=7)||u.disabled)continue;
        int range=0, power=0;
        switch(u.type){
        case 124:range=244;power=32;break;
        case 3:case 8:case 58:range=260;power=21;break;
        case 0:case 16:range=190;power=8;break;
        default:continue;
        }
        range+=24;
        for(int y=std::max(0,(u.y-range)/step);y<=std::min(height-1,(u.y+range)/step);++y)
            for(int x=std::max(0,(u.x-range)/step);x<=std::min(width-1,(u.x+range)/step);++x)
                if(d2(pos(u),{x*step+16,y*step+16}) < range*range)cost[y*width+x]+=power;
    }
    const auto cell=[](Point p){return std::max(0,std::min(height-1,p.y/step))*width+std::max(0,std::min(width-1,p.x/step));};
    const int from=cell(a), to=cell(b);
    using Node=std::pair<int,int>;
    std::priority_queue<Node,std::vector<Node>,std::greater<Node>> todo;
    todo.push({0,from});dist[from]=0;
    while(!todo.empty()){
        const auto item=todo.top();todo.pop();
        int i=item.second,x=i%width,y=i/width;
        if(i==to)break;
        if(dist[i]!=item.first)continue;
        for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
            int xx=x+dx,yy=y+dy;
            if((dx==0&&dy==0)||xx<0||yy<0||xx>=width||yy>=height)continue;
            int j=yy*width+xx,v=dist[i]+(cost[j]+cost[i])*(dx&&dy?14:10);
            if(v<dist[j]){previous[j]=i;dist[j]=v;todo.push({v,j});}
        }
    }
    std::vector<Point> path;
    for(int i=to;i!=from;i=previous[i]){
        assert(i>=0);
        path.push_back({i%width*step+16,i/width*step+16});
    }
    std::reverse(path.begin(),path.end());
    std::vector<Point> corners;
    for(size_t i=0;i<path.size();++i){
        if(i>0&&i+1<path.size() && (path[i].x-path[i-1].x)*(path[i+1].y-path[i].y)==
                                 (path[i].y-path[i-1].y)*(path[i+1].x-path[i].x))continue;
        corners.push_back(path[i]);
    }
    if(corners.empty()) corners.push_back(b);
    corners.back()=b;
    return corners;
}
std::vector<Point> east_route(UnitView* units, unsigned count, Point from, Point drop) {
    auto path = route(units, count, from, {6050, 2432});
    for (Point next : {Point{6090, 1660}, Point{6075, 570}, Point{6048, 180}, drop})
        path.push_back(next);
    return path;
}
struct Trip {std::vector<Point> path; size_t i=0; bool unload=false;};
int main(int argc,char** argv){
    assert(argc==4);
    Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_terran_campaign_5_ground_zero));
    const std::string out=argv[3];assert(e.start_recording(out.c_str()));e.set_fog_enabled(false);
    // Approach outside the southern defenses, then land east of the objective.
    const Point aim{5728, 272}, drop{5856, 192};
    bool landed = false;
    unsigned voices=0;bool sent=false,siege_research=false;std::map<unsigned,Trip> trips;
    std::set<unsigned> assigned;
    for(unsigned f=0;f<16500&&e.snapshot().winner<0;++f){
        if(!e.tick()){printf("ERROR %s\n",e.error());return 1;}
        SoundEvent sound[256]; const auto ns=e.drain_sounds(sound,256);
        for(unsigned i=0;i<ns;++i) if(sound[i].id>=campaign_sound_base)++voices;
        if(f%12||f<3700)continue;
        UnitView list[1700];const auto count=e.visible_units(list,1700);
        std::vector<UnitView> ships, tanks, army;
        for(unsigned i=0;i<count;++i){
            auto u=list[i];
            if(u.owner==0){
                if(u.type==dropship)ships.push_back(u);
                if(u.type==tank)tanks.push_back(u);
                if (u.type == sieged_tank || u.type == tank || u.type == wraith ||
                    u.type == unit(UnitTypes::Terran_Goliath) ||
                    u.type == unit(UnitTypes::Terran_Marine) || u.type == unit(UnitTypes::Terran_Ghost))
                    army.push_back(u);
                if(u.type==unit(UnitTypes::Terran_Machine_Shop)&&!siege_research&&pick(e,u))
                    siege_research=e.perform(option_code(OptionKind::research,unsigned(TechTypes::Tank_Siege_Mode)));
            }
        }
        if(!sent && f%24==0){
            for(const auto& ship:ships){
                if(ship.cargo >= 2 || ship.order==unsigned(Orders::PickupTransport) || !pick(e,ship))continue;
                const UnitView* tank=nullptr;int score=400*400;
                for(const auto& u:tanks){
                    auto id=e.own_unit_at(u.x,u.y);
                    const int d=d2(pos(ship),pos(u));
                    if(id&&!assigned.count(id)&&d<score){tank=&u;score=d;}
                }
                if(tank&&e.perform(order(Orders::PickupTransport),tank->x,tank->y))
                    assigned.insert(e.own_unit_at(tank->x,tank->y));
            }
            unsigned loaded=0;
            for(const auto& u:ships)loaded+=u.cargo;
            if(loaded>=5 || (f>5100&&loaded>=4)){
                for(const auto& u:ships) {
                    const auto id=e.own_unit_at(u.x,u.y);
                    if (!id || !u.cargo) continue;
                    auto path = east_route(list, count, pos(u), drop);
                    trips.emplace(id,Trip{path});
                }
                sent=!trips.empty();
                printf("launch f%u %u loaded in %zu ships, tech %d\n",f,loaded,trips.size(),e.researched(unsigned(TechTypes::Tank_Siege_Mode)));
            }
        }
        if(sent)for(const auto& u:ships){
            auto it=trips.find(e.own_unit_at(u.x,u.y));
            if(it==trips.end()||!pick(e,u))continue;
            auto& t=it->second;
            if(d1(pos(u),t.path[t.i])<68 && t.i+1<t.path.size())++t.i;
            const auto next=t.path[t.i];
            if(t.i+1==t.path.size()&&d1(pos(u),drop)<110&&u.cargo&&!t.unload){
                t.unload=e.perform(order(Orders::MoveUnload),drop.x,drop.y);
                if(t.unload)landed = true;
                if(t.unload)printf("unload f%u at%d,%d from%d,%d cargo%u\n",f,drop.x,drop.y,u.x,u.y,u.cargo);
            }else if(!t.unload && f%24==0)e.move_selected(next.x,next.y);
        }
        if(f%48==0&&sent)for(const auto& u:army){
            if(u.type==wraith){
                if(!pick(e,u))continue;
                auto it=trips.find(e.own_unit_at(u.x,u.y));
                if(it==trips.end()){
                    auto path = east_route(list, count, pos(u), drop);
                    it=trips.emplace(e.own_unit_at(u.x,u.y),Trip{path}).first;
                }
                auto& t=it->second;
                if(t.i+1<t.path.size()&&d1(pos(u),t.path[t.i])<58)++t.i;
                auto next=t.path[t.i];
                if(d2(aim,pos(u))<580*580)e.command(Command::context,aim.x,aim.y);
                else e.move_selected(next.x,next.y);
                continue;
            }
            if(d2(pos(u),aim)>1100*1100||!pick(e,u))continue;
            if(u.type==tank||u.type==sieged_tank) {
                if(d2(pos(u),aim)<430*430&&u.type==tank&&e.researched(unsigned(TechTypes::Tank_Siege_Mode))
                   && e.perform(option_code(OptionKind::siege)))continue;
                if(e.count_units(7,command_center,false))e.command(Command::context,aim.x,aim.y);
            }
        }

    }
    const auto s=e.snapshot();printf("WINNER %d frame%u next=%s voices%u\n",s.winner,e.frame(),s.next_scenario.c_str(),voices);
    assert(s.winner == 0 && landed && !e.count_units(7, command_center, false));
    assert(s.next_scenario == "Terran06.scx" && voices >= 22);
    const auto save = out + ".ob3";
    SaveView view;
    assert(e.save_game(save.c_str(), view));
    Engine replay;
    assert(replay.load_save(argv[1], save.c_str(), (out + "-replay").c_str(),
                            (out + "-map.chk").c_str(), view));
    assert(replay.snapshot().winner == 0 && replay.state_digest() == e.state_digest());
    puts("Ground Zero: original ending, victory and exact replay PASS");
}
