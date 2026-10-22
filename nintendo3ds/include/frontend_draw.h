#pragma once
#include "frontend.h"
#include "ui_art.h"
namespace port3ds {
inline void ui_text(C2D_TextBuf buf,const char* text,float x,float y,float size,unsigned color,bool center=false){C2D_Text t;C2D_TextParse(&t,buf,text);C2D_TextOptimize(&t);if(center){float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);x-=w/2;}C2D_DrawText(&t,C2D_WithColor,x,y,0,size,size,color);}
inline void ui_border(UiRect r,unsigned color){C2D_DrawLine(r.x,r.y,color,r.x+r.w,r.y,color,1,0);C2D_DrawLine(r.x,r.y+r.h,color,r.x+r.w,r.y+r.h,color,1,0);C2D_DrawLine(r.x,r.y,color,r.x,r.y+r.h,color,1,0);C2D_DrawLine(r.x+r.w,r.y,color,r.x+r.w,r.y+r.h,color,1,0);}
inline void draw_frontend(Frontend& f,UiArt& art,C2D_TextBuf text,unsigned frame){
 const auto green=C2D_Color32(104,180,68,255),bright=C2D_Color32(195,255,146,255),white=C2D_Color32(183,185,241,255),dim=C2D_Color32(76,88,69,255);
 auto label=[&](const char* text_,int id,float x,float y,float size=.40f){ui_text(text,text_,x,y,size,f.hover==id?bright:green,true);};
 const bool campaign=f.screen==Frontend::Screen::campaign;
 art.draw(campaign?"campaign":"custom",0,0);
 if(campaign){
  const char* races[3]={f.brood?"Terr":"Zerg",f.brood?"Prot":"Terr",f.brood?"Zerg":"Prot"};
  const char* names[3]={f.brood?"Terran":"Zerg",f.brood?"Protoss":"Terran",f.brood?"Zerg":"Protoss"};
  const char* episodes[3]={f.brood?"Episode V":"Episode II",f.brood?"Episode IV":"Episode I",f.brood?"Episode VI":"Episode III"};
  for(int i=0;i<3;++i){const float cx=i==0?56:i==1?164:264,feet=i==1?127:151;char key[64];unsigned w=0,h=0;
   std::snprintf(key,sizeof key,"disk%u",frame%4);art.draw(key,cx-37,feet-6);
   std::snprintf(key,sizeof key,"v%u_%s%u",f.brood?1:0,races[i],frame%4);if(art.size(key,w,h))art.draw(key,cx-w/2.f,feet-h,1,1,f.hover==i?1.f:.30f);
   if(f.hover==i && f.dialog==Frontend::Dialog::none){std::snprintf(key,sizeof key,"v%u_%sOn%u",f.brood?1:0,races[i],frame%4);if(art.size(key,w,h))art.draw(key,cx-w/2.f,feet-h);}
   label(names[i],i,cx,feet+11);ui_text(text,episodes[i],cx,feet+23,.36f,white,true);
  }
  ui_text(text,"Load Saved",168,182,.37f,dim,true);ui_text(text,"Load Replay",168,221,.37f,dim,true);
  label("Play Custom",3,165,202);label("Game type",4,274,202,.37f);
 }else{
  ui_text(text,"Create Game",55,32,.43f,white,true);ui_text(text,"Maps",30,51,.34f,white);
  for(unsigned i=0;i<6 && f.scroll+i<f.maps.size();++i){unsigned n=f.scroll+i;const auto& m=f.maps[n];char name[36];std::snprintf(name,sizeof name,"%s%.26s",m.folder?"> ":"",m.name.c_str());if(n==f.selected_map)C2D_DrawRectSolid(29,65+i*10,0,145,10,C2D_Color32(29,47,24,255));ui_text(text,name,31,65+i*10,.31f,f.hover==int(100+n)?bright:green);}
  label("^",11,178,73);label("v",12,178,108);
  ui_text(text,"Player1",17,157,.34f,white);label(Frontend::race_name(f.race),6,132,155,.35f);
  ui_text(text,"Computer",17,170,.34f,green);ui_text(text,Frontend::race_name(f.race),132,170,.34f,green,true);
  const char* types[]={"Melee","Free for all","Use Map Settings"};ui_text(text,"Game Type",29,131,.30f,white);label(types[f.game_type],7,133,131,.30f);
  ui_text(text,"Subtype",29,143,.30f,white);label(f.subtype?"Teams":"Standard",8,133,143,.30f);
  const char* speeds[]={"Slowest","Slower","Slow","Normal","Fast","Faster","Fastest"};label(speeds[f.speed],9,252,143,.34f);
  if(f.selected_map<f.maps.size()){ui_text(text,f.maps[f.selected_map].name.c_str(),251,31,.30f,white,true);ui_text(text,"2 Players",251,122,.34f,green,true);}
  ui_text(text,"Player1",247,131,.32f,green,true);
  label("Play",10,259,191,.42f);label("Cancel",13,271,210,.39f);
 }
 if(f.save_failed)ui_text(text,"Settings could not be saved",160,16,.32f,white,true);
 if(f.dialog!=Frontend::Dialog::none){
  C2D_DrawRectSolid(0,0,0,320,240,C2D_Color32(0,0,0,150));
  C2D_DrawRectSolid(14,30,0,292,188,C2D_Color32(5,9,8,255));ui_border({14,30,292,188},C2D_Color32(105,110,103,255));ui_border({17,33,286,182},C2D_Color32(170,35,23,255));
  if(f.dialog==Frontend::Dialog::version){ui_text(text,"Select Game Type",160,43,.55f,white,true);art.draw("logo_sc",37,63);if(f.hover==30)ui_border({34,65,252,49},bright);label("Original",30,160,108,.34f);art.draw("logo_bw",37,123);if(f.hover==31)ui_border({34,126,252,51},bright);label("Expansion",31,160,173,.34f);label("Cancel",32,160,194);}
  else if(f.dialog==Frontend::Dialog::unsupported){ui_text(text,"Campaign",160,58,.55f,white,true);ui_text(text,"Campaigns are not supported",160,103,.44f,white,true);ui_text(text,"in this version.",160,121,.44f,white,true);label("OK",32,160,185,.45f);}
  else {ui_text(text,"Exit StarCraft?",160,78,.55f,white,true);label("Exit",33,95,176,.45f);label("Cancel",32,225,176,.45f);}
 }
 art.draw("cursor",f.x,f.y);
}
}
