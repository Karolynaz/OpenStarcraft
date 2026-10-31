#pragma once
#include "frontend_draw.h"
#include "result_art.h"
namespace port3ds {
inline void draw_result_top(ResultArt& art,const Snapshot& state,const char* title,C2D_TextBuf text){
 art.top();
 const auto white=C2D_Color32(215,230,235,255),green=C2D_Color32(104,235,92,255);
 C2D_DrawRectSolid(12,12,0,376,216,C2D_Color32(5,10,20,225));
 ui_border({12,12,376,216},C2D_Color32(88,102,160,255));
 ui_text(text,title,200,24,.70f,white,true);
 const char* labels[]={"Time","Units completed","Buildings completed","Units remaining","Buildings remaining","Minerals gathered","Gas gathered"};
 const unsigned values[]={0,state.units_completed,state.buildings_completed,state.players[0].workers+state.players[0].army,state.players[0].buildings,state.minerals_gathered,state.gas_gathered};
 for(unsigned i=0;i<7;++i){char value[32];if(!i)std::snprintf(value,sizeof value,"%02u:%02u",state.frame/24/60,state.frame/24%60);else std::snprintf(value,sizeof value,"%u",values[i]);
  ui_text(text,labels[i],30,64+21*i,.56f,white);ui_text(text,value,342,64+21*i,.56f,green,true);}
}
inline void draw_result_bottom(ResultArt& art,C2D_TextBuf text){
 art.bottom();const auto r=ResultScreen::ok();
 C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(4,10,20,225));
 ui_border(r,C2D_Color32(100,145,240,255));ui_centered(text,"Ok",r,.66f,C2D_Color32(185,255,125,255));
}
}
