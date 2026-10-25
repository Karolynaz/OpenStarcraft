#include <3ds.h>
#include "audio.h"
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <unistd.h>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};}
static std::vector<unsigned char> pcm;static std::atomic<unsigned> received{0},segments{0};
static void put(std::ofstream& f,unsigned v){char b[4]={char(v),char(v>>8),char(v>>16),char(v>>24)};f.write(b,4);}
int main(){char path[]="/tmp/sc3ds-long-speech-XXXXXX";assert(mkdtemp(path));std::string root=path;
 for(unsigned i=1;i<=3;++i){std::ofstream f(root+"/terran"+std::to_string(i)+".pcm",std::ios::binary);f<<std::string(4096,'a');}
 pcm.resize(700123);for(unsigned i=0;i<pcm.size();++i)pcm[i]=(i*17+i/139)%256;
 {std::ofstream f(root+"/effects.bin",std::ios::binary);f.write("OBWAUD2\0",8);put(f,1);put(f,pcm.size());put(f,1);put(f,1300);put(f,0);put(f,pcm.size());put(f,200);put(f,16);f.write((char*)pcm.data(),pcm.size());}
 port3ds::Audio audio;assert(audio.load(path,true));assert(audio.stats().cache_bytes<=4*1024*1024);
 fake::queue_observer()=+[](unsigned channel,const ndspWaveBuf* wave){if(!channel)return;unsigned at=received.load();assert(wave->nsamples<=128*1024 && at+wave->nsamples<=pcm.size());assert(!memcmp(wave->data_vaddr,pcm.data()+at,wave->nsamples));received+=wave->nsamples;++segments;const_cast<ndspWaveBuf*>(wave)->status=NDSP_WBUF_DONE;};
 port3ds::SoundEvent event{1300,0,0};audio.update(&event,1,0,0,false);
 for(unsigned n=0;n<500 && received<pcm.size();++n)svcSleepThread(5000000);
 assert(received==pcm.size() && segments==6 && audio.stats().played==1 && !audio.stats().io_errors);audio.reset();audio.unload();assert(fake::allocations==0);fake::queue_observer()=nullptr;
 for(unsigned i=1;i<=3;++i)std::remove((root+"/terran"+std::to_string(i)+".pcm").c_str());std::remove((root+"/effects.bin").c_str());rmdir(path);
 puts("31-second campaign speech: six byte-exact reusable 128KiB chunks, one event, unchanged 256KiB long-voice allocation, IO0 PASS");
}
