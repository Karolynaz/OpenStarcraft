// Original Countdown map with normal fog, a paid fleet and public transport orders.

#include "engine.h"
#include "bwenums.h"
#include "sound_ids.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
using namespace port3ds;
using namespace bwgame;
struct Point { int x, y; };
const std::vector<Point> shuttle_route = {
    {608, 2720}, {2016, 2720}, {2976, 1760}, {3040, 1760}, {3104, 1696},
    {3488, 1696}, {3744, 1952}, {3744, 2336}, {3808, 2400},
};
const std::vector<Point> artanis_route = {
    {2016, 5600}, {2016, 5024}, {2592, 4448}, {2592, 4384}, {2656, 4320},
    {2656, 2080}, {2976, 1760}, {3040, 1760}, {3104, 1696}, {3488, 1696},
    {3744, 1952}, {3744, 2336}, {3808, 2400},
};
constexpr unsigned id(UnitTypes type) { return unsigned(type); }
constexpr unsigned artanis = id(UnitTypes::Hero_Artanis);
constexpr unsigned zeratul = id(UnitTypes::Hero_Zeratul);
constexpr unsigned shuttle = id(UnitTypes::Protoss_Shuttle);
constexpr unsigned probe = id(UnitTypes::Protoss_Probe);
constexpr unsigned nexus = id(UnitTypes::Protoss_Nexus);
constexpr unsigned robotics = id(UnitTypes::Protoss_Robotics_Facility);
constexpr unsigned support = id(UnitTypes::Protoss_Robotics_Support_Bay);
constexpr unsigned pylon = id(UnitTypes::Protoss_Pylon);
constexpr unsigned cannon = id(UnitTypes::Protoss_Photon_Cannon);
constexpr unsigned stargate = id(UnitTypes::Protoss_Stargate);
constexpr unsigned scout = id(UnitTypes::Protoss_Scout);
constexpr unsigned refinery = id(UnitTypes::Protoss_Assimilator);
constexpr unsigned core = id(UnitTypes::Protoss_Cybernetics_Core);
constexpr unsigned speed = unsigned(UpgradeTypes::Gravitic_Drive);
int distance(int x, int y, const UnitView& unit) {
    return std::abs(x - unit.x) + std::abs(y - unit.y);
}
bool place(Engine& game, unsigned type, const UnitView& worker, Point anchor, int max_ring) {
    if (!game.select(worker.x, worker.y)) return false;
    for (int ring = 2; ring <= max_ring; ++ring) {
        for (int y = -ring; y <= ring; ++y) for (int x = -ring; x <= ring; ++x) {
            if (std::max(std::abs(x), std::abs(y)) != ring) continue;
            const int px = anchor.x + x * 32, py = anchor.y + y * 32;
            if (game.placement_unit(type, px, py).valid &&
                game.perform(option_code(OptionKind::build, type), px, py)) return true;
        }
    }
    return false;
}
int main(int argc, char** argv) {
    assert(argc == 4);
    const std::string session = argv[3];
    Engine game;
    assert(game.load(argv[1], argv[2], Scenario::bw_protoss_campaign_8));
    assert(game.start_recording(session.c_str()) && game.fog_enabled());
    constexpr unsigned min_fleet = 8;
    Point goal{3836,2396}, temple{3728,2304}, depot{2192,5840};
    unsigned pending = 0, placed = 0, prior_count = 0, voices = 0, waypoint = 0;
    unsigned shuttle_id = 0;
    bool started = false, delivered = false, artanis_delivered = false;
    bool observed_countdown = false;
    const auto& route = shuttle_route;
    const auto& hero_route = artanis_route;
    unsigned hero_waypoint = 0;
    for (unsigned frame = 0; frame < 45000 && game.snapshot().winner < 0; ++frame) {
        if (!game.tick()) { fprintf(stderr,"engine frame%u %s\n",frame,game.error()); return 1; }
        SoundEvent events[256];
        const auto count = game.drain_sounds(events,256);
        for(unsigned i=0;i<count;++i) if(events[i].id>=campaign_sound_base) ++voices;
        const auto state = game.snapshot();
        if (state.countdown_active && !observed_countdown) {
            observed_countdown = true;
            printf("defense armed f%u seconds%u\n", game.frame(), state.countdown_seconds);
            assert(state.countdown_seconds == 900);
            fflush(stdout);
        }
        if (frame % 48) continue;
        UnitView list[1700];
        const auto n = game.visible_units(list,1700);
        std::vector<UnitView> workers, ships, army, minerals, gases, buildings;
        UnitView hero{}, flyer{}, ship{}, shrine{};
        for (unsigned i=0;i<n;++i) {
            const auto& u = list[i];
            if (u.mineral) minerals.push_back(u);
            if (u.geyser) gases.push_back(u);
            if (u.type==175) shrine=u;
            if (u.owner || !u.completed) continue;
            if (u.type == probe) workers.push_back(u);
            if (u.type == shuttle) ships.push_back(u);
            if (u.type == zeratul) hero=u;
            if (u.type == artanis) flyer=u;
            if (u.type == scout || u.type==65 || u.type==66) army.push_back(u);
            if (u.type >= 106 && u.type <= 175) buildings.push_back(u);
            if (u.type == shuttle && game.own_unit_at(u.x,u.y) == shuttle_id) ship=u;
        }
        if (frame % 1200 == 0) {
            printf("f%u cash%d,%d workers%zu scouts%u ships%zu hero%d %d,%d flyer%d %d,%d loaded%u path%u/%zu temple:%u hp=%d / %d timer:%u voices%u\n",
                   frame,state.minerals,state.gas,workers.size(),game.count_units(0,scout),ships.size(),
                   hero.hp,hero.x,hero.y,flyer.hp,flyer.x,flyer.y,ship.cargo,waypoint,route.size(),
                   game.count_units(6,175,false),shrine.hp,shrine.shields,state.countdown_seconds,voices);
            fflush(stdout);
        }
        if (pending && (frame > placed + 480 || game.count_units(0,pending,false) > prior_count)) pending=0;
        unsigned want=0;
        Point anchor=depot;
        if (!game.count_units(0,refinery,false)) want=refinery;
        else if (state.supply_max-state.supply < 8 && game.count_units(0,pylon,false) == game.count_units(0,pylon)) want=pylon;
        else if (!game.count_units(0,core,false)) want=core;
        else if (!game.count_units(0,robotics,false)) want=robotics;
        else if (!game.count_units(0,support,false)) want=support;
        else if (game.count_units(0,stargate,false)<2) want=stargate;
        if (started && game.count_units(0,cannon,false)<30 && [&](){for(const auto& w:workers) if(distance(temple.x,temple.y,w)<640)return true;return false;}()) {
            anchor=temple;
            bool powered=false;
            for (const auto& b:buildings) if(b.type==pylon && distance(temple.x,temple.y,b)<320) powered=true;
            want=powered?cannon:pylon;
        }
        const UnitView* builder=nullptr;
        for(const auto& w:workers) if (!builder || distance(anchor.x,anchor.y,w)<distance(anchor.x,anchor.y,*builder)) builder=&w;
        if (!pending && want && builder && state.minerals>=150) {
            bool built=false;
            if (want==refinery) {
                if(game.select(builder->x,builder->y)) for(const auto& g:gases)
                    if(distance(anchor.x,anchor.y,g)<720 && game.placement_unit(want,g.x,g.y).valid &&
                       game.perform(option_code(OptionKind::build,want),g.x,g.y)){built=true;break;}
            } else built=place(game,want,*builder,anchor,12);
            if(built){printf("build %u frame%u\n",want,frame);pending=want;placed=frame;prior_count=game.count_units(0,want,false);}
        }
        for(const auto& b:buildings) {
            if(!game.select(b.x,b.y) || game.selection_info().queue_count) continue;
            if(b.type==nexus && workers.size()<22) game.perform(option_code(OptionKind::train,probe));
            if(b.type==robotics && ships.size()<2) game.perform(option_code(OptionKind::train,shuttle));
            if(b.type==support && !game.upgrade_level(speed)) game.perform(option_code(OptionKind::upgrade,speed));
            if(b.type==stargate) game.perform(option_code(OptionKind::train,scout));
        }
        if(frame%240==0) {
            unsigned gas_workers=0;
            for(const auto& w: workers) {
                if(observed_countdown && distance(temple.x,temple.y,w)<520) continue;
                const UnitView* target=nullptr;
                int best=1100;
                for(const auto& m:minerals) if(distance(w.x,w.y,m)<best){target=&m;best=distance(w.x,w.y,m);}
                for(const auto& g:buildings) if(gas_workers<3 &&
                    g.type==refinery && state.gas<700 && distance(w.x,w.y,g)<500) {
                    if(game.select(w.x,w.y)) game.command(Command::context,g.x+40,g.y+16);
                    target=nullptr;
                    ++gas_workers;
                    break;
                }
                if((w.order==unsigned(Orders::PlayerGuard)||w.order==unsigned(Orders::Guard)||
                    w.order==unsigned(Orders::Stop)) && target && game.select(w.x,w.y))
                    game.command(Command::context,target->x,target->y);
            }
        }
        if(!started && !ships.empty() && (hero.hp || ship.cargo)) {
            if(!shuttle_id) {ship=ships.front();shuttle_id=game.own_unit_at(ship.x,ship.y);}
            if(!ship.hp) continue;
            if(hero.hp && !ship.cargo) { if(distance(hero.x,hero.y,ship)>160 && game.select(ship.x,ship.y)) game.move_selected(hero.x,hero.y); else if(game.select(hero.x,hero.y)) game.command(Command::context,ship.x,ship.y); }
            if(ship.cargo && game.count_units(0,scout)>=min_fleet) {
                started=true;
                const auto& builder_probe = workers.back();
                if (ship.cargo <= 6 && game.select(builder_probe.x,builder_probe.y))
                    game.command(Command::context,ship.x,ship.y);
                printf("departed f%u, %zu route points\n",frame,route.size());
                fflush(stdout);
            }
        }
        if(started && !delivered && ship.hp) {
            if(waypoint<route.size() && distance(route[waypoint].x,route[waypoint].y,ship)<100) ++waypoint;
            if(waypoint<route.size() && game.select(ship.x,ship.y)) game.move_selected(route[waypoint].x, route[waypoint].y);
            if(waypoint==route.size() && ship.cargo && game.select(ship.x,ship.y))
                game.perform(option_code(OptionKind::order,unsigned(Orders::MoveUnload)),goal.x,goal.y);
            if(waypoint==route.size() && hero.hp && game.select(hero.x,hero.y)) {
                if(distance(goal.x,goal.y,hero)<40) delivered=true;
                else game.move_selected(goal.x,goal.y);
            }
        }
        if(started && !artanis_delivered && flyer.hp && game.select(flyer.x,flyer.y)) {
            if(hero_waypoint<hero_route.size() && distance(hero_route[hero_waypoint].x,hero_route[hero_waypoint].y,flyer)<110)
                ++hero_waypoint;
            Point next=hero_waypoint<hero_route.size()?hero_route[hero_waypoint]:goal;
            game.move_selected(next.x,next.y);
            if(distance(goal.x,goal.y,flyer)<28) artanis_delivered=true;
        }
        if (started && frame % 96 == 0 && !observed_countdown) {
            for (const auto& w:workers) if (distance(goal.x,goal.y,w)<400 && game.select(w.x,w.y))
                game.move_selected(3700,2490);
        }
        if(started && frame%96==0)
            for(const auto& u:army) if(u.type==scout && game.select(u.x,u.y)) {
                game.command(Command::attack,temple.x,temple.y);
            }
    }
    printf("real Countdown outcome %d frame%u timer_observed=%d voices=%u\n",
           game.snapshot().winner,game.frame(),observed_countdown,voices);
    assert(game.snapshot().winner == 0 && observed_countdown && voices >= 7);
    const auto save = session + "-victory.ob3";
    SaveView view;
    assert(game.save_game(save.c_str(), view));
    Engine replay;
    assert(replay.load_save(argv[1], save.c_str(), (session + "-replay").c_str(),
                            (session + "-map.chk").c_str(), view));
    assert(replay.snapshot().winner == 0 && replay.state_digest() == game.state_digest());
    puts("Countdown authored arrival, countdown defense, victory and exact replay PASS");
}
