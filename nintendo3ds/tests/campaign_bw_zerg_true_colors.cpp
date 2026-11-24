#include "bwenums.h"
#include "engine.h"
#include "sound_ids.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace port3ds;
using bwgame::UnitTypes;

int main(int argc, char** argv) {
    assert(argc==4);
    const std::string session=argv[3];
    const auto save=session+".ob3";
    const auto recording=session+".journal";
    const unsigned hatchery=unsigned(UnitTypes::Zerg_Hatchery);
    const unsigned lurker=unsigned(UnitTypes::Zerg_Lurker);
    const unsigned drone=unsigned(UnitTypes::Zerg_Drone);

    Engine engine;
    assert(engine.load(argv[1],argv[2],Scenario::bw_zerg_campaign_5));
    assert(engine.start_recording(recording.c_str()));
    assert(engine.fog_enabled());
    assert(engine.count_units(0,hatchery)==0);
    assert(engine.count_units(4,lurker)==3);

    unsigned voices=0;
    unsigned handoff_frame=0;
    for(unsigned i=0;i<1800;++i) {
        assert(engine.tick());
        SoundEvent sounds[256];
        const auto count=engine.drain_sounds(sounds,256);
        for(unsigned j=0;j<count;++j)
            if(sounds[j].id>=campaign_sound_base)++voices;
        if(!handoff_frame && engine.count_units(0,hatchery)==2)
            handoff_frame=engine.frame();
    }

    const auto state=engine.snapshot();
    assert(handoff_frame && voices>=3);
    assert(state.winner==-1 && state.human_race==0 && state.scripted);
    assert(state.mission_camera_serial>=2 && !state.objectives.empty());
    assert(engine.count_units(0,hatchery)==2 && engine.count_units(0,lurker)==3);
    assert(engine.count_units(4,lurker)==0 && engine.count_units(0,drone)>=6);

    UnitView units[1700];
    const auto count=engine.visible_units(units,1700);
    const UnitView* worker=nullptr;
    const UnitView* minerals=nullptr;
    for(unsigned i=0;i<count;++i) {
        const auto& unit=units[i];
        if(unit.owner==0 && unit.type==drone && unit.y<350)worker=&unit;
        if(unit.mineral && unit.y<275 && unit.x>2100 && unit.x<2560)minerals=&unit;
    }
    assert(worker && minerals && engine.select(worker->x,worker->y));
    assert(engine.selection_info().own && engine.selection_info().type==drone);
    assert(engine.command(Command::context,minerals->x,minerals->y));
    for(unsigned i=0;i<650;++i)assert(engine.tick());
    assert(engine.snapshot().minerals_gathered>state.minerals_gathered);
    SaveView view;
    view.camera_x=state.mission_camera_x-200;
    view.camera_y=state.mission_camera_y-120;
    assert(engine.save_game(save.c_str(),view));

    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1],save.c_str(),(session+".replay").c_str(),
                           (session+".chk").c_str(),restored));
    assert(restored.camera_x==view.camera_x && restored.camera_y==view.camera_y);
    assert(replay.state_digest()==engine.state_digest());
    for(unsigned i=0;i<96;++i)assert(engine.tick() && replay.tick());
    assert(replay.state_digest()==engine.state_digest());
    std::printf("True Colors handoff frame %u, %u voice events, command and save replay PASS\n",
                handoff_frame,voices);
}
