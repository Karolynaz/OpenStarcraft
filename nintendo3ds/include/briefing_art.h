#pragma once
#include "frontend.h"
#include "ui_art.h"
#include "menu_animation.h"
namespace port3ds {
// Original menu composition and four unmirrored, on-demand speaker portraits.
// Native dual-screen story pages are released before briefing. Only the
// 64 KiB race-room thumbnail + 32 KiB metal frame survive into warming.
class BriefingArt {
 struct Motion {char key[16]{},hover[16]{};float x=0,y=0,w=0,h=0;unsigned flags=0,widget=0;};
 std::vector<Motion> motions_;unsigned origin_=~0u,hover_origin_=0;bool hovered_=false;
 UiArt shot_,frame_,room_,story_,upper_,panels_;bool story_ready_=false,room_ready_=false;std::array<C3D_Tex,4> faces_{};
 std::array<unsigned,4> portraits_{{110,110,110,110}};unsigned mission_=~0u;
 void clear_faces(){bool any=false;for(const auto& face:faces_)any=any || face.data;if(any)C3D_FrameSync();for(auto& face:faces_)if(face.data){C3D_TexDelete(&face);face={};}portraits_.fill(110);}
public:
 ~BriefingArt(){clear();}
 void clear(){shot_.clear();frame_.clear();room_.clear();story_.clear();upper_.clear();panels_.clear();story_ready_=room_ready_=false;clear_faces();mission_=~0u;motions_.clear();origin_=~0u;}
 bool load_shot(const Frontend& f,const char* root){const auto folder=std::string(root)+"campaign/";const char* race=campaign::missions[f.mission].race==0?"zerg":campaign::missions[f.mission].race==2?"protoss":"terran";return shot_.load((folder+"briefing-loading-"+race+".bin").c_str()) && (frame_.load((folder+"briefing-frame-"+race+".bin").c_str()) || frame_.load((folder+"briefing-frame.bin").c_str()));}
 void release_room(){room_.clear();story_.clear();upper_.clear();panels_.clear();story_ready_=room_ready_=false;clear_faces();}
 bool sync(const Frontend& f,const char* root){
  if(!f.rich_briefing || !f.presentation()){if(mission_!=~0u)clear();return true;}
  bool changed=false;
  if(mission_!=f.mission){changed=true;clear();const auto folder=std::string(root)+"campaign/";
   if(!load_shot(f,root))return false;
   mission_=f.mission;
   if(FILE* file=std::fopen((folder+campaign::missions[f.mission].key+".motion").c_str(),"r")){Motion m;while(motions_.size()<3 && std::fscanf(file,"%15s%15s%f%f%f%f%u%u",m.key,m.hover,&m.x,&m.y,&m.w,&m.h,&m.flags,&m.widget)==8)motions_.push_back(m);std::fclose(file);}
  }
  if(f.dialog==Frontend::Dialog::story){
   if(!story_ready_){changed=true;const auto path=std::string(root)+"campaign/"+campaign::missions[f.mission].key+"-story.bin";
    if(!story_.load(path.c_str())){clear();return false;}story_ready_=true;}
  }else{
   // Release both 256 KiB story pages before room, movies and voices load.
   if(story_ready_){changed=true;story_.clear();story_ready_=false;}
   if(!room_ready_){changed=true;const char* race=campaign::missions[f.mission].race==0?"zerg":campaign::missions[f.mission].race==2?"protoss":"terran";
    const auto folder=std::string(root)+"campaign/";
    if(!room_.load((folder+"briefing-"+race+".bin").c_str()) || !upper_.load((folder+"briefing-upper-"+race+".bin").c_str()) || !panels_.load((folder+"briefing-panels-"+race+".bin").c_str())){clear();return false;}room_ready_=true;}
  }
  std::array<unsigned,4> cast{{110,110,110,110}};
  if(f.dialog==Frontend::Dialog::briefing && !f.briefing_cast.empty())cast=f.briefing_cast[std::min(f.briefing_page,unsigned(f.briefing_cast.size()-1))];
  for(unsigned slot=0;slot<4;++slot){const unsigned id=cast[slot];auto& face=faces_[slot];
   if(id==portraits_[slot])continue;changed=true;
   C3D_FrameSync();if(face.data){C3D_TexDelete(&face);face={};}portraits_[slot]=110;
   if(id>=110)continue;
   const auto path=std::string(root)+"portraits/"+std::to_string(id)+".bin";
   if(FILE* file=std::fopen(path.c_str(),"rb")){unsigned short pixels[48*48];bool ok=std::fread(pixels,2,48*48,file)==48*48 && std::fgetc(file)==EOF;std::fclose(file);
    if(ok && C3D_TexInit(&face,64,64,GPU_RGBA5551)){auto* out=static_cast<unsigned short*>(face.data);std::memset(out,0,64*64*2);
     for(unsigned y=0;y<48;++y)for(unsigned x=0;x<48;++x){unsigned m=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);out[((y/8)*8+x/8)*64+m]=pixels[y*48+x];}
     C3D_TexSetFilter(&face,GPU_LINEAR,GPU_LINEAR);C3D_TexFlush(&face);portraits_[slot]=id;
   }}
  }if(changed)C2D_Prepare();return true;
 }
 void shot(){shot_.draw("shot",0,0,400.f/256,240.f/128);}
 bool briefing_top(){return upper_.draw("top",0,0);}
 bool story_top(){return story_.draw("top",0,0);}
 bool story_bottom(){return story_.draw("bottom",0,0);}
 void frame(){unsigned w=128,h=128;frame_.size("frame",w,h);frame_.draw("frame",0,0,400.f/w,240.f/h);}
 void text_panel(const Frontend& f){panels_.draw("text",0,f.briefing_loading?0:-240*f.briefing_slide());}
 void room(const Frontend& f,MenuAnimation& animation,unsigned tick){
  room_.draw("room",0,0);if(origin_==~0u)origin_=tick;
  const bool hover=f.hover==116;if(hover!=hovered_){hovered_=hover;hover_origin_=tick;}
  for(unsigned i=0;i<motions_.size();++i){const auto& m=motions_[i];if((m.flags&8) && !hover)continue;const auto* key=hover && m.hover[0]!='-'?m.hover:m.key;unsigned w,h;if(!animation.size(key,w,h))continue;
   const bool hover_clip=(m.flags&8) || key==m.hover;
   animation.draw(key,4+i,tick-(hover_clip?hover_origin_:origin_),m.x,m.y,((m.flags&2) && !hover)?.65f:1.f,m.w/w,m.h/h,m.flags&4);
  }
  const float slide=f.briefing_slide();
  // Each exported layer is cropped losslessly in its atlas. These are its
  // original canvas offsets, identical for all three race exports.
  panels_.draw("left",-320*slide,111);
  panels_.draw("right",212+320*slide,8);
  panels_.draw("top",0,-240*slide);
  panels_.draw("cancel",108,182+240*slide);
 }
 void portraits(const Frontend& f,MenuAnimation& animation,unsigned tick){
  for(unsigned i=0;i<4;++i){if(portraits_[i]>=110 || !faces_[i].data)continue;
   const auto r=BriefingLayout::face(i);const float slide=f.briefing_slide();
   const float x=r.x+(i==0?-320:i==3?320:0)*slide,y=r.y+((i==1 || i==2)?-240:0)*slide;
   char key[32];const bool talk=f.briefing_speaking && f.briefing_slot()==i;std::snprintf(key,sizeof key,"p%u_%s",portraits_[i],talk?"talk":"idle");unsigned w,h;
   if(animation.size(key,w,h) && animation.draw(key,i,tick,x,y,1,float(r.w)/w,float(r.h)/h))continue;
   Tex3DS_SubTexture sub={48,48,0,1,.75f,.25f};C2D_DrawImageAt({&faces_[i],&sub},x,y,0,nullptr,float(r.w)/48,float(r.h)/48);
  }
  // Active speaker outline is drawn by frontend_draw with Figma geometry,
  // not by the old64x59 frame sprites.
 }
};
}
