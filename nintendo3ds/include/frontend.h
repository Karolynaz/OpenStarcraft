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
 enum class Screen {campaign,custom};enum class Dialog {none,version,unsupported,exit,error,map_list,ai_tests,briefing};
 Screen screen=Screen::campaign;Dialog dialog=Dialog::none;
 bool brood=true,play=false,load=false,quit=false,save_failed=false;
 bool saved_notice=false,campaign_start=false;
 unsigned mission=0;std::array<bool,campaign::count> completed{};
 std::vector<std::string> briefing;unsigned briefing_page=0;
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
  if(FILE* progress=std::fopen((std::string(root)+"campaign/progress.cfg").c_str(),"r")){char line[80];while(std::fgets(line,sizeof line,progress)){for(unsigned i=0;i<campaign::count;++i)if(std::string(line)==std::string(campaign::missions[i].key)+"=1\n" || (i==0 && std::string(line)=="terran=1\n"))completed[i]=true;}std::fclose(progress);}
 }
 void save(){
  const auto temp=settings+".tmp";FILE* f=std::fopen(temp.c_str(),"w");if(!f){save_failed=true;return;}
  const bool ok=std::fprintf(f,"OPENBW_PROFILE1\nversion=%u\nrace=%u\nname=Player1\ndifficulty=%u\n",brood?1:0,race,difficulty)>0;
  const bool closed=std::fclose(f)==0;save_failed=!ok || !closed || std::rename(temp.c_str(),settings.c_str())!=0;
 }
 void stick(int dx,int dy,float dt){auto axis=[](int v){return std::abs(v)<=20?0.f:std::copysign(std::min(1.f,(std::abs(v)-20)/136.f),float(v));};x=std::max(0.f,std::min(319.f,x+axis(dx)*190*dt));y=std::max(0.f,std::min(239.f,y-axis(dy)*190*dt));advance(dt);hover=hit(int(x),int(y));}
 int hit(int px,int py)const{
  if(leaving || transition<1)return -1;
  if(dialog==Dialog::briefing){if(UiRect{178,198,124,28}.hit(px,py))return 110;return UiRect{18,198,124,28}.hit(px,py)?111:-1;}
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
 void activate(int id){
  if(id<0 || leaving || transition<1)return;
  if(dialog==Dialog::briefing){if(id==110){if(briefing_page+1<briefing.size())++briefing_page;else {dialog=Dialog::none;campaign_start=true;play=true;}}else if(id==111)back();return;}
  if(dialog==Dialog::ai_tests){if(id>=100 && id<104){campaign_start=false;observer_test=unsigned(id-100);dialog=Dialog::none;play=true;}else if(id==32)dialog=Dialog::none;return;}
  if(dialog==Dialog::map_list){if(id>=70 && id<75 && scroll+unsigned(id-70)<maps.size()){selected_map=scroll+unsigned(id-70);preview_dirty=true;dialog=Dialog::none;}else if(id==90 || id==91){if(id==90)scroll=scroll>=5?scroll-5:0;else if(scroll+5<maps.size())scroll+=5;}else if(id==32)dialog=Dialog::none;return;}
  if(dialog!=Dialog::none){if(dialog==Dialog::version && (id==30 || id==31)){brood=id==31;save();dialog=Dialog::none;}else if(id==33 && dialog==Dialog::exit)quit=true;else if(id==32)dialog=Dialog::none;return;}
  if(screen==Screen::campaign){if(id==5){load=true;}else if(id<3){
   {static const unsigned original_races[]={0,1,2},brood_races[]={1,2,0};mission=campaign::opening(brood,(brood?brood_races:original_races)[id]);briefing.clear();briefing_page=0;const auto path=base+"/../campaign/"+campaign::missions[mission].key+".brief";FILE* file=std::fopen(path.c_str(),"rb");
    if(!file){std::snprintf(error,sizeof error,"Install the private campaign data package.");dialog=Dialog::error;return;}
    std::string page;for(unsigned n=0;n<16384;++n){int c=std::fgetc(file);if(c==EOF){if(!page.empty())briefing.push_back(page);break;}if(c=='\f'){briefing.push_back(page);page.clear();}else if(c=='\r'){}else if(c>=32 || c=='\n')page+=char(c);}std::fclose(file);
    if(briefing.empty()){std::snprintf(error,sizeof error,"Campaign briefing is empty.");dialog=Dialog::error;return;}
    dialog=Dialog::briefing;x=240;y=210;
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
 void back(){if(dialog==Dialog::briefing && briefing_page){--briefing_page;return;}if(leaving || transition<1)return;if(dialog!=Dialog::none)dialog=Dialog::none;else if(screen==Screen::custom)navigate(Screen::campaign);else dialog=Dialog::exit;}
};
}
