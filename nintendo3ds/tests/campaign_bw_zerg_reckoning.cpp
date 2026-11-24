#include "engine.h"
#include "sound_ids.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace port3ds;

int main(int argc, char** argv) {
    assert(argc==4);
    const std::string prefix=argv[3];
    const auto save=prefix+".ob3";
    Engine engine;
    assert(engine.load(argv[1],argv[2],Scenario::bw_zerg_campaign_9));
    assert(engine.start_recording((prefix+".journal").c_str()));
    assert(engine.fog_enabled());
    for(unsigned i=0;i<1200;++i)assert(engine.tick());
    const auto start=engine.snapshot();
    assert(start.winner==-1 && start.countdown_active);
    assert(start.countdown_seconds>1700 && start.countdown_seconds<1800);
    assert(engine.save_game(save.c_str(),SaveView{}));

    Engine replay;
    SaveView view;
    assert(replay.load_save(argv[1],save.c_str(),(prefix+".resume").c_str(),
                            (prefix+".chk").c_str(),view));
    assert(engine.state_digest()==replay.state_digest());
    assert(replay.snapshot().countdown_seconds==start.countdown_seconds);
    unsigned voices=0;
    while(engine.frame()<45000 && engine.snapshot().winner==-1) {
        assert(engine.tick() && replay.tick());
        SoundEvent sounds[128];
        const auto count=engine.drain_sounds(sounds,128);
        for(unsigned i=0;i<count;++i)if(sounds[i].id>=campaign_sound_base)++voices;
        replay.drain_sounds(sounds,128);
    }
    const auto finish=engine.snapshot();
    assert(finish.winner==1 && finish.countdown_seconds==0);
    assert(voices>0 && replay.snapshot().winner==finish.winner);
    assert(engine.state_digest()==replay.state_digest());
    std::printf("The Reckoning countdown defeat at frame %u, %u voice cues, exact replay PASS\n",
                finish.frame,voices);
}
