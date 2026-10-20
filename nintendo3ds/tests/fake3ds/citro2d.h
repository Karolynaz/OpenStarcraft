#pragma once
#include "3ds.h"
#include <vector>
using u16=uint16_t;
struct C3D_Tex{void* data=nullptr;};
struct Tex3DS_SubTexture{u16 width,height;float left,top,right,bottom;};
struct C2D_Image{C3D_Tex* tex;Tex3DS_SubTexture* subtex;};
struct C2D_ImageTint{unsigned color=0;float blend=0;};
enum{GPU_RGBA5551,GPU_RGBA4,GPU_NEAREST,GPU_CLAMP_TO_EDGE,GPU_GREATER,GPU_GEQUAL,GPU_ALWAYS,GPU_WRITE_ALL,GPU_WRITE_COLOR,GPU_BLEND_ADD,GPU_SRC_ALPHA,GPU_ONE,GPU_ONE_MINUS_SRC_ALPHA};
namespace fake {struct DepthState{bool enabled=false,alpha=false,additive=false;unsigned write=GPU_WRITE_COLOR;};inline DepthState& depth_state(){static DepthState s;return s;}struct Draw{C3D_Tex* tex;Tex3DS_SubTexture sub;float x,y,depth,sx,sy;unsigned tint;float blend;bool depth_test,alpha_test;unsigned depth_write;bool additive;};extern std::vector<Draw> draws;}
inline unsigned linearSpaceFree(){return 4*1024*1024-fake::allocations.load()*512*1024;}
inline bool C3D_TexInit(C3D_Tex* t,unsigned w,unsigned h,unsigned){t->data=linearAlloc(w*h*2);return t->data;}
inline void C3D_TexDelete(C3D_Tex* t){linearFree(t->data);t->data=nullptr;}
inline void C3D_FrameSync(){}
inline void C3D_TexSetFilter(C3D_Tex*,unsigned,unsigned){}
inline void C3D_TexSetWrap(C3D_Tex*,unsigned,unsigned){}
inline void C3D_TexFlush(C3D_Tex*){}
inline void C3D_TexBind(unsigned,C3D_Tex*){}
inline void C2D_Flush(){}
inline void C3D_AlphaBlend(unsigned,unsigned,unsigned,unsigned dst,unsigned,unsigned){fake::depth_state().additive=dst==GPU_ONE;}
inline void C3D_AlphaTest(bool enabled,unsigned,unsigned){fake::depth_state().alpha=enabled;}
inline void C3D_DepthTest(bool enabled,unsigned,unsigned write){fake::depth_state().enabled=enabled;fake::depth_state().write=write;}
inline unsigned C2D_Color32(unsigned r,unsigned g,unsigned b,unsigned a){return r|(g<<8)|(b<<16)|(a<<24);}
inline void C2D_PlainImageTint(C2D_ImageTint* t,unsigned c,float blend){t->color=c;t->blend=blend;}
inline bool C2D_DrawImageAt(C2D_Image i,float x,float y,float depth,C2D_ImageTint* tint,float sx,float sy){fake::draws.push_back({i.tex,*i.subtex,x,y,depth,sx,sy,tint?tint->color:0xff000000,tint?tint->blend:0,fake::depth_state().enabled,fake::depth_state().alpha,fake::depth_state().write,fake::depth_state().additive});return true;}
