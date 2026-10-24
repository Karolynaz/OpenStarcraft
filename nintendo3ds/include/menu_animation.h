#pragma once
#include <citro2d.h>
#include <array>
#include <vector>
#include <cstdio>
#include <cstring>
namespace port3ds {
// Menu-only frame bank. Seven double-buffered 128px surfaces stay bounded even
// when the original SMK cycles contain many frames. Released before gameplay.
class MenuAnimation {
 struct Entry{char name[40];unsigned w,h,count,offset;};
 struct Slot{C3D_Tex tex[2]{};unsigned buffer=0,entry=~0u,frame=~0u;};
 FILE* file_=nullptr;std::vector<Entry> entries_;std::array<Slot,7> slots_{};
 std::array<unsigned short,128*128> pixels_{};
public:
 ~MenuAnimation(){clear();}
 void clear(){if(file_)std::fclose(file_);file_=nullptr;C3D_FrameSync();for(auto& s:slots_){for(auto& t:s.tex)if(t.data){C3D_TexDelete(&t);t={};}s.entry=s.frame=~0u;}entries_.clear();}
 bool load(const char* path){clear();file_=std::fopen(path,"rb");if(!file_)return false;char magic[8];unsigned n;
  bool ok=std::fread(magic,1,8,file_)==8 && !std::memcmp(magic,"OB3AN001",8) && std::fread(&n,4,1,file_)==1 && n>0 && n<=16;
  if(ok){entries_.resize(n);ok=std::fread(entries_.data(),sizeof(Entry),n,file_)==n;}
  if(ok){std::fseek(file_,0,SEEK_END);const long size=std::ftell(file_);for(auto& e:entries_)if(!std::memchr(e.name,0,40) || !e.w || !e.h || e.w>128 || e.h>128 || !e.count || e.count>256 || e.offset<12+n*sizeof(Entry) || e.offset+uint64_t(e.w)*e.h*e.count*2>uint64_t(size))ok=false;}
  if(ok)for(auto& s:slots_)for(auto& t:s.tex){if(!C3D_TexInit(&t,128,128,GPU_RGBA5551)){ok=false;break;}C3D_TexSetFilter(&t,GPU_NEAREST,GPU_NEAREST);C3D_TexSetWrap(&t,GPU_CLAMP_TO_EDGE,GPU_CLAMP_TO_EDGE);}
  if(!ok)clear();return ok;
 }
 bool size(const char* name,unsigned& w,unsigned& h)const{for(auto& e:entries_)if(!std::strcmp(e.name,name)){w=e.w;h=e.h;return true;}return false;}
 bool draw(const char* name,unsigned slot,unsigned tick,float x,float y,float opacity=1){
  if(!file_ || slot>=slots_.size())return false;unsigned index=0;for(;index<entries_.size();++index)if(!std::strcmp(entries_[index].name,name))break;if(index==entries_.size())return false;
  const auto& e=entries_[index];const unsigned frame=tick%e.count;auto& s=slots_[slot];
  if(s.entry!=index || s.frame!=frame){if(std::fseek(file_,e.offset+frame*e.w*e.h*2,SEEK_SET) || std::fread(pixels_.data(),2,e.w*e.h,file_)!=e.w*e.h)return false;
   // The frontend begins every frame with SYNCDRAW. The alternate surface
   // is safe to upload without another GPU/vblank wait inside the frame.
   s.buffer^=1;auto* dst=static_cast<unsigned short*>(s.tex[s.buffer].data);std::memset(dst,0,128*128*2);
   for(unsigned row=0;row<e.h;++row)for(unsigned col=0;col<e.w;++col){const unsigned m=(col&1)|((row&1)<<1)|((col&2)<<1)|((row&2)<<2)|((col&4)<<2)|((row&4)<<3);dst[((row/8)*16+col/8)*64+m]=pixels_[row*e.w+col];}
   C3D_TexFlush(&s.tex[s.buffer]);s.entry=index;s.frame=frame;
  }
  Tex3DS_SubTexture sub={u16(e.w),u16(e.h),0,1,e.w/128.f,1-e.h/128.f};C2D_ImageTint tint;C2D_PlainImageTint(&tint,C2D_Color32(255,255,255,unsigned(opacity*255)),0);return C2D_DrawImageAt({&s.tex[s.buffer],&sub},x,y,0,&tint,1,1);
 }
};
}
