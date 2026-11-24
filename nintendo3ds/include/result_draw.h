#pragma once
#include "frontend_draw.h"
#include "result_art.h"
namespace port3ds {
inline void draw_result_panel(const char* title,C2D_TextBuf text){
 C2D_DrawRectSolid(12,12,0,376,216,C2D_Color32(5,10,20,225));
 ui_border({12,12,376,216},C2D_Color32(88,102,160,255));
 ui_text(text,title,200,24,.70f,C2D_Color32(215,230,235,255),true);
}
class MissionObjectivesPanel {
 std::string value_;
 std::vector<std::string> lines_;
 float size_=.56f;
 void layout(const std::string& value){
  auto measure=C2D_TextBufNew(256);
  if(!measure)return;
  value_=value;
  // Wrap actual font widths and fit all authored lines between the title and frame.
  for(size_=.56f;;size_*=.9f){
   lines_.clear();
   size_t at=0;
   while(at<value.size()){
    while(at<value.size() && (value[at]==' ' || value[at]=='\r' || value[at]=='\n'))++at;
    if(at==value.size())break;
    size_t end=at,space=at;
    while(end<value.size() && value[end]!='\n' && value[end]!='\r' && end-at<240){
     if(ui_width(measure,value.substr(at,end-at+1).c_str(),size_)>332)break;
     if(value[end]==' ')space=end;
     ++end;
    }
    if(end<value.size() && value[end]!='\n' && value[end]!='\r' && value[end]!=' ' && space>at)end=space;
    if(end==at)++end;
    lines_.push_back(value.substr(at,end-at));
    at=end;
   }
   if(lines_.size()*30.f*size_<=160.f)break;
  }
  C2D_TextBufDelete(measure);
 }
public:
 void draw(const std::string& value,C2D_TextBuf text){
  draw_result_panel("Mission Objectives",text);
  if(value_!=value)layout(value);
  for(unsigned i=0;i<lines_.size();++i)
    ui_text(text,lines_[i].c_str(),34,56+30.f*size_*i,size_,C2D_Color32(215,230,235,255));
 }
};
inline void draw_result_top(ResultArt& art,const Snapshot& state,const char* title,C2D_TextBuf text,bool save_failed){
 art.top();
 const auto white=C2D_Color32(215,230,235,255),green=C2D_Color32(104,235,92,255);
 draw_result_panel(title,text);
 const char* labels[]={"Time","Units completed","Buildings completed","Units remaining","Buildings remaining","Minerals gathered","Gas gathered"};
 const unsigned values[]={0,state.units_completed,state.buildings_completed,state.players[0].workers+state.players[0].army,state.players[0].buildings,state.minerals_gathered,state.gas_gathered};
 for(unsigned i=0;i<7;++i){char value[32];if(!i)std::snprintf(value,sizeof value,"%02u:%02u",state.frame/24/60,state.frame/24%60);else std::snprintf(value,sizeof value,"%u",values[i]);
  ui_text(text,labels[i],30,64+21*i,.56f,white);ui_text(text,value,342,64+21*i,.56f,green,true);}
 if(save_failed)ui_text(text,"Campaign progress could not be saved",200,209,.43f,C2D_Color32(255,185,140,255),true);
}
inline void draw_result_bottom(ResultArt& art,C2D_TextBuf text,unsigned choice,bool defeat,bool save_failed){
 art.bottom();
 const auto total=ResultScreen::count(defeat,save_failed);
 for(unsigned i=0;i<total;++i){
 const auto r=ResultScreen::button(i,total);
 C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(4,10,20,225));
 ui_border(r,C2D_Color32(100,145,240,i==choice?255:140));ui_centered(text,ResultScreen::label(i,defeat,save_failed),r,.56f,i==choice?C2D_Color32(185,255,125,255):C2D_Color32(215,230,235,255));
 }
}
}
