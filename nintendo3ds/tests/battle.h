#pragma once
#include "engine.h"
#include <functional>
#include <cstdio>
#include <stdexcept>
inline void battle_test(const char* data,const char* map,
                        const std::function<void(port3ds::Engine&)>& observe={}) {
    using namespace port3ds;
    Engine engine;
    if(!engine.load(data,map,Scenario::ai_battle)) throw std::runtime_error(engine.error());
    UnitView start[128]; const auto count=engine.visible_units(start,128);
    for(unsigned i=0;i<count;++i) if(start[i].owner<2 && start[i].max_hp==1500) {
        if(!engine.select(start[i].x,start[i].y) || !engine.store_group(start[i].owner)) throw std::runtime_error("Observer bookmarks");
    }
    if(engine.command(Command::train)) throw std::runtime_error("Observer must not command AI armies");
    unsigned peak=0; bool damage=false; unsigned frame=0;
    for(;frame!=36000;++frame) {
        if(!engine.tick()) throw std::runtime_error(engine.error());
        if(observe) observe(engine);
        if(frame%24) continue;
        const auto s=engine.snapshot();
        for(const auto& p:s.players) if(p.live>40) throw std::runtime_error("AI exceeds controller unit budget");
        if(s.units>peak) peak=s.units;
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
    for(const auto& p:s.players) if(!p.ai || p.trained<6 || p.built<2 || !p.waves)
        throw std::runtime_error("Both AIs must gather, construct, train and launch waves");
    if(!damage || !s.players[0].lost || !s.players[1].lost) throw std::runtime_error("AI battle did not produce damage and losses on both sides");
    std::printf("AI vs AI PASS: frame=%u peak_objects=%u winner=%d losses=%u/%u\n",s.frame,peak,s.winner,s.players[0].lost,s.players[1].lost);
    if(!engine.load(data,map,Scenario::versus_ai)) throw std::runtime_error(engine.error());
    for(unsigned n=0;n!=2400;++n) { if(!engine.tick()) throw std::runtime_error(engine.error()); }
    const auto manual=engine.snapshot();
    if(manual.observer || manual.players[0].ai || !manual.players[1].ai || manual.players[0].workers!=4 || manual.players[0].minerals!=50 || manual.players[1].workers<6)
        throw std::runtime_error("Player vs AI ownership separation");
    std::puts("Observer order isolation / dead bookmark pruning / manual player vs AI PASS");
}
