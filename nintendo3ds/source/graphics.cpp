#include "graphics.h"
#include "graphics_format.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
namespace port3ds {
namespace {
struct Input {
    FILE* file;
    explicit Input(const char* path) : file(std::fopen(path, "rb")) { if (!file) throw std::runtime_error("Prepare graphics.bin on the computer first"); }
    ~Input() { std::fclose(file); }
    void read(void* dst, unsigned size) { if (std::fread(dst, 1, size, file) != size) throw std::runtime_error("Truncated graphics.bin"); }
    unsigned u16() { unsigned char b[2]; read(b,2); return b[0] | (b[1]<<8); }
    unsigned u32() { const auto low=u16(); return low | (u16()<<16); }
};
}
Graphics::~Graphics() { clear(); }
void Graphics::clear() {
    if (page_count_) C3D_FrameSync();
    for (unsigned i=0;i<page_count_;++i) C3D_TexDelete(&pages_[i]);
    page_count_=0; loaded_=false;
    tiles_.clear(); map_.clear(); sprites_.clear();
}
unsigned Graphics::texture_bytes() const { return page_count_*atlas_page_bytes; }
bool Graphics::load(const char* path, unsigned signature) {
    clear();
    try {
        Input in(path); char magic[8]; in.read(magic,8);
        if (std::memcmp(magic,"OB3DGFX1",8)) throw std::runtime_error("Unsupported graphics.bin version");
        width_=in.u32(); height_=in.u32();
        if (in.u32()!=signature) throw std::runtime_error("Graphics belongs to a different map; prepare again");
        const unsigned pages=in.u32(), tile_count=in.u32(), sprite_count=in.u32(), cells=in.u32();
        if (width_<32 || width_>64 || height_<32 || height_>64 || cells!=width_*height_ ||
            !pages || pages>atlas_max_pages || !tile_count || tile_count>4096 || sprite_count>10000)
            throw std::runtime_error("Graphics exceeds original 3DS limits");
        auto rect = [&]() {
            Rect r{in.u16(),in.u16(),in.u16(),in.u16(),in.u16()};
            if (r.page>=pages || r.x+r.width>atlas_side || r.y+r.height>atlas_side)
                throw std::runtime_error("Atlas rectangle out of bounds");
            return r;
        };
        minimap_=rect(); tiles_.resize(tile_count); map_.resize(cells); sprites_.resize(sprite_count);
        for(auto& r:tiles_) r=rect();
        for(auto& t:map_) { t=in.u16(); if(t>=tile_count) throw std::runtime_error("Invalid tile lookup"); }
        unsigned previous=0;
        for(unsigned i=0;i<sprite_count;++i) {
            auto& s=sprites_[i]; s.key=in.u32(); s.rect=rect();
            if(i && s.key<=previous) throw std::runtime_error("Unsorted sprite lookup");
            previous=s.key;
        }
        for(unsigned i=0;i<pages;++i) {
            if(!C3D_TexInit(&pages_[i],atlas_side,atlas_side,GPU_RGBA5551))
                throw std::runtime_error("Not enough linear RAM for graphics atlas");
            ++page_count_;
            in.read(pages_[i].data,atlas_page_bytes);
            C3D_TexSetFilter(&pages_[i],GPU_NEAREST,GPU_NEAREST);
            C3D_TexSetWrap(&pages_[i],GPU_CLAMP_TO_EDGE,GPU_CLAMP_TO_EDGE);
            C3D_TexFlush(&pages_[i]);
        }
        if(std::fgetc(in.file)!=EOF) throw std::runtime_error("Unexpected graphics data after atlas pages");
        loaded_=true; std::snprintf(error_,sizeof error_,"Graphics: %u atlas pages",pages); return true;
    } catch(const std::exception& e) {
        std::snprintf(error_,sizeof error_,"%s",e.what()); clear(); return false;
    }
}
C2D_Image Graphics::image(const Rect& r, Tex3DS_SubTexture& sub) {
    sub={static_cast<u16>(r.width),static_cast<u16>(r.height),r.x/512.0f,1-r.y/512.0f,
         (r.x+r.width)/512.0f,1-(r.y+r.height)/512.0f};
    return {&pages_[r.page],&sub};
}
void Graphics::draw(const Rect& r, float x, float y, float sx, float sy, bool flip, bool shadow) {
    if(!r.width || !r.height) return;
    Tex3DS_SubTexture sub; const auto img=image(r,sub);
    C2D_ImageTint tint;
    if(shadow) C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,100),1);
    C2D_DrawImageAt(img,flip?x+r.width*sx:x,y,0,shadow?&tint:nullptr,flip?-sx:sx,sy);
}
void Graphics::terrain(int cx,int cy) {
    if(!ready()) return;
    const unsigned x0=std::max(0,cx/32), y0=std::max(0,cy/32);
    const unsigned x1=std::min(width_,static_cast<unsigned>((cx+431)/32));
    const unsigned y1=std::min(height_,static_cast<unsigned>((cy+271)/32));
    // Terrain quads do not overlap; group by texture for bounded draw batches.
    for(unsigned page=0;page<page_count_;++page)
        for(unsigned y=y0;y<y1;++y) for(unsigned x=x0;x<x1;++x) {
            const auto& r=tiles_[map_[y*width_+x]];
            if(r.page==page) draw(r,static_cast<int>(x*32)-cx,static_cast<int>(y*32)-cy);
        }
}
unsigned Graphics::sprites(const SpriteView* views,std::size_t count,int cx,int cy) {
    if(!ready()) return count;
    unsigned missing=0;
    for(std::size_t i=0;i<count;++i) {
        const auto& v=views[i]; const auto key=sprite_key(v.image,v.frame,v.owner);
        const auto it=std::lower_bound(sprites_.begin(),sprites_.end(),key,[](const Sprite& a,unsigned b){return a.key<b;});
        if(it==sprites_.end() || it->key!=key) { ++missing; continue; }
        draw(it->rect,v.x-cx,v.y-cy,1,1,v.flipped,v.shadow);
    }
    return missing;
}
void Graphics::minimap(float x,float y,float width,float height) {
    if(ready()) draw(minimap_,x,y,width/minimap_.width,height/minimap_.height);
}
}
