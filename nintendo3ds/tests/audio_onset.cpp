// DSP contract double: queued/buffer-playing silence is not yet audible speech.
#include <3ds.h>
#include "audio.h"
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};}
static void put(std::ofstream& f,unsigned n){for(unsigned i=0;i<4;++i)f.put(char(n>>(i*8)));}
int main(){using namespace port3ds;const auto root=std::string("/tmp/sc3ds-onset-")+std::to_string(getpid());assert(!mkdir(root.c_str(),0700));
 for(unsigned i=1;i<=3;++i){std::ofstream f(root+"/terran"+std::to_string(i)+".pcm",std::ios::binary);f<<std::string(4096,0);}
 {Audio music;assert(music.load(root.c_str(),true,"terran",nullptr,0,true));assert(music.stats().ready && music.stats().cache_bytes==0 && !music.contains(1200));music.unload();assert(fake::allocations==0);}
 {std::ofstream f(root+"/effects.bin",std::ios::binary);f.write("OBWAUD2\0",8);put(f,1);put(f,22050);put(f,1);put(f,1200);put(f,0);put(f,22050);put(f,10);put(f,0);f<<std::string(11025,0)<<std::string(11025,40);}
 const auto shared=root+"/shared.bin";
 {std::ofstream f(shared,std::ios::binary);f.write("OBWAUD2\0",8);put(f,1);put(f,2048);put(f,1);put(f,93);put(f,0);put(f,2048);put(f,10);put(f,0);f<<std::string(2048,30);}
 fake::queue_observer()=+[](unsigned channel,const ndspWaveBuf* wave){if(channel)const_cast<ndspWaveBuf*>(wave)->status=NDSP_WBUF_PLAYING;};
 {Audio audio;assert(audio.load(root.c_str(),true,"terran",nullptr,0,false,false,shared.c_str()));assert(audio.contains(1200) && audio.contains(93) && !audio.contains(1201));SoundEvent event{1200,0,0};audio.update(&event,1,0,0,false);
  assert(audio.active(1200) && !audio.playing(1200));
  for(unsigned i=0;i<100 && audio.stats().voices==0;++i)svcSleepThread(10000000);assert(audio.stats().voices==1);assert(!audio.playing(1200));svcSleepThread(150000000);assert(!audio.playing(1200));svcSleepThread(450000000);assert(audio.playing(1200));audio.reset();assert(!audio.active(1200) && !audio.playing(1200));
  event.id=93;audio.update(&event,1,0,0,false);
  for(unsigned i=0;i<100 && audio.stats().played<2;++i)svcSleepThread(10000000);
  assert(audio.stats().played==2 && !audio.stats().missing && !audio.stats().io_errors);
  audio.unload();}
 fake::queue_observer()=nullptr;assert(!fake::allocations && !fake::queued);for(unsigned i=1;i<=3;++i)unlink((root+"/terran"+std::to_string(i)+".pcm").c_str());unlink((root+"/effects.bin").c_str());unlink(shared.c_str());rmdir(root.c_str());puts("Music-only, streamed speech onset and shared effect fallback PASS");
}
