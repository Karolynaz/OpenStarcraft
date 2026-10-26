#pragma once
#include "frontend.h"
#include "ui_art.h"
namespace port3ds {
// Original menu composition and four unmirrored, on-demand speaker portraits.
// Only the 64 KiB shot + 32 KiB metal frame survive into the warming frame.
class BriefingArt {
 UiArt shot_,frame_,room_;std::array<C3D_Tex,4> faces_{};
 std::array<unsigned,4> portraits_{{110,110,110,110}};unsigned mission_=~0u;
 void clear_faces(){bool any=false;for(const auto& face:faces_)any=any || face.data;if(any)C3D_FrameSync();for(auto& face:faces_)if(face.data){C3D_TexDelete(&face);face={};}portraits_.fill(110);}
public:
 ~BriefingArt(){clear();}
 void clear(){shot_.clear();frame_.clear();room_.clear();clear_faces();mission_=~0u;}
 bool load_shot(const Frontend& f,const char* root){const auto folder=std::string(root)+"campaign/";return shot_.load((folder+campaign::missions[f.mission].key+"-shot.bin").c_str()) && frame_.load((folder+"briefing-frame.bin").c_str());}
 void release_room(){room_.clear();clear_faces();}
 bool sync(const Frontend& f,const char* root){
  if(!f.rich_briefing || !f.presentation()){if(mission_!=~0u)clear();return true;}
  if(mission_!=f.mission){clear();const auto folder=std::string(root)+"campaign/";
   if(!load_shot(f,root))return false;
   const char* race=campaign::missions[f.mission].race==0?"zerg":campaign::missions[f.mission].race==2?"protoss":"terran";
   if(!room_.load((folder+"briefing-"+race+".bin").c_str())){clear();return false;}mission_=f.mission;
  }
  std::array<unsigned,4> cast{{110,110,110,110}};
  if(f.dialog==Frontend::Dialog::briefing && !f.briefing_cast.empty())cast=f.briefing_cast[std::min(f.briefing_page,unsigned(f.briefing_cast.size()-1))];
  for(unsigned slot=0;slot<4;++slot){const unsigned id=cast[slot];auto& face=faces_[slot];
   if(id==portraits_[slot])continue;
   C3D_FrameSync();if(face.data){C3D_TexDelete(&face);face={};}portraits_[slot]=110;
   if(id>=110)continue;
   const auto path=std::string(root)+"portraits/"+std::to_string(id)+".bin";
   if(FILE* file=std::fopen(path.c_str(),"rb")){unsigned short pixels[48*48];bool ok=std::fread(pixels,2,48*48,file)==48*48 && std::fgetc(file)==EOF;std::fclose(file);
    if(ok && C3D_TexInit(&face,64,64,GPU_RGBA5551)){auto* out=static_cast<unsigned short*>(face.data);std::memset(out,0,64*64*2);
     for(unsigned y=0;y<48;++y)for(unsigned x=0;x<48;++x){unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);out[((y/8)*8+x/8)*64+m]=pixels[y*48+x];}
     C3D_TexSetFilter(&face,GPU_LINEAR,GPU_LINEAR);C3D_TexFlush(&face);portraits_[slot]=id;
   }}
  }return true;
 }
 void shot(){shot_.draw("shot",0,0,400.f/256,240.f/128);}
 void frame(){frame_.draw("frame",12,18,376.f/128,206.f/128);}
 void room(){room_.draw("room",0,0);}
 void portraits(const Frontend& f){
  static const float x[]={16.5f,91.5f,169,239.5f},y[]={49,13,13,49};
  for(unsigned i=0;i<4;++i){if(portraits_[i]>=110 || !faces_[i].data)continue;
   Tex3DS_SubTexture sub={48,48,0,1,.75f,.25f};C2D_DrawImageAt({&faces_[i],&sub},x[i],y[i],0,nullptr,64.f/48,59.f/48);
  }
  const auto active=f.briefing_slot();if(active<4){char key[16];std::snprintf(key,sizeof key,"active%u",active);room_.draw(key,x[active],y[active]);}
 }
};
}
