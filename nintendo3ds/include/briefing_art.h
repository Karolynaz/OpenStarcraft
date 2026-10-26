#pragma once
#include "frontend.h"
#include "ui_art.h"
namespace port3ds {
// Menu-only room and a single on-demand portrait. The one-page shot remains
// through core loading, then is released after the first gameplay frame. Its 64 KiB texture
// fits the original-model linear headroom alongside the loaded world.
class BriefingArt {
 UiArt shot_,room_;C3D_Tex face_{};unsigned mission_=~0u,portrait_=110;
public:
 ~BriefingArt(){clear();}
 void clear(){shot_.clear();room_.clear();if(face_.data){C3D_FrameSync();C3D_TexDelete(&face_);face_={};}mission_=~0u;portrait_=110;}
 bool load_shot(const Frontend& f,const char* root){return shot_.load((std::string(root)+"campaign/"+campaign::missions[f.mission].key+"-shot.bin").c_str());}
 void release_room(){room_.clear();if(face_.data){C3D_FrameSync();C3D_TexDelete(&face_);face_={};}portrait_=110;}
 bool sync(const Frontend& f,const char* root){
  if(!f.rich_briefing || !f.presentation()){if(mission_!=~0u)clear();return true;}
  if(mission_!=f.mission){clear();const auto folder=std::string(root)+"campaign/";
   if(!shot_.load((folder+campaign::missions[f.mission].key+"-shot.bin").c_str()))return false;
   const char* race=campaign::missions[f.mission].race==0?"zerg":campaign::missions[f.mission].race==2?"protoss":"terran";
   if(!room_.load((folder+"briefing-"+race+".bin").c_str())){clear();return false;}mission_=f.mission;
  }
  const unsigned id=f.dialog==Frontend::Dialog::briefing?f.briefing_portrait():110;
  if(id!=portrait_){portrait_=110;if(face_.data){C3D_FrameSync();C3D_TexDelete(&face_);face_={};}
   if(id<110){const auto path=std::string(root)+"portraits/"+std::to_string(id)+".bin";
    if(FILE* file=std::fopen(path.c_str(),"rb")){unsigned short pixels[48*48];bool ok=std::fread(pixels,2,48*48,file)==48*48 && std::fgetc(file)==EOF;std::fclose(file);
     if(ok && C3D_TexInit(&face_,64,64,GPU_RGBA5551)){auto* out=static_cast<unsigned short*>(face_.data);std::memset(out,0,64*64*2);
      for(unsigned y=0;y<48;++y)for(unsigned x=0;x<48;++x){unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);out[((y/8)*8+x/8)*64+m]=pixels[y*48+x];}
      C3D_TexSetFilter(&face_,GPU_LINEAR,GPU_LINEAR);C3D_TexFlush(&face_);portrait_=id;
   }}}
  }return true;
 }
 void shot(){shot_.draw("shot",0,0,400.f/256,240.f/128);}
 void room(){room_.draw("room",0,0);}
 void portrait(){if(portrait_>=110 || !face_.data)return;Tex3DS_SubTexture sub={48,48,0,1,.75f,.25f};C2D_DrawImageAt({&face_,&sub},31,69,0,nullptr,1.4f,1.4f);}
};
}
