#pragma once
#include <array>
#include <string>
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
 unsigned game_type=0; // 0 melee, 1 free-for-all, 2 use map settings.
 // Native race IDs: Zerg=0, Terran=1, Protoss=2; -1 closes a slot.
 std::array<int,3> races{{1,1,-1}};
};

struct AiObserverTest {
 const char* label;
 unsigned scenario;
 int race;
 bool mixed=false;

 std::string map_path(const std::string& maps) const {
  return maps+(mixed?"/AI/three-islands.scm":"/AI/islands.scm");
 }

 MatchSetup match() const {
  MatchSetup m;
  m.enabled=m.observer=true;
  m.difficulty=AiDifficulty::medium;
  m.races={{race,race,-1}};
  if(mixed){
   m.races={{1,2,0}};
   m.teams={{0,1,1}};
   m.side_difficulty={{1,0,0}};
  }
  return m;
 }
};
constexpr std::array<AiObserverTest,4> ai_observer_tests{{
 {"Terran island AI vs AI",5,1},
 {"Protoss island AI vs AI",12,2},
 {"Zerg island AI vs AI",14,0},
 {"Medium vs 2 Easy (all races)",5,1,true},
}};
}
