#include "actions.h"
#include <fstream>
#include <cstdio>
using namespace bwgame;
int main(int argc,char** argv) {
 if(argc!=4)return 2;
 try {
  auto assets=data_loading::data_files_directory(argv[1]);game_player p(assets);
  data_loading::mpq_file<> archive(argv[2]);a_vector<uint8_t> chk;archive(chk,"staredit/scenario.chk");
  game_load_functions f(p.st());f.load_map_data(chk.data(),chk.size(),[&]{f.setup_info.create_no_units=true;});
  size_t at=0;uint16_t tile=0;bool chosen=false;
  while(at+8<=chk.size()) {
   uint32_t len=data_loading::value_at<uint32_t,true>(chk.data()+at+4);
   if(len>chk.size()-at-8)throw std::runtime_error("Bad arena source");
   if(!std::memcmp(chk.data()+at,"MTXM",4)) {
    for(size_t i=0;i<len/2 && i<p.st().tiles.size();++i) {
     const auto flags=p.st().tiles[i].flags;
     if((flags&tile_t::flag_walkable) && !(flags&(tile_t::flag_partially_walkable|tile_t::flag_unbuildable))) {
      tile=chk[at+8+i*2]|(chk[at+9+i*2]<<8);chosen=true;break;
     }
    }
    if(!chosen)throw std::runtime_error("No fully buildable arena tile");
    for(size_t i=0;i<len;i+=2){chk[at+8+i]=tile&255;chk[at+9+i]=tile>>8;}
   }
   at+=8+len;
  }
  if(!chosen)throw std::runtime_error("No terrain in source map");
  std::ofstream out(argv[3],std::ios::binary);out.write((const char*)chk.data(),chk.size());if(!out)throw std::runtime_error("Arena write failed");
  std::printf("Prepared private flat 64x64 stress/lab arena, source MTXM tile %u\n",tile);
 }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
