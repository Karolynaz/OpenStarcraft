// The Big Push on the original map, with a paid economy and ordinary console orders.
#include "engine.h"
#include "bwenums.h"
#include "result_screen.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace port3ds;
using namespace bwgame;

struct Point { int x, y; };
static int distance(int x, int y, int xx, int yy) {
    return (x-xx)*(x-xx)+(y-yy)*(y-yy);
}
static bool order(Engine& e, Orders o, int x=0, int y=0) {
    return e.perform(option_code(OptionKind::order,unsigned(o)),x,y);
}
static bool option(Engine& e, OptionKind k,unsigned v=0,int x=0,int y=0) {
    return e.perform(option_code(k,v),x,y);
}

int main(int argc,char** argv) {
    assert(argc==4 || argc==5);
    const std::string prefix=argv[3];
    const bool defeat=argc==5 && std::string(argv[4])=="defeat";
    assert(argc==4 || defeat);
    const auto scenario=Scenario::terran_campaign_8;
    Engine e;
    assert(e.load(argv[1],argv[2],scenario));
    assert(e.start_recording(prefix.c_str()));
    e.set_fog_enabled(false);
    if(defeat) {
        for(unsigned frame=0;frame<18000 && e.snapshot().winner<0;++frame) {
            assert(e.tick());
            if(frame%96)continue;
            UnitView list[1700];
            const auto n=e.visible_units(list,1700);
            for(unsigned i=0;i<n;++i) {
                const auto& u=list[i];
                if(!u.owner && u.type==unsigned(UnitTypes::Hero_Norad_II) && e.select(u.x,u.y))
                    e.move_selected(3456,816);
            }
        }
        assert(e.snapshot().winner==1 && e.count_units(0,unsigned(UnitTypes::Hero_Norad_II))==0);
        printf("The Big Push: authored Duke-loss defeat at frame %u PASS\n",e.frame());
        return 0;
    }
    const Point home={1600,3712}, expansion={2656,3250};
    unsigned pending=0,pending_frame=0,pending_count=0;
    bool delivered=false, attacking=false, relocated=false;
    std::map<unsigned,Point> landing;
    std::set<unsigned> gas, transfer;
    bool expansion_landed=false;
    for(unsigned f=0;f<85000 && e.snapshot().winner<0;++f) {
        assert(e.tick());
        SoundEvent audio[256];e.drain_sounds(audio,256);
        if(f%48)continue;
        UnitView list[1700];
        const auto n=e.visible_units(list,1700);
        std::vector<UnitView> own, workers, hostiles, minerals, geysers;
        unsigned hostile_buildings=0;
        for(unsigned i=0;i<n;++i) {
            auto& u=list[i];
            if(!u.owner && u.completed) {
                own.push_back(u);
                if(u.worker)workers.push_back(u);
            }
            if((u.owner==4 || u.owner==5) && !u.disabled) {
                hostiles.push_back(u);
                if(u.type>=106 && u.type<130)++hostile_buildings;
            }
            if(u.mineral)minerals.push_back(u);
            if(u.geyser)geysers.push_back(u);
        }
        const auto state=e.snapshot();
        const auto count=[&](unsigned type){return e.count_units(0,type,false);};
        const auto id=[&](const UnitView& u){return e.own_unit_at(u.x,u.y);};
        auto is_idle=[](const UnitView& u){return u.order==1 || u.order==2 || u.order==3;};

        for(const auto& u:own)if(u.flying && u.type>=106 && u.type<=116 &&
            u.order!=unsigned(Orders::BuildingLand) && e.select(u.x,u.y)) {
            const auto key=id(u);
            Point near=home;
            if(u.type==106 && f>6500 && relocated)near=expansion;
            if(u.type==106 && !relocated && f>6000 && f<12000)near=expansion;
            if(u.type==114)near={1728,3856};
            if(u.type==116)near=f<22000?Point{1920,3808}:Point{1056,3808};
            if(u.type==113)near={1856,3664};
            if(u.type==111)near={2048,3776};
            bool ok=false;
            for(int radius=0;radius<15 && !ok;++radius)
            for(int y=-radius;y<=radius && !ok;++y)
            for(int x=-radius;x<=radius && !ok;++x) {
                if(std::abs(x)!=radius && std::abs(y)!=radius)continue;
                const Point p={near.x+x*32,near.y+y*32};
                if(p.x<128 || p.x>3800 || p.y<near.y-80 || p.y>3970)continue;
                bool free=true;
                for(auto entry:landing)
                    if(entry.first!=key && distance(p.x,p.y,entry.second.x,entry.second.y)<180*180)free=false;
                if(free && option(e,OptionKind::land,0,p.x,p.y)){landing[key]=p;ok=true;}
            }
        }
        const UnitView* first_cc=nullptr;
        const UnitView* second_cc=nullptr;
        const UnitView* refinery=nullptr;
        for(const auto& u:own) {
            if(u.type==106 && !u.flying) {
                if(distance(u.x,u.y,expansion.x,expansion.y)<400*400)second_cc=&u;
                else if(!first_cc)first_cc=&u;
            }
            if(u.refinery)refinery=&u;
        }
        if(first_cc && !second_cc && !relocated && f>24500)for(const auto& u:own)
            if(&u!=first_cc && u.type==106 && !u.flying && e.select(u.x,u.y) && !e.snapshot().queue &&
               option(e,OptionKind::liftoff)){relocated=true;break;}
        if(second_cc && !expansion_landed) {
            expansion_landed=true;
            printf("Expansion landed f%u at %d,%d\n",f,second_cc->x,second_cc->y);
        }
        if(!delivered) {
            const UnitView* dropship=nullptr;
            const UnitView* dock_worker=nullptr;
            for(const auto& u:own) {
                if(u.type==11 && (!dropship || u.x<dropship->x))dropship=&u;
                if(u.worker && u.x>3300 && u.y>3600)dock_worker=&u;
            }
            if(dropship && e.select(dropship->x,dropship->y)) {
                if(dock_worker && dropship->cargo<4)order(e,Orders::PickupTransport,dock_worker->x,dock_worker->y);
                else if(dropship->cargo){
                    if(distance(dropship->x,dropship->y,1632,3660)<96*96)option(e,OptionKind::unload);
                    else e.move_selected(1632,3660);
                } else if(!dock_worker){delivered=true;printf("Workers delivered f%u\n",f);}
            }
        }
        if(second_cc && f%480==0 && transfer.size()<7) {
            for(const auto& w:workers) {
                auto uid=id(w);
                if(uid && !gas.count(uid) && !transfer.count(uid) && distance(w.x,w.y,home.x,home.y)<520*520 && e.select(w.x,w.y)){
                    if(e.command(Command::context,2560,3088))transfer.insert(uid);
                    if(transfer.size()>=7)break;
                }
            }
        }
        for(const auto& w:workers)if(is_idle(w) && e.select(w.x,w.y)){
            const UnitView* target=nullptr;
            if(refinery && gas.count(id(w))){
                e.command(Command::context,refinery->x+40,refinery->y+16);
                continue;
            }
            int best=0x7fffffff;
            for(const auto& m:minerals){
                int d=distance(w.x,w.y,m.x,m.y);
                if(d<best){best=d;target=&m;}
            }
            if(target)e.command(Command::context,target->x,target->y);
        }
        if(refinery && f%240==0 && workers.size()>=7) {
            std::vector<UnitView> candidates=workers;
            std::sort(candidates.begin(),candidates.end(),[&](const UnitView& a,const UnitView& b){
                return distance(a.x,a.y,refinery->x,refinery->y)<distance(b.x,b.y,refinery->x,refinery->y);
            });
            unsigned assigned=0;
            for(const auto& w:candidates)if(gas.count(id(w)))++assigned;
            for(const auto& w:candidates)if(assigned<3 && !gas.count(id(w)) &&
                distance(w.x,w.y,refinery->x,refinery->y)<550*550 && e.select(w.x,w.y) &&
                e.command(Command::context,refinery->x+40,refinery->y+16)){gas.insert(id(w));++assigned;}
        }

        if(pending && (count(pending)>pending_count || f>pending_frame+1300))pending=0;
        const unsigned want=f>24000 && !count(112) && state.minerals>300?112:
            f>24000 && count(124)<4 && count(122) && state.minerals>300?124:
            f>24000 && !count(122) && state.minerals>300?122:
            state.supply_max-state.supply<6 && state.supply_max<200 ?109:
            !count(110)?110:
            count(113)<(f>24000?5u:3u)?113:
            !count(112)?112:
            !count(123)?123:
            count(111)<3 && workers.size()>13?111:0;
        if(want && !pending && delivered && state.minerals>=75) {
            bool built=false;
            for(const auto& w:workers) {
                if(built)break;
                if(gas.count(id(w)) || distance(w.x,w.y,home.x,home.y)>570*570 || !e.select(w.x,w.y))continue;
                if(want==110) {
                    for(const auto& g:geysers)if(distance(g.x,g.y,home.x,home.y)<500*500 &&
                        e.placement_unit(want,g.x,g.y).valid && option(e,OptionKind::build,want,g.x,g.y)){built=true;break;}
                } else for(int r=0;r<=14 && !built;++r)
                    for(int y=-r;y<=r && !built;++y)for(int x=-r;x<=r && !built;++x) {
                        if(std::abs(x)!=r && std::abs(y)!=r)continue;
                        const Point p={1792+x*32,3776+y*32};
                        if(p.y<3584 || p.y>3970)continue;
                        bool free=true;
                        for(const auto& u:own)if((u.type==114 || u.type==116 || u.type==113) && !u.flying &&
                            distance(p.x,p.y,u.x+84,u.y)<118*118)free=false;
                        if(free && e.placement_unit(want,p.x,p.y).valid && option(e,OptionKind::build,want,p.x,p.y))built=true;
                    }
            }
            if(built){pending=want;pending_frame=f;pending_count=count(want);}
        }

        const auto tanks=count(5)+count(30);
        for(const auto& u:own)if(!u.flying && u.type>=106 && u.type<130 && e.select(u.x,u.y) && !e.snapshot().queue) {
            if(u.type==106 && workers.size()<36 && delivered && (workers.size()<7 || state.supply_max-state.supply>=4))
                option(e,OptionKind::train,7);
            if(u.type==111 && state.minerals>120 && count(0)<(f>22500?36u:14u))
                option(e,OptionKind::train,0);
            if(u.type==113 && tanks<(f>22500?32u:18u)) {
                if(f>24000 && count(123) && count(3)<16 && tanks>=10)
                    option(e,OptionKind::train,3);
                if(!option(e,OptionKind::train,5))option(e,OptionKind::addon,120);
            }
            if(u.type==120 && !e.researched(unsigned(TechTypes::Tank_Siege_Mode)))
                option(e,OptionKind::research,unsigned(TechTypes::Tank_Siege_Mode));
            if(u.type==112 && f>24000){option(e,OptionKind::research,unsigned(TechTypes::Stim_Packs));option(e,OptionKind::upgrade,unsigned(UpgradeTypes::U_238_Shells));}
            if(u.type==122 && f>24000){option(e,OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Infantry_Weapons));option(e,OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Infantry_Armor));}
            if(u.type==123 && state.gas>220 && tanks>=8) {
                option(e,OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Vehicle_Weapons));
                option(e,OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Vehicle_Plating));
            }
        }

        if(f<23000?tanks>=12:tanks>=18 || tanks+count(3)>=25)attacking=true;
        if(tanks+(f>23000?count(3):0)<6)attacking=false;
        const UnitView* target=nullptr;
        int best=0x7fffffff;
        const Point staging=second_cc?expansion:home;
        const Point objective = e.count_units(5,106)?Point{1376,2096}:Point{3456,816};
        for(const auto& u:hostiles){
            if(attacking && (u.type<106 || u.type>=130))continue;
            if(!attacking && f>22500 && u.type<106 && u.flying)continue;
            int d=distance(u.x,u.y,attacking?objective.x:staging.x,attacking?objective.y:staging.y);
            if(attacking && f>22500){
                int near=0x7fffffff;
                for(const auto& ally:own)if(ally.type==5 || ally.type==30 || ally.type==3)
                    near=std::min(near,distance(u.x,u.y,ally.x,ally.y));
                if(near<1800*1800)d=near+(u.type==106?0:32000);
            }
            if((attacking || d<720*720) && d<best){best=d;target=&u;}
        }
        for(const auto& u:own)if(f%96==0) {
            if(u.type==29 && e.select(u.x,u.y)){
                if(f>22000 || !target || !attacking || u.hp<170)e.move_selected(home.x,home.y);
                else if(u.order!=unsigned(Orders::FireYamatoGun)){
                    if(u.energy<150 || distance(u.x,u.y,target->x,target->y)>560*560 ||
                        !order(e,Orders::FireYamatoGun,target->x,target->y))
                        e.command(Command::attack,target->x,target->y);
                }
                continue;
            }
            if((u.type==0 || u.type==1 || u.type==3 || u.type==5 || u.type==8 || u.type==30) &&
               e.select(u.x,u.y)) {
                if(!target){
                    if(u.type!=30 && distance(u.x,u.y,staging.x,staging.y)>300*300)e.move_selected(staging.x,staging.y);
                    continue;
                }
                if(u.type==0 && f>22500 && attacking && u.hp>=30 && distance(u.x,u.y,target->x,target->y)<200*200 && e.researched(unsigned(TechTypes::Stim_Packs)))option(e,OptionKind::stim);
                if(u.type==30){if(distance(u.x,u.y,target->x,target->y)>380*380)option(e,OptionKind::siege);continue;}
                if(u.type==5 && distance(u.x,u.y,target->x,target->y)<310*310 &&
                    e.researched(unsigned(TechTypes::Tank_Siege_Mode)) && option(e,OptionKind::siege))continue;
                e.command(Command::attack,target->x,target->y);
            }
        }
        if(f%240==0)for(const auto& u:own)if(u.type==29 && u.hp+45<u.max_hp &&
            distance(u.x,u.y,home.x,home.y)<440*440) {
            unsigned repairs=0;
            for(const auto& w:workers)if(repairs<3 && !gas.count(id(w)) &&
                distance(w.x,w.y,u.x,u.y)<300*300 && e.select(w.x,w.y) &&
                e.command(Command::context,u.x,u.y))++repairs;
        }
        if(f%3600==0){
            printf("f%u resources%d/%d scv%zu CC%u/%d Duke%u buildings%u enemy%zu tanks%u goliaths%u pending%u attack%d\n",
                f,state.minerals,state.gas,workers.size(),count(106),!!second_cc,count(29),hostile_buildings,
                hostiles.size(),tanks,count(3),pending,attacking);
            fflush(stdout);
        }
    }
    printf("Big Push result%d at%u Duke%u\n",e.snapshot().winner,e.frame(),e.count_units(0,29));
    fflush(stdout);
    const auto outcome=e.snapshot();
    assert(expansion_landed && outcome.winner==0);
    assert(e.count_units(0,29)==1 && outcome.minerals_gathered>0);
    assert(ResultScreen::next_mission(unsigned(scenario),true,outcome.next_scenario)==
           campaign::key_index("terran11"));
    assert(e.save_game((prefix+".sav").c_str(),SaveView{}));
    Engine replay;SaveView view;
    assert(replay.load_save(argv[1],(prefix+".sav").c_str(),(prefix+"-replay").c_str(),(prefix+"-map").c_str(),view));
    assert(replay.state_digest()==e.state_digest());
    puts("Original Big Push victory and exact replay PASS");
}
