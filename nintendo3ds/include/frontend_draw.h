#pragma once
#include "frontend.h"
#include "ui_art.h"
#include "menu_animation.h"
#include "briefing_art.h"
namespace port3ds {
inline void ui_text(C2D_TextBuf buf,const char* text,float x,float y,float size,unsigned color,bool center=false){C2D_Text t;C2D_TextParse(&t,buf,text);C2D_TextOptimize(&t);if(center){float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);x-=w/2;}C2D_DrawText(&t,C2D_WithColor,x,y,0,size,size,color);}
inline void ui_centered(C2D_TextBuf buf,const char* value,UiRect r,float size,unsigned color){
 C2D_Text t;C2D_TextParse(&t,buf,value);C2D_TextOptimize(&t);float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);
 C2D_DrawText(&t,C2D_WithColor,r.x+(r.w-w)/2,r.y+(r.h-h)/2,0,size,size,color);
}
inline void ui_border(UiRect r,unsigned color){C2D_DrawLine(r.x,r.y,color,r.x+r.w,r.y,color,1,0);C2D_DrawLine(r.x,r.y+r.h,color,r.x+r.w,r.y+r.h,color,1,0);C2D_DrawLine(r.x,r.y,color,r.x,r.y+r.h,color,1,0);C2D_DrawLine(r.x+r.w,r.y,color,r.x+r.w,r.y+r.h,color,1,0);}
inline void briefing_speaker_border(UiRect r){
 const auto red=C2D_Color32(255,0,0,255);const float radius=5;
 C2D_DrawLine(r.x+radius,r.y,red,r.x+r.w-radius,r.y,red,1,0);
 C2D_DrawLine(r.x+radius,r.y+r.h,red,r.x+r.w-radius,r.y+r.h,red,1,0);
 C2D_DrawLine(r.x,r.y+radius,red,r.x,r.y+r.h-radius,red,1,0);
 C2D_DrawLine(r.x+r.w,r.y+radius,red,r.x+r.w,r.y+r.h-radius,red,1,0);
 for(unsigned corner=0;corner<4;++corner){const float cx=r.x+(corner==0 || corner==3?radius:r.w-radius),cy=r.y+(corner<2?radius:r.h-radius);
  const float begin=(corner+2)*1.57079632679f;
  for(unsigned i=0;i<6;++i){const float a=begin+i*1.57079632679f/6,b=begin+(i+1)*1.57079632679f/6;
   C2D_DrawLine(cx+radius*std::cos(a),cy+radius*std::sin(a),red,cx+radius*std::cos(b),cy+radius*std::sin(b),red,1,0);}
 }
}
// Fit readable text by ellipsis/line limits, keeping the font size fixed.
inline float ui_width(C2D_TextBuf buf,const char* value,float size){C2D_TextBufClear(buf);C2D_Text t;C2D_TextParse(&t,buf,value);float w,h;C2D_TextGetDimensions(&t,size,size,&w,&h);return w;}
inline bool ui_fits(const char* value,float size,float width){auto measure=C2D_TextBufNew(256);if(!measure)return false;const bool fit=ui_width(measure,value,size)<=width;C2D_TextBufDelete(measure);return fit;}
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
inline void draw_campaign_top(const Frontend& f,BriefingArt& art,C2D_TextBuf text,bool loading=false){
 const auto green=C2D_Color32(104,205,68,255),white=C2D_Color32(183,185,241,255);
 if(f.dialog==Frontend::Dialog::story && !loading){
  art.story_top();
  C2D_DrawRectSolid(0,0,0,400,240,C2D_Color32(0,0,0,185));
  for(unsigned i=0;i<f.story_lines.size();++i){const float y=18+i*Frontend::story_line_height-f.story_scroll;
   if(y>=18 && y+Frontend::story_line_height<=18+Frontend::story_view_height)ui_text(text,f.story_lines[i].c_str(),18,y,Frontend::story_font,green);}
  return;
 }
 if(loading)art.shot();else art.briefing_top();
 C2D_DrawRectSolid(0,0,0,400,240,C2D_Color32(0,0,0,51));
 if(loading)art.frame();else art.text_panel(f);
 const auto area=BriefingLayout::text();
 if(loading || f.briefing_loading){ui_text(text,"Mission objectives",area.x+area.w/2,area.y,.60f,white,true);ui_limited(text,f.objectives.c_str(),area.x,area.y+28,.60f,area.w,7,white);}
 else if(!f.briefing_ready())return;
 else if(f.briefing_done)ui_centered(text,"Press (Y) to replay briefing",area,.60f,green);
 else if(f.briefing_started && f.briefing_page<f.briefing.size()){
  const auto& value=f.briefing[f.briefing_page];unsigned lines=1;for(char c:value)if(c=='\n')++lines;
  const float height=Frontend::briefing_font*30,overflow=std::max(0.f,lines*height-area.h);
  const float duration=f.briefing_page<f.briefing_times.size()?f.briefing_times[f.briefing_page]/1000.f:5.f;
  const float offset=overflow*std::max(0.f,std::min(1.f,(f.briefing_elapsed-1.5f)/std::max(1.f,duration-3.f)));
  size_t at=0;unsigned row=0;while(at<value.size()){auto end=value.find('\n',at);if(end==std::string::npos)end=value.size();const float y=area.y+row++*height-offset;
   if(y>=area.y && y+height<=area.y+area.h)ui_text(text,value.substr(at,end-at).c_str(),area.x,y,Frontend::briefing_font,white);at=end+1;}
 }
}
inline void draw_campaign_bottom(const Frontend& f,BriefingArt& art,UiArt& menu,C2D_TextBuf text,MenuAnimation& animation,unsigned tick){
 const auto green=C2D_Color32(104,205,68,255),bright=C2D_Color32(195,255,146,255);
 if(f.dialog==Frontend::Dialog::story){
  art.story_bottom();
  Frontend layout;layout.story=f.location;layout.layout_story([&](const char* line){auto b=C2D_TextBufNew(160);const auto w=ui_width(b,line,.62f);C2D_TextBufDelete(b);return w;},288);
  const float y=232-std::min(12u,unsigned(layout.story_lines.size()))*18.6f;
  for(unsigned i=0;i<layout.story_lines.size() && i<12;++i)ui_text(text,layout.story_lines[i].c_str(),16,y+i*18.6f,.62f,green);
  return;
 }
 art.room(f,animation,tick);art.portraits(f,animation,tick);
 if(f.briefing_loading){const char* dots[]={"Loading.","Loading..","Loading..."};ui_centered(text,dots[(tick/3)%3],{0,0,320,240},.65f,green);return;}
 const bool zerg=campaign::missions[f.mission].race==0;
 const UiRect start={zerg?237:238,zerg?188:190,66,11};
 ui_centered(text,"Start",start,.40f,f.hover==116?bright:green);
 if(f.briefing_ready()){
  ui_centered(text,"Cancel",BriefingLayout::cancel(),.45f,f.hover==111?bright:green);
  const auto active=f.briefing_slot();if(f.briefing_speaking && active<4)briefing_speaker_border(BriefingLayout::face(active));
 }

 menu.draw("cursor",f.x,f.y);
}
inline void draw_frontend_top(const Frontend& f,UiArt& art,C2D_TextBuf text){
 if(f.dialog==Frontend::Dialog::briefing){
  art.draw("custom_top",0,0);
  C2D_DrawRectSolid(8,8,0,384,224,C2D_Color32(4,8,14,245));
  art.draw("frame_top",8,8,.96f,224.f/240);
  const auto white=C2D_Color32(183,185,241,255);
  ui_text(text,"Mission briefing",200,22,.60f,white,true);
  if(f.briefing_page<f.briefing.size())ui_limited(text,f.briefing[f.briefing_page].c_str(),26,56,Frontend::briefing_font,Frontend::briefing_width,Frontend::briefing_rows,white);
  return;
 }
 if(f.screen==Frontend::Screen::campaign){art.draw("title",0,0);return;}
 art.draw("custom_top",0,0);const float slide=420*f.slide();
 // One continuous backing covers text and map, inside a padded metal frame.
 C2D_DrawRectSolid(12-slide,12,0,376,216,C2D_Color32(6,10,20,220));
 art.draw("frame_top",12-slide,12,.94f,.90f);
 const auto white=C2D_Color32(183,185,241,255),green=C2D_Color32(104,205,68,255);
 ui_limited(text,f.preview.title[0]?f.preview.title:"Map preview",26-slide,30,.62f,156,2,white);
 char info[60];std::snprintf(info,sizeof info,"Players: %u",f.preview.players);ui_text(text,info,26-slide,71,.54f,green);
 ui_limited(text,f.preview.valid?f.preview.description:f.preview.error,26-slide,98,.52f,156,5,white);
 std::snprintf(info,sizeof info,"Size: %ux%u",f.preview.width,f.preview.height);ui_text(text,info,26-slide,194,.49f,white);
 if(f.preview.valid){const float ratio=f.preview.width/float(f.preview.height),w=ratio>=1?176:176*ratio,h=ratio>=1?176/ratio:176;
  const float x=196+(176-w)/2-slide,y=32+(176-h)/2;
  art.draw_preview(x,y,w,h);ui_border({int(x)-1,int(y)-1,int(w)+2,int(h)+2},C2D_Color32(150,35,24,255));
 }
}
inline void draw_frontend(Frontend& f,UiArt& art,MenuAnimation& animation,C2D_TextBuf text,unsigned frame){
 const auto green=C2D_Color32(104,180,68,255),bright=C2D_Color32(195,255,146,255),white=C2D_Color32(183,185,241,255),dim=C2D_Color32(76,88,69,255);
 auto label=[&](const char* text_,int id,float x,float y,float size=.40f){ui_text(text,text_,x,y,size,f.hover==id?bright:green,true);};
 if(f.dialog==Frontend::Dialog::briefing){
  // The touch screen owns navigation; objectives and speech live only above.
  art.draw("custom_bottom",0,0);
  C2D_DrawRectSolid(14,24,0,292,164,C2D_Color32(4,8,14,235));
  art.draw("frame_top",14,24,292.f/400,164.f/240);
  ui_limited(text,campaign::missions[f.mission].title,47,63,.65f,226,3,white);
  if(campaign::adjacent(f.mission,1)!=f.mission){label("<",112,26,38,.62f);label(">",113,294,38,.62f);}
  char page[48];std::snprintf(page,sizeof page,"%u / %u",f.briefing_page+1,unsigned(f.briefing.size()));
  ui_text(text,page,160,151,.58f,white,true);
  for(unsigned i=0;i<2;++i){const UiRect r={i?178:18,198,124,28};
   C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(4,8,15,245));
   art.draw(i?"custom_ok":"custom_cancel",i?178:0,198);
   ui_centered(text,i?(f.briefing_page+1<f.briefing.size()?"Next (A)":"Begin (A)"):"Back (B)",r,.56f,f.hover==int(i?110:111)?bright:green);
  }
  art.draw("cursor",f.x,f.y);return;
 }
 const bool campaign=f.screen==Frontend::Screen::campaign;
 if(campaign){
  const float shift=340*f.slide();art.draw("campaign_left",-shift,0);art.draw("campaign_right",160+shift,0);
  art.draw("title_button",112-shift,8,96.f/136,22.f/24);ui_centered(text,"AI tests",{int(112-shift),8,96,22},.49f,f.hover==6?bright:green);
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
  // A single frame holds both side controls and the centered lower control.
  C2D_DrawRectSolid(8-shift,164,0,304,74,C2D_Color32(3,7,12,245));
  art.draw("title_panel",8-shift,164);
  const char* button_names[]={"Load","Play Custom","Options"};
  for(unsigned i=0;i<3;++i){auto r=Frontend::campaign_button(i);r.x+=int(i==1?shift:-shift);
   C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(2,5,5,255));art.draw("title_button",r.x,r.y);
   art.draw(i==0?"title_leg_left":i==1?"title_leg_right":"title_leg_bottom",i==0?r.x-12:i==1?r.x+r.w:r.x+(r.w-26)/2,i==2?r.y+r.h:r.y+2);
   const bool hovered=!f.leaving && f.transition==1 && f.dialog==Frontend::Dialog::none && Frontend::campaign_button(i).hit(int(f.x),int(f.y));
   const auto color=hovered?bright:green;
   ui_centered(text,button_names[i],r,.55f,color);
  }
 }else{
  art.draw("custom_bottom",0,0);
  const float shift=340*f.slide();
  const int panel_x=14-int(shift);
  C2D_DrawRectSolid(panel_x,24,0,292,166,C2D_Color32(8,10,19,235));art.draw("custom_panel",panel_x,24);
  const char* labels[]={"Game Type","Map","Player","AI1","AI2","Difficulty"};const char* types[]={"Melee","Free for all","Use Map Settings"};

  char map_name[38]{};std::snprintf(map_name,sizeof map_name,"%.27s",f.preview.title[0]?f.preview.title:f.selected_map<f.maps.size()?f.maps[f.selected_map].name.c_str():"No maps");
  const char* values[]={types[f.game_type],map_name,Frontend::race_name(f.race),f.ai_name(0),f.ai_name(1),f.difficulty?"Medium":"Easy"};
  for(unsigned i=0;i<Frontend::row_count;++i){const bool enabled=i<3 || i==5 || f.ai_available(i-3);const auto color=enabled?green:dim;
   ui_text(text,labels[i],35-shift,Frontend::row_y(i),.50f,white);
   ui_text(text,"<",123-shift,Frontend::row_y(i)-1,.58f,f.hover==int(40+i*2)?bright:color,true);
   ui_limited(text,values[i],141-shift,Frontend::row_y(i),.52f,132,1,color);
   ui_text(text,">",288-shift,Frontend::row_y(i)-1,.58f,f.hover==int(41+i*2)?bright:color,true);
  }
  for(unsigned i=0;i<2;++i){const float x=(i?178:18)+(i?shift:-shift);
   C2D_DrawRectSolid(x,196,0,124,28,C2D_Color32(4,8,15,230));
   art.draw(i?"custom_ok":"custom_cancel",(i?178:0)+(i?shift:-shift),196);
   ui_centered(text,i?"OK":"Cancel",{int(x),196,124,28},.59f,f.hover==int(i?10:13)?bright:green);
  }

 }
 if(f.save_failed)ui_text(text,"Settings could not be saved",160,16,.32f,white,true);
 if(f.dialog!=Frontend::Dialog::none){
  C2D_DrawRectSolid(0,0,0,320,240,C2D_Color32(0,0,0,150));
  C2D_DrawRectSolid(14,30,0,292,188,C2D_Color32(5,9,8,255));ui_border({14,30,292,188},C2D_Color32(105,110,103,255));ui_border({17,33,286,182},C2D_Color32(170,35,23,255));
  if(f.dialog==Frontend::Dialog::load_slots){
   ui_text(text,"Load Game",160,38,.58f,white,true);
   for(unsigned i=0;i<5;++i){const UiRect r={24,60+int(i)*28,272,24};if(f.hover==int(120+i))ui_border(r,bright);ui_limited(text,(ui_fits(f.load_labels[i].c_str(),.43f,260)?f.load_labels[i]:f.load_short_labels[i]).c_str(),30,r.y+4,.43f,260,1,f.load_enabled[i]?(f.hover==int(120+i)?bright:green):dim);}
   ui_text(text,"B: Cancel",160,204,.44f,green,true);
  }
  else if(f.dialog==Frontend::Dialog::ai_tests){
   ui_text(text,"AI observer scenarios",160,43,.55f,white,true);
   const char* names[]={"Terran island AI vs AI","Protoss island AI vs AI","Zerg island AI vs AI","Medium vs 2 Easy (all races)"};
   for(unsigned i=0;i<4;++i)ui_centered(text,names[i],{34,70+int(i)*28,252,24},.46f,f.hover==int(100+i)?bright:green);
   label("Cancel",32,160,194,.44f);
  }
  else if(f.dialog==Frontend::Dialog::map_list){
   ui_text(text,"Select map",160,40,.58f,white,true);
   for(unsigned i=0;i<5 && f.scroll+i<f.maps.size();++i){const auto index=f.scroll+i;const auto& name=f.maps[index].name;auto slash=name.find_last_of('/');const auto title=name.substr(slash==std::string::npos?0:slash+1);
    const bool selected=index==f.selected_map;const auto color=f.hover==int(70+i)?bright:selected?white:green;
    if(selected)C2D_DrawRectSolid(24,66+i*24,0,272,23,C2D_Color32(28,42,29,255));
    ui_limited(text,title.c_str(),30,69+i*24,.49f,258,1,color);
   }
   label("<",90,50,195,.52f);label("Cancel",32,160,195,.47f);label(">",91,270,195,.52f);
  }
  else if(f.dialog==Frontend::Dialog::version){ui_text(text,"Select Game",160,43,.55f,white,true);art.draw("logo_sc",37,63);label("Original",30,160,108,.42f);art.draw("logo_bw",37,123);label("Expansion",31,160,173,.42f);label("Cancel",32,160,194);}
  else if(f.dialog==Frontend::Dialog::error){ui_text(text,"Cannot start match",160,51,.50f,white,true);C2D_Text t;C2D_TextParse(&t,text,f.error);C2D_DrawText(&t,C2D_WithColor|C2D_WordWrap,29,85,0,.42f,.42f,white,260.f);label("OK",32,160,185,.45f);}
  else if(f.dialog==Frontend::Dialog::unsupported){ui_text(text,f.saved_notice?"Load Saved":"Campaign",160,58,.55f,white,true);ui_text(text,"Available now: Original Terran 1",160,103,.44f,white,true);ui_text(text,"Choose Original in Options.",160,121,.44f,white,true);label("OK",32,160,185,.45f);}
  else {ui_text(text,"Exit StarCraft?",160,78,.55f,white,true);label("Exit",33,95,176,.45f);label("Cancel",32,225,176,.45f);}
 }
 art.draw("cursor",f.x,f.y);
}
}
