#pragma once
#include <citro2d.h>
#include <string>
// Capture renderer text calls, without pretending to reproduce system fonts.
using C2D_TextBuf=int*;
struct C2D_Text{std::string value;};
enum{C2D_WithColor=1,C2D_WordWrap=2};
namespace text_calls{std::vector<std::string> values;}
inline C2D_TextBuf C2D_TextBufNew(unsigned){return new int;}
inline void C2D_TextBufDelete(C2D_TextBuf p){delete p;}
inline void C2D_TextBufClear(C2D_TextBuf){}
inline void C2D_TextParse(C2D_Text* t,C2D_TextBuf,const char* s){t->value=s;}
inline void C2D_TextOptimize(C2D_Text*){}
inline void C2D_TextGetDimensions(C2D_Text* t,float sx,float sy,float* w,float* h){*w=t->value.size()*16*sx;*h=30*sy;}
template<typename... T>inline void C2D_DrawText(C2D_Text* t,unsigned,float,float,float,float,float,unsigned,T...){text_calls::values.push_back(t->value);}
inline void C2D_DrawRectSolid(float,float,float,float,float,unsigned){}
inline void C2D_DrawLine(float,float,unsigned,float,float,unsigned,float,float){}
