#pragma once
#include "frontend.h"
#include "ui_art.h"
#include "menu_animation.h"
namespace port3ds {
inline void ui_text(C2D_TextBuf buf,const char* text,float x,float y,float size,unsigned color,bool center=false){C2D_Text t;C2D_TextParse(&t,buf,text);C2D_TextOptimize(&t);if(center){float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);x-=w/2;}C2D_DrawText(&t,C2D_WithColor,x,y,0,size,size,color);}
inline void ui_border(UiRect r,unsigned color){C2D_DrawLine(r.x,r.y,color,r.x+r.w,r.y,color,1,0);C2D_DrawLine(r.x,r.y+r.h,color,r.x+r.w,r.y+r.h,color,1,0);C2D_DrawLine(r.x,r.y,color,r.x,r.y+r.h,color,1,0);C2D_DrawLine(r.x+r.w,r.y,color,r.x+r.w,r.y+r.h,color,1,0);}
// Fit readable text by ellipsis/line limits, keeping the font size fixed.
inline float ui_width(C2D_TextBuf buf,const char* value,float size){C2D_TextBufClear(buf);C2D_Text t;C2D_TextParse(&t,buf,value);float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);return w;}
inline void ui_limited(C2D_TextBuf buf,const char* value,float x,float y,float size,float width,unsigned lines,unsigned color){
 auto measure=C2D_TextBufNew(160);if(!measure)return;
 const char* at=value;const float line_height=size*30;
 for(unsigned row=0;row<lines && *at;++row){while(*at==' ' || *at=='\n' || *at=='\r')++at;if(!*at)break;char line[160]{};unsigned n=0,last_space=0;
  while(at[n] && at[n]!='\n' && n<150){line[n]=at[n];line[n+1]=0;if(ui_width(measure,line,size)>width){line[n]=0;break;}if(at[n]==' ')last_space=n;++n;}
  if(!n){++at;continue;}
  if(at[n] && at[n]!='\n' && last_space && row+1<lines){n=last_space;line[n]=0;}
  if(row+1==lines && at[n]){while(n && (std::snprintf(line+n,sizeof(line)-n,"..."),ui_width(measure,line,size)>width))line[--n]=0;}
  ui_text(buf,line,x,y+row*line_height,size,color);at+=n;
 }
 C2D_TextBufDelete(measure);
}
inline void draw_frontend_top(const Frontend& f,UiArt& art,C2D_TextBuf text){
 if(f.screen==Frontend::Screen::campaign){art.draw("title",0,0);return;}
 art.draw("custom_top",0,0);const float slide=420*f.slide();
 art.draw("frame_top",-slide,0);
 const auto white=C2D_Color32(183,185,241,255),green=C2D_Color32(104,205,68,255);
 C2D_DrawRectSolid(15-slide,20,0,144,201,C2D_Color32(6,10,20,190));
 ui_limited(text,f.preview.title[0]?f.preview.title:"Map preview",21-slide,28,.60f,132,2,white);
 ui_limited(text,f.preview.valid?f.preview.description:f.preview.error,21-slide,82,.47f,132,6,white);
 char info[60];std::snprintf(info,sizeof info,"Players: %u",f.preview.players);ui_text(text,info,21-slide,180,.52f,green);
 std::snprintf(info,sizeof info,"Size: %ux%u",f.preview.width,f.preview.height);ui_text(text,info,21-slide,200,.47f,white);
 if(f.selected_map<f.maps.size())ui_limited(text,f.maps[f.selected_map].name.c_str(),21-slide,218,.37f,132,1,white);
 if(f.preview.valid){const float ratio=f.preview.width/float(f.preview.height),w=ratio>=1?208:208*ratio,h=ratio>=1?208/ratio:208;
  const float x=178+(208-w)/2+slide,y=16+(208-h)/2;
  art.draw_preview(x,y,w,h);ui_border({int(x)-1,int(y)-1,int(w)+2,int(h)+2},C2D_Color32(150,35,24,255));
 }
}
inline void draw_frontend(Frontend& f,UiArt& art,MenuAnimation& animation,C2D_TextBuf text,unsigned frame){
 const auto green=C2D_Color32(104,180,68,255),bright=C2D_Color32(195,255,146,255),white=C2D_Color32(183,185,241,255),dim=C2D_Color32(76,88,69,255);
 auto label=[&](const char* text_,int id,float x,float y,float size=.40f){ui_text(text,text_,x,y,size,f.hover==id?bright:green,true);};
 const bool campaign=f.screen==Frontend::Screen::campaign;
 if(campaign){
  const float shift=340*f.slide();art.draw("campaign_left",-shift,0);art.draw("campaign_right",160+shift,0);
  const char* races[3]={f.brood?"Terr":"Zerg",f.brood?"Prot":"Terr",f.brood?"Zerg":"Prot"};
  const char* names[3]={f.brood?"Terran":"Zerg",f.brood?"Protoss":"Terran",f.brood?"Zerg":"Protoss"};
  const char* episodes[3]={f.brood?"Episode V":"Episode II",f.brood?"Episode IV":"Episode I",f.brood?"Episode VI":"Episode III"};
  for(int i=0;i<3;++i){const float cx=56+i*104+(i==2?shift:-shift),feet=113;char key[64];unsigned w=0,h=0;
   // One disk slot is shared: every actor draws the same immutable frame.
   if(animation.size("disk",w,h))animation.draw("disk",3,frame,cx-w/2.f,feet-4);
   const bool hover=f.hover==i && f.dialog==Frontend::Dialog::none;
   std::snprintf(key,sizeof key,"v%u_%s%s",f.brood?1:0,races[i],"");
   if(animation.size(key,w,h))animation.draw(key,unsigned(i),frame,cx-w/2.f,feet-h,hover?1.f:.64f);
   if(hover){std::snprintf(key,sizeof key,"v%u_%sOn",f.brood?1:0,races[i]);if(animation.size(key,w,h))animation.draw(key,4+unsigned(i),frame,cx-w/2.f,feet-h);}
   label(names[i],i,cx,130,.57f);ui_text(text,episodes[i],cx,149,.42f,white,true);
  }
  const char* button_names[]={"Load Saved","Load Replay","Play Custom","Game Type"};static const int ids[]={5,5,3,4};
  for(unsigned i=0;i<4;++i){auto r=Frontend::campaign_button(i);r.x+=int(i%2?shift:-shift);
   C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(2,5,5,255));art.draw("menu_button",r.x,r.y);
   const bool hovered=!f.leaving && f.transition==1 && f.dialog==Frontend::Dialog::none && Frontend::campaign_button(i).hit(int(f.x),int(f.y));
   const auto color=hovered?bright:i<2?C2D_Color32(117,143,117,255):green;
   ui_text(text,button_names[i],r.x+r.w/2,r.y+5,.55f,color,true);
  }
 }else{
  art.draw("custom_bottom",0,0);
  const float shift=340*f.slide();
  const int panel_x=14-int(shift);
  C2D_DrawRectSolid(panel_x+8,48,0,276,140,C2D_Color32(8,10,19,235));art.draw("custom_panel",panel_x,24);
  ui_text(text,"Create settings",160-shift,3,.60f,white,true);
  const char* labels[]={"Game Type","Map","Player","AI1","AI2","Difficulty"};const char* types[]={"Melee","Free for all","Use Map Settings"};

  char map_name[38]{};std::snprintf(map_name,sizeof map_name,"%.27s",f.preview.title[0]?f.preview.title:f.selected_map<f.maps.size()?f.maps[f.selected_map].name.c_str():"No maps");
  const char* values[]={types[f.game_type],map_name,Frontend::race_name(f.race),f.ai_name(0),f.ai_name(1),f.difficulty?"Medium":"Easy"};
  for(unsigned i=0;i<Frontend::row_count;++i){const bool enabled=i<3 || i==5 || f.ai_available(i-3);const auto color=enabled?green:dim;
   ui_text(text,labels[i],25-shift,Frontend::row_y(i),.50f,white);
   ui_text(text,"<",123-shift,Frontend::row_y(i)-1,.58f,f.hover==int(40+i*2)?bright:color,true);
   ui_limited(text,values[i],141-shift,Frontend::row_y(i),.52f,132,1,color);
   ui_text(text,">",288-shift,Frontend::row_y(i)-1,.58f,f.hover==int(41+i*2)?bright:color,true);
  }
  for(unsigned i=0;i<2;++i){const float x=(i?178:18)+(i?shift:-shift);
   C2D_DrawRectSolid(x,196,0,124,28,C2D_Color32(4,8,15,230));
   art.draw(i?"custom_ok":"custom_cancel",(i?178:0)+(i?shift:-shift),196);
   ui_text(text,i?"OK":"Cancel",x+62,201,.59f,f.hover==int(i?10:13)?bright:green,true);
  }

 }
 if(f.save_failed)ui_text(text,"Settings could not be saved",160,16,.32f,white,true);
 if(f.dialog!=Frontend::Dialog::none){
  C2D_DrawRectSolid(0,0,0,320,240,C2D_Color32(0,0,0,150));
  C2D_DrawRectSolid(14,30,0,292,188,C2D_Color32(5,9,8,255));ui_border({14,30,292,188},C2D_Color32(105,110,103,255));ui_border({17,33,286,182},C2D_Color32(170,35,23,255));
  if(f.dialog==Frontend::Dialog::map_list){
   ui_text(text,"Select map",160,40,.58f,white,true);
   for(unsigned i=0;i<5 && f.scroll+i<f.maps.size();++i){const auto index=f.scroll+i;const auto& name=f.maps[index].name;auto slash=name.find_last_of('/');const auto title=name.substr(slash==std::string::npos?0:slash+1);
    const bool selected=index==f.selected_map;const auto color=f.hover==int(70+i)?bright:selected?white:green;
    if(selected)C2D_DrawRectSolid(24,66+i*24,0,272,23,C2D_Color32(28,42,29,255));
    ui_limited(text,title.c_str(),30,69+i*24,.49f,258,1,color);
   }
   label("<",90,50,195,.52f);label("Cancel",32,160,195,.47f);label(">",91,270,195,.52f);
  }
  else if(f.dialog==Frontend::Dialog::version){ui_text(text,"Select Game Type",160,43,.55f,white,true);art.draw("logo_sc",37,63);if(f.hover==30)ui_border({34,65,252,49},bright);label("Original",30,160,108,.34f);art.draw("logo_bw",37,123);if(f.hover==31)ui_border({34,126,252,51},bright);label("Expansion",31,160,173,.34f);label("Cancel",32,160,194);}
  else if(f.dialog==Frontend::Dialog::error){ui_text(text,"Cannot start match",160,51,.50f,white,true);C2D_Text t;C2D_TextParse(&t,text,f.error);C2D_DrawText(&t,C2D_WithColor|C2D_WordWrap,29,85,0,.42f,.42f,white,260.f);label("OK",32,160,185,.45f);}
  else if(f.dialog==Frontend::Dialog::unsupported){ui_text(text,"Campaign",160,58,.55f,white,true);ui_text(text,"Campaigns are not supported",160,103,.44f,white,true);ui_text(text,"in this version.",160,121,.44f,white,true);label("OK",32,160,185,.45f);}
  else {ui_text(text,"Exit StarCraft?",160,78,.55f,white,true);label("Exit",33,95,176,.45f);label("Cancel",32,225,176,.45f);}
 }
 art.draw("cursor",f.x,f.y);
}
}
