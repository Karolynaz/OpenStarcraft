// Exercise the public transport commands on a privately supplied campaign CHK.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

using namespace port3ds;
using namespace bwgame;

int main(int argc, char** argv) {
    assert(argc == 3);
    Engine engine;
    assert(engine.load(argv[1], argv[2], Scenario::bw_protoss_campaign_4));

    constexpr int destination_x = 740, destination_y = 3800;
    const auto unload = option_code(OptionKind::order, unsigned(Orders::MoveUnload));
    bool ordered = false, flew_with_cargo = false, landed = false;
    unsigned shuttle_id = 0;
    int departure_x = 0, departure_y = 0;
    for (unsigned frame = 0; frame < 480 && !landed; ++frame) {
        assert(engine.tick());
        if (frame % 12) continue;
        UnitView units[1700], shuttle{};
        const auto count = engine.visible_units(units, 1700);
        for (std::size_t i = 0; i < count; ++i) {
            const auto& unit = units[i];
            if (unit.owner != 0 || unit.type != unsigned(UnitTypes::Protoss_Shuttle)) continue;
            if ((!ordered && unit.x < 690) ||
                (ordered && engine.own_unit_at(unit.x, unit.y) == shuttle_id)) {
                shuttle = unit;
                break;
            }
        }
        if (!shuttle.hp) continue;

        if (!ordered && shuttle.cargo < 2) {
            assert(engine.select(shuttle.x, shuttle.y));
            if (!shuttle.cargo) assert(!engine.perform(unload, destination_x, destination_y));
            for (std::size_t i = 0; i < count; ++i) {
                const auto& unit = units[i];
                if (unit.owner == 0 && unit.type == unsigned(UnitTypes::Protoss_Dragoon) &&
                    engine.select(unit.x, unit.y))
                    engine.command(Command::context, shuttle.x, shuttle.y);
            }
        } else if (!ordered && engine.select(shuttle.x, shuttle.y)) {
            MenuOption options[9];
            const auto total = engine.options(OptionPage::primary, options, 9);
            bool offered = false;
            for (unsigned i = 0; i < total; ++i)
                if (options[i].code == unload) offered = options[i].target && options[i].enabled;
            assert(offered);
            shuttle_id = engine.own_unit_at(shuttle.x, shuttle.y);
            departure_x = shuttle.x;
            departure_y = shuttle.y;
            ordered = engine.perform(unload, destination_x, destination_y);
            assert(ordered);
        } else if (ordered) {
            if (shuttle.cargo && std::abs(shuttle.x-departure_x) + std::abs(shuttle.y-departure_y) > 160)
                flew_with_cargo = true;
            landed = !shuttle.cargo &&
                std::abs(shuttle.x-destination_x) + std::abs(shuttle.y-destination_y) < 45;
        }
    }
    assert(ordered && flew_with_cargo && landed);
    std::printf("Campaign Shuttle accepts a ground target and unloads at frame %u PASS\n", engine.frame());
}
