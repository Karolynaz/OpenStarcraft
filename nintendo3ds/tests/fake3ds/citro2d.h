#pragma once
#include "3ds.h"
#include <vector>
using u16=uint16_t;
struct C3D_Tex{void* data=nullptr;};
struct Tex3DS_SubTexture{u16 width,height;float left,top,right,bottom;};
struct C2D_Image{C3D_Tex* tex;Tex3DS_SubTexture* subtex;};
struct C2D_ImageTint{};
enum{GPU_RGBA5551,GPU_NEAREST,GPU_CLAMP_TO_EDGE};
namespace fake {struct Draw{C3D_Tex* tex;Tex3DS_SubTexture sub;};extern std::vector<Draw> draws;}
inline unsigned linearSpaceFree(){return 4*1024*1024-fake::allocations.load()*512*1024;}
inline bool C3D_TexInit(C3D_Tex* t,unsigned w,unsigned h,unsigned){t->data=linearAlloc(w*h*2);return t->data;}
inline void C3D_TexDelete(C3D_Tex* t){linearFree(t->data);t->data=nullptr;}
inline void C3D_FrameSync(){}
inline void C3D_TexSetFilter(C3D_Tex*,unsigned,unsigned){}
inline void C3D_TexSetWrap(C3D_Tex*,unsigned,unsigned){}
inline void C3D_TexFlush(C3D_Tex*){}
inline void C3D_TexBind(unsigned,C3D_Tex*){}
inline void C2D_Flush(){}
inline unsigned C2D_Color32(unsigned,unsigned,unsigned,unsigned){return 0;}
inline void C2D_PlainImageTint(C2D_ImageTint*,unsigned,float){}
inline bool C2D_DrawImageAt(C2D_Image i,float,float,float,C2D_ImageTint*,float,float){fake::draws.push_back({i.tex,*i.subtex});return true;}
