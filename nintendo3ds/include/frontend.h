#pragma once
#include <algorithm>
#include <functional>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include "map_preview.h"
#include "skirmish.h"
#include "campaign_catalogue.h"
namespace port3ds {
struct UiRect {int x,y,w,h;bool hit(int px,int py)const{return px>=x && py>=y && px<x+w && py<y+h;}};
struct Frontend {
 enum class Screen {campaign,custom};enum class Dialog {none,version,unsupported,exit,error,map_list,ai_tests,briefing,story};
 Screen screen=Screen::campaign;Dialog dialog=Dialog::none;
 bool brood=true,play=false,load=false,quit=false,save_failed=false;
 bool saved_notice=false,campaign_start=false;
 unsigned mission=0;std::array<bool,campaign::count> completed{};std::array<std::string,6> next_missions{};
 std::vector<std::string> briefing;std::vector<unsigned> briefing_voices;unsigned briefing_page=0;bool briefing_paginated=false;
 // Shared by pagination and drawing: top-screen text must never be clipped
 // using the old, smaller bottom-screen paragraph dimensions.
 static constexpr float briefing_font=.65f,briefing_width=348.f;
 static constexpr unsigned briefing_rows=8;
 std::string story,location,objectives;std::vector<std::string> story_lines;
 std::vector<unsigned> briefing_portraits,briefing_times,briefing_groups,briefing_slots;
 std::vector<std::array<unsigned,4>> briefing_cast;
 bool rich_briefing=false,briefing_done=false,briefing_loading=false;
 unsigned briefing_serial=0;float briefing_elapsed=0,story_scroll=0,story_pause=3;
 static constexpr float story_font=.62f,story_line_height=18.6f;
 unsigned briefing_slot()const{return !briefing_done && briefing_page<briefing_slots.size()?briefing_slots[briefing_page]:4;}
 unsigned briefing_portrait()const{return !briefing_done && briefing_page<briefing_portraits.size()?briefing_portraits[briefing_page]:110;}
 bool presentation()const{return dialog==Dialog::story || dialog==Dialog::briefing;}
 static std::string read_text(const std::string& path){std::string value;if(FILE* f=std::fopen(path.c_str(),"rb")){for(unsigned n=0;n<16384;++n){int c=std::fgetc(f);if(c==EOF)break;if(c!='\r')value+=char(c);}std::fclose(f);}return value;}
 void layout_story(const std::function<float(const char*)>& measure,float width){
  if(!story_lines.empty())return;size_t at=0;
  while(at<story.size()){
   if(story[at]=='\n'){story_lines.push_back("");++at;continue;}
   while(at<story.size() && story[at]==' ')++at;if(at>=story.size())break;
   size_t n=0,space=0;std::string line;
   while(at+n<story.size() && story[at+n]!='\n' && n<150){line+=story[at+n];if(measure(line.c_str())>width){line.pop_back();break;}if(story[at+n]==' ')space=n;++n;}
   if(!n){n=1;line=story.substr(at,1);}else if(at+n<story.size() && story[at+n]!='\n' && space){n=space;line=story.substr(at,n);}
   story_lines.push_back(line);at+=n;if(at<story.size() && story[at]=='\n')++at;
  }
 }
 void scroll_story(float delta){story_scroll=std::max(0.f,std::min(std::max(0.f,float(story_lines.size())*story_line_height-174.f),story_scroll+delta));story_pause=3;}
 void begin_briefing(){dialog=Dialog::briefing;x=270;y=194;hover=116;briefing_page=0;briefing_done=briefing.empty();briefing_elapsed=0;++briefing_serial;}
 void replay_briefing(){if(rich_briefing && dialog==Dialog::briefing)begin_briefing();}
 void update_presentation(float dt,bool speaking){
  if(dialog==Dialog::story){story_pause=std::max(0.f,story_pause-dt);if(!story_pause)story_scroll=std::min(std::max(0.f,float(story_lines.size())*story_line_height-174.f),story_scroll+dt*9);}
  if(!rich_briefing || dialog!=Dialog::briefing || briefing_done)return;
  briefing_elapsed+=dt;
  const unsigned ms=briefing_page<briefing_times.size()?briefing_times[briefing_page]:5000;
  const bool continuation=briefing_page+1<briefing_groups.size() && briefing_groups[briefing_page]==briefing_groups[briefing_page+1];
  if(briefing_elapsed>=ms/1000.f && (continuation || !speaking)){briefing_elapsed=0;if(++briefing_page>=briefing.size())briefing_done=true;}
 }
 unsigned briefing_sound()const{return !briefing_done && briefing_page<briefing_voices.size()?briefing_voices[briefing_page]:0;}
 // The renderer supplies real font widths, so no spoken paragraph is cut by
 // the dialog's line limit. Continuation pages retain the same voice ID.
 void paginate_briefing(const std::function<float(const char*)>& measure,float width,unsigned rows){
  if(briefing_paginated || !rows)return;
  std::vector<std::string> pages;std::vector<unsigned> voices,portraits,times,groups,slots;std::vector<std::array<unsigned,4>> casts;
  for(unsigned i=0;i<briefing.size();++i){const unsigned voice=i<briefing_voices.size()?briefing_voices[i]:0;const auto begin=pages.size();const char* at=briefing[i].c_str();std::string page;unsigned line_count=0;
   while(*at){while(*at==' ' || *at=='\n' || *at=='\r')++at;if(!*at)break;std::string line;unsigned n=0,space=0;
    while(at[n] && at[n]!='\n' && n<150){line+=at[n];if(measure(line.c_str())>width){line.pop_back();break;}if(at[n]==' ')space=n;++n;}
    if(!n){line.assign(1,*at);n=1;}else if(at[n] && at[n]!='\n' && space){n=space;line.assign(at,n);}
    if(!page.empty())page+='\n';page+=line;at+=n;
    if(++line_count==rows){pages.push_back(page);voices.push_back(voice);page.clear();line_count=0;}
   }
   if(!page.empty()){pages.push_back(page);voices.push_back(voice);}
   const auto count=pages.size()-begin;
   for(unsigned n=0;n<count;++n){groups.push_back(i);slots.push_back(i<briefing_slots.size()?briefing_slots[i]:3);casts.push_back(i<briefing_cast.size()?briefing_cast[i]:std::array<unsigned,4>{{110,110,110,i<briefing_portraits.size()?briefing_portraits[i]:110}});portraits.push_back(i<briefing_portraits.size()?briefing_portraits[i]:110);const unsigned original=i<briefing_times.size()?briefing_times[i]:5000;times.push_back(std::max(4000u,unsigned(original/std::max(size_t(1),count))));}
  }
  briefing.swap(pages);briefing_voices.swap(voices);briefing_portraits.swap(portraits);briefing_times.swap(times);briefing_groups.swap(groups);briefing_slots.swap(slots);briefing_cast.swap(casts);briefing_page=0;briefing_paginated=true;
 }

