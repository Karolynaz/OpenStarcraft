// Use a purchased map's terrain and native data to exercise both hero hangars.
#include "actions.h"
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace bwgame;

int main(int argc, char** argv) {
    assert(argc == 3);
    game_player player(data_loading::data_files_directory(argv[1]));
    std::ifstream input(argv[2], std::ios::binary);
    a_vector<uint8_t> map((std::istreambuf_iterator<char>(input)), {});
    game_load_functions loader(player.st());
    loader.load_map_data(map.data(), map.size(), [&] {
        loader.setup_info.create_no_units = true;
        auto& human = player.st().players[0];
        human.race = race_t::protoss;
        human.controller = player_t::controller_occupied;
    });
    auto& state = player.st();
    state.game->triggers.clear();
    for (auto& triggers : state.running_triggers) triggers.clear();
    state.current_minerals[0] = 1000;
    auto& engine = player.funcs();
    action_state actions;
    action_functions commands(state, actions);

    for (const auto hero_type : {UnitTypes::Hero_Gantrithor, UnitTypes::Hero_Warbringer}) {
        const bool carrier = hero_type == UnitTypes::Hero_Gantrithor;
        const xy pos = carrier ? xy{520, 520} : xy{700, 700};
        auto* hero = engine.trigger_create_unit(engine.get_unit_type(hero_type), pos, 0);
        assert(hero);
        auto& inside = carrier ? hero->carrier.inside_units : hero->reaver.inside_units;
        auto& outside = carrier ? hero->carrier.outside_units : hero->reaver.outside_units;
        const auto& inside_count = carrier ? hero->carrier.inside_count : hero->reaver.inside_count;
        const auto& outside_count = carrier ? hero->carrier.outside_count : hero->reaver.outside_count;
        commands.action_select(0, hero);

        for (bool deployed : {false, true}) {
            assert(commands.action_train_fighter(0));
            for (unsigned frame = 0; frame < 1000 && !inside_count; ++frame) engine.next_frame();
            assert(inside_count == 1 && !inside.empty());
            auto* fighter = deployed ? engine.release_fighter(hero) : &inside.front();
            assert(fighter && fighter->fighter.parent == hero);
            assert(inside_count == (deployed ? 0 : 1) && outside_count == (deployed ? 1 : 0));
            engine.kill_unit(fighter);
            for (unsigned frame = 0; frame < 32; ++frame) engine.next_frame();
            assert(inside_count == 0 && inside.empty());
            assert(outside_count == 0 && outside.empty());
        }
        assert(commands.action_train_fighter(0));
        for (unsigned frame = 0; frame < 1000 && !inside_count; ++frame) engine.next_frame();
        assert(inside_count == 1 && !inside.empty());
    }

    puts("Gantrithor and Warbringer: train, destroy inside/outside fighters, then refill PASS");
}
