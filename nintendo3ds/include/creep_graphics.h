#pragma once
#include "bwgame.h"
#include <array>
namespace port3ds {namespace detail {
// Reserved adapter image IDs; they share the bounded sprite GPU cache.
constexpr unsigned creep_tile_image=65500,creep_edge_image=65501;
// OpenBW's original UI uses common variants 0..5 and rare variants 6..12.
// CV5 entries 13..15 are unused on some tilesets and produce black squares.
inline unsigned creep_variant(unsigned x,unsigned y){
 uint32_t seed=(y*64+x+1)*22695477u+1;const unsigned rare=((seed>>16)&0x7fff)%100;
 seed=seed*22695477u+1;const unsigned choice=(seed>>16)&0x7fff;
 return rare<4?6+choice%7:choice%6;
}
struct CreepGraphics {
 std::array<std::array<uint16_t,1024>,16> tiles{};
 bwgame::grp_t edges;
 std::array<unsigned,256> edge_frames{};
 template<class Load> void load(const bwgame::state& st,const bwgame::a_string& terrain,const std::array<uint16_t,256>& palette,Load load_asset){
  using namespace bwgame;a_vector<uint8_t> vr4,vx4,grp;load_asset(vr4,terrain+".vr4");load_asset(vx4,terrain+".vx4");load_asset(grp,terrain+".grp");
  edges=read_grp(data_loading::data_reader_le(grp.data(),grp.data()+grp.size()));
  for(unsigned f=0;f<16;++f){const unsigned id=st.game->cv5.at(1).mega_tile_index[f];
   for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){const unsigned off=id*32+((y/8)*4+x/8)*2;if(off+2>vx4.size())throw std::runtime_error("Creep VX4 bounds");
    unsigned mini=vx4[off]|vx4[off+1]<<8;const unsigned src=mini/2*64+y%8*8+((mini&1)?7-x%8:x%8);if(src>=vr4.size())throw std::runtime_error("Creep VR4 bounds");tiles[f][y*32+x]=palette[vr4[src]];
   }
  }
  std::array<unsigned,256> neighbors{};std::array<unsigned,128> frames{};
  for(unsigned i=0;i<256;++i){unsigned v=0;if(i&2)v|=16;if(i&8)v|=36;if(i&16)v|=9;if(i&64)v|=2;if((i&192)==192)v|=1;if((i&96)==96)v|=4;if((i&3)==3)v|=32;if((i&6)==6)v|=8;if((v&33)==33 || (v&12)==12)v|=64;neighbors[i]=v;}
  unsigned n=0;for(unsigned i=0;i<128;++i)if(std::find(neighbors.begin(),neighbors.end(),i)!=neighbors.end())frames[i]=n++;
  for(unsigned i=0;i<256;++i)edge_frames[i]=frames[neighbors[i]];
 }
};
}}
