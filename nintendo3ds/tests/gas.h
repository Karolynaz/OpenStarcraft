#pragma once
#include "engine.h"
#include <functional>
#include <vector>
#include <stdexcept>
#include <cstdio>
inline void gas_tests(const char* data,const char* map,const std::function<void(port3ds::Engine&)>& observe={}) {
    using namespace port3ds;Engine e;
    auto require=[&](bool ok,const char* text) {if(!ok) {std::fprintf(stderr,"Gas test: %s / %s\n",text,e.command_status());throw std::runtime_error(text);}};
    auto units=[&]() {std::vector<UnitView> u(192);u.resize(e.visible_units(u.data(),u.size()));return u;};
    auto tick=[&](unsigned n) {while(n--) {require(e.tick(),"tick");if(observe) observe(e);}};
    require(e.load(data,map,Scenario::melee),"load");tick(24);
    UnitView base{},geyser{},worker{};
    for(auto u:units()) if(u.owner==0 && !u.worker) base=u;
    int best=0x7fffffff;
    for(auto u:units()) if(u.geyser) {int d=(u.x-base.x)*(u.x-base.x)+(u.y-base.y)*(u.y-base.y);if(d<best) {best=d;geyser=u;}}
    require(best<512*512,"nearby geyser");
    for(auto w:units()) if(w.owner==0 && w.worker) {
        worker=w;require(e.select(w.x,w.y),"select");
        require(!e.command(Command::refinery,geyser.x,geyser.y),"reject unaffordable refinery");
        UnitView mineral{};int distance=0x7fffffff;
        for(auto m:units()) if(m.mineral) {int d=(m.x-w.x)*(m.x-w.x)+(m.y-w.y)*(m.y-w.y);if(d<distance) {distance=d;mineral=m;}}
        require(e.command(Command::context,mineral.x,mineral.y),"mine minerals");
    }
    tick(2400);require(e.snapshot().minerals>=200,"earn construction funds");
    e.select_workers();require(e.command(Command::stop),"stop mineral income");tick(48);
    for(auto w:units()) if(w.owner==0 && w.worker) worker=w;
    auto place=[&]() {
        require(e.select(worker.x,worker.y),"select builder");
        require(!e.placement(Command::refinery,base.x,base.y).valid,"refinery requires geyser");
        const auto p=e.placement(Command::refinery,geyser.x+20,geyser.y+20);
        require(p.valid && p.x+p.width/2==geyser.x && p.y+p.height/2==geyser.y,"snap footprint to geyser");
        require(e.command(Command::refinery,geyser.x+20,geyser.y+20),"build refinery");
    };
    const auto minerals=e.snapshot().minerals;place();UnitView refinery{};
    for(unsigned n=0;n<1200;++n) {tick(1);for(auto u:units()) if(u.owner==0 && u.refinery) refinery=u;if(refinery.max_hp) break;}
    require(refinery.max_hp==750 && !refinery.completed,"normal refinery foundation");
    require(e.snapshot().minerals==minerals-100,"refinery charges 100 earned minerals");
    require(e.select(refinery.x,refinery.y) && e.command(Command::cancel),"cancel unfinished refinery");tick(24);
    require(e.snapshot().minerals==minerals-25,"75 percent cancellation refund");
    bool restored=false;for(auto u:units()) if(u.geyser && u.x==geyser.x && u.y==geyser.y) restored=true;
    require(restored,"cancellation restores neutral geyser");
    for(auto w:units()) if(w.owner==0 && w.worker) worker=w;
    place();refinery={};
    for(unsigned n=0;n<3600;++n) {tick(1);for(auto u:units()) if(u.owner==0 && u.refinery && u.completed) refinery=u;if(refinery.completed) break;}
    require(refinery.completed,"refinery completes at native build speed");
    require(!e.placement(Command::refinery,geyser.x,geyser.y).valid,"occupied geyser cannot build twice");
    for(auto w:units()) if(w.owner==0 && w.worker) worker=w;
    require(e.select(worker.x,worker.y) && e.activate(refinery.x,refinery.y),"A on own refinery assigns SCV to gas");
    require(e.snapshot().selection==SelectionKind::workers,"context keeps SCV selected");
    require(e.store_group(3),"bookmark gas worker");
    auto visible_workers=[&]() {unsigned n=0;for(auto u:units()) if(u.owner==0 && u.worker) ++n;return n;};
    bool hidden=false;
    for(unsigned n=0;n<1200;++n) {tick(1);if(visible_workers()<e.snapshot().players[0].workers) {hidden=true;break;}}
    require(hidden && !e.recall_group(3),"hidden gas worker cannot be selected");
    bool returned=false;
    for(unsigned n=0;n<1200;++n) {tick(1);if(visible_workers()==e.snapshot().players[0].workers) {returned=true;break;}}
    require(returned && e.recall_group(3) && e.snapshot().selected==1,"gas worker bookmark survives temporary hiding");
    tick(1800);require(e.snapshot().gas>0 && e.snapshot().gas%8==0,"native gas cargo delivered to command center");
    std::printf("Refinery snap / 100 mineral cost / 75 percent refund / restore geyser / contextual gas delivery / hidden-worker bookmark PASS: gas=%d\n",e.snapshot().gas);
}
