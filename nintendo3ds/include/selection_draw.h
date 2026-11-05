#pragma once
#include "selection_info.h"
#include "frontend_draw.h"
#include "graphics.h"
#include "panel_text.h"
#include "hud_layout.h"
namespace port3ds {
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
 selection_text(art,first.c_str(),second.empty()?16:12);if(!second.empty())selection_text(art,second.c_str(),23);
 char text[96];
 if(info.work!=WorkKind::none){
  const char* label=info.work==WorkKind::training?"Training":info.work==WorkKind::research?"Research":info.work==WorkKind::upgrade?"Upgrading":info.work==WorkKind::morph?"Morphing":"Building";
  selection_text(art,label,37);
  const float progress=info.total>0?std::max(0.f,std::min(1.f,1-info.remaining/float(info.total))):0;
  C2D_DrawRectSolid(20,116,0,77,3,blue);C2D_DrawRectSolid(21,117,0,75*progress,1,green);
 }else{
  if(info.max_energy)std::snprintf(text,sizeof text,"K%d E%d/%d",info.kills,info.energy,info.max_energy);
  else std::snprintf(text,sizeof text,"Kills: %d",info.kills);
  selection_text(art,text,37);
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
  for(unsigned i=0;i<info.upgrade_count;++i){const auto c=HudLayout::upgrade(i);graphics.ui_image(engine.command_icon(info.upgrades[i].icon),c.x,c.y,1,engine);
   C2D_DrawRectSolid(c.x+24,c.y+22,0,12,12,C2D_Color32(12,20,32,235));
   std::snprintf(text,sizeof text,"%d",info.upgrades[i].level);panel_text(art,text,c.x+27,c.y+22,'W',9);
  }
 }
}
}
