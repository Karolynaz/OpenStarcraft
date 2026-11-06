#include "fake3ds/ui_text.h"
#include "result_draw.h"
#include "pause_menu.h"
#include <cassert>
#include <cstdio>
namespace fake {
std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;
}
int main(int argc,char**argv){
 assert(argc==2);using namespace port3ds;
 assert(PauseMenu::count(PauseMenu::Page::result)==1);
 assert(ResultScreen::hit(160,218)==0 && ResultScreen::hit(20,20)<0);
 assert(ResultScreen::next_mission(16,true,"Terran02.scx")==campaign::key_index("terran02"));
 assert(ResultScreen::next_mission(16,true,"")==campaign::key_index("terran02"));
 assert(ResultScreen::next_mission(21,true,"Zerg02.scx")==campaign::key_index("expzerg02"));
 assert(ResultScreen::next_mission(39,true,"Zerg03.scx")==campaign::key_index("expzerg03"));
 assert(ResultScreen::next_mission(40,true,"")==~0u);
 assert(ResultScreen::next_mission(16,false,"Terran02.scx")==~0u);
 assert(ResultScreen::next_mission(7,true,"")==~0u);
 assert(ResultScreen::next_mission(38,true,"")==campaign::key_index("zerg09"));
 assert(ResultScreen::next_mission(46,true,"")==~0u);
 assert(ResultScreen::next_mission(16,true,"../Terran02.scx")==~0u);
 for(unsigned race=0;race<3;++race)for(bool won:{false,true}){
  ResultArt art;assert(art.load(argv[1],race,won));assert(fake::allocations==2);
  Snapshot s;s.frame=24*125;s.units_completed=40;s.buildings_completed=12;
  s.minerals_gathered=1234;s.gas_gathered=567;s.players[0].workers=7;s.players[0].army=8;s.players[0].buildings=9;
  fake::draws.clear();text_calls::values.clear();draw_result_top(art,s,won?"Victory!":"Defeat",nullptr);
  assert(fake::draws.size()==1 && fake::draws[0].sub.width==400 && fake::draws[0].sub.height==240 && fake::draws[0].sx==1 && fake::draws[0].sy==1);
  assert(text_calls::values.front()==(won?"Victory!":"Defeat") && text_calls::values[2]=="02:05");
  assert(text_calls::values.back()=="567");
  fake::draws.clear();text_calls::values.clear();draw_result_bottom(art,nullptr);
  assert(fake::draws.size()==1 && fake::draws[0].sub.width==320 && fake::draws[0].sub.height==240);
  assert(text_calls::values==std::vector<std::string>{"Ok"});
  art.clear();assert(fake::allocations==0);
 }
 puts("All six race/outcome screen pairs, top-only statistics, bottom-only Ok, native sizes, texture cleanup and safe campaign/custom routing PASS");
}
