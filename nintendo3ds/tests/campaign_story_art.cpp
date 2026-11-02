// Host GPU contract: both story crops are native-size and their texture pages
// are freed before briefing. This does not claim to emulate the physical GPU.
#include <citro2d.h>
#include <string>
#include <cassert>
#include <cstdio>
#include "fake3ds/ui_text.h"
#include "frontend_draw.h"
namespace fake {
std::atomic<unsigned> queued{0},allocations{0},pauses{0};
std::vector<Draw> draws;
}
int main(int argc,char**argv){
 assert(argc==2);using namespace port3ds;
 for(unsigned mission=0;mission<campaign::count;++mission){
  Frontend f;f.base=std::string(argv[1])+"maps";f.open_briefing(mission);
  assert(f.rich_briefing && f.dialog==Frontend::Dialog::story);
  BriefingArt art;assert(art.sync(f,argv[1]));
  fake::draws.clear();assert(art.story_top() && art.story_bottom());
  assert(fake::draws.size()==2);
  for(unsigned i=0;i<2;++i){const auto& draw=fake::draws[i];
   assert(draw.x==0 && draw.y==0 && draw.sx==1 && draw.sy==1);
   assert(draw.sub.width==(i?320:400) && draw.sub.height==240);
  }
  f.story="STORY HISTORY";f.location="MISSION NAME\nLOCATION";f.story_lines.clear();
  f.layout_story([](const char* s){return std::strlen(s)*16*Frontend::story_font;},Frontend::story_width);
  text_calls::values.clear();draw_campaign_top(f,art,nullptr);
  assert(text_calls::values==std::vector<std::string>{"STORY HISTORY"});
  text_calls::values.clear();UiArt menu;MenuAnimation animation;
  draw_campaign_bottom(f,art,menu,nullptr,animation,0);
  assert((text_calls::values==std::vector<std::string>{"MISSION NAME","LOCATION"}));
  f.begin_briefing();assert(art.sync(f,argv[1]));
  UiArt cursor;MenuAnimation empty_animation;
  fake::draws.clear();art.room(f,empty_animation,0);
  assert(fake::draws.size()==5);
  assert(fake::draws[1].x==-320 && fake::draws[2].x==532 && fake::draws[3].y==-240 && fake::draws[4].y==422);
  fake::draws.clear();art.text_panel(f);assert(fake::draws.size()==1 && fake::draws[0].y==-240);
  text_calls::values.clear();draw_campaign_top(f,art,nullptr);assert(text_calls::values.empty());
  f.update_presentation(Frontend::briefing_entry_seconds,false);
  assert(f.briefing_ready() && f.briefing_slide()==0);
  fake::draws.clear();art.room(f,empty_animation,1);
  assert(fake::draws[1].x==0 && fake::draws[1].y==111 && fake::draws[2].x==212 && fake::draws[3].y==0 && fake::draws[4].y==182);
  fake::draws.clear();art.portraits(f,empty_animation,1);
  for(const auto& face:fake::draws){
   assert(face.sub.width==48 && face.sub.height==48 && face.sx==73.f/48 && face.sy==67.f/48);
   bool found=false;for(unsigned i=0;i<4;++i){const auto r=BriefingLayout::face(i);found=found || (face.x==r.x && face.y==r.y);}
   assert(found && face.sx>0 && face.sy>0); // Native Figma rectangle, never mirrored.
  }
  text_calls::values.clear();draw_campaign_bottom(f,art,cursor,nullptr,empty_animation,1);
  assert((text_calls::values==std::vector<std::string>{"Start","Cancel"}));
  fake::draws.clear();assert(art.briefing_top());
  assert(fake::draws.size()==1 && fake::draws[0].sub.width==400 && fake::draws[0].sub.height==240 && fake::draws[0].sx==1 && fake::draws[0].sy==1);
  fake::draws.clear();text_calls::values.clear();draw_campaign_top(f,art,nullptr);
  assert(fake::draws.size()==2 && fake::draws[0].sub.width==400 && fake::draws[1].sub.width>=392);
  assert(text_calls::values.empty()); // Speech has not started: no early subtitles.
  f.briefing_started=true;draw_campaign_top(f,art,nullptr);assert(!text_calls::values.empty());
  f.activate(116);f.update_presentation(.3f,false);fake::draws.clear();text_calls::values.clear();
  draw_campaign_top(f,art,nullptr);assert(text_calls::values[0]=="Mission objectives");
  draw_campaign_bottom(f,art,cursor,nullptr,empty_animation,3);
  assert(std::find(text_calls::values.begin(),text_calls::values.end(),"Start")==text_calls::values.end());
  assert(std::find(text_calls::values.begin(),text_calls::values.end(),"Cancel")==text_calls::values.end());
  assert(!art.story_top() && !art.story_bottom());
  art.release_room();assert(!art.briefing_top());fake::draws.clear();art.shot();
  assert(fake::draws.size()==1 && fake::draws[0].sub.width==256 && fake::draws[0].sub.height==128);
  // Only the existing small briefing photo and race frame survive loading.
  assert(fake::allocations==2);
  art.clear();assert(fake::allocations==0);
 }
 printf("All %u native-sized dual-screen story crops, release before briefing, bounded loading textures and cleanup PASS\n",campaign::count);
}
