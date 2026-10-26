// Synthetic, owned-data-free ADPCM output bounds and stereo-tail regression.
#include "data_loading.h"
#include <cassert>
#include <cstdio>
using namespace bwgame::data_loading;
int main(){uint8_t mono[]={0,2,7,0,0x80,0x80,0x80};uint8_t out[12];
 for(unsigned cap=0;cap<9;++cap){std::fill(out,out+12,0xCD);auto n=decompress_adpcm(mono,sizeof mono,out,cap,1);assert(n<=cap && !(n&1));for(unsigned i=cap;i<12;++i)assert(out[i]==0xCD);for(unsigned i=0;i<n;i+=2)assert(out[i]==7 && out[i+1]==0);}
 uint8_t stereo[]={0,2,7,0,9,0,0x80};std::fill(out,out+12,0xCD);assert(decompress_adpcm(stereo,sizeof stereo,out,12,2)==6);assert(out[4]==7 && out[5]==0 && out[6]==0xCD);
 bool invalid=false;try{decompress_adpcm(mono,sizeof mono,out,12,3);}catch(const std::runtime_error&){invalid=true;}assert(invalid);mono[1]=32;invalid=false;try{decompress_adpcm(mono,sizeof mono,out,12,1);}catch(const std::runtime_error&){invalid=true;}assert(invalid);
 puts("ADPCM bounded/odd/zero output, canary guards, incomplete stereo frame and invalid channel/shift PASS");}
