#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
namespace port3ds {
struct UiRect {int x,y,w,h;bool hit(int px,int py)const{return px>=x && py>=y && px<x+w && py<y+h;}};
struct Frontend {
 enum class Screen {campaign,custom};enum class Dialog {none,version,unsupported,exit};
 Screen screen=Screen::campaign;Dialog dialog=Dialog::none;
 bool brood=true,play=false,quit=false,save_failed=false;
 unsigned race=1,speed=4,game_type=0,subtype=0,scroll=0,selected_map=0;
 float x=160,y=206;int hover=-1;
 std::string directory,base,settings;
 struct Map{std::string name;bool folder;};std::vector<Map> maps;
 static const char* race_name(unsigned r){return r==0?"Zerg":r==2?"Protoss":"Terran";}
 unsigned scenario()const{return race==0?15:race==2?13:4;}
 void scan(){
  maps.clear();scroll=selected_map=0;if(directory!=base)maps.push_back({"..",true});
  if(auto* d=opendir(directory.c_str())){while(auto* e=readdir(d)){
   std::string n=e->d_name;if(n.empty() || n[0]=='.')continue;struct stat s{};if(stat((directory+"/"+n).c_str(),&s))continue;
   const auto dot=n.find_last_of('.');std::string ext=dot==std::string::npos?"":n.substr(dot);for(auto& c:ext)c=char(std::tolower(c));
   if(S_ISDIR(s.st_mode) || ext==".scx" || ext==".scm" || ext==".chk")maps.push_back({n,bool(S_ISDIR(s.st_mode))});
  }closedir(d);}
  std::sort(maps.begin(),maps.end(),[](const Map& a,const Map& b){return a.folder!=b.folder?a.folder>b.folder:a.name<b.name;});
 }
 void init(const char* root){base=std::string(root)+"maps";directory=base;settings=std::string(root)+"player1.cfg";
  if(FILE* f=std::fopen(settings.c_str(),"r")){unsigned v,r;if(std::fscanf(f,"OPENBW_PROFILE1\nversion=%u\nrace=%u",&v,&r)==2){brood=v==1;race=r<3?r:1;}std::fclose(f);}else save();scan();
 }
 void save(){
  const auto temp=settings+".tmp";FILE* f=std::fopen(temp.c_str(),"w");if(!f){save_failed=true;return;}
  const bool ok=std::fprintf(f,"OPENBW_PROFILE1\nversion=%u\nrace=%u\nname=Player1\n",brood?1:0,race)>0;
  const bool closed=std::fclose(f)==0;save_failed=!ok || !closed || std::rename(temp.c_str(),settings.c_str())!=0;
 }
 void stick(int dx,int dy,float dt){auto axis=[](int v){return std::abs(v)<=20?0.f:std::copysign(std::min(1.f,(std::abs(v)-20)/136.f),float(v));};x=std::max(0.f,std::min(319.f,x+axis(dx)*190*dt));y=std::max(0.f,std::min(239.f,y-axis(dy)*190*dt));hover=hit(int(x),int(y));}
 int hit(int px,int py)const{
  if(dialog==Dialog::version){if(UiRect{34,67,252,48}.hit(px,py))return 30;if(UiRect{34,128,252,48}.hit(px,py))return 31;if(UiRect{112,192,96,19}.hit(px,py))return 32;return -1;}
  if(dialog==Dialog::unsupported)return UiRect{112,183,96,22}.hit(px,py)?32:-1;
  if(dialog==Dialog::exit){if(UiRect{45,174,100,23}.hit(px,py))return 33;return UiRect{175,174,100,23}.hit(px,py)?32:-1;}
  if(screen==Screen::campaign){
   if(UiRect{236,201,78,15}.hit(px,py))return 4;if(UiRect{111,201,104,15}.hit(px,py))return 3;
   if(UiRect{121,181,94,15}.hit(px,py) || UiRect{119,220,97,15}.hit(px,py))return 5;
   if(UiRect{5,25,103,157}.hit(px,py))return 0;if(UiRect{112,10,99,140}.hit(px,py))return 1;if(UiRect{218,25,96,157}.hit(px,py))return 2;
  }else{
   for(unsigned i=0;i<6;++i)if(UiRect{30,65+int(i)*10,143,10}.hit(px,py) && scroll+i<maps.size())return 100+int(scroll+i);
   if(UiRect{174,65,10,29}.hit(px,py))return 11;if(UiRect{174,95,10,30}.hit(px,py))return 12;
   if(UiRect{95,155,76,12}.hit(px,py))return 6;
   if(UiRect{90,130,85,11}.hit(px,py))return 7;if(UiRect{90,142,85,11}.hit(px,py))return 8;
   if(UiRect{209,141,85,13}.hit(px,py))return 9;if(UiRect{221,188,78,17}.hit(px,py))return 10;if(UiRect{238,208,66,15}.hit(px,py))return 13;
  }return -1;
 }
 void activate(int id){
  if(id<0)return;
  if(dialog!=Dialog::none){if(dialog==Dialog::version && (id==30 || id==31)){brood=id==31;save();dialog=Dialog::none;}else if(id==33 && dialog==Dialog::exit)quit=true;else if(id==32)dialog=Dialog::none;return;}
  if(screen==Screen::campaign){if(id<3 || id==5)dialog=Dialog::unsupported;else if(id==3){screen=Screen::custom;scan();}else if(id==4)dialog=Dialog::version;}
  else if(id>=100){selected_map=unsigned(id-100);if(selected_map<maps.size() && maps[selected_map].folder){if(maps[selected_map].name=="..")directory=directory.substr(0,directory.find_last_of('/'));else directory+="/"+maps[selected_map].name;scan();}}
  else if(id==6){race=(race+1)%3;save();}else if(id==7)game_type=(game_type+1)%3;else if(id==8)subtype=(subtype+1)%2;else if(id==9)speed=(speed+1)%7;else if(id==10 && selected_map<maps.size() && !maps[selected_map].folder){save();play=true;}else if(id==11 && scroll) --scroll;else if(id==12 && scroll+6<maps.size())++scroll;else if(id==13)screen=Screen::campaign;
 }
 void back(){if(dialog!=Dialog::none)dialog=Dialog::none;else if(screen==Screen::custom)screen=Screen::campaign;else dialog=Dialog::exit;}
};
}
