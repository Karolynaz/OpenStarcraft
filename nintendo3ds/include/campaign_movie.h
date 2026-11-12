#pragma once
#include "audio.h"
#include <citro2d.h>
#include <cstdio>
#include <array>
#include <memory>
#include <string>

namespace port3ds {
inline const char* campaign_movie_after(unsigned scenario) {
    switch(scenario) {
    case 22: return "wasteland-patrol";
    case 31: return "norad-ii";
    case 53: return "open-rebellion";
    case 55: return "inauguration";
    case 26: return "dream";
    case 35: return "amerigo";
    case 38: return "warp";
    case 56: return "invasion-of-aiur";
    case 27: return "fall-of-fenix";
    case 47: return "ambush";
    case 49: return "return-to-aiur";
    case 52: return "death-of-the-overmind";
    default: return nullptr;
    }
}

class CampaignMovie {
    static constexpr unsigned width=256, height=128, fps=12;
    static constexpr unsigned frame_bytes=width*height*2;
    std::unique_ptr<FILE,decltype(&std::fclose)> file_{nullptr,&std::fclose};
    C3D_Tex texture_{};
    Audio audio_;
    std::unique_ptr<std::array<u16,width*height>> pixels_;
    unsigned frames_=0, frame_=~0u;

public:
    ~CampaignMovie(){if(texture_.data){C3D_FrameSync();C3D_TexDelete(&texture_);}}

    bool open(const std::string& directory,const char* name,bool dsp_ready){
        file_.reset(std::fopen((directory+name+".bin").c_str(),"rb"));
        if(!file_ || std::fseek(file_.get(),0,SEEK_END))return false;
        const long size=std::ftell(file_.get());
        if(size<=0 || size%frame_bytes || size/frame_bytes>fps*600)return false;
        frames_=unsigned(size/frame_bytes);
        if(!C3D_TexInit(&texture_,width,height,GPU_RGB565))return false;
        C3D_TexSetFilter(&texture_,GPU_LINEAR,GPU_LINEAR);
        pixels_=std::make_unique<std::array<u16,width*height>>();
        if(!update(0))return false;
        audio_.load(directory.c_str(),dsp_ready,name,nullptr,0,true,true);
        return true;
    }

    bool update(u64 elapsed_us){
        const auto next=elapsed_us*fps/1000000;
        if(next>=frames_)return false;
        if(next==frame_)return true;
        if(std::fseek(file_.get(),long(next*frame_bytes),SEEK_SET)
           || std::fread(pixels_->data(),1,frame_bytes,file_.get())!=frame_bytes)return false;
        auto* dst=static_cast<u16*>(texture_.data);
        for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){
            const unsigned morton=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);
            dst[((y/8)*(width/8)+x/8)*64+morton]=(*pixels_)[y*width+x];
        }
        C3D_TexFlush(&texture_);
        frame_=unsigned(next);
        return true;
    }

    void draw(){
        static constexpr Tex3DS_SubTexture area={width,height,0,1,1,0};
        C2D_DrawImageAt({&texture_,&area},0,24,0,nullptr,400.f/width,192.f/height);
    }
};
}
