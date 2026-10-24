#pragma once
#include "selection_info.h"
#include "frontend_draw.h"
#include "graphics.h"
namespace port3ds {
inline void draw_selection_info(const SelectionInfo& info,Graphics& graphics,const Engine& engine,C2D_TextBuf buf){
 const auto green=C2D_Color32(70,240,85,255),white=C2D_Color32(190,200,245,255),blue=C2D_Color32(45,110,230,255);
 C2D_DrawRectSolid(15,166,0,202,66,C2D_Color32(0,0,0,255));
 if(!graphics.ui_image(engine.selection_wireframe(),16,info.max_shields?170:175,info.max_shields?.58f:.65f,engine))graphics.ui_image(engine.command_icon(info.type),18,177,1.1f,engine);
 char text[160];
 if(info.max_shields){std::snprintf(text,sizeof text,"%d/%d",info.shields,info.max_shields);ui_text(buf,text,38,208,.36f,blue,true);}std::snprintf(text,sizeof text,"%d/%d",info.hp,info.max_hp);ui_text(buf,text,38,220,.38f,green,true);
 ui_text(buf,info.name,70,166,.48f,white);
 if(info.work!=WorkKind::none){
  const auto active=ProductionPanel::cell(0);ui_border({active.x,active.y,active.w,active.h},blue);
  graphics.ui_image(engine.command_icon(info.work_icon),active.x+2,active.y+2,.57f,engine);
  ui_limited(buf,info.work_name?info.work_name:"",98,182,.43f,116,1,white);
  const float progress=info.total>0?std::max(0.f,std::min(1.f,1-info.remaining/float(info.total))):0;
  C2D_DrawRectSolid(98,199,0,116,6,blue);C2D_DrawRectSolid(99,200,0,114,4,C2D_Color32(8,20,40,255));C2D_DrawRectSolid(99,200,0,114*progress,4,green);
  if(info.work==WorkKind::training){
   ui_text(buf,"1",89,196,.36f,white,true);
   for(unsigned i=1;i<5;++i){const auto c=ProductionPanel::cell(i);ui_border({c.x,c.y,c.w,c.h},blue);
    if(i<info.queue_count)graphics.ui_image(engine.command_icon(info.queue[i]),c.x+2,c.y+2,.57f,engine);
    else {std::snprintf(text,sizeof text,"%u",i+1);ui_text(buf,text,c.x+12,c.y+4,.64f,blue,true);}
   }
  }else {std::snprintf(text,sizeof text,"%u%%  Touch icon: cancel",unsigned(progress*100));ui_text(buf,text,98,211,.38f,white);}
 }else{
  if(info.supply_provided){std::snprintf(text,sizeof text,"Used %d   Provided %d\nTotal %d   Max 200\nArmor %d",info.supply_used,info.supply_provided,info.supply_total,info.armor);}
  else if(info.max_shields && info.max_energy){std::snprintf(text,sizeof text,"Sh %d/%d  E %d/%d\nArmor %d  Kills %d\nGround %d  Air %d",info.shields,info.max_shields,info.energy,info.max_energy,info.armor,info.kills,info.ground_damage,info.air_damage);}
  else if(info.max_shields){std::snprintf(text,sizeof text,"Shields %d/%d\nArmor %d  Kills %d\nGround %d  Air %d",info.shields,info.max_shields,info.armor,info.kills,info.ground_damage,info.air_damage);}
  else if(info.max_energy){std::snprintf(text,sizeof text,"Energy %d/%d\nArmor %d  Kills %d\nGround %d  Air %d",info.energy,info.max_energy,info.armor,info.kills,info.ground_damage,info.air_damage);}
  else std::snprintf(text,sizeof text,"Armor %d   Kills %d\nGround %d   Air %d\n%s",info.armor,info.kills,info.ground_damage,info.air_damage,info.completed?"":"Under construction");
  ui_text(buf,text,70,183,.43f,white);
 }
}
}
