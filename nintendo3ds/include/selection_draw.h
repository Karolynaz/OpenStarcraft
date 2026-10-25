#pragma once
#include "selection_info.h"
#include "frontend_draw.h"
#include "graphics.h"
#include "panel_text.h"
namespace port3ds {
inline void draw_selection_info(const SelectionInfo& info,Graphics& graphics,const Engine& engine,UiArt& art){
 const auto green=C2D_Color32(70,240,85,255),blue=C2D_Color32(45,110,230,255);
 C2D_DrawRectSolid(13,153,0,224,80,C2D_Color32(0,0,0,255));
 // Figma's wireframe is 56px; the user explicitly requested its native 64px.
 if(!graphics.ui_image(engine.selection_wireframe(),17,153,1,engine))graphics.ui_image(engine.command_icon(info.type),17,153,64.f/36,engine);
 char text[192];std::snprintf(text,sizeof text,"%d/%d",info.hp,info.max_hp);panel_text(art,text,17,219,'G',116);
 if(info.max_shields){const int x=21+panel_width(art,text);std::snprintf(text,sizeof text,"%d/%d",info.shields,info.max_shields);panel_text(art,text,x,219,'B',133-x);}
 panel_text(art,info.name,94,156,'W',143);
 if(info.work!=WorkKind::none){
  const auto active=ProductionPanel::cell(0);ui_border({active.x,active.y,active.w,active.h},blue);
  graphics.ui_image(engine.command_icon(info.work_icon),active.x+2,active.y+2,22.f/36,engine);
  panel_text(art,info.work_name?info.work_name:"",124,174,'W',110);
  const float progress=info.total>0?std::max(0.f,std::min(1.f,1-info.remaining/float(info.total))):0;
  C2D_DrawRectSolid(124,190,0,110,6,blue);C2D_DrawRectSolid(125,191,0,108,4,C2D_Color32(8,20,40,255));C2D_DrawRectSolid(125,191,0,108*progress,4,green);
  if(info.work==WorkKind::training){
   for(unsigned i=1;i<5;++i){const auto c=ProductionPanel::cell(i);ui_border({c.x,c.y,c.w,c.h},blue);
    if(i<info.queue_count)graphics.ui_image(engine.command_icon(info.queue[i]),c.x+2,c.y+2,20.f/36,engine);
    else {std::snprintf(text,sizeof text,"%u",i+1);panel_text(art,text,c.x+9,c.y+5,'B',12);}
   }
  }else {std::snprintf(text,sizeof text,"%u%% Touch icon: cancel",unsigned(progress*100));panel_text(art,text,94,207,'W',140);}
 }else if(info.supply_provided){
  std::snprintf(text,sizeof text,"Used %d  Provided %d\nTotal %d  Max 200\nArmor %d  Lv%d",info.supply_used,info.supply_provided,info.supply_total,info.armor,info.armor_level);panel_text(art,text,94,174,'W',143);
 }else{
  std::snprintf(text,sizeof text,"Armor %d Lv%d  Kills %d",info.armor,info.armor_level,info.kills);panel_text(art,text,94,174,'W',143);
  int y=186;
  if(info.max_shields){std::snprintf(text,sizeof text,"Shields Lv%d",info.shield_level);panel_text(art,text,94,y);y+=12;}
  if(info.ground_weapon){std::snprintf(text,sizeof text,"Ground %d Lv%d",info.ground_damage,info.ground_level);panel_text(art,text,94,y);y+=12;}
  if(info.air_weapon && y<=210){std::snprintf(text,sizeof text,"Air %d Lv%d",info.air_damage,info.air_level);panel_text(art,text,94,y);y+=12;}
  if(info.max_energy && y<=210){std::snprintf(text,sizeof text,"Energy %d/%d",info.energy,info.max_energy);panel_text(art,text,94,y);}
 }
}
}
