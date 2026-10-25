#pragma once
#include <array>
namespace port3ds {
constexpr unsigned ai_unit_budget=56;
enum class AiDifficulty : unsigned { easy, medium };
// Console-friendly, deterministic pacing. Native costs and sight rules apply.
struct AiProfile {
 unsigned workers,army,budget,think_frames,first_attack,wave_delay,wave_size;
};
inline AiProfile ai_profile(AiDifficulty d){return d==AiDifficulty::easy?AiProfile{8,14,42,24,24*60*6,24*60,8}:AiProfile{14,24,56,12,24*60*3,24*35,16};}
struct MatchSetup {
 bool enabled=false;
 AiDifficulty difficulty=AiDifficulty::easy;
 bool observer=false;
 std::array<unsigned,3> teams{{0,1,2}};
 // -1 follows the ordinary lobby difficulty; observer fixtures can mix levels.
 std::array<int,3> side_difficulty{{-1,-1,-1}};
 unsigned game_type=0; // 0 melee, 1 free-for-all; UMS is rejected explicitly.
 // Native race IDs: Zerg=0, Terran=1, Protoss=2; -1 closes a slot.
 std::array<int,3> races{{1,1,-1}};
};
}
