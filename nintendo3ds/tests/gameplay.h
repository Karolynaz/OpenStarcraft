#pragma once
#include "engine.h"
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <functional>

// Exercise only the public adapter used by the physical buttons. No resource,
// unit, health, visibility or build-time cheats are applied to engine state.
inline void gameplay_tests(const char* data, const char* map,
                           const std::function<void(port3ds::Engine&)>& observe = {}) {
    using namespace port3ds;
    Engine engine;
    auto require = [&](bool ok, const char* text) {
        if (!ok) { std::fprintf(stderr, "%s: %s\n", text, engine.command_status()); throw std::runtime_error(text); }
    };
    auto units = [&]() {
        std::vector<UnitView> u(128); u.resize(engine.visible_units(u.data(),u.size())); return u;
    };
    auto tick = [&](unsigned n) { while (n--) { require(engine.tick(), "simulation tick"); if (observe) observe(engine); } };
    require(engine.load(data,map,Scenario::melee), "melee load");
    tick(24);
    UnitView base{}, worker{};
    std::vector<UnitView> workers;
    for (const auto& u : units()) if (u.owner == 0) {
        if (u.worker) workers.push_back(u);
        else base = u;
    }
    require(workers.size() == 4 && base.max_hp == 1500, "starting Terran base");
    require(engine.select_workers()==4,"L+X selects all starting workers");
    require(engine.store_group(2),"R+Down stores worker group");
    engine.clear_selection();
    int group_x=0,group_y=0;
    require(engine.recall_group(2,&group_x,&group_y) && engine.snapshot().selected==4,"recall worker group");
    require(group_x>0 && group_y>0,"group camera center");
    require(engine.select(base.x + 35,base.y), "building edge selection");
    require(engine.command(Command::train), "train SCV");
    require(engine.snapshot().minerals == 0 && engine.snapshot().queue == 1, "training charges minerals");
    require(!engine.command(Command::train), "reject unaffordable training");
    require(engine.command(Command::cancel), "cancel queued SCV");
    require(engine.snapshot().minerals == 50 && engine.snapshot().queue == 0, "training cancellation refund");
    require(engine.command(Command::train), "train SCV again");
    for (const auto& w : workers) {
        UnitView closest{}; int distance = 0x7fffffff;
        for (const auto& m : units()) if (m.mineral) {
            const int d=(m.x-w.x)*(m.x-w.x)+(m.y-w.y)*(m.y-w.y);
            if (d < distance) { distance=d; closest=m; }
        }
        require(engine.select(w.x,w.y), "select worker");
        require(engine.command(Command::context,closest.x,closest.y), "gather order");
    }
    tick(2400);
    unsigned own_workers = 0;
    for (const auto& u : units()) if (u.owner == 0 && u.worker) { ++own_workers; worker=u; }
    require(own_workers == 5, "SCV training completes");
    require(engine.snapshot().minerals >= 250, "mineral cargo returned to base");
    std::printf("Gather/train PASS: minerals=%d workers=%u\n",engine.snapshot().minerals,own_workers);
    auto build = [&](Command kind, unsigned hp) {
        require(engine.select(worker.x,worker.y), "select builder");
        const int minerals=engine.snapshot().minerals;
        require(!engine.command(kind,-64,-64), "reject off-map construction");
        require(engine.snapshot().minerals == minerals, "invalid placement costs nothing");
        bool placed=false;
        for (int radius=96;radius<=384 && !placed;radius+=32)
            for (int dy=-radius;dy<=radius && !placed;dy+=32)
                for (int dx=-radius;dx<=radius && !placed;dx+=32) {
                    if (std::abs(dx)!=radius && std::abs(dy)!=radius) continue;
                    placed=engine.command(kind,base.x+dx,base.y+dy);
                }
        require(placed, "find legal nearby building site");
        for (unsigned n=0;n!=3600;++n) {
            tick(1);
            if (n%24 == 0) for (const auto& u : units())
                if (u.owner==0 && u.max_hp==static_cast<int>(hp) && u.completed) return u;
        }
        throw std::runtime_error("Construction did not complete");
    };
    const int supply=engine.snapshot().supply_max;
    build(Command::depot,500);
    require(engine.snapshot().supply_max == supply+8, "depot provides eight supply");
    // The builder's current location changes during construction.
    for (const auto& u : units()) if (u.selected) worker=u;
    auto barracks=build(Command::barracks,1000);
    require(engine.select(barracks.x,barracks.y), "select barracks");
    require(engine.command(Command::train), "train Marine");
    tick(1200);
    bool marine=false;
    for (const auto& u : units()) if (u.owner==0 && u.max_hp==40 && u.completed) marine=true;
    require(marine,"Marine training completes");
    std::puts("Depot / supply / Barracks / Marine production PASS");

    // Produce a group larger than the native twelve-unit action selection.
    // This checks real batching, not a test-only spawn or resource shortcut.
    unsigned army=1;
    for(unsigned n=0;n!=16000 && army<13;++n) {
        if(n%24==0) {
            army=0;
            for(const auto& u:units()) if(u.owner==0 && u.max_hp==40 && u.completed) ++army;
            if(army<13 && engine.snapshot().queue==0) engine.command(Command::train);
        }
        tick(1);
    }
    require(army==13,"produce thirteen Marines");
    int min_x=8192,min_y=8192,max_x=0,max_y=0;
    for(const auto& u:units()) if(u.owner==0 && u.max_hp==40 && u.completed) {
        min_x=std::min(min_x,u.x);min_y=std::min(min_y,u.y);max_x=std::max(max_x,u.x);max_y=std::max(max_y,u.y);
    }
    require(max_x-min_x<400 && max_y-min_y<240,"army fits on screen");
    require(engine.select_army(min_x,min_y)==13,"X selects all thirteen Marines");
    require(engine.store_group(0),"store military group");
    require(engine.command(Command::context,min_x+160,min_y),"batched group context command");
    tick(240);
    engine.clear_selection();
    require(engine.recall_group(0) && engine.snapshot().selected==13,"recall military group after movement");
    std::puts("Worker/military selection / bookmarks / thirteen-unit batched orders PASS");

    require(engine.load(data,map,Scenario::two_units),"combat reload");
    require(!engine.recall_group(0),"reload clears bookmarks");
    tick(24);
    UnitView own{}, enemy{};
    for (const auto& u : units()) (u.owner==0 ? own : enemy)=u;
    require(engine.select(own.x,own.y),"select Marine");
    require(!engine.command(Command::depot,256,256),"Marine cannot build");
    require(engine.command(Command::attack,own.x+96,own.y),"attack-move");
    tick(100);
    require(engine.command(Command::stop),"stop");
    require(engine.command(Command::context,enemy.x,enemy.y),"target enemy");
    bool damaged=false, killed=false;
    for (unsigned n=0;n!=24000;++n) {
        tick(1);
        if (n%24==0) {
            auto current=units();
            for (const auto& u : current) if (u.hp < u.max_hp) damaged=true;
            if (current.size()<2) { killed=true; break; }
        }
    }
    if (!damaged || !killed) for (const auto& u : units())
        std::printf("Combat remaining owner=%u at %d,%d HP=%d/%d\n",u.owner,u.x,u.y,u.hp,u.max_hp);
    require(damaged && killed,"combat deals damage and kills");
    // Death iscripts can spawn further images after the unit leaves the live
    // list. Observe their full aftermath when preparing the graphics profile.
    tick(240);
    std::puts("Attack-move / stop / contextual attack / damage / death PASS");
}
