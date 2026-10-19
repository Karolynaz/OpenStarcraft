#include "graphics.h"
#include "race_catalogue.h"
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <3ds.h>
#include <atomic>
#include <cassert>
#include <vector>
#include <cmath>
#include <cstdio>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char** argv){
 assert(argc==5 || argc==6);using namespace port3ds;const bool protoss=argc==6 && std::string(argv[5])=="protoss",zerg=argc==6 && std::string(argv[5])=="zerg";
 {
  Engine e;assert(e.load(argv[1],argv[2],zerg?Scenario::zerg_battle:protoss?Scenario::protoss_battle:Scenario::terran_battle));Graphics g;assert(g.load(argv[3],e.terrain_signature()));
  // Compare the runtime decoder against independently exported original GRP
  // palette indices, including every frame of representative Terran bodies,
  // turrets and shadows, both player remaps and neutral colors.
  std::ifstream input(argv[4],std::ios::binary);std::vector<unsigned char> raw((std::istreambuf_iterator<char>(input)),{});assert(raw.size()>1200);
  size_t at=8;auto u16=[&](){assert(at+2<=raw.size());unsigned v=raw[at]|raw[at+1]<<8;at+=2;return v;};auto u32=[&](){auto lo=u16();return lo|(u16()<<16);};
  u32();u32();u32();uint16_t palette[256];
  for(unsigned i=0;i<256;++i){palette[i]=((raw[at+i*4]>>3)<<11)|((raw[at+i*4+1]>>3)<<6)|((raw[at+i*4+2]>>3)<<1)|1;}at+=1024;
  unsigned char team[128];std::copy_n(raw.data()+at,128,team);at+=128;auto cells=u32();at+=cells*2;auto tiles=u32();at+=tiles*1026;
  using bwgame::ImageTypes;
  std::set<unsigned> check={unsigned(ImageTypes::IMAGEID_Marine),unsigned(ImageTypes::IMAGEID_Firebat),unsigned(ImageTypes::IMAGEID_Ghost),unsigned(ImageTypes::IMAGEID_SCV),unsigned(ImageTypes::IMAGEID_Battlecruiser),unsigned(ImageTypes::IMAGEID_Wraith),unsigned(ImageTypes::IMAGEID_Goliath_Base),unsigned(ImageTypes::IMAGEID_Goliath_Turret),unsigned(ImageTypes::IMAGEID_Siege_Tank_Tank_Base),unsigned(ImageTypes::IMAGEID_Siege_Tank_Tank_Turret),unsigned(ImageTypes::IMAGEID_Marine_Shadow)};
  if(protoss)check={unsigned(ImageTypes::IMAGEID_Probe),unsigned(ImageTypes::IMAGEID_Zealot),unsigned(ImageTypes::IMAGEID_Dragoon),unsigned(ImageTypes::IMAGEID_High_Templar),unsigned(ImageTypes::IMAGEID_Dark_Templar_Unit),unsigned(ImageTypes::IMAGEID_Carrier),unsigned(ImageTypes::IMAGEID_Interceptor),unsigned(ImageTypes::IMAGEID_Scout),unsigned(ImageTypes::IMAGEID_Shuttle),unsigned(ImageTypes::IMAGEID_Observer),unsigned(ImageTypes::IMAGEID_Reaver)};
  if(zerg)check={unsigned(ImageTypes::IMAGEID_Drone),unsigned(ImageTypes::IMAGEID_Zergling),unsigned(ImageTypes::IMAGEID_Hydralisk),unsigned(ImageTypes::IMAGEID_Mutalisk),unsigned(ImageTypes::IMAGEID_Overlord),unsigned(ImageTypes::IMAGEID_Ultralisk),unsigned(ImageTypes::IMAGEID_Queen),unsigned(ImageTypes::IMAGEID_Defiler),unsigned(ImageTypes::IMAGEID_Guardian),unsigned(ImageTypes::IMAGEID_Devourer),unsigned(ImageTypes::IMAGEID_Lurker)};
  if(zerg){
   std::ifstream ref(std::string(argv[4])+".creep",std::ios::binary);std::vector<unsigned char> c((std::istreambuf_iterator<char>(ref)),{});assert(c.size()>16392 && std::string((const char*)c.data(),7)=="OBWCRP1");size_t p=8;std::vector<uint16_t> px(512*512);unsigned compared=0;
   for(unsigned f=0;f<16;++f){assert(e.sprite_pixels(65500,f,11,px.data(),px.size()));for(unsigned i=0;i<1024;++i){assert(px[i]==palette[c.at(p++)]);++compared;}}
   auto read16=[&](){unsigned n=c.at(p)|c.at(p+1)<<8;p+=2;return n;};const auto frames=read16();
   for(unsigned f=0;f<frames;++f){const auto w=read16(),h=read16();auto info=e.sprite_info(65501,f);assert(info.width==w && info.height==h && e.sprite_pixels(65501,f,11,px.data(),px.size()));for(unsigned i=0;i<w*h;++i){const auto color=read16();assert(px[i]==(color==65535?0:palette[color]));++compared;}}
   assert(p==c.size());std::printf("Original creep megatiles and GRP edges reference PASS: %u pixels\n",compared);
  }
  unsigned reference_pixels=0;std::vector<uint16_t> decoded(512*512);auto groups=u32();
  for(unsigned g=0;g<groups;++g){auto id=u16(),frames=u16();for(unsigned f=0;f<frames;++f){auto w=u16(),h=u16();assert(at+w*h*2<=raw.size());
   if(check.count(id)){for(unsigned owner: {0u,1u,11u}){assert(e.sprite_pixels(id,f,owner,decoded.data(),decoded.size()));for(unsigned i=0;i<w*h;++i){unsigned c=raw[at+i*2]|raw[at+i*2+1]<<8;uint16_t expected=0;if(c!=65535){if(c>=8 && c<16)c=team[owner*8+c-8];expected=palette[c];}assert(decoded[i]==expected);++reference_pixels;}}}
   at+=w*h*2;
  }}assert(at==raw.size());assert(reference_pixels>1000000);std::printf("Original GRP runtime color/team/alpha reference PASS: %u pixels\n",reference_pixels);
  SpriteView sprites[2048];std::vector<uint16_t> expected(512*512);unsigned checked=0;
  for(unsigned frame=0;frame<800;++frame){
   assert(e.tick());if(frame%4)continue;
   // Exercise the front, both armies and viewport motion, keeping real source
   // animations, deaths, mine shots, turret rotations and matrix overlays.
   const bool creep_view=zerg && frame%5==0;const int cx=creep_view?(frame%2?1648:0):frame%3==0?420:frame%3==1?800:1100,cy=creep_view?1700:600;
   auto n=e.visible_images(sprites,2048,cx,cy);fake::draws.clear();assert(g.sprites(sprites,n,cx,cy,e)==0);
   size_t out=0;
   for(unsigned i=0;i<n;++i){const auto& v=sprites[i];auto info=e.sprite_info(v.image,v.frame);assert(info.valid);
    if(!info.width || !info.height)continue;assert(out<fake::draws.size());const auto& d=fake::draws[out++];
    assert(d.sub.width==info.width && d.sub.height==info.height);assert(e.sprite_pixels(v.image,v.frame,v.owner,expected.data(),expected.size()));
    const unsigned ox=std::lround(d.sub.left*512),oy=std::lround((1-d.sub.top)*512);const auto* tex=(const uint16_t*)d.tex->data;
    // Verify every pixel at the point of drawing; retained/cache-reused frames
    // must agree just as exactly as cache misses, including transparent holes.
    for(unsigned y=0;y<info.height;++y)for(unsigned x=0;x<info.width;++x){
     unsigned px=ox+x,py=oy+y;unsigned low=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);
     assert(tex[((py/8)*64+px/8)*64+low]==expected[y*info.width+x]);++checked;
    }
   }
   assert(out==fake::draws.size());
  }
  assert(!g.cache_overflows());std::printf("Bounded sprite cache / pinned frame lifetime / exact swizzled RGBA / moving full-race battle PASS: %u checked pixels, %u misses\n",checked,g.cache_misses());
 }
 assert(fake::allocations==0);
}
