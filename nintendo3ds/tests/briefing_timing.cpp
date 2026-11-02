#include "frontend.h"
#include <cassert>
#include <cstring>
#include <cstdio>
using namespace port3ds;
int main(){
 Frontend f;f.rich_briefing=true;f.briefing={"one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen"};f.briefing_voices={1400};f.briefing_times={12000};
 f.paginate_briefing([](const char* s){return float(strlen(s))*10;},50,2);
 assert(f.briefing.size()==1 && f.briefing_times[0]==12000 && std::count(f.briefing[0].begin(),f.briefing[0].end(),'\n')>2);
 f.begin_briefing(false);f.update_presentation(40,false);assert(!f.briefing_started && f.briefing_elapsed==0 && !f.briefing_done);
 f.update_presentation(4,true);assert(f.briefing_started && f.briefing_elapsed==4 && f.briefing_page==0);
 f.update_presentation(8,true);assert(!f.briefing_done);f.update_presentation(.1,false);assert(f.briefing_done);
 f.replay_briefing();assert(!f.briefing_started && f.briefing_elapsed==0);
 f.activate(116);assert(f.briefing_loading && !f.play);f.update_presentation(.2,false);const auto first=f.briefing_slide();f.update_presentation(.2,false);assert(f.briefing_slide()>first && !f.play);f.update_presentation(.3,false);assert(f.play && f.briefing_slide()==1);
 puts("Long utterance stays whole; subtitles wait for playback; voice end gates advance; replay and loading exit PASS");
}
