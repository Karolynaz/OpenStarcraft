#pragma once
#include "selection_info.h"
#include "frontend_draw.h"
#include "graphics.h"
#include "panel_text.h"
#include "hud_layout.h"
namespace port3ds {
inline void draw_unit_bars(const UnitView& u,int cx,int cy,Graphics& graphics,const Engine& engine){
 const int width=std::min(255,(u.bar_width-1)/3*3);
 const int x=u.x-cx-width/2,y=u.bar_y-cy;
 if(width<=0 || x+width<=0 || x>=400 || y+15<=0 || y>=240)return;
 const int rows=1+(u.max_shields>0)+(u.max_energy>0);
 C2D_DrawRectSolid(x-1,y-1,0,width+2,4*rows+1,C2D_Color32(8,12,13,240));
 auto row=[&](int at,int value,int max_value,unsigned color){
  const auto segments=width/3;
  const unsigned filled=3*std::min(segments,std::max(value>0?1:0,(value*segments+max_value/2)/max_value));
  if(filled)graphics.ui_image({0,0,selection_bar_image,color,11},x,at,1,engine,filled);
  if(filled<unsigned(width))graphics.ui_image({0,0,selection_bar_image,0,11},x+int(filled),at,1,engine,width-filled,filled);
 };
 int at=y;
 if(u.max_shields){row(at,u.shields,u.max_shields,4);at+=4;}
 row(at,u.hp,u.max_hp,u.hp*3>=u.max_hp*2?1:u.hp*3>=u.max_hp?2:3);at+=4;
 if(u.max_energy)row(at,u.energy,u.max_energy,5);
}
inline void draw_group_cell(HudLayout::Rect cell){
 C2D_DrawRectSolid(cell.x,cell.y,0,cell.w,cell.h,C2D_Color32(9,21,44,255));
 ui_border({cell.x,cell.y,cell.w,cell.h},C2D_Color32(46,94,172,255));
}
inline void draw_group_wireframe(const UnitView& u,Graphics& graphics,const Engine& engine,HudLayout::Rect cell){
 draw_group_cell(cell);
 const auto frame=engine.unit_wireframe(u.type,u.hp,u.max_hp,u.shields,true);
 if(!graphics.ui_image(frame,cell.x+2,cell.y+1,frame.image==group_wireframe_image?1.f:.5f,engine))
    graphics.ui_image(engine.command_icon(u.type),cell.x+4,cell.y+3,.8f,engine);
}
inline void selection_text(UiArt& art,const char* text,int y,char color='W'){
 const int width=std::min(86,panel_width(art,text));panel_text(art,text,58-width/2,y,color,86);
}
inline void draw_selection_info(const SelectionInfo& info,Graphics& graphics,const Engine& engine,UiArt& art){
 const auto green=C2D_Color32(70,240,85,255),blue=C2D_Color32(45,110,230,255);
 // Long building/hero names wrap inside the two-line title area.
 std::string first,second;const char* p=info.name;
 while(*p){const char* begin=p;while(*p && *p!=' ')++p;std::string word(begin,p);if(*p)++p;
  auto candidate=first.empty()?word:first+" "+word;
  if(second.empty() && panel_width(art,candidate.c_str())<=86)first=candidate;
  else {if(!second.empty())second+=' ';second+=word;}
 }
 selection_text(art,first.c_str(),second.empty()?21:17);if(!second.empty())selection_text(art,second.c_str(),28);
 const int stats_y=second.empty()?37:41;
 char text[96];
 if(info.work!=WorkKind::none){
  const char* label=info.work==WorkKind::training?"Training":info.work==WorkKind::research?"Research":info.work==WorkKind::upgrade?"Upgrading":info.work==WorkKind::morph?"Morphing":"Building";
  selection_text(art,label,stats_y);
  const float progress=info.total>0?std::max(0.f,std::min(1.f,1-info.remaining/float(info.total))):0;
  C2D_DrawRectSolid(20,116,0,77,3,blue);C2D_DrawRectSolid(21,117,0,75*progress,1,green);
 }else{
  if(info.max_energy)std::snprintf(text,sizeof text,"K%d E%d/%d",info.kills,info.energy,info.max_energy);
  else std::snprintf(text,sizeof text,"Kills: %d",info.kills);
  selection_text(art,text,stats_y);
 }
 if(!graphics.ui_image(engine.selection_wireframe(),27,51,1,engine))graphics.ui_image(engine.command_icon(info.type),41,66,1,engine);
 std::snprintf(text,sizeof text,"%d/%d",info.hp,info.max_hp);selection_text(art,text,120,'G');
 if(info.max_shields){std::snprintf(text,sizeof text,"%d/%d",info.shields,info.max_shields);selection_text(art,text,134,'B');}
 else if(info.supply_provided){std::snprintf(text,sizeof text,"Supply %d/%d",info.supply_used,info.supply_total);selection_text(art,text,134);}
 if(info.work!=WorkKind::none){
  const unsigned slots=info.work==WorkKind::training?5:1;
  for(unsigned i=0;i<slots;++i){const auto c=ProductionPanel::cell(i);ui_border({c.x,c.y,c.w,c.h},blue);
   const unsigned icon=i==0?info.work_icon:info.queue[i];
   if(i==0 || i<info.queue_count)graphics.ui_image(engine.command_icon(icon),c.x+3,c.y+1,22.f/36,engine);
   else {std::snprintf(text,sizeof text,"%u",i+1);panel_text(art,text,c.x+12,c.y+5,'B',12);}
  }
 }else{
  for(unsigned i=0;i<info.upgrade_count;++i){const auto c=HudLayout::upgrade(i);graphics.ui_image(engine.command_icon(info.upgrades[i].icon,info.upgrades[i].level>0),c.x,c.y,1,engine);
   C2D_DrawRectSolid(c.x+24,c.y+22,0,12,12,C2D_Color32(0,0,0,255));
   std::snprintf(text,sizeof text,"%d",info.upgrades[i].level);panel_text(art,text,c.x+27,c.y+22,'W',9);
  }
  if(info.carried_powerup && info.upgrade_count<4){const auto c=HudLayout::upgrade(info.upgrade_count);graphics.ui_image(engine.command_icon(info.carried_powerup),c.x,c.y,1,engine);panel_text(art,"Carry",c.x+2,c.y+22,'W',34);}
 }
}
}
