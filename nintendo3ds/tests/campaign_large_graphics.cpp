// TEMP no-trigger geometry fixtures only. Does not assert campaign completion.
#include "graphics.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <vector>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char**argv){assert(argc==4);port3ds::Engine e;assert(e.load(argv[1],argv[2],port3ds::Scenario::terran_campaign));for(unsigned i=0;i<48;++i)assert(e.tick());auto s=e.snapshot();assert(s.width>128 || s.height>128);assert(e.terrain_signature());{
port3ds::Graphics g;assert(g.load(argv[3],e.terrain_signature()));assert(g.texture_bytes()==7*512*512*2+64*64*4+256*256*4);
for(int y:{0,int(s.height*32-240)})for(int x:{0,int(s.width*32-400)}){port3ds::SpriteView views[4096];auto n=e.visible_images(views,4096,x,y);g.terrain(x,y);g.sprites(views,n,x,y,e);fake::draws.clear();g.fog(e,x,y);assert(fake::draws.size()==1);const auto* pixels=static_cast<const uint16_t*>(fake::draws[0].tex->data);for(unsigned ty=0;ty<256;++ty)for(unsigned tx=0;tx<256;++tx){const auto cell=e.fog_cell(tx,ty);const unsigned m=(tx&1)|((ty&1)<<1)|((tx&2)<<1)|((ty&2)<<2)|((tx&4)<<2)|((ty&4)<<3);assert(pixels[((ty/8)*32+tx/8)*64+m]==(cell==port3ds::FogCell::visible?0:cell==port3ds::FogCell::explored?10:15));}g.fog_minimap(e,8,8,96,128);assert(fake::draws.size()==2);fake::draws.clear();}
printf("%ux%u far-corner terrain/sprites + full256 fog Morton mask + minimap / seven atlas pages PASS\n",s.width,s.height);}
assert(fake::allocations==0);}
