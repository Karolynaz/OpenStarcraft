#include "frontend.h"
#include <cassert>
#include <cstdio>
#include <cstring>
int main(int argc,char**argv){using namespace port3ds;assert(argc==2);
 for(unsigned i=0;i<campaign::count;++i){Frontend f;f.base=std::string(argv[1])+"/maps";f.open_briefing(i);
  assert(f.rich_briefing && f.dialog==Frontend::Dialog::story && !f.story.empty() && !f.location.empty() && !f.objectives.empty());
  assert(f.briefing.size()==f.briefing_voices.size() && f.briefing.size()==f.briefing_portraits.size() && f.briefing.size()==f.briefing_cast.size() && f.briefing.size()==f.briefing_slots.size());
  if(i==0)assert(f.briefing_cast[0][3]==17 && f.briefing_slots[0]==3);
  f.layout_story([](const char* t){return std::strlen(t)*7.f;},Frontend::story_width);assert(!f.story_lines.empty());
  f.scroll_story(100000);assert(f.story_scroll==std::max(0.f,float(f.story_lines.size())*Frontend::story_line_height-Frontend::story_view_height));f.scroll_story(-100000);assert(f.story_scroll==0);
  f.update_presentation(4,false);assert(f.story_scroll>=0 && !f.play);
  f.paginate_briefing([](const char* t){return std::strlen(t)*8.f;},Frontend::briefing_width,8);
  for(unsigned j=0;j<f.briefing.size();++j){assert(f.briefing_portraits[j]<=110 && f.briefing_slots[j]<=4);for(auto id:f.briefing_cast[j])assert(id<=110);assert(std::count(f.briefing[j].begin(),f.briefing[j].end(),'\n')<8);}
  assert(f.hit(80,220)<0 && f.hit(240,220)<0);const auto mission=f.mission;
  f.direction(3);f.direction(1);f.stick(-120,0,.1f);f.stick(120,0,.1f);
  assert(f.hover==-1 && f.mission==mission && f.dialog==Frontend::Dialog::story);
  f.confirm();assert(f.dialog==Frontend::Dialog::briefing && f.briefing_page==0 && !f.play);
  for(unsigned n=0;n<2000 && !f.briefing_done;++n)f.update_presentation(1,false);
  assert(f.briefing_done && f.briefing_sound()==0 && f.briefing_portrait()==110 && !f.play);
  auto serial=f.briefing_serial;f.replay_briefing();assert(f.briefing_serial!=serial && !f.briefing_done && f.briefing_page==0);
  assert(f.hit(270,194)==116 && f.hit(260,214)==111 && f.hit(180,208)==117);f.activate(117);assert(f.briefing_page==0 && !f.briefing_done);f.activate(116);
  assert(f.briefing_loading && f.campaign_start && f.play && f.scenario()==campaign::missions[i].scenario);
  f.play=false;f.back();assert(f.dialog==Frontend::Dialog::none);
 }
 Frontend f;f.rich_briefing=true;f.dialog=Frontend::Dialog::briefing;f.briefing={"a","b"};f.briefing_times={4000,4000};f.briefing_groups={0,0};f.update_presentation(4,true);assert(f.briefing_page==1);f.update_presentation(4,true);assert(!f.briefing_done);f.update_presentation(.1,false);assert(f.briefing_done);
 printf("All %u private presentations: story/scroll/objectives separation/timed dialogue/portrait metadata/replay/start/cancel PASS\n",campaign::count);
}
