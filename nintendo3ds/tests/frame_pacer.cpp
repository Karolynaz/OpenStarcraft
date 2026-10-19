#include "frame_pacer.h"
#include <cassert>
#include <cstdio>
int main() {
    port3ds::FramePacer pacer;
    assert(pacer.needs_sync(100,100));pacer.began(101,101);
    assert(pacer.needs_sync(101,101)); // A fast frame must wait.
    assert(pacer.needs_sync(102,101)); // Both displays must advance.
    assert(!pacer.needs_sync(102,102)); // Work already consumed a refresh.
    pacer.began(102,102);assert(pacer.needs_sync(102,102));
    assert(!pacer.needs_sync(106,106)); // A slow frame must not add another wait.
    pacer.began(0xffffffffu,0xffffffffu);assert(!pacer.needs_sync(0,0));
    // Simulate busy/idle CPU work: only one submission per observed pair of
    // display refreshes, including work spanning several display intervals.
    unsigned submissions=0,previous=0;
    port3ds::FramePacer model;model.began(0,0);
    for(unsigned time=0;time<1200;++time) {
        unsigned refresh=time/20;
        if(!model.needs_sync(refresh,refresh)) {
            assert(refresh>previous);previous=refresh;
            ++submissions;model.began(refresh,refresh);
            if(submissions%3==0)time+=43;
        }
    }
    assert(submissions>20 && submissions<60);
    std::puts("Display pacing / fast-frame cap / missed refresh / dual-screen / wrap PASS");
}
