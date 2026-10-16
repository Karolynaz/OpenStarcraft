#include "input.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace port3ds;
    InputRouter r; InputFrame f;
    f.l=true; assert(r.read(f).action==InputAction::none);
    f.x=true; assert(r.read(f).action==InputAction::workers);
    f.l=false; assert(r.read(f).action==InputAction::army);
    f.x=false;f.y=true; assert(r.read(f).action==InputAction::primary);
    f.l=true;assert(r.read(f).action==InputAction::secondary);
    f={};f.r=true;assert(r.read(f).action==InputAction::none);
    f.direction=2;f.time=1000000;auto e=r.read(f);
    assert(e.action==InputAction::store && e.slot==2);
    f.r=false;f.time+=100000;e=r.read(f);assert(e.action==InputAction::recall && !e.center);
    f.time+=300000;e=r.read(f);assert(e.center);
    f.time+=400000;assert(!r.read(f).center);
    f.direction=0;f.time+=100000;assert(!r.read(f).center);
    assert(r.read(f,true).action==InputAction::previous);
    f.direction=2;assert(r.read(f,true).action==InputAction::next);
    f={};f.a=true;assert(r.read(f).action==InputAction::confirm);
    f={};f.b=true;assert(r.read(f).action==InputAction::cancel);
    std::puts("A/B / modifier chords / group store-recall / 350 ms double tap / menu navigation PASS");
}
