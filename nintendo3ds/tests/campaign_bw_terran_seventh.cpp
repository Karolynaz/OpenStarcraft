#include "engine.h"
#include "bwenums.h"
#include "campaign_map.h"
#include "sound_ids.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <unistd.h>

using namespace port3ds;
using namespace bwgame;

static void test_opening(const char* data_root, const char* map_path) {
    Engine engine;
    assert(engine.load(data_root, map_path, Scenario::bw_terran_campaign_7));
    assert(engine.fog_enabled());

    constexpr unsigned duran = unsigned(UnitTypes::Hero_Samir_Duran);
    constexpr unsigned medic = unsigned(UnitTypes::Terran_Medic);
    constexpr unsigned marine = unsigned(UnitTypes::Terran_Marine);
    assert(engine.count_units(0, duran) == 0);

    unsigned dialogue_events = 0;
    for (unsigned frame = 0; frame < 960; ++frame) {
        assert(engine.tick());
        SoundEvent sounds[128];
        const auto count = engine.drain_sounds(sounds, 128);
        if (count && !engine.snapshot().transmission.empty())
            ++dialogue_events;
    }

    assert(dialogue_events >= 3);
    assert(engine.snapshot().winner == -1);
    assert(engine.count_units(0, duran) == 1);
    assert(engine.count_units(0, medic) == 2);
    assert(engine.count_units(0, marine) > 0);

    UnitView units[1700];
    const auto count = engine.visible_units(units, 1700);
    for (std::size_t i = 0; i < count; ++i) {
        const auto& unit = units[i];
        if (unit.owner == 0 && unit.type == duran) {
            assert(engine.select(unit.x, unit.y));
            assert(engine.command(Command::context, 348, 284));
            puts("Patriot's Blood: original intro combat, sound events and playable Duran handover PASS");
            return;
        }
    }
    assert(false && "Duran must be visible and selectable after the handover");
}

static void test_override_terminal(const char* data_root, const char* map_path) {
    std::ifstream original(map_path, std::ios::binary);
    std::vector<uint8_t> map((std::istreambuf_iterator<char>(original)), {});
    assert(!map.empty());

    // Exercise the authored ending in isolation by adding one Marine
    // just outside the terminal location. All trigger records stay intact.
    bool added = false;
    for (std::size_t offset = 0; offset + 8 <= map.size();) {
        const unsigned size = campaign::get(map.data() + offset + 4);
        const auto section = offset + 8;
        assert(size <= map.size() - section);
        if (std::memcmp(map.data() + offset, "UNIT", 4) == 0) {
            assert(size % 36 == 0);
            std::array<uint8_t, 36> marine{};
            campaign::put(marine.data() + 4, (3076u << 16) | 3892u);
            marine[17] = marine[18] = marine[19] = 100;
            map.insert(map.begin() + section + size, marine.begin(), marine.end());
            campaign::put(map.data() + offset + 4, size + marine.size());
            added = true;
            break;
        }
        offset = section + size;
    }
    assert(added);

    char temporary_directory[] = "/tmp/sc3ds-patriots-blood-XXXXXX";
    assert(mkdtemp(temporary_directory));
    const std::string fixture = std::string(temporary_directory) + "/override.chk";
    {
        std::ofstream out(fixture, std::ios::binary);
        out.write(reinterpret_cast<const char*>(map.data()), map.size());
        assert(out);
    }

    Engine engine;
    const bool loaded = engine.load(data_root, fixture.c_str(), Scenario::bw_terran_campaign_7);
    assert(std::remove(fixture.c_str()) == 0);
    assert(rmdir(temporary_directory) == 0);
    assert(loaded && engine.fog_enabled());
    for (unsigned i = 0; i < 32; ++i) assert(engine.tick());
    assert(engine.snapshot().winner == -1);
    assert(engine.select(3892, 3076));
    assert(engine.command(Command::context, 3892, 3012));

    constexpr unsigned shutdown_voice = campaign_sound_base + 457;
    constexpr unsigned orders_voice = campaign_sound_base + 24;
    bool heard_shutdown = false;
    bool heard_orders = false;
    while (engine.snapshot().winner < 0 && engine.frame() < 1500) {
        assert(engine.tick());
        SoundEvent sounds[128];
        const auto count = engine.drain_sounds(sounds, 128);
        for (std::size_t i = 0; i < count; ++i) {
            if (sounds[i].id == shutdown_voice) heard_shutdown = true;
            if (sounds[i].id == orders_voice) {
                assert(heard_shutdown);
                heard_orders = true;
            }
        }
    }
    assert(engine.snapshot().winner == 0 && heard_shutdown && heard_orders);
    std::printf("Patriot's Blood: original override, ending voice events and victory at frame %u PASS (added Marine fixture)\n",
                engine.frame());
}

int main(int argc, char** argv) {
    assert(argc == 3);
    test_opening(argv[1], argv[2]);
    test_override_terminal(argv[1], argv[2]);
}
