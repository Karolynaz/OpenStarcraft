#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>

using namespace port3ds;
using namespace bwgame;

int main(int argc, char** argv) {
    assert(argc == 3);
    Engine engine;
    assert(engine.load(argv[1], argv[2], Scenario::bw_terran_campaign_8));
    assert(engine.fog_enabled());

    constexpr unsigned terran = 0;
    constexpr unsigned brown_zerg = 5;
    constexpr unsigned bunker = unsigned(UnitTypes::Terran_Bunker);
    constexpr unsigned command_center = unsigned(UnitTypes::Terran_Command_Center);
    constexpr unsigned beacon = unsigned(UnitTypes::Special_Zerg_Beacon);
    constexpr unsigned torrasque = unsigned(UnitTypes::Hero_Torrasque);
    constexpr unsigned spawn_time = 15*60*24;
    bool saw_garrison = false;
    unsigned first_torrasque = 0;
    for (unsigned frame = 0; frame < 30000 && engine.snapshot().winner < 0; ++frame) {
        assert(engine.tick());
        SoundEvent sounds[128];
        engine.drain_sounds(sounds, 128);
        if (engine.frame() == 2) {
            assert(engine.snapshot().minerals == 1500 && engine.snapshot().gas == 300);
            assert(!engine.snapshot().objectives.empty());
            assert(engine.count_units(7, beacon) == 4);
        }
        if (!saw_garrison && frame < 120 && frame % 8 == 0) {
            UnitView units[1700];
            const auto count = engine.visible_units(units, 1700);
            for (std::size_t i = 0; i < count; ++i)
                if (units[i].owner == terran && units[i].type == bunker && units[i].cargo)
                    saw_garrison = true;
        }
        if (!first_torrasque && engine.count_units(brown_zerg, torrasque))
            first_torrasque = engine.frame();
    }

    assert(saw_garrison);
    assert(first_torrasque >= spawn_time && first_torrasque < spawn_time+60);
    assert(engine.snapshot().winner == 1);
    assert(engine.count_units(terran, command_center) == 0);
    assert(engine.count_units(terran, bunker) == 0);
    printf("To Chain the Beast: authored garrison, Torrasque at frame %u and base-loss defeat at frame %u PASS\n",
           first_torrasque, engine.frame());
}
