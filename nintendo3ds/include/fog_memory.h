#pragma once
#include "engine.h"
#include "bwgame.h"
namespace port3ds {namespace detail {
// Only copy information while native player-zero sight reveals it. Never keep
// native pointers: unit slots can be recycled or hidden buildings can change.
class FogMemory {
    struct Building {
        bwgame::unit_id_32 id;
        int x=0,y=0;
        unsigned owner=0,type=0,stamp=0,count=0;
        bool active=false,visible=false;
        std::array<SpriteView,16> images;
    };
    std::array<Building,64> buildings_;
    std::array<unsigned char,4096> creep_{};
public:
    static bool image(const bwgame::state_functions& f,const bwgame::sprite_t& sprite,const bwgame::image_t& img,unsigned layer,SpriteView& out) {
        using namespace bwgame;
        if((img.flags&image_t::flag_hidden) && !(img.modifier>=2 && img.modifier<=7) && !(sprite.flags&sprite_t::flag_burrowed))return false;
        if(img.modifier<0 || (img.modifier>7 && img.modifier!=9 && img.modifier!=10 && img.modifier!=12))return false;
        const auto p=f.get_image_map_position(&img);
        const bool trail=img.image_type->id==ImageTypes::IMAGEID_High_Templar_Glow;
        const bool glow=img.modifier==9 && img.image_type->color_shift!=5;
        out={p.x,p.y,unsigned(img.image_type->id),unsigned(img.frame_index),unsigned(sprite.owner),
            (img.flags&image_t::flag_horizontally_flipped)!=0,img.modifier==10,
            ((std::uint64_t(sprite.elevation_level)*8192+sprite.position.y)*4096+sprite.index)*64+layer,
            ((img.modifier>=2 && img.modifier<=7) || (sprite.flags&sprite_t::flag_burrowed))?112u:img.modifier==12?96u:trail?128u:img.modifier==9?(glow?255u:128u):255u,glow};
        return true;
    }
    void reset(){for(auto& b:buildings_){b.active=false;b.visible=false;b.count=0;}creep_.fill(0);}
    void update(const bwgame::state_functions& f) {
        using namespace bwgame;const auto& st=f.st;
        const unsigned w=st.game->map_tile_width,h=st.game->map_tile_height;
        for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)if(!(st.tiles[y*w+x].visible&1))creep_[y*64+x]=(st.tiles[y*w+x].flags&tile_t::flag_has_creep)?1:0;
        for(auto& b:buildings_)b.visible=false;
        for(const auto* u:ptr(st.visible_units)) {
            if(u->owner==0 || f.unit_dead(u) || (!f.ut_building(u) && !f.unit_is_mineral_field(u) && !f.unit_is(u,UnitTypes::Resource_Vespene_Geyser)) || f.u_flying(u) ||
               (u->sprite->flags&sprite_t::flag_hidden) || !(u->sprite->visibility_flags&1) || f.unit_is_undetected(u,0))continue;
            const auto id=f.get_unit_id_32(u);Building* slot=nullptr;
            for(auto& b:buildings_)if(b.active && b.id==id){slot=&b;break;}
            if(!slot)for(auto& b:buildings_)if(!b.active){slot=&b;break;}
            if(!slot)slot=&*std::min_element(buildings_.begin(),buildings_.end(),[](const Building& a,const Building& b){return a.stamp<b.stamp;});
            auto& b=*slot;b.active=true;b.visible=true;b.id=id;b.x=u->sprite->position.x;b.y=u->sprite->position.y;
            b.owner=u->owner;b.type=unsigned(u->unit_type->id);b.stamp=st.current_frame;b.count=0;
            unsigned layer=0;for(const auto& img:reverse(u->sprite->images))if(b.count<b.images.size() && image(f,*u->sprite,img,layer++,b.images[b.count]))++b.count;
        }
        for(auto& b:buildings_)if(b.active && !b.visible) {
            const unsigned x=unsigned(b.x/32),y=unsigned(b.y/32);
            // Re-observing an empty footprint removes the memory. Looking up
            // a hidden unit by ID here would reveal unseen deaths or lift-off.
            if(x>=w || y>=h || !(st.tiles[y*w+x].visible&1))b.active=false;
        }
    }
    bool creep(unsigned x,unsigned y)const{return x<64 && y<64 && creep_[y*64+x];}
    std::size_t images(SpriteView* output,std::size_t capacity,int cx,int cy,const bwgame::state_functions& f) const {
        std::size_t n=0;
        for(const auto& b:buildings_)if(b.active && !b.visible)for(unsigned i=0;i<b.count && n<capacity;++i){
            const auto& v=b.images[i];const auto& grp=f.st.global->image_grp.at(v.image);
            const auto& frame=grp->frames.at(v.frame);
            if(v.x>=cx+400 || v.y>=cy+240 || v.x+int(frame.size.x)<=cx || v.y+int(frame.size.y)<=cy)continue;
            output[n++]=v;
        }
        return n;
    }
    std::size_t markers(UnitView* output,std::size_t capacity)const {
        std::size_t n=0;for(const auto& b:buildings_)if(b.active && !b.visible && n<capacity){output[n]={};output[n].x=b.x;output[n].y=b.y;output[n].owner=b.owner;output[n++].type=b.type;}return n;
    }
};
}}
