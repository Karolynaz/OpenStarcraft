#include "bwenums.h"
#include "engine.h"
#include "sound_ids.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace port3ds;
using bwgame::UnitTypes;

static UnitView zeratul_view(const Engine& engine) {
    UnitView units[1700];
    const auto count=engine.visible_units(units,1700);
    for(unsigned i=0;i<count;++i)
        if(units[i].owner==0 && units[i].type==unsigned(UnitTypes::Hero_Zeratul))
            return units[i];
    return {};
}

static void verify_final_transmission(const char* data_root, const char* checkpoint,
                                      const std::string& prefix) {
    Engine engine;
    SaveView view;
    assert(engine.load_save(data_root,checkpoint,(prefix+".final.resume").c_str(),
                            (prefix+".final.chk").c_str(),view));
    const auto scene=engine.snapshot();
    assert(engine.fog_enabled() && scene.winner==-1);
    assert(scene.mission_camera_x==2640 && scene.mission_camera_y==2848);
    const auto zeratul=zeratul_view(engine);
    assert(zeratul.hp>0);
    unsigned voices=0;
    while(engine.frame()<24000 && engine.snapshot().winner==-1) {
        assert(engine.tick());
        const auto current=zeratul_view(engine);
        assert(current.hp==zeratul.hp && current.shields==zeratul.shields);
        assert(current.x==zeratul.x && current.y==zeratul.y);
        SoundEvent sounds[128];
        const auto count=engine.drain_sounds(sounds,128);
        for(unsigned i=0;i<count;++i)
            if(sounds[i].id>=campaign_sound_base)++voices;
    }
    assert(voices>0 && engine.snapshot().winner==0);
    assert(engine.snapshot().next_scenario=="Zerg10.scx");
    std::printf("Dark Origin final transmission advanced to Omega at frame %u, %u voice cues PASS\n",
                engine.frame(),voices);
}

int main(int argc, char** argv) {
    assert(argc==4 || argc==5);
    const std::string prefix=argv[3];
    const auto save=prefix+".ob3";
    const unsigned zeratul=unsigned(UnitTypes::Hero_Zeratul);
    const unsigned dark_archon=unsigned(UnitTypes::Protoss_Dark_Archon);

    Engine engine;
    assert(engine.load(argv[1],argv[2],Scenario::bw_zerg_bonus));
    assert(engine.start_recording((prefix+".journal").c_str()));
    assert(engine.fog_enabled() && engine.snapshot().human_race==2);
    assert(engine.count_units(0,zeratul)==0);

    for(unsigned i=0;i<64;++i)assert(engine.tick());
    assert(engine.save_game(save.c_str(),SaveView{}));
    Engine replay;
    SaveView view;
    assert(replay.load_save(argv[1],save.c_str(),(prefix+".resume").c_str(),
                           (prefix+".chk").c_str(),view));
    assert(replay.state_digest()==engine.state_digest());

    unsigned voices=0;
    for(unsigned i=0;i<2600 && !engine.count_units(0,zeratul);++i) {
        assert(engine.tick() && replay.tick());
        SoundEvent sounds[256];
        const auto count=engine.drain_sounds(sounds,256);
        for(unsigned j=0;j<count;++j)
            if(sounds[j].id>=campaign_sound_base)++voices;
    }
    assert(engine.count_units(0,zeratul)==1);
    assert(engine.count_units(0,dark_archon)>=1);
    assert(engine.snapshot().winner==-1 && voices);
    assert(engine.state_digest()==replay.state_digest());
    std::printf("Dark Origin handoff at frame %u, %u voice cues, exact replay PASS\n",
                engine.frame(),voices);
    if(argc==5)verify_final_transmission(argv[1],argv[4],prefix);
}
