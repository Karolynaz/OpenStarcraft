#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>
// Independent software composition oracle: compare the old ordered sprite
// stream against the actual reordered draw calls, depth flags and texture bytes.
inline std::uint32_t blend_pixel(std::uint32_t dst,std::uint16_t pixel,unsigned alpha,bool black) {
    std::uint32_t result=0;
    for(unsigned channel=0;channel<3;++channel) {
        const unsigned shift=channel==0?11:channel==1?6:1;
        const unsigned src=black?0:((pixel>>shift)&31)*255/31;
        const unsigned old=(dst>>(channel*8))&255;
        result|=((src*alpha+old*(255-alpha)+127)/255)<<(channel*8);
    }
    return result;
}
inline void verify_composite(const port3ds::Engine& engine,const port3ds::SpriteView* views,unsigned count,int cx,int cy) {
    constexpr unsigned width=400,height=240;
    std::vector<std::uint32_t> reference(width*height,0x182818),actual=reference;
    std::vector<float> depths(width*height,0);
    std::vector<std::uint16_t> pixels(512*512);
    auto raster=[&](int left,int top,unsigned w,unsigned h,bool flip,auto pixel,auto write) {
        for(int y=std::max(0,top);y<std::min(int(height),top+int(h));++y)
            for(int x=std::max(0,left);x<std::min(int(width),left+int(w));++x) {
                const unsigned sx=flip?w-1-(x-left):x-left;
                write(unsigned(y)*width+x,pixel(sx,unsigned(y-top)));
            }
    };
    for(unsigned i=0;i<count;++i) {
        const auto& v=views[i];const auto info=engine.sprite_info(v.image,v.frame);
        assert(engine.sprite_pixels(v.image,v.frame,v.owner,pixels.data(),pixels.size()));
        raster(v.x-cx,v.y-cy,info.width,info.height,v.flipped,
            [&](unsigned x,unsigned y){return pixels[y*info.width+x];},
            [&](unsigned at,std::uint16_t c){if(c&1)reference[at]=blend_pixel(reference[at],c,v.shadow?100:v.opacity,v.shadow);});
    }
    for(const auto& d:fake::draws) {
        assert(d.alpha_test && d.depth_test);
        const auto* data=static_cast<const std::uint16_t*>(d.tex->data);
        const unsigned ox=std::lround(d.sub.left*512),oy=std::lround((1-d.sub.top)*512);
        const int left=std::lround(d.x+(d.sx<0?-int(d.sub.width):0));
        raster(left,std::lround(d.y),d.sub.width,d.sub.height,d.sx<0,
            [&](unsigned x,unsigned y){
                const unsigned px=ox+x,py=oy+y;
                const unsigned low=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);
                return data[((py/8)*64+px/8)*64+low];
            },
            [&](unsigned at,std::uint16_t c){
                if(!(c&1) || d.depth<depths[at])return;
                actual[at]=blend_pixel(actual[at],c,d.tint>>24,d.blend==1);
                if(d.depth_write==GPU_WRITE_ALL)depths[at]=d.depth;
            });
    }
    assert(reference==actual && "Texture grouping changed overlap, transparent holes or translucent layering");
    assert(!fake::depth_state().enabled && "UI must draw above world sprites");
}
