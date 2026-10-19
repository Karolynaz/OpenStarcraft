#include "graphics.h"
#include "data_loading.h"
#include <atomic>
#include <cassert>
#include <cstdio>
#include <vector>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
int main(int argc,char** argv){
 assert(argc==4);using namespace port3ds;
 auto assets=bwgame::data_loading::data_files_directory(argv[1]);bwgame::a_vector<uint8_t> wpe;assets(wpe,"tileset/platform.wpe");assert(wpe.size()==1024);
 auto color=[&](unsigned c){return uint16_t(((wpe[c*4]>>3)<<11)|((wpe[c*4+1]>>3)<<6)|((wpe[c*4+2]>>3)<<1)|1);};
 unsigned frames=0,pixels=0;
 {
  Engine engine;assert(engine.load(argv[1],argv[2],Scenario::protoss_lab));Graphics graphics;assert(graphics.load(argv[3],engine.terrain_signature()));
  const auto before=engine.snapshot();
  for(unsigned kind=0;kind<cursor_count;++kind){
   bwgame::a_vector<uint8_t> raw;assets(raw,bwgame::a_string("cursor/")+cursor_file(kind)+".grp");
   auto u16=[&](unsigned p){return unsigned(raw.at(p))|(unsigned(raw.at(p+1))<<8);};
   auto u32=[&](unsigned p){return u16(p)|(u16(p+2)<<16);};const unsigned count=u16(0);assert(count && count<=16 && u16(2)==128 && u16(4)==128);
   for(unsigned frame=0;frame<count;++frame){
    const unsigned at=6+frame*8,ox=raw[at],oy=raw[at+1],w=raw[at+2],h=raw[at+3],base=u32(at+4);
    std::vector<uint16_t> expected(w*h,0),actual(w*h);
    for(unsigned y=0;y<h;++y){unsigned p=base+u16(base+y*2),x=0;while(x<w){const unsigned token=raw.at(p++);if(token&128){x+=token&127;assert(x<=w);}else if(token&64){const unsigned n=token&63,c=raw.at(p++);assert(n && x+n<=w);for(unsigned j=0;j<n;++j)expected[y*w+x++]=color(c);}else {assert(token && x+token<=w);for(unsigned j=0;j<token;++j)expected[y*w+x++]=color(raw.at(p++));}}}
    auto view=engine.cursor_image(CursorKind(kind),frame*100000);assert(view.image==cursor_image_base+kind && view.frame==frame && view.x==int(ox)-64 && view.y==int(oy)-64);
    const auto info=engine.sprite_info(view.image,view.frame);assert(info.valid && info.width==w && info.height==h);
    assert(engine.sprite_pixels(view.image,view.frame,0,actual.data(),actual.size()));assert(actual==expected);
    fake::draws.clear();assert(graphics.cursor(view,100,80,engine));assert(fake::draws.size()==1);const auto d=fake::draws[0];assert(d.x==100+int(ox)-64 && d.y==80+int(oy)-64 && d.sub.width==w && d.sub.height==h);
    const auto* texture=static_cast<const uint16_t*>(d.tex->data);
    for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x){const unsigned morton=(x&1)|((y&1)<<1)|((x&2)<<1)|((y&2)<<2)|((x&4)<<2)|((y&4)<<3);assert(texture[((y/8)*8+x/8)*64+morton]==(x<w && y<h?expected[y*w+x]:0));++pixels;}
    assert(engine.cursor_image(CursorKind(kind),(frame+count)*100000).frame==frame);++frames;
   }
  }
  UnitView units[512];auto n=engine.visible_units(units,512);bool own=false,enemy=false,neutral=false;
  for(unsigned i=0;i<n;++i){const auto& u=units[i];const auto kind=engine.cursor_kind(u.x,u.y),target=engine.cursor_kind(u.x,u.y,true);if(u.owner==0 && kind==CursorKind::own){assert(target==CursorKind::target);own=true;}if(u.owner==1 && kind==CursorKind::enemy){assert(target==CursorKind::target_enemy);enemy=true;}if(u.owner>1 && kind==CursorKind::neutral){assert(target==CursorKind::target_neutral);neutral=true;}}
  assert(own && enemy && neutral);assert(engine.cursor_kind(2000,0)==CursorKind::arrow && engine.cursor_kind(2000,0,true)==CursorKind::target);
  assert(!engine.sprite_info(cursor_image_base,999).valid && !engine.cursor_image(CursorKind::count,0).image);
  assert(engine.snapshot().frame==before.frame && engine.snapshot().units==before.units && engine.snapshot().selected==before.selected);
 }
 assert(fake::allocations==0);
 assert(edge_cursor(0,0,-100,100)==CursorKind::up_left && edge_cursor(399,0,100,100)==CursorKind::up_right);
 assert(edge_cursor(0,239,-100,-100)==CursorKind::down_left && edge_cursor(399,239,100,-100)==CursorKind::down_right);
 assert(edge_cursor(200,0,0,100)==CursorKind::up && edge_cursor(200,239,0,-100)==CursorKind::down);
 assert(edge_cursor(0,100,-100,0)==CursorKind::left && edge_cursor(399,100,100,0)==CursorKind::right);
 assert(edge_cursor(0,0,0,0)==CursorKind::arrow && edge_cursor(200,120,100,100)==CursorKind::arrow);
 std::printf("Original cursor GRP/palette/hotspot/animation/GPU upload PASS: %u cursor types / %u frames / %u texture pixels; contextual hover, edge directions and no simulation changes PASS\n",cursor_count,frames,pixels);
}
