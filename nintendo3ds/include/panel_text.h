#pragma once
#include "ui_art.h"
namespace port3ds {
// Fixed 10px Roboto Bold, native glyph pixels and a 12px line pitch.
inline int panel_width(UiArt& art,const char* text){int width=0;for(;*text && *text!='\n';++text){char key[24];std::snprintf(key,sizeof key,"t10_W_%02x",unsigned(static_cast<unsigned char>(*text)));unsigned w=0,h=0;if(art.size(key,w,h))width+=w;}return width;}
inline void panel_text(UiArt& art,const char* text,int x,int y,char color='W',int max_width=145){
 const int start=x;for(;*text;++text){if(*text=='\n'){x=start;y+=12;continue;}char key[24];std::snprintf(key,sizeof key,"t10_%c_%02x",color,unsigned(static_cast<unsigned char>(*text)));unsigned w=0,h=0;if(!art.size(key,w,h))continue;if(x+int(w)>start+max_width){while(*text && *text!='\n')++text;if(!*text)break;--text;continue;}art.draw(key,x,y);x+=w;}
}
}
