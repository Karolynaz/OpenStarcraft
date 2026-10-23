#pragma once
#include <citro2d.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdint>
namespace port3ds {
// Menu art is released before the simulation/atlas allocation peak. The HUD
// alone remains resident during a match (256 KiB of linear RAM).
class UiArt {
 struct Entry{char name[40];unsigned page,x,y,w,h;};
 C3D_Tex preview_{};bool preview_ready_=false;
 std::vector<Entry> entries_;std::vector<C3D_Tex> pages_;unsigned width_=0,height_=0;
public:
 ~UiArt(){clear();}
 void clear(){if(preview_.data){C3D_FrameSync();C3D_TexDelete(&preview_);preview_={};preview_ready_=false;}if(!pages_.empty())C3D_FrameSync();for(auto& p:pages_)if(p.data)C3D_TexDelete(&p);pages_.clear();entries_.clear();}
 bool load(const char* path){
  clear();FILE* f=std::fopen(path,"rb");if(!f)return false;
  char magic[8];unsigned header[4];bool ok=std::fread(magic,1,8,f)==8 && !std::memcmp(magic,"OB3UI001",8) && std::fread(header,4,4,f)==4;
  if(ok)ok=header[0]==512 && (header[1]==256 || header[1]==512) && header[2]>0 && header[2]<=9 && header[3]<=256;
  if(ok){width_=header[0];height_=header[1];entries_.resize(header[3]);ok=std::fread(entries_.data(),sizeof(Entry),entries_.size(),f)==entries_.size();}
  if(ok)for(const auto& e:entries_)if(!std::memchr(e.name,0,40) || e.page>=header[2] || !e.w || !e.h || e.x+e.w>width_ || e.y+e.h>height_)ok=false;
  if(ok){pages_.resize(header[2]);for(auto& p:pages_){if(!C3D_TexInit(&p,width_,height_,GPU_RGBA5551)){ok=false;break;}if(std::fread(p.data,2,width_*height_,f)!=width_*height_){ok=false;break;}C3D_TexSetFilter(&p,GPU_NEAREST,GPU_NEAREST);C3D_TexSetWrap(&p,GPU_CLAMP_TO_EDGE,GPU_CLAMP_TO_EDGE);C3D_TexFlush(&p);}}
  if(ok)ok=std::fgetc(f)==EOF;std::fclose(f);if(!ok)clear();return ok;
 }
 void preview(const std::uint16_t* pixels){
  C3D_FrameSync();if(!preview_.data && !C3D_TexInit(&preview_,128,128,GPU_RGBA5551))return;
  auto* out=static_cast<std::uint16_t*>(preview_.data);for(unsigned y=0;y<128;++y)for(unsigned x=0;x<128;++x){const unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);out[((y/8)*16+x/8)*64+m]=pixels[y*128+x];}
  C3D_TexSetFilter(&preview_,GPU_NEAREST,GPU_NEAREST);C3D_TexSetWrap(&preview_,GPU_CLAMP_TO_EDGE,GPU_CLAMP_TO_EDGE);C3D_TexFlush(&preview_);preview_ready_=true;
 }
 bool draw_preview(float x,float y,float w,float h){if(!preview_ready_)return false;Tex3DS_SubTexture sub={128,128,0,1,1,0};return C2D_DrawImageAt({&preview_,&sub},x,y,0,nullptr,w/128,h/128);}
 bool size(const char* name,unsigned& w,unsigned& h)const{for(const auto& e:entries_)if(!std::strcmp(e.name,name)){w=e.w;h=e.h;return true;}return false;}
 bool draw(const char* name,float x,float y,float sx=1,float sy=1,float brightness=1){
  for(const auto& e:entries_)if(!std::strcmp(e.name,name)){Tex3DS_SubTexture sub={static_cast<u16>(e.w),static_cast<u16>(e.h),e.x/float(width_),1-e.y/float(height_),(e.x+e.w)/float(width_),1-(e.y+e.h)/float(height_)};C2D_ImageTint tint;C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,255),1-brightness);return C2D_DrawImageAt({&pages_[e.page],&sub},x,y,0,brightness<1?&tint:nullptr,sx,sy);}return false;
 }
};
}
