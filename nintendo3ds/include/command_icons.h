#pragma once
#include "bwgame.h"
#include "engine.h"
namespace port3ds {namespace detail {
constexpr unsigned command_icon_image=65380,command_button_image=65381,resource_icon_image=65382;
// Original GRP command art uses 16 ink indices. ticon.pcx contains remap rows,
// not the display palette: its PCX palette deliberately has placeholder colors.
class CommandIcons {
public:
    bwgame::grp_t grp;
    std::array<std::array<uint16_t,14*14>,7> resources{};bool has_resources=false;
    std::array<std::array<uint16_t,256>,2> colors;
    std::array<uint16_t,48*40> button;
    template<class Load,class Decode>void load(Load read,Decode pcx,unsigned race){
        using namespace bwgame;a_vector<uint8_t> bytes;
        read(bytes,"unit/cmdbtns/cmdicons.grp");grp=read_grp(data_loading::data_reader_le(bytes.data(),bytes.data()+bytes.size()));
        if(grp.width!=36 || grp.height!=34 || grp.frames.size()<228 || grp.frames.size()>512)throw std::runtime_error("Command icon GRP bounds");
        for(const auto& f:grp.frames)if(f.offset.x+f.size.x>36 || f.offset.y+f.size.y>34)throw std::runtime_error("Command icon frame bounds");
        const auto name=a_string("game/")+(race==0?"zconsole.pcx":race==2?"pconsole.pcx":"tconsole.pcx");
        bytes.clear();read(bytes,name);
        if(bytes.size()<897 || bytes[bytes.size()-769]!=12)throw std::runtime_error("Console PCX palette absent");
        std::array<uint16_t,256> palette;const auto* p=bytes.data()+bytes.size()-768;
        for(unsigned i=0;i<256;++i)palette[i]=((p[i*3]>>3)<<11)|((p[i*3+1]>>3)<<6)|((p[i*3+2]>>3)<<1)|1;
        const auto remap=pcx("unit/cmdbtns/ticon.pcx");if(remap.size()<32)throw std::runtime_error("Command icon remap bounds");
        for(unsigned state=0;state<2;++state){colors[state]=palette;for(unsigned i=0;i<16;++i)colors[state][i]=palette[remap[state*16+i]];}
        const auto console=pcx(name);if(console.size()!=640*480)throw std::runtime_error("Console dimensions");
        for(unsigned y=0;y<40;++y)for(unsigned x=0;x<48;++x)button[y*48+x]=palette[console[(354+y)*640+490+x]];
        // game/icons.grp uses raw palette indices, unlike command GRP RLE.
        // Keep command art available in loose installations missing this file.
        has_resources=false;
        try {
            bytes.clear();read(bytes,"game/icons.grp");
            if(bytes.size()<6 || data_loading::value_at<uint16_t,true>(bytes.data())<7 || bytes.size()<6+7*8)throw std::runtime_error("Resource icon header");
            for(unsigned frame=0;frame<7;++frame){const auto* p=bytes.data()+6+frame*8;
                const unsigned offset=data_loading::value_at<uint32_t,true>(p+4);
                if(p[2]!=14 || p[3]!=14 || offset>bytes.size() || bytes.size()-offset<196)throw std::runtime_error("Resource icon bounds");
                for(unsigned i=0;i<196;++i)resources[frame][i]=palette[bytes[offset+i]];
            }
            has_resources=true;
        }catch(const std::exception&){}

    }
    SpriteInfo info(unsigned frame)const {if(frame>=grp.frames.size())return {};const auto& f=grp.frames[frame];return {unsigned(f.size.x),unsigned(f.size.y),true};}
    SpriteView view(unsigned frame,bool enabled)const {
        if(frame>=grp.frames.size())return {};const auto& f=grp.frames[frame];return {int(f.offset.x),int(f.offset.y),command_icon_image,frame,enabled?0u:1u,false,false,0,255};
    }
    bool pixels(unsigned frame,bool enabled,uint16_t* output,std::size_t capacity)const {
        const auto bounds=info(frame);if(!bounds.valid || capacity<std::size_t(bounds.width)*bounds.height)return false;
        const auto& f=grp.frames[frame];const auto& palette=colors[enabled?0:1];std::fill_n(output,std::size_t(bounds.width)*bounds.height,0);
        for(unsigned y=0;y<bounds.height;++y){const auto* p=f.data_container.data()+f.line_data_offset[y];unsigned x=0;
            while(x<bounds.width){unsigned token=*p++,n=token&127;if(token&128){x+=n;continue;}
                if(token&64){n=std::min(token&63,bounds.width-x);const auto color=palette[*p++];std::fill_n(output+y*bounds.width+x,n,color);x+=n;}
                else {const unsigned end=x+std::min(n,bounds.width-x);while(x<end)output[y*bounds.width+x++]=palette[*p++];}
            }
        }return true;
    }
};
}}
