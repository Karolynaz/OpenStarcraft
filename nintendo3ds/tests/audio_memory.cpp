#include <3ds.h>
#include "audio.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <new>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::atomic<unsigned> max_new{~0u};std::atomic<int> fail_chunk{-1};}
void* operator new(std::size_t n){if(n>fake::max_new)throw std::bad_alloc();if(n==16384 && fake::fail_chunk>=0 && fake::fail_chunk.fetch_sub(1)==0)throw std::bad_alloc();if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}
static void u32(std::ofstream& f,unsigned n){for(unsigned i=0;i<4;++i)f.put(n>>(i*8));}
int main(){
 char path[]="/tmp/openbw-audio-memory-XXXXXX";assert(mkdtemp(path));std::string root=path;
 constexpr unsigned offset=65536-17,samples=196625,bytes=offset+samples+31;
 {std::ofstream f(root+"/effects.bin",std::ios::binary);f.write("OBWAUD2\0",8);u32(f,1);u32(f,bytes);u32(f,1);u32(f,69);u32(f,offset);u32(f,samples);u32(f,1);u32(f,0);for(unsigned i=0;i<bytes;++i)f.put(char((i*29+7)&255));}
 for(unsigned n=1;n<=3;++n){std::ofstream f(root+"/terran"+std::to_string(n)+".pcm",std::ios::binary);for(unsigned i=0;i<4096;++i)f.put('a'+n);}
 {
  port3ds::Audio audio;fake::max_new=16384;bool threw=false,loaded=false;
  try{loaded=audio.load(path,true);}catch(const std::bad_alloc&){threw=true;}
  fake::max_new=~0u;assert(!threw && "Audio::load must handle allocation failure without terminating with active DSP");assert(loaded && "SFX must load from fragmented blocks without a large contiguous allocation");
  fake::queue_observer()=[](unsigned channel,const ndspWaveBuf* b){if(!channel)return;assert(b->nsamples==samples);const auto* data=static_cast<const unsigned char*>(b->data_vaddr);for(unsigned i=0;i<samples;++i)assert(data[i]==(((i+offset)*29+7)&255));};
  port3ds::SoundEvent event{69,100,100};audio.update(&event,1,0,0,false);assert(audio.stats().played==1);fake::queue_observer()=nullptr;audio.unload();assert(fake::allocations==0 && fake::queued==0);
  fake::max_new=0;assert(!audio.load(path,true));fake::max_new=~0u;assert(!audio.stats().ready && fake::allocations==0);
  for(int failure=0;failure<16;++failure){
   fake::fail_chunk=failure;assert(audio.load(path,true));fake::fail_chunk=-1;
   assert(audio.stats().cache_bytes==unsigned(failure)*16384);
   fake::queue_observer()=+[](unsigned channel,const ndspWaveBuf* b){if(!channel)return;assert(b->nsamples==samples);const auto* data=static_cast<const unsigned char*>(b->data_vaddr);for(unsigned i=0;i<samples;++i)assert(data[i]==(((i+offset)*29+7)&255));};
   audio.update(&event,1,0,0,false);
   for(unsigned n=0;n<300 && !audio.stats().played;++n)svcSleepThread(5000000);
   assert(audio.stats().played==1 && audio.stats().io_errors==0);
   if(failure<13)assert(audio.stats().streamed==1);
   audio.reset();audio.unload();fake::queue_observer()=nullptr;assert(fake::allocations==0 && fake::queued==0);
  }
  // Cancel an in-flight uncached voice, then close while IO can be pending.
  fake::fail_chunk=0;assert(audio.load(path,true));fake::fail_chunk=-1;
  audio.update(&event,1,0,0,false);audio.reset();svcSleepThread(30000000);assert(!audio.stats().voices);audio.unload();
  for(unsigned repeat=0;repeat<5;++repeat){fake::fail_chunk=0;assert(audio.load(path,true));fake::fail_chunk=-1;audio.update(&event,1,0,0,false);audio.unload();assert(fake::allocations==0 && fake::queued==0);}

  fake::fail_chunk=0;assert(audio.load(path,true));fake::fail_chunk=-1;
  assert(!truncate((root+"/effects.bin").c_str(),40+65536));audio.update(&event,1,0,0,false);
  for(unsigned n=0;n<100 && !audio.stats().io_errors;++n)svcSleepThread(5000000);
  assert(audio.stats().io_errors>0 && audio.stats().ready && !audio.stats().played);audio.unload();assert(fake::allocations==0);
 }
 for(unsigned n=1;n<=3;++n)std::remove((root+"/terran"+std::to_string(n)+".pcm").c_str());std::remove((root+"/effects.bin").c_str());rmdir(path);
 std::puts("16 KiB fragmented cache / streamed fallback / byte equality / cancel/unload / read-failure recovery PASS");
}
