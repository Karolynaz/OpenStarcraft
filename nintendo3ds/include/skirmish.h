#pragma once
#include <array>
namespace port3ds {
struct MatchSetup {
 bool enabled=false;
 // Native race IDs: Zerg=0, Terran=1, Protoss=2; -1 closes a slot.
 std::array<int,3> races{{1,1,-1}};
};
}
