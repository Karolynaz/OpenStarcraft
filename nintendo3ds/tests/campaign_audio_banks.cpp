#include <3ds.h>
#include "audio.h"
#include "campaign_catalogue.h"
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};}
static const unsigned char* expected=nullptr;static unsigned expected_bytes=0;static std::atomic<unsigned> received{0};
static unsigned u32(const std::vector<unsigned char>& b,unsigned p){assert(p+4<=b.size());return b[p]|b[p+1]<<8|b[p+2]<<16|unsigned(b[p+3])<<24;}
int main(int argc,char**argv){assert(argc>=2 && argc<=4);const unsigned limit=argc>=3?unsigned(std::strtoul(argv[2],nullptr,10)):4*1024*1024;std::string root=argv[1];
 for(const auto& mission:port3ds::campaign::missions){if(argc==4 && std::string(argv[3])!=mission.key)continue;const auto bank_path=root+"/campaign/"+mission.key+"-effects.bin";std::ifstream input(bank_path,std::ios::binary);std::vector<unsigned char> bank((std::istreambuf_iterator<char>(input)),{});assert(bank.size()>20);unsigned count=u32(bank,8),payload=20+count*20;port3ds::Audio audio;assert(audio.load((root+"/audio").c_str(),true,"terran",bank_path.c_str(),limit));assert(audio.stats().cache_bytes<=limit);
  fake::queue_observer()=+[](unsigned channel,const ndspWaveBuf* wave){if(!channel)return;unsigned at=received;assert(at+wave->nsamples<=expected_bytes);assert(!memcmp(wave->data_vaddr,expected+at,wave->nsamples));received+=wave->nsamples;const_cast<ndspWaveBuf*>(wave)->status=NDSP_WBUF_DONE;};
  unsigned checked=0;
  for(unsigned i=0;i<count;++i){unsigned p=20+i*20,id=u32(bank,p);if(id<1200)continue;expected=bank.data()+payload+u32(bank,p+4);expected_bytes=u32(bank,p+8);received=0;auto before=audio.stats().played;port3ds::SoundEvent event{id,0,0};audio.update(&event,1,0,0,false);for(unsigned n=0;n<400 && received<expected_bytes;++n)svcSleepThread(5000000);assert(received==expected_bytes && audio.stats().played==before+1 && !audio.stats().io_errors);++checked;audio.reset();svcSleepThread(10000000);}
  printf("%s: %u complete original dialogue WAVs byte-exact through bounded mixer, IO0 PASS\n",mission.key,checked);fflush(stdout);audio.unload();fake::queue_observer()=nullptr;assert(fake::allocations==0 && fake::queued==0);
 }
}
