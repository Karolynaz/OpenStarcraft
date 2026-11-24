#pragma once
#include "bwgame.h"
#include "engine.h"
namespace port3ds {namespace detail {
constexpr unsigned command_icon_image=65380,command_button_image=65381,resource_icon_image=65382,wireframe_image=65383;
// Original GRP command art uses 16 ink indices. ticon.pcx contains remap rows,
// not the display palette: its PCX palette deliberately has placeholder colors.
class CommandIcons {
public:
    bwgame::grp_t grp,wireframes,group_wireframes;bool has_wireframes=false;
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
        has_wireframes=false;
        try{bytes.clear();read(bytes,"unit/wirefram/wirefram.grp");wireframes=read_grp(data_loading::data_reader_le(bytes.data(),bytes.data()+bytes.size()));
            if(wireframes.width!=64 || wireframes.height!=64 || wireframes.frames.size()!=228)throw std::runtime_error("Wireframe GRP bounds");
            for(const auto& f:wireframes.frames)if(f.offset.x+f.size.x>64 || f.offset.y+f.size.y>64)throw std::runtime_error("Wireframe crop bounds");has_wireframes=true;
        }catch(const std::exception&){wireframes=grp_t{};}
        try{
            bytes.clear();read(bytes,"unit/wirefram/grpwire.grp");
            group_wireframes=read_grp(data_loading::data_reader_le(bytes.data(),bytes.data()+bytes.size()));
            if(group_wireframes.width!=32 || group_wireframes.height!=32 || group_wireframes.frames.size()>228)
                throw std::runtime_error("Group wireframe GRP bounds");
            for(const auto& f:group_wireframes.frames)if(f.offset.x+f.size.x>32 || f.offset.y+f.size.y>32)
                throw std::runtime_error("Group wireframe crop bounds");
        }catch(const std::exception&){group_wireframes=grp_t{};}
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
    bool wire_pixels(unsigned frame,uint16_t* out,size_t capacity,bool group=false)const {
        const auto& art=group?group_wireframes:wireframes;
        const unsigned dim=group?32:64,steps_per_shield=group?9:3;
        if(frame>=228*2*steps_per_shield || frame%228>=art.frames.size() || capacity<dim*dim)return false;
        std::fill_n(out,dim*dim,0);
        const unsigned state=frame/228,steps=state%steps_per_shield;
        const unsigned damage=group?(steps==0?0:steps<=5?1:2):steps;
        const bool shield=state>=steps_per_shield;const auto& f=art.frames[frame%228];
        auto rgb=[](unsigned r,unsigned g,unsigned b){return uint16_t((r>>3)<<11|(g>>3)<<6|(b>>3)<<1|1);};
        auto color=[&](unsigned c)->uint16_t{
            if(!c)return 0;if(c==192 || c==193)return shield?(c==192?rgb(72,136,255):rgb(48,92,190)):0;
            if(c>=216 && c<=219){static const unsigned gradient[8][3]={{64,240,64},{150,220,40},{232,140,20},{228,50,24},{200,70,170},{110,60,190},{50,50,160},{28,28,100}};const auto& g=gradient[std::min(7u,c-216+2*damage)];return rgb(g[0],g[1],g[2]);}
            const unsigned part=group?steps/4+((c&3)<steps%4):damage;
            return part==0?rgb(44,228,52):part==1?rgb(232,208,16):rgb(216,24,24);
        };
        for(unsigned y=0;y<f.size.y;++y){const auto* p=f.data_container.data()+f.line_data_offset[y];unsigned x=0;
            while(x<f.size.x){unsigned token=*p++,n=token&127;if(token&128){x+=n;continue;}if(token&64){n=std::min(token&63,unsigned(f.size.x)-x);const auto c=color(*p++);while(n--)out[(y+f.offset.y)*dim+f.offset.x+x++]=c;}
            else {n=std::min(n,unsigned(f.size.x)-x);while(n--)out[(y+f.offset.y)*dim+f.offset.x+x++]=color(*p++);}}
        }return true;
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
