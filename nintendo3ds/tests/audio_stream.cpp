#include <3ds.h>
#include "audio.h"
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};}
static const unsigned char* expected=nullptr;static unsigned expected_bytes=0,expected_samples=0;
static unsigned u32(const std::vector<unsigned char>& b,unsigned p){assert(p+4<=b.size());return b[p]|b[p+1]<<8|b[p+2]<<16|unsigned(b[p+3])<<24;}
int main(int argc,char** argv){assert(argc==2);const std::string base=argv[1];
 for(const char* race:{"terran","protoss","zerg","all"}){
  const bool all=std::string(race)=="all";const auto folder=base+(std::string(race)=="terran" || all?"":"/"+std::string(race));const auto effect_path=folder+(all?"/effects-all.bin":"/effects.bin");std::ifstream file(effect_path,std::ios::binary);std::vector<unsigned char> bank((std::istreambuf_iterator<char>(file)),{});assert(bank.size()>20);
  const bool v2=!memcmp(bank.data(),"OBWAUD2\0",8);unsigned count=u32(bank,8),width=v2?u32(bank,16):2,header=v2?20:16,payload=header+count*20;
  port3ds::Audio audio;assert(audio.load(folder.c_str(),true,all?"terran":race,all?effect_path.c_str():nullptr));assert(audio.stats().cache_bytes<=4*1024*1024);
  fake::queue_observer()=+[](unsigned channel,const ndspWaveBuf* wave){if(!channel)return;assert(wave->nsamples==expected_samples);assert(!memcmp(wave->data_vaddr,expected,expected_bytes));};
  for(unsigned i=0;i<count;++i){const auto record=header+i*20;const unsigned offset=u32(bank,record+4),samples=u32(bank,record+8);expected=bank.data()+payload+offset;expected_bytes=samples*width;expected_samples=samples;
   const auto before=audio.stats().played;port3ds::SoundEvent event{u32(bank,record),100,100};audio.update(&event,1,0,0,false);
   for(unsigned n=0;n<300 && audio.stats().played==before;++n)svcSleepThread(5000000);
   assert(audio.stats().played==before+1 && audio.stats().io_errors==0);audio.reset();
  }
  const auto stats=audio.stats();printf("%s: %u original effects byte-exact, %u streamed, %u KiB cache, IO errors %u PASS\n",race,count,stats.streamed,stats.cache_bytes/1024,stats.io_errors);fflush(stdout);assert(stats.streamed>0);audio.unload();fake::queue_observer()=nullptr;assert(fake::allocations==0 && fake::queued==0);
 }
}
