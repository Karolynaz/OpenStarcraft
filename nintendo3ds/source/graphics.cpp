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
    tiles_.clear(); map_.clear(); sprites_.clear();decltype(cache_){}.swap(cache_);std::vector<uint16_t>().swap(scratch_);terrain_pages_=0;std::memset(occupied_,0,sizeof occupied_);
}
unsigned Graphics::texture_bytes() const { return page_count_*atlas_page_bytes; }
bool Graphics::load(const char* path, unsigned signature) {
    clear();
    try {
        Input in(path); char magic[8]; in.read(magic,8);
        if (std::memcmp(magic,"OB3DGFX2",8)) throw std::runtime_error("Unsupported graphics.bin version");
        width_=in.u32(); height_=in.u32();
        if (in.u32()!=signature) throw std::runtime_error("Graphics belongs to a different map; prepare again");
        const unsigned pages=in.u32(), tile_count=in.u32(), sprite_count=in.u32(), cells=in.u32();
        if (width_<32 || width_>64 || height_<32 || height_>64 || cells!=width_*height_ ||
            !pages || pages>atlas_max_pages || !tile_count || tile_count>4096 || sprite_count!=0 || pages>3)
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
        terrain_pages_=page_count_;
        while(page_count_<8 && linearSpaceFree()>atlas_page_bytes+256*1024){if(!C3D_TexInit(&pages_[page_count_],atlas_side,atlas_side,GPU_RGBA5551))throw std::runtime_error("Not enough RAM for sprite cache");++page_count_;}
        if(page_count_<terrain_pages_+3)throw std::runtime_error("Not enough linear RAM for sprite working set");
        cache_.reserve(2048);scratch_.resize(512*512);frame_=0;cache_misses_=cache_overflows_=upload_pixels_=0;
        if(std::fgetc(in.file)!=EOF) throw std::runtime_error("Unexpected graphics data after atlas pages");
        loaded_=true; std::snprintf(error_,sizeof error_,"Terrain %u pages + bounded GRP cache",terrain_pages_); return true;
    } catch(const std::exception& e) {
        std::snprintf(error_,sizeof error_,"%s",e.what()); clear(); return false;
    }
}
C2D_Image Graphics::image(const Rect& r, Tex3DS_SubTexture& sub) {
    sub={static_cast<u16>(r.width),static_cast<u16>(r.height),r.x/512.0f,1-r.y/512.0f,
         (r.x+r.width)/512.0f,1-(r.y+r.height)/512.0f};
    return {&pages_[r.page],&sub};
}
void Graphics::draw(const Rect& r, float x, float y, float sx, float sy, bool flip, bool shadow,unsigned opacity,float depth) {
    if(!r.width || !r.height) return;
    Tex3DS_SubTexture sub; const auto img=image(r,sub);
    C2D_ImageTint tint;
    if(!shadow && opacity<255)C2D_PlainImageTint(&tint,C2D_Color32(255,255,255,opacity),0);
    if(shadow) C2D_PlainImageTint(&tint,C2D_Color32(0,0,0,100),1);
    C2D_DrawImageAt(img,flip?x+r.width*sx:x,y,depth,(shadow || opacity<255)?&tint:nullptr,flip?-sx:sx,sy);
}
void Graphics::terrain(int cx,int cy) {
    if(!ready()) return;
    const unsigned x0=std::max(0,cx/32), y0=std::max(0,cy/32);
    const unsigned x1=std::min(width_,static_cast<unsigned>((cx+431)/32));
    const unsigned y1=std::min(height_,static_cast<unsigned>((cy+271)/32));
    // Terrain quads do not overlap; group by texture for bounded draw batches.
    for(unsigned page=0;page<terrain_pages_;++page)
        for(unsigned y=y0;y<y1;++y) for(unsigned x=x0;x<x1;++x) {
            const auto& r=tiles_[map_[y*width_+x]];
            if(r.page==page) draw(r,static_cast<int>(x*32)-cx,static_cast<int>(y*32)-cy);
        }
}
void Graphics::release(const Rect& r) {
    const unsigned w=(r.width+31)/32,h=(r.height+31)/32,x=r.x/32,y=r.y/32;
    const unsigned mask=((1u<<w)-1)<<x;for(unsigned n=0;n<h;++n)occupied_[r.page][y+n]&=~mask;
}
bool Graphics::allocate(unsigned w,unsigned h,Rect& result) {
    const unsigned bw=(w+31)/32,bh=(h+31)/32;if(!bw || !bh || bw>16 || bh>16)return false;
    for(;;){
        for(unsigned p=terrain_pages_;p<page_count_;++p)for(unsigned y=0;y+bh<=16;++y)for(unsigned x=0;x+bw<=16;++x){
            const unsigned mask=((1u<<bw)-1)<<x;bool free=true;
            for(unsigned n=0;n<bh;++n)if(occupied_[p][y+n]&mask){free=false;break;}
            if(!free)continue;for(unsigned n=0;n<bh;++n)occupied_[p][y+n]|=mask;
            result={p,x*32,y*32,w,h};return true;
        }
        auto oldest=cache_.end();for(auto it=cache_.begin();it!=cache_.end();++it)if(it->second.age!=frame_ && (oldest==cache_.end() || it->second.age<oldest->second.age))oldest=it;
        if(oldest==cache_.end())return false;release(oldest->second.rect);cache_.erase(oldest);
    }
}
bool Graphics::cache_frame(const SpriteView& v,const Engine& engine,bool dirty[8]) {
    const auto key=sprite_key(v.image,v.frame,v.owner);auto found=cache_.find(key);
    if(found!=cache_.end()){found->second.age=frame_;return true;}
    const auto info=engine.sprite_info(v.image,v.frame);if(!info.valid)return false;
    if(!info.width || !info.height){cache_.emplace(key,Cached{{terrain_pages_,0,0,0,0},frame_});return true;}
    Rect r;if(!allocate(info.width,info.height,r))return false;
    if(!engine.sprite_pixels(v.image,v.frame,v.owner,scratch_.data(),scratch_.size())){release(r);return false;}
    auto* out=(uint16_t*)pages_[r.page].data;
    for(unsigned y=0;y<r.height;++y)for(unsigned x=0;x<r.width;++x){
        const unsigned px=r.x+x,py=r.y+y;
        const unsigned morton=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);
        out[((py/8)*64+px/8)*64+morton]=scratch_[y*r.width+x];
    }
    cache_.emplace(key,Cached{r,frame_});dirty[r.page]=true;++cache_misses_;upload_pixels_+=r.width*r.height;return true;
}
unsigned Graphics::sprites(const SpriteView* views,std::size_t count,int cx,int cy,const Engine& engine) {
    if(!ready())return count;++frame_;bool dirty[8]{};
    // Pin every texture used by this frame before any eviction/upload. No
    // queued Citro2D sprite can reference a rectangle overwritten this frame.
    for(size_t i=0;i<count;++i){auto it=cache_.find(sprite_key(views[i].image,views[i].frame,views[i].owner));if(it!=cache_.end())it->second.age=frame_;}
    bool fits=true;for(size_t i=0;i<count;++i)if(!cache_frame(views[i],engine,dirty)){fits=false;break;}
    if(!fits){
        cache_.clear();std::memset(occupied_,0,sizeof occupied_);
        std::vector<size_t> sorted(count);for(size_t i=0;i<count;++i)sorted[i]=i;
        std::sort(sorted.begin(),sorted.end(),[&](size_t a,size_t b){const auto x=engine.sprite_info(views[a].image,views[a].frame),y=engine.sprite_info(views[b].image,views[b].frame);return x.height*x.width>y.height*y.width;});
        for(auto i:sorted)cache_frame(views[i],engine,dirty);
    }
    // C3D_FrameEnd(0) flushes the linear heap once, including these uploads.
    // Avoid a second whole-page CPU flush for every cache miss.
    bool changed=false;for(unsigned p=terrain_pages_;p<page_count_;++p)changed=changed||dirty[p];
    if(changed){C2D_Flush();C3D_TexBind(0,&pages_[terrain_pages_]);}
    unsigned missing=0;
    const Cached* ready[2048]{};
    if(count>2048) { // Public API fallback; the application caps views at 2048.
        for(size_t i=0;i<count;++i){const auto& v=views[i];auto it=cache_.find(sprite_key(v.image,v.frame,v.owner));
            if(it==cache_.end()){++missing;++cache_overflows_;continue;}
            draw(it->second.rect,v.x-cx,v.y-cy,1,1,v.flipped,v.shadow,v.opacity);
        }
        return missing;
    }
    for(size_t i=0;i<count;++i) {
        const auto& v=views[i];auto it=cache_.find(sprite_key(v.image,v.frame,v.owner));
        if(it==cache_.end()){++missing;++cache_overflows_;continue;}
        ready[i]=&it->second;
    }
    // Binary-alpha bodies can be grouped by texture. Distinct depth values
    // preserve their original painter order, including transparent GRP holes.
    // Translucent effects follow in painter order against the opaque depth;
    // they must not be grouped, because alpha blending is order-dependent.
    C2D_Flush();C3D_AlphaTest(true,GPU_GREATER,0);
    C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_ALL);
    const float depth_step=1.f/(count+1);
    for(unsigned page=terrain_pages_;page<page_count_;++page)for(size_t i=0;i<count;++i) {
        const auto& v=views[i];const auto* entry=ready[i];
        if(!entry || entry->rect.page!=page || v.shadow || v.opacity<255)continue;
        draw(entry->rect,v.x-cx,v.y-cy,1,1,v.flipped,false,255,(i+1)*depth_step);
    }
    C2D_Flush();C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_COLOR);
    for(size_t i=0;i<count;++i) {
        const auto& v=views[i];if(!ready[i] || (!v.shadow && v.opacity==255))continue;
        draw(ready[i]->rect,v.x-cx,v.y-cy,1,1,v.flipped,v.shadow,v.opacity,(i+1)*depth_step);
    }
    // The following selection bars, cursor, text and minimap are UI overlays.
    C2D_Flush();C3D_DepthTest(false,GPU_ALWAYS,GPU_WRITE_COLOR);
    return missing;
}
void Graphics::minimap(float x,float y,float width,float height) {
    if(ready()) draw(minimap_,x,y,width/minimap_.width,height/minimap_.height);
}
}
