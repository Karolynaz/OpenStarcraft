#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace port3ds { namespace campaign {
inline unsigned get(const uint8_t* p){return unsigned(p[0])|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
inline void put(uint8_t* p,unsigned v){for(unsigned i=0;i<4;++i)p[i]=v>>(8*i);}
inline unsigned swap_player(unsigned v,unsigned human=1){return v==0?human:v==human?0:v;}
// Private extraction only: the 3DS input/UI uses player zero. Preserve the
// mission's forces, masks, restrictions and explicit trigger player references.
template<class Bytes> void normalize(Bytes& bytes,unsigned human=1){
 if(human>=8)throw std::runtime_error("Campaign human player bounds");if(!human)return;
 for(size_t at=0;at+8<=bytes.size();){
  auto* tag=bytes.data()+at;unsigned n=get(tag+4);at+=8;if(n>bytes.size()-at)throw std::runtime_error("Campaign chunk bounds");auto* p=bytes.data()+at;
  auto is=[&](const char* s){return !std::memcmp(tag,s,4);};
  auto rows=[&](unsigned offset,unsigned width){if(offset+(human+1)*width>n)throw std::runtime_error("Campaign player table bounds");for(unsigned j=0;j<width;++j)std::swap(p[offset+j],p[offset+human*width+j]);};
  if(is("OWNR") || is("IOWN") || is("SIDE") || is("FORC") || is("COLR"))rows(0,1);
  else if(is("PUNI")){rows(0,228);rows(13*228,228);}
  else if(is("UPGR") || is("PUPx") || is("PTEC") || is("PTEx")){
   unsigned width=is("UPGR")?46:is("PUPx")?61:is("PTEC")?24:44;rows(0,width);rows(12*width,width);rows(26*width,width);
  }else if(is("MASK"))for(unsigned j=0;j<n;++j)p[j]=(p[j]&~(1u|(1u<<human)))|((p[j]&1)<<human)|((p[j]>>human)&1);
  else if(is("UNIT")){if(n%36)throw std::runtime_error("Campaign unit bounds");for(unsigned j=0;j<n;j+=36)p[j+16]=swap_player(p[j+16],human);}
  else if(is("THG2")){if(n%10)throw std::runtime_error("Campaign doodad bounds");for(unsigned j=0;j<n;j+=10)p[j+6]=swap_player(p[j+6],human);}
  else if(is("TRIG") || is("MBRF")){
   if(n%2400)throw std::runtime_error("Campaign trigger bounds");
   for(unsigned j=0;j<n;j+=2400){auto* t=p+j;std::swap(t[2372],t[2372+human]);
    if(is("MBRF"))continue;
    for(unsigned c=0;c<16;++c){auto* q=t+c*20;unsigned type=q[15];if((type>=2 && type<=10) || (type>=14 && type<=21))put(q+4,swap_player(get(q+4),human));}
    for(unsigned a=0;a<64;++a){auto* q=t+320+a*32;unsigned type=q[26];if((type>=22 && type<=27) || type==38 || type==39 || (type>=42 && type<=46) || (type>=48 && type<=53) || type==57 || type==11)put(q+16,swap_player(get(q+16),human));
     if(type==48)put(q+20,swap_player(get(q+20),human));}
   }
  }
  at+=n;
 }
}
}}