 unsigned race=1,speed=4,game_type=0,subtype=0,scroll=0,selected_map=0;
 std::array<unsigned,2> ai{{3,3}};
 unsigned difficulty=0,observer_test=~0u;
 static constexpr unsigned row_count=6;
 static int row_y(unsigned row){return 51+int(row)*23;}
 char error[192]{};
 bool leaving=false;Screen destination=Screen::campaign;
 MapPreview preview;bool preview_dirty=true;float transition=1;
 float x=234,y=185;int hover=-1;
 std::string directory,base,settings;
 struct Map{std::string name;bool folder;};std::vector<Map> maps;
 static const char* race_name(unsigned r){return r==0?"Zerg":r==2?"Protoss":"Terran";}
 unsigned scenario()const{if(campaign_start)return campaign::missions[mission].scenario;if(observer_test<4)return observer_test==1?12:observer_test==2?14:5;return race==0?15:race==2?13:4;}
 unsigned capacity()const{return preview.valid?std::min(3u,preview.players):0;}
 bool ai_available(unsigned i)const{return capacity()>i+1;}
 const char* ai_name(unsigned i)const{return !ai_available(i) || ai[i]==4?"Off":ai[i]==3?"Random":race_name(ai[i]);}
 std::string map_path()const{if(campaign_start)return base+"/../campaign/"+campaign::missions[mission].key+".chk";if(observer_test<4)return base+(observer_test==3?"/AI/three-islands.scm":"/AI/islands.scm");return selected_map<maps.size()?directory+"/"+maps[selected_map].name:"";}
 MatchSetup match(unsigned seed)const{MatchSetup m;if(campaign_start)return m;m.enabled=true;if(observer_test<4){m.observer=true;m.difficulty=AiDifficulty::medium;int r=observer_test==1?2:observer_test==2?0:1;m.races={{r,r,-1}};if(observer_test==3){m.races={{1,2,0}};m.teams={{0,1,1}};m.side_difficulty={{1,0,0}};}return m;}m.game_type=game_type;m.difficulty=difficulty?AiDifficulty::medium:AiDifficulty::easy;m.races[0]=int(race);for(unsigned i=0;i<2;++i){seed=seed*1664525u+1013904223u;m.races[i+1]=!ai_available(i) || ai[i]==4?-1:ai[i]==3?int((seed>>16)%3):int(ai[i]);}return m;}
 static UiRect campaign_button(unsigned i){return i==2?UiRect{92,205,136,24}:UiRect{18+int(i)*148,174,136,24};}
 void navigate(Screen to){if(leaving || transition<1 || screen==to)return;destination=to;leaving=true;transition=1;}
 void advance(float dt){
  if(leaving){transition=std::max(0.f,transition-dt/.38f);if(transition<.00001f){screen=destination;leaving=false;transition=0;if(screen==Screen::custom)preview_dirty=true;}}
  else transition=std::min(1.f,transition+dt/.38f);
 }
 float slide()const{float left=1-transition;return left*left*left;}
 void scan(){
  preview_dirty=true;
  maps.clear();scroll=selected_map=0;
  std::function<void(const std::string&,unsigned)> visit=[&](const std::string& prefix,unsigned depth){
   if(depth>8 || maps.size()>=256)return;
   if(auto* d=opendir((base+"/"+prefix).c_str())){while(auto* e=readdir(d)){
    std::string n=e->d_name;if(n.empty() || n[0]=='.')continue;struct stat st{};const auto relative=prefix+n;
    if(stat((base+"/"+relative).c_str(),&st))continue;
    if(S_ISDIR(st.st_mode)){visit(relative+"/",depth+1);continue;}
    const auto dot=n.find_last_of('.');std::string ext=dot==std::string::npos?"":n.substr(dot);for(auto& c:ext)c=char(std::tolower(static_cast<unsigned char>(c)));
    if((ext==".scx" || ext==".scm" || ext==".chk") && maps.size()<256)maps.push_back({relative,false});
   }closedir(d);}
  };visit("",0);
  std::sort(maps.begin(),maps.end(),[](const Map& a,const Map& b){return a.folder!=b.folder?a.folder>b.folder:a.name<b.name;});
 }
 void init(const char* root){base=std::string(root)+"maps";directory=base;settings=std::string(root)+"player1.cfg";
  if(FILE* f=std::fopen(settings.c_str(),"r")){unsigned v,r;if(std::fscanf(f,"OPENBW_PROFILE1\nversion=%u\nrace=%u",&v,&r)==2){brood=v==1;race=r<3?r:1;}char line[128];while(std::fgets(line,sizeof line,f)){unsigned value;if(std::sscanf(line,"difficulty=%u",&value)==1)difficulty=value==1?1:0;}std::fclose(f);}else save();scan();
  if(FILE* progress=std::fopen((std::string(root)+"campaign/progress.cfg").c_str(),"r")){char line[80];while(std::fgets(line,sizeof line,progress)){std::string route=line;if(route.compare(0,5,"next=")==0){route=route.substr(5);while(!route.empty() && (route.back()=='\n' || route.back()=='\r'))route.pop_back();auto ep=campaign::key_episode(route);if(ep<next_missions.size())next_missions[ep]=route;}for(unsigned i=0;i<campaign::count;++i)if(std::string(line)==std::string(campaign::missions[i].key)+"=1\n" || (i==0 && std::string(line)=="terran=1\n"))completed[i]=true;}std::fclose(progress);}
 }
 bool complete_mission(unsigned scenario,const std::string& filename){auto i=campaign::index(scenario);if(i>=campaign::count)return false;auto done=completed;auto routes=next_missions;done[i]=true;const auto& m=campaign::missions[i];routes[(m.brood?3:0)+m.race]=campaign::next_key(scenario,filename);const auto path=base+"/../campaign/progress.cfg",temp=path+".tmp";FILE* out=std::fopen(temp.c_str(),"w");if(!out)return false;bool ok=std::fprintf(out,"OPENBW_CAMPAIGN1\n")>0;for(unsigned j=0;j<done.size();++j)ok=(std::fprintf(out,"%s=%u\n",campaign::missions[j].key,done[j]?1:0)>0)&&ok;for(const auto& route:routes)if(!route.empty())ok=(std::fprintf(out,"next=%s\n",route.c_str())>0)&&ok;const bool closed=std::fclose(out)==0;if(!ok || !closed || std::rename(temp.c_str(),path.c_str()))return false;completed=done;next_missions=routes;return true;}
 void save(){
  const auto temp=settings+".tmp";FILE* f=std::fopen(temp.c_str(),"w");if(!f){save_failed=true;return;}
  const bool ok=std::fprintf(f,"OPENBW_PROFILE1\nversion=%u\nrace=%u\nname=Player1\ndifficulty=%u\n",brood?1:0,race,difficulty)>0;
  const bool closed=std::fclose(f)==0;save_failed=!ok || !closed || std::rename(temp.c_str(),settings.c_str())!=0;
 }
 void stick(int dx,int dy,float dt){auto axis=[](int v){return std::abs(v)<=20?0.f:std::copysign(std::min(1.f,(std::abs(v)-20)/136.f),float(v));};x=std::max(0.f,std::min(319.f,x+axis(dx)*190*dt));y=std::max(0.f,std::min(239.f,y-axis(dy)*190*dt));advance(dt);hover=hit(int(x),int(y));}
 int hit(int px,int py)const{
  if(leaving || transition<1)return -1;
  if(dialog==Dialog::story){if(UiRect{15,12,24,24}.hit(px,py))return 112;if(UiRect{281,12,24,24}.hit(px,py))return 113;return UiRect{170,208,136,28}.hit(px,py)?115:UiRect{14,208,136,28}.hit(px,py)?111:-1;}
  if(dialog==Dialog::briefing && rich_briefing){if(UiRect{237,209,68,12}.hit(px,py))return 111;if(UiRect{153,203,69,13}.hit(px,py))return 117;return UiRect{237,188,68,14}.hit(px,py)?116:-1;}
  if(dialog==Dialog::briefing){if(campaign::adjacent(mission,1)!=mission){if(UiRect{15,30,24,24}.hit(px,py))return 112;if(UiRect{281,30,24,24}.hit(px,py))return 113;}if(UiRect{178,198,124,28}.hit(px,py))return 110;return UiRect{18,198,124,28}.hit(px,py)?111:-1;}
  if(dialog==Dialog::ai_tests){for(unsigned i=0;i<4;++i)if(UiRect{34,70+int(i)*28,252,24}.hit(px,py))return 100+i;return UiRect{112,192,96,22}.hit(px,py)?32:-1;}
  if(dialog==Dialog::map_list){
   for(unsigned i=0;i<5;++i)if(scroll+i<maps.size() && UiRect{25,67+int(i)*24,270,23}.hit(px,py))return 70+i;
   if(UiRect{25,194,65,22}.hit(px,py))return 90;if(UiRect{230,194,65,22}.hit(px,py))return 91;
   return UiRect{112,194,96,22}.hit(px,py)?32:-1;
  }
  if(dialog==Dialog::version){if(UiRect{34,67,252,48}.hit(px,py))return 30;if(UiRect{34,128,252,48}.hit(px,py))return 31;if(UiRect{112,192,96,19}.hit(px,py))return 32;return -1;}
  if(dialog==Dialog::unsupported || dialog==Dialog::error)return UiRect{112,183,96,22}.hit(px,py)?32:-1;
  if(dialog==Dialog::exit){if(UiRect{45,174,100,23}.hit(px,py))return 33;return UiRect{175,174,100,23}.hit(px,py)?32:-1;}
  if(screen==Screen::campaign){
   if(UiRect{112,8,96,22}.hit(px,py))return 6;
   static const int ids[]={5,3,4};for(unsigned i=0;i<3;++i)if(campaign_button(i).hit(px,py))return ids[i];
   for(unsigned i=0;i<3;++i)if(UiRect{8+int(i)*104,20,96,143}.hit(px,py))return int(i);
  }else{
   if(transition<1)return -1;

   for(unsigned row=0;row<row_count;++row){if(row>=3 && row<5 && !ai_available(row-3))continue;
    if(UiRect{112,row_y(row)-3,23,20}.hit(px,py))return 40+row*2;
    if(UiRect{277,row_y(row)-3,23,20}.hit(px,py))return 41+row*2;
    if(UiRect{139,row_y(row)-3,135,20}.hit(px,py))return row==1?60:41+row*2;
   }
   if(UiRect{18,196,124,28}.hit(px,py))return 13;
   if(UiRect{178,196,124,28}.hit(px,py))return 10;
  }return -1;
 }
 void open_briefing(unsigned index){mission=index;briefing.clear();briefing_voices.clear();briefing_portraits.clear();briefing_times.clear();briefing_groups.clear();briefing_cast.clear();briefing_slots.clear();story.clear();story_lines.clear();location.clear();objectives.clear();rich_briefing=briefing_done=briefing_loading=false;story_scroll=0;story_pause=3;briefing_elapsed=0;briefing_page=0;++briefing_serial;briefing_paginated=false;const auto path=base+"/../campaign/"+campaign::missions[mission].key+".brief";FILE* file=std::fopen(path.c_str(),"rb");
    if(!file){std::snprintf(error,sizeof error,"Install the private campaign data package.");dialog=Dialog::error;return;}
    std::string page;for(unsigned n=0;n<16384;++n){int c=std::fgetc(file);if(c==EOF){if(!page.empty())briefing.push_back(page);break;}if(c=='\f'){briefing.push_back(page);page.clear();}else if(c=='\r'){}else if(c>=32 || c=='\n')page+=char(c);}std::fclose(file);
    if(briefing.empty()){std::snprintf(error,sizeof error,"Campaign briefing is empty.");dialog=Dialog::error;return;}
    briefing_voices.assign(briefing.size(),0);if(FILE* voices=std::fopen((path+"-audio").c_str(),"r")){for(auto& id:briefing_voices){unsigned value=0;if(std::fscanf(voices,"%u",&value)!=1)break;if(value>=1200 && value<2300)id=value;}std::fclose(voices);}
    const auto stem=base+"/../campaign/"+campaign::missions[mission].key;
    story=read_text(stem+".intro");location=read_text(stem+".location");
    if(!story.empty()){if(FILE* meta=std::fopen((stem+".brief-info").c_str(),"r")){
      std::vector<std::string> spoken;std::vector<unsigned> voices;bool valid=true;FILE* cast_file=std::fopen((stem+".brief-cast").c_str(),"r");
      for(unsigned i=0;i<briefing.size();++i){unsigned type,portrait,ms;if(std::fscanf(meta,"%u%u%u",&type,&portrait,&ms)!=3){valid=false;break;}
       std::array<unsigned,4> cast{{110,110,110,portrait}};unsigned slot=3;
       if(cast_file && std::fscanf(cast_file,"%u%u%u%u%u",&cast[0],&cast[1],&cast[2],&cast[3],&slot)!=5){valid=false;break;}
       if(type==4){if(!objectives.empty())objectives+='\n';objectives+=briefing[i];}
       else {briefing_cast.push_back(cast);briefing_slots.push_back(slot);spoken.push_back(briefing[i]);voices.push_back(briefing_voices[i]);briefing_portraits.push_back(portrait);briefing_times.push_back(std::max(4000u,std::min(ms,120000u)));}}
      if(cast_file)std::fclose(cast_file);std::fclose(meta);if(valid){briefing.swap(spoken);briefing_voices.swap(voices);rich_briefing=true;}
    }}
    dialog=rich_briefing?Dialog::story:Dialog::briefing;x=240;y=215;
 }
 void activate(int id){
  if(id<0 || leaving || transition<1)return;
  if(dialog==Dialog::story){if(id==112 || id==113)open_briefing(campaign::adjacent(mission,id==112?-1:1));else if(id==115 || id==110)begin_briefing();else if(id==111)dialog=Dialog::none;return;}
  if(dialog==Dialog::briefing && rich_briefing){if(id==116){briefing_loading=true;campaign_start=true;play=true;}else if(id==117)replay_briefing();else if(id==111)dialog=Dialog::none;return;}
  if(dialog==Dialog::briefing){if(id==112 || id==113){open_briefing(campaign::adjacent(mission,id==112?-1:1));return;}if(id==110){if(briefing_page+1<briefing.size())++briefing_page;else {dialog=Dialog::none;campaign_start=true;play=true;}}else if(id==111)back();return;}
  if(dialog==Dialog::ai_tests){if(id>=100 && id<104){campaign_start=false;observer_test=unsigned(id-100);dialog=Dialog::none;play=true;}else if(id==32)dialog=Dialog::none;return;}
  if(dialog==Dialog::map_list){if(id>=70 && id<75 && scroll+unsigned(id-70)<maps.size()){selected_map=scroll+unsigned(id-70);preview_dirty=true;dialog=Dialog::none;}else if(id==90 || id==91){if(id==90)scroll=scroll>=5?scroll-5:0;else if(scroll+5<maps.size())scroll+=5;}else if(id==32)dialog=Dialog::none;return;}
  if(dialog!=Dialog::none){if(dialog==Dialog::version && (id==30 || id==31)){brood=id==31;save();dialog=Dialog::none;}else if(id==33 && dialog==Dialog::exit)quit=true;else if(id==32)dialog=Dialog::none;return;}
  if(screen==Screen::campaign){if(id==5){load=true;}else if(id<3){
   {static const unsigned original_races[]={0,1,2},brood_races[]={1,2,0};mission=campaign::opening(brood,(brood?brood_races:original_races)[id]);for(unsigned n=0;n<campaign::count && completed[mission];++n){unsigned next=campaign::adjacent(mission,1);if(next==campaign::opening(brood,(brood?brood_races:original_races)[id]))break;mission=next;}const auto& selected=campaign::missions[mission];const auto preferred=campaign::key_index(next_missions[(selected.brood?3:0)+selected.race]);if(preferred<campaign::count)mission=preferred;open_briefing(mission);
   }
  }else if(id==3){campaign_start=false;observer_test=~0u;navigate(Screen::custom);}else if(id==4)dialog=Dialog::version;else if(id==6)dialog=Dialog::ai_tests;}
  else if(id==60){scroll=selected_map/5*5;dialog=Dialog::map_list;x=160;y=77+(selected_map-scroll)*24;}
  else if(id>=40 && id<52)adjust(unsigned(id-40)/2,id%2?1:-1);
  else if(id==10 && preview.valid && selected_map<maps.size() && !maps[selected_map].folder){save();play=true;}
  else if(id==13)navigate(Screen::campaign);

 }
 void adjust(unsigned row,int direction){
  auto cycle=[&](unsigned value,unsigned count){return (value+count+direction)%count;};
  if(row==0)game_type=cycle(game_type,3);
  else if(row==1){
   if(maps.empty())return;unsigned index=selected_map;
   for(unsigned n=0;n<maps.size();++n){index=cycle(index,unsigned(maps.size()));if(!maps[index].folder){selected_map=index;preview_dirty=true;break;}}
  }else if(row==2){race=cycle(race,3);save();}
  else if(row<5 && ai_available(row-3))ai[row-3]=cycle(ai[row-3],5);
  else if(row==5){difficulty=cycle(difficulty,2);save();}
 }
 void direction(unsigned direction){
  if(dialog==Dialog::story){if(direction==0 || direction==2)scroll_story(direction==0?-36:36);else activate(direction==1?113:112);return;}
  if(dialog==Dialog::briefing && rich_briefing)return;
  if(dialog==Dialog::briefing && (direction==1 || direction==3)){activate(direction==1?113:112);return;}
  if(screen==Screen::custom && dialog==Dialog::map_list && !leaving && transition==1){
   if(maps.empty())return;
   if(direction==1 || direction==3){activate(direction==1?91:90);x=160;y=77;}
   else {unsigned row=y<67 || y>=187?0:unsigned(y-67)/24;unsigned index=std::min(scroll+row,unsigned(maps.size()-1));index=(index+maps.size()+(direction==0?-1:1))%maps.size();scroll=index/5*5;x=160;y=77+(index-scroll)*24;}
   hover=hit(int(x),int(y));return;
  }
  if(screen!=Screen::custom || dialog!=Dialog::none || leaving || transition<1)return;
  unsigned row=0;float distance=999;
  for(unsigned i=0;i<row_count;++i)if(std::abs(y-row_y(i))<distance){distance=std::abs(y-row_y(i));row=i;}
  if(direction==1 || direction==3)adjust(row,direction==1?1:-1);
  else {row=(row+row_count+(direction==0?-1:1))%row_count;x=208;y=row_y(row);}hover=hit(int(x),int(y));
 }
 void back(){if(rich_briefing && presentation()){dialog=Dialog::none;return;}if(dialog==Dialog::briefing && briefing_page){--briefing_page;return;}if(leaving || transition<1)return;if(dialog!=Dialog::none)dialog=Dialog::none;else if(screen==Screen::custom)navigate(Screen::campaign);else dialog=Dialog::exit;}
};
}
