// Host GPU contract: both story crops are native-size and their texture pages
// are freed before briefing. This does not claim to emulate the physical GPU.
#include <citro2d.h>
#include <string>
#include <cassert>
#include <cstdio>
// Capture renderer text calls, without pretending to reproduce system fonts.
using C2D_TextBuf=int*;
struct C2D_Text{std::string value;};
enum{C2D_WithColor=1,C2D_WordWrap=2};
namespace text_calls{std::vector<std::string> values;}
inline C2D_TextBuf C2D_TextBufNew(unsigned){return new int;}
inline void C2D_TextBufDelete(C2D_TextBuf p){delete p;}
inline void C2D_TextBufClear(C2D_TextBuf){}
inline void C2D_TextParse(C2D_Text* t,C2D_TextBuf,const char* s){t->value=s;}
inline void C2D_TextOptimize(C2D_Text*){}
inline void C2D_TextGetDimensions(C2D_Text* t,float sx,float sy,float* w,float* h){*w=t->value.size()*16*sx;*h=30*sy;}
template<typename... T>inline void C2D_DrawText(C2D_Text* t,unsigned,float,float,float,float,float,unsigned,T...){text_calls::values.push_back(t->value);}
inline void C2D_DrawRectSolid(float,float,float,float,float,unsigned){}
inline void C2D_DrawLine(float,float,unsigned,float,float,unsigned,float,float){}
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
  assert((text_calls::values==std::vector<std::string>{"MISSION NAME","LOCATION","Cancel (B)","Briefing (A)"}));
  f.begin_briefing();assert(art.sync(f,argv[1]));
  assert(!art.story_top() && !art.story_bottom());
  art.release_room();fake::draws.clear();art.shot();
  assert(fake::draws.size()==1 && fake::draws[0].sub.width==256 && fake::draws[0].sub.height==128);
  // Only the existing small briefing photo and race frame survive loading.
  assert(fake::allocations==2);
  art.clear();assert(fake::allocations==0);
 }
 printf("All %u native-sized dual-screen story crops, release before briefing, bounded loading textures and cleanup PASS\n",campaign::count);
}
