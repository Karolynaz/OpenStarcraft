#pragma once
#include "engine.h"
#include "protoss_catalogue.h"
#include "zerg_catalogue.h"
#include <functional>
#include <cstdio>
#include <stdexcept>
inline void battle_test(const char* data,const char* map,
                        const std::function<void(port3ds::Engine&)>& observe={},
                        port3ds::Scenario scenario=port3ds::Scenario::ai_battle) {
    using namespace port3ds;
    const bool protoss=scenario==Scenario::protoss_ai_battle;
    const bool zerg=scenario==Scenario::zerg_ai_battle;
    Engine engine;
    if(!engine.load(data,map,scenario)) throw std::runtime_error(engine.error());
    if(protoss) {
        const auto initial=engine.snapshot();
        for(unsigned p=0;p<2;++p)if(initial.players[p].workers!=4 || initial.players[p].army || initial.players[p].buildings!=1 || initial.players[p].minerals!=50 || initial.players[p].gas || engine.count_units(p,unsigned(bwgame::UnitTypes::Protoss_Nexus))!=1 || engine.count_units(p,unsigned(bwgame::UnitTypes::Protoss_Pylon),false))
            throw std::runtime_error("Protoss opening must start with four Probes, one Nexus, fifty minerals and no free Pylons/army");
    }
    if(zerg) {
        const auto initial=engine.snapshot();
        for(unsigned p=0;p<2;++p)if(initial.players[p].workers!=4 || initial.players[p].buildings!=1 || initial.players[p].minerals!=50 || initial.players[p].gas || engine.count_units(p,unsigned(bwgame::UnitTypes::Zerg_Hatchery))!=1 || engine.count_units(p,unsigned(bwgame::UnitTypes::Zerg_Overlord))!=1 || engine.count_units(p,unsigned(bwgame::UnitTypes::Zerg_Zergling),false))
            throw std::runtime_error("Zerg opening must start with four Drones, Hatchery, Overlord, fifty minerals and no free army");
    }
    UnitView bases[2]{};UnitView start[128]; const auto count=engine.visible_units(start,128);
    for(unsigned i=0;i<count;++i) if(start[i].owner<2 && start[i].max_hp==(zerg?1250:protoss?750:1500)) {
        bases[start[i].owner]=start[i];
        if(!engine.select(start[i].x,start[i].y) || !engine.store_group(start[i].owner)) throw std::runtime_error("Observer bookmarks");
    }
    if(engine.command(Command::train)) throw std::runtime_error("Observer must not command AI armies");
    unsigned peak=0; bool damage=false;bool dragoons[2]={false,false},gas[2]={false,false}; unsigned frame=0;
    for(;frame!=36000;++frame) {
        if(!engine.tick()) throw std::runtime_error(engine.error());
        if(observe) observe(engine);
        if(frame%24) continue;
        const auto s=engine.snapshot();
        for(const auto& p:s.players) if(p.live>40) throw std::runtime_error("AI exceeds controller unit budget");
        if(s.units>peak) peak=s.units;
        if(protoss || zerg)for(unsigned p=0;p<2;++p){gas[p]=gas[p] || s.players[p].gas>0;dragoons[p]=dragoons[p] || engine.count_units(p,unsigned(zerg?bwgame::UnitTypes::Zerg_Hydralisk:bwgame::UnitTypes::Protoss_Dragoon))>0;}
        UnitView units[128];const auto n=engine.visible_units(units,128);
        for(unsigned i=0;i<n;++i) if(units[i].owner<2 && units[i].completed && units[i].hp<units[i].max_hp) damage=true;
        if(frame%2400==0) std::printf("AI frame=%u Red W%u A%u B%u M%d lost%u waves%u / Blue W%u A%u B%u M%d lost%u waves%u\n",
            s.frame,s.players[0].workers,s.players[0].army,s.players[0].buildings,s.players[0].minerals,s.players[0].lost,s.players[0].waves,
            s.players[1].workers,s.players[1].army,s.players[1].buildings,s.players[1].minerals,s.players[1].lost,s.players[1].waves);
        if(s.winner>=0) break;
    }
    // Observe delayed death scripts, even after the AI declares a winner.
    for(unsigned n=0;n!=240;++n) { if(!engine.tick()) throw std::runtime_error(engine.error()); if(observe) observe(engine); }
    const auto s=engine.snapshot();
    if(s.winner<0) throw std::runtime_error("AI battle did not finish");
    if(s.winner<2 && engine.recall_group(1-s.winner)) throw std::runtime_error("Dead bookmark must not select a recycled unit");
    for(unsigned i=0;i<2;++i) if(!s.players[i].ai || s.players[i].trained<6 || s.players[i].built<2 || !s.players[i].waves)
        throw std::runtime_error("Both AIs must gather, construct, train and launch waves");
    if(scenario==Scenario::gas_battle && (!s.players[0].gas || !s.players[1].gas)) throw std::runtime_error("Both AIs must harvest real gas");
    if(protoss || zerg)std::printf("Economy gas observed %u/%u, completed ranged unit observed %u/%u\n",gas[0],gas[1],dragoons[0],dragoons[1]);
    if((protoss || zerg) && (!gas[0] || !gas[1] || !dragoons[0] || !dragoons[1]))throw std::runtime_error("Both AIs must gather gas and complete native ranged units");
    if(!damage || !s.players[0].lost || !s.players[1].lost) throw std::runtime_error("AI battle did not produce damage and losses on both sides");
    std::printf("AI vs AI PASS: frame=%u peak_objects=%u winner=%d losses=%u/%u\n",s.frame,peak,s.winner,s.players[0].lost,s.players[1].lost);
    if(!engine.load(data,map,zerg?Scenario::zerg_versus_ai:protoss?Scenario::protoss_versus_ai:Scenario::versus_ai)) throw std::runtime_error(engine.error());
    for(unsigned n=0;n!=2400;++n) { if(!engine.tick()) throw std::runtime_error(engine.error()); }
    const auto manual=engine.snapshot();
    if(manual.observer || manual.players[0].ai || !manual.players[1].ai || manual.players[0].workers!=4 || manual.players[0].minerals!=50 || manual.players[1].workers<6)
        throw std::runtime_error("Player vs AI ownership separation");
    if(protoss) {
        UnitView units[128];const auto n=engine.visible_units(units,128);bool selected=false;
        for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==unsigned(bwgame::UnitTypes::Protoss_Nexus))selected=engine.select(units[i].x,units[i].y);
        if(!selected || !engine.perform(option_code(OptionKind::train,unsigned(bwgame::UnitTypes::Protoss_Probe))) || engine.snapshot().minerals!=0)
            throw std::runtime_error("Manual Nexus must accept paid Probe production");
        for(unsigned i=0;i<600;++i)if(!engine.tick())throw std::runtime_error(engine.error());
        if(engine.count_units(0,unsigned(bwgame::UnitTypes::Protoss_Probe))!=5)throw std::runtime_error("Manual Probe production completes");
    }
    if(zerg) {
        UnitView units[128];const auto n=engine.visible_units(units,128);bool selected=false;
        for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==unsigned(bwgame::UnitTypes::Zerg_Larva))selected=engine.select(units[i].x,units[i].y);
        if(!selected || !engine.perform(option_code(OptionKind::morph,unsigned(bwgame::UnitTypes::Zerg_Drone))) || engine.snapshot().minerals!=0)
            throw std::runtime_error("Manual Larva must accept paid Drone morph");
        for(unsigned i=0;i<600;++i)if(!engine.tick())throw std::runtime_error(engine.error());
        if(engine.count_units(0,unsigned(bwgame::UnitTypes::Zerg_Drone))!=5)throw std::runtime_error("Manual Drone morph completes");
        auto visible=engine.visible_units(units,128);UnitView drone{};for(unsigned i=0;i<visible;++i)if(units[i].selected)drone=units[i];
        if(!drone.worker || !drone.completed || !engine.activate(bases[1].x,bases[1].y))throw std::runtime_error("New Drone must accept contextual scouting");
        for(unsigned i=0;i<120;++i)if(!engine.tick())throw std::runtime_error(engine.error());
        visible=engine.visible_units(units,128);UnitView moved{};for(unsigned i=0;i<visible;++i)if(units[i].selected)moved=units[i];
        std::printf("Paid Drone contextual scout (%d,%d) -> (%d,%d), enemy base (%d,%d), order %u\n",drone.x,drone.y,moved.x,moved.y,bases[1].x,bases[1].y,moved.order);
        if(!moved.worker || std::abs(moved.x-drone.x)+std::abs(moved.y-drone.y)<100)throw std::runtime_error("Paid Drone must actually move after A scouting order");
    }
    std::puts("Observer order isolation / dead bookmark pruning / manual player vs AI PASS");
}
