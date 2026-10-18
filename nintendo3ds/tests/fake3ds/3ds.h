#pragma once
// Host contract double: models DSP buffer ownership; it does not emulate audio.
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>
using u64=uint64_t;
constexpr u64 U64_MAX=~u64(0);
enum { NDSP_WBUF_FREE,NDSP_WBUF_QUEUED,NDSP_WBUF_PLAYING,NDSP_WBUF_DONE };
enum { NDSP_INTERP_LINEAR,NDSP_FORMAT_MONO_PCM16,NDSP_FORMAT_STEREO_PCM16 };
struct ndspWaveBuf {const void* data_vaddr=nullptr;unsigned nsamples=0;unsigned char status=0;};
using Thread=std::thread*;
namespace fake {extern std::atomic<unsigned> queued;extern std::atomic<unsigned> allocations;extern std::atomic<unsigned> pauses;}
inline void* linearAlloc(unsigned n) {++fake::allocations;return std::malloc(n);}
inline void linearFree(void* p) {assert(fake::queued==0);--fake::allocations;std::free(p);}
inline void ndspChnWaveBufClear(unsigned) {fake::queued=0;}
inline void ndspChnReset(unsigned) {}
inline void ndspChnSetRate(unsigned,float) {}
inline void ndspChnSetInterp(unsigned,unsigned) {}
inline void ndspChnSetFormat(unsigned,unsigned) {}
inline void ndspChnSetMix(unsigned,float*) {}
inline void ndspChnSetPaused(unsigned,bool) {++fake::pauses;}
inline void DSP_FlushDataCache(const void*,unsigned) {}
inline void ndspChnWaveBufAdd(unsigned,ndspWaveBuf* b) {assert(b->data_vaddr && b->nsamples);assert(b->status==NDSP_WBUF_FREE);b->status=NDSP_WBUF_QUEUED;++fake::queued;}
inline unsigned ndspGetDroppedFrames() {return 0;}
inline u64 osGetTime() {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
inline void svcSleepThread(u64 n) {std::this_thread::sleep_for(std::chrono::nanoseconds(n));}
inline Thread threadCreate(void(*f)(void*),void* arg,unsigned,unsigned,unsigned,bool) {return new std::thread(f,arg);}
inline void threadJoin(Thread t,u64) {t->join();}
inline void threadFree(Thread t) {delete t;}
