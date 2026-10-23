#pragma once
#include <array>
namespace port3ds {
constexpr unsigned ai_unit_budget=56;
struct MatchSetup {
 bool enabled=false;
 unsigned game_type=0; // 0 melee, 1 free-for-all; UMS is rejected explicitly.
 // Native race IDs: Zerg=0, Terran=1, Protoss=2; -1 closes a slot.
 std::array<int,3> races{{1,1,-1}};
};
}
