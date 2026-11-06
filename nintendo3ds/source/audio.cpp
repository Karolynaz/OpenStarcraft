#include "audio.h"
#include "sound_ids.h"
#include <3ds.h>
#include <array>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <cstdio>
#include <cstring>
#include <algorithm>
namespace port3ds {
namespace {
constexpr unsigned voice_count=8, voice_bytes=128*1024, chunk_bytes=32768, bank_chunk_bytes=16384, cache_bytes=4*1024*1024;
unsigned read32(FILE* f) { unsigned char b[4]; if(std::fread(b,1,4,f)!=4) return ~0u; return b[0]|b[1]<<8|b[2]<<16|unsigned(b[3])<<24; }
}
struct Audio::Impl {
    struct Effect { unsigned id,offset,samples,priority,flags; unsigned cached=~0u; };
    struct Voice {
        ndspWaveBuf wave{},tail{};unsigned segment=0,sent=0;bool segmented=false; unsigned id=~0u,priority=0,capacity=0;
        char* data=nullptr;unsigned offset=0,bytes=0,filled=0;
        std::atomic<unsigned> audible_sample{~0u};mutable std::atomic<u64> audible_clock{0};
        std::atomic<unsigned> pending{0};std::atomic<bool> cancelled{false};
    };
    std::vector<Effect> effects;std::array<short,audio_sound_limit> effect_index;
    // Small allocations tolerate fragmentation after native scenario reloads.
    std::array<std::unique_ptr<char[]>,cache_bytes/bank_chunk_bytes> pcm;
    unsigned resident=0,stream_slot=0;long effect_base=0;
    FILE* effects_file=nullptr;std::atomic<unsigned> streamed{0};
    std::atomic_flag voice_lock=ATOMIC_FLAG_INIT;
    void lock(){while(voice_lock.test_and_set(std::memory_order_acquire))svcSleepThread(1000);}
    void unlock(){voice_lock.clear(std::memory_order_release);}
    std::array<Voice,voice_count> voices{};
    ndspWaveBuf music[3]{};
    char* linear=nullptr;
    char track_paths[3][256]{};
    FILE* file=nullptr;
    Thread thread=nullptr;
    std::atomic<bool> running{false}, paused{false};
    std::atomic<unsigned> underruns{0}, io_errors{0}, buffers{0}, track{0};
    AudioStats counters;
    char message[100]="Audio not loaded";
    u64 last_played[audio_sound_limit]{};
    unsigned next_track=1,sample_bytes=2;bool long_voice=false;
    bool open_track() {
        if(file) std::fclose(file);
        file=std::fopen(track_paths[next_track-1],"rb");
        track=next_track; next_track=next_track%3+1;
        if(!file) {++io_errors;return false;} return true;
    }
    bool fill(ndspWaveBuf& b) {
        auto* dst=(char*)b.data_vaddr;
        unsigned filled=0;
        while(filled<chunk_bytes) {
            if(!file && !open_track()) return false;
            const auto n=std::fread(dst+filled,1,chunk_bytes-filled,file);
            filled+=n;
            if(filled==chunk_bytes) break;
            if(std::ferror(file)) {++io_errors;return false;}
            if(!n && std::ftell(file)==0) {++io_errors;return false;}
            if(!open_track()) return false;
        }
        b.nsamples=chunk_bytes/4;b.status=NDSP_WBUF_FREE;
        DSP_FlushDataCache(dst,chunk_bytes);ndspChnWaveBufAdd(0,&b);++buffers;return true;
    }
    void detect_speech(Voice& v,const char* pcm,unsigned bytes,unsigned offset){
        if(v.audible_sample.load()!=~0u)return;
        for(unsigned at=0;at+256*sample_bytes<=bytes;at+=256*sample_bytes){unsigned energy=0;
            for(unsigned i=0;i<256;++i){int value=sample_bytes==1?int(static_cast<signed char>(pcm[at+i])):int(static_cast<int16_t>(uint16_t(static_cast<unsigned char>(pcm[at+i*2]))|uint16_t(static_cast<unsigned char>(pcm[at+i*2+1]))<<8))/256;energy+=unsigned(std::abs(value));}
            if(energy>=256*3){v.audible_sample=(offset+at)/sample_bytes+256;return;}
        }
    }
    // At most one 16 KiB effect read per pass; refill music before SFX work.
    // All SD effect IO is on this thread. Main-thread short effects use RAM.
    void stream_effect() {
        for(unsigned n=0;n<voice_count;++n){auto& v=voices[stream_slot++%voice_count];
            if(!v.pending.load(std::memory_order_acquire))continue;
            if(v.cancelled.load()){v.pending.store(0,std::memory_order_release);continue;}
            if(v.segmented){
                auto& wave=v.segment%2?v.tail:v.wave;
                if(wave.status==NDSP_WBUF_QUEUED || wave.status==NDSP_WBUF_PLAYING)continue;
                const unsigned target=std::min(voice_bytes,v.bytes-v.sent),bytes=std::min(bank_chunk_bytes,target-v.filled);
                char* dst=v.data+(v.segment%2)*voice_bytes;
                const bool ok=std::fseek(effects_file,effect_base+long(v.offset+v.sent+v.filled),SEEK_SET)==0 && std::fread(dst+v.filled,1,bytes,effects_file)==bytes;
                if(!ok){++io_errors;v.pending.store(0,std::memory_order_release);return;}
                detect_speech(v,dst+v.filled,bytes,v.sent+v.filled);v.filled+=bytes;
                if(v.filled==target){DSP_FlushDataCache(dst,target);lock();
                    if(!v.cancelled.load()){wave={};wave.data_vaddr=dst;wave.nsamples=target/sample_bytes;ndspChnWaveBufAdd(unsigned(&v-voices.data())+1,&wave);if(!v.sent)++streamed;}
                    unlock();v.sent+=target;v.filled=0;++v.segment;if(v.sent==v.bytes)v.pending.store(0,std::memory_order_release);
                }
                return;
            }
            const unsigned bytes=std::min(bank_chunk_bytes,v.bytes-v.filled);
            bool ok=std::fseek(effects_file,effect_base+long(v.offset+v.filled),SEEK_SET)==0 && std::fread(v.data+v.filled,1,bytes,effects_file)==bytes;
            if(!ok){++io_errors;v.pending.store(0,std::memory_order_release);return;}
            detect_speech(v,v.data+v.filled,bytes,v.filled);v.filled+=bytes;
            if(v.filled==v.bytes){
                DSP_FlushDataCache(v.data,v.bytes);lock();
                if(!v.cancelled.load()){ndspChnWaveBufAdd(unsigned(&v-voices.data())+1,&v.wave);++streamed;}
                unlock();v.pending.store(0,std::memory_order_release);
            }
            return;
        }
    }
    static void stream(void* arg) {
        auto& s=*(Impl*)arg;
        bool starved=false;
        while(s.running.load()) {
            if(!s.paused.load()) {
                unsigned done=0;for(auto& b:s.music) if(b.status==NDSP_WBUF_DONE) ++done;
                if(done==3 && !starved) {++s.underruns;starved=true;}
                for(auto& b:s.music) if(b.status==NDSP_WBUF_DONE) {
                    if(!s.fill(b)) {s.running=false;break;}
                    starved=false;
                }
            }
            if(!s.paused.load())s.stream_effect();
            svcSleepThread(5000000);
        }
    }
    void close() {
        running=false;
        if(thread) {threadJoin(thread,U64_MAX);threadFree(thread);thread=nullptr;}
        if(linear) {
            for(unsigned i=0;i<=voice_count;++i) ndspChnWaveBufClear(i);
            linearFree(linear);linear=nullptr;
        }
        if(file) {std::fclose(file);file=nullptr;}
        if(effects_file){std::fclose(effects_file);effects_file=nullptr;}
        for(auto& block:pcm)block.reset();
        std::vector<Effect>().swap(effects);resident=0;counters.ready=false;
    }
    ~Impl() {close();}
};
Audio::Audio():impl_(new Impl) {}
Audio::~Audio()=default;
bool Audio::load(const char* root,bool dsp_ready,const char* music_prefix,const char* effects_path,unsigned cache_limit,bool music_only) {
    auto& s=*impl_;s.close();s.counters={};s.underruns=0;s.io_errors=0;s.buffers=0;s.track=0;s.next_track=1;s.streamed=0;s.stream_slot=0;s.effect_index.fill(-1);s.long_voice=false;
    std::fill(std::begin(s.last_played),std::end(s.last_played),0);
    auto fail=[&](const char* m) {s.close();std::snprintf(s.message,sizeof s.message,"%s",m);return false;};
    if(!dsp_ready) return fail("DSP unavailable: check /3ds/dspfirm.cdc");
    try {
        char path[256];
        const int length=effects_path?std::snprintf(path,sizeof path,"%s",effects_path):std::snprintf(path,sizeof path,"%s/effects.bin",root);
        if(length<0 || unsigned(length)>=sizeof path)return fail("Audio path too long");
        for(unsigned i=0;i<3;++i) {
            const int n=std::snprintf(s.track_paths[i],sizeof s.track_paths[i],"%s/%s%u.pcm",root,music_prefix,i+1);
            if(n<0 || unsigned(n)>=sizeof s.track_paths[i])return fail("Music path too long");
        }
        if(!music_only){
        std::unique_ptr<FILE,decltype(&std::fclose)> input(std::fopen(path,"rb"),&std::fclose);
        FILE* f=input.get();
        if(!f) return fail("Missing audio/effects.bin");
        char magic[8];bool valid=std::fread(magic,1,8,f)==8;
        const bool v2=valid && !std::memcmp(magic,"OBWAUD2\0",8);valid=valid && (v2 || !std::memcmp(magic,"OBWAUD1\0",8));
        const auto count=read32(f),size=read32(f);s.sample_bytes=v2?read32(f):2;
        valid=valid && count>0 && count<=1200 && size>0 && size<=64*1024*1024 && (s.sample_bytes==1 || s.sample_bytes==2);
        if(!valid) return fail("Invalid audio header");
        s.effects.reserve(count);s.long_voice=false;
        for(unsigned n=0;n<count;++n) {
            Impl::Effect e{read32(f),read32(f),read32(f),read32(f),read32(f)};
            if(e.id>=audio_sound_limit || e.offset>size || !e.samples || e.samples>(size-e.offset)/s.sample_bytes || e.samples>(e.id>=campaign_sound_base?2*1024*1024:voice_bytes*2)/s.sample_bytes) valid=false;
            if(e.samples*s.sample_bytes>voice_bytes)s.long_voice=true;
            if(e.id<audio_sound_limit){if(s.effect_index[e.id]>=0)valid=false;s.effect_index[e.id]=short(n);}
            s.effects.push_back(e);
        }
        s.effect_base=std::ftell(f);
        if(std::fseek(f,0,SEEK_END) || std::ftell(f)!=s.effect_base+long(size))valid=false;
        if(!valid)return fail("Invalid audio effect bounds");
        // Cache the shortest effects first, so combat sounds remain immediate.
        // A fragmented heap may yield a smaller cache; uncached audio still
        // plays through the bounded background reader without dropping music.
        cache_limit=std::min(cache_bytes,cache_limit)/bank_chunk_bytes*bank_chunk_bytes;
        for(auto& block:s.pcm){if(s.resident>=size || s.resident>=cache_limit)break;try{block.reset(new char[bank_chunk_bytes]);s.resident+=bank_chunk_bytes;}catch(const std::bad_alloc&){break;}}
        std::vector<unsigned> order(count);for(unsigned i=0;i<count;++i)order[i]=i;
        std::stable_sort(order.begin(),order.end(),[&](unsigned a,unsigned b){return s.effects[a].samples<s.effects[b].samples;});
        unsigned used=0;
        for(unsigned index:order){auto& effect=s.effects[index];const unsigned bytes=effect.samples*s.sample_bytes;
            if(bytes>voice_bytes*2 || bytes>s.resident-used)continue;
            if(std::fseek(f,s.effect_base+long(effect.offset),SEEK_SET))return fail("Cannot seek audio effect");
            effect.cached=used;unsigned remaining=bytes;
            while(remaining){const unsigned pos=used%bank_chunk_bytes,n=std::min(remaining,bank_chunk_bytes-pos);
                if(std::fread(s.pcm[used/bank_chunk_bytes].get()+pos,1,n,f)!=n)return fail("Cannot read audio effect");
                used+=n;remaining-=n;
            }
        }
        s.effects_file=input.release();
        }
        const unsigned voice_storage=music_only?0:(voice_count+(s.long_voice?1:0))*voice_bytes;
        s.linear=(char*)linearAlloc(voice_storage+3*chunk_bytes);
        if(!s.linear) return fail("Not enough linear memory for audio");
        for(unsigned i=0;i<=voice_count;++i) {
            ndspChnReset(i);ndspChnSetRate(i,22050);ndspChnSetInterp(i,NDSP_INTERP_LINEAR);
            ndspChnSetFormat(i,i?(s.sample_bytes==1?NDSP_FORMAT_MONO_PCM8:NDSP_FORMAT_MONO_PCM16):NDSP_FORMAT_STEREO_PCM16);
            float mix[12]={};mix[0]=mix[1]=i?0.5f:0.25f;ndspChnSetMix(i,mix);
        }
        for(unsigned i=0;i<voice_count;++i) {s.voices[i].wave={};s.voices[i].tail={};s.voices[i].pending=0;s.voices[i].cancelled=false;s.voices[i].id=~0u;s.voices[i].audible_sample=~0u;s.voices[i].audible_clock=0;s.voices[i].data=music_only?nullptr:s.linear+(i?i+(s.long_voice?1:0):0)*voice_bytes;s.voices[i].capacity=(music_only?0:voice_bytes)*((!i && s.long_voice)?2:1);}
        for(unsigned i=0;i<3;++i) {
            s.music[i]={};s.music[i].data_vaddr=s.linear+voice_storage+i*chunk_bytes;
            if(!s.fill(s.music[i])) return fail("Cannot stream music");
        }
        s.paused=false;s.running=true;
        // Use the application core, including on original 3DS. No extra CPU mode.
        s.thread=threadCreate(Impl::stream,&s,16*1024,0x31,0,false);
        if(!s.thread) return fail("Cannot create music streaming thread");
        s.counters.ready=true;std::snprintf(s.message,sizeof s.message,music_only?"Music-only streaming / %u KiB SFX cache":"Music + 8 voices / %u KiB SFX cache",s.resident/1024);return true;
    } catch(const std::bad_alloc&) {return fail("Not enough heap memory for audio");}
      catch(const std::exception&) {return fail("Cannot load audio");}
}
void Audio::unload(){impl_->close();}
void Audio::reset() {
    auto& s=*impl_;if(!s.counters.ready) return;
    s.lock();for(unsigned i=0;i<voice_count;++i) {s.voices[i].cancelled=true;ndspChnWaveBufClear(i+1);s.voices[i].wave={};s.voices[i].tail={};}s.unlock();
}
void Audio::update(const SoundEvent* events,unsigned count,int cx,int cy,bool paused) {
    auto& s=*impl_;if(!s.counters.ready) return;
    const bool was=s.paused.exchange(paused);
    if(was!=paused) for(unsigned i=0;i<=voice_count;++i) ndspChnSetPaused(i,paused);
    if(paused) return;
    const auto now=osGetTime();
    for(unsigned n=0;n<count;++n) {
        const auto& e=events[n];
        const int distance=e.x||e.y?std::max(0,std::max(cx-e.x,e.x-cx-400))+std::max(0,std::max(cy-e.y,e.y-cy-240)):0;
        if(distance>=512) {++s.counters.culled;continue;}
        const int index=e.id<audio_sound_limit?s.effect_index[e.id]:-1;
        if(index<0) {++s.counters.missing;continue;}
        auto it=s.effects.begin()+index;
        if(now-s.last_played[e.id]<80) continue;
        unsigned slot=voice_count,priority=it->priority;bool duplicate=false;
        for(unsigned i=0;i<voice_count;++i) {
            auto& v=s.voices[i];if(it->samples*s.sample_bytes>v.capacity && !(i==0 && s.long_voice && it->samples*s.sample_bytes>voice_bytes*2))continue;
            if(v.pending.load(std::memory_order_acquire)){if(v.id==e.id && (it->flags&0x10))duplicate=true;continue;}const bool busy=v.wave.status==NDSP_WBUF_QUEUED || v.wave.status==NDSP_WBUF_PLAYING || v.tail.status==NDSP_WBUF_QUEUED || v.tail.status==NDSP_WBUF_PLAYING;
            if(busy && v.id==e.id && (it->flags&0x10)) duplicate=true;
            if(!busy) {slot=i;priority=0;}
            else if(v.priority<priority) {slot=i;priority=v.priority;}
        }
        if(duplicate) continue;
        if(slot==voice_count) {++s.counters.voice_drops;continue;}
        auto& v=s.voices[slot];ndspChnWaveBufClear(slot+1);v.wave={};v.tail={};v.segmented=it->samples*s.sample_bytes>voice_bytes*2;v.segment=v.sent=0;v.audible_sample=~0u;v.audible_clock=0;
        unsigned offset=it->cached,remaining=it->samples*s.sample_bytes,copied=0;
        while(offset!=~0u && remaining) {
            const unsigned position=offset%bank_chunk_bytes,bytes=std::min(remaining,bank_chunk_bytes-position);
            std::memcpy(v.data+copied,s.pcm[offset/bank_chunk_bytes].get()+position,bytes);
            offset+=bytes;copied+=bytes;remaining-=bytes;
        }
        if(it->cached!=~0u) s.detect_speech(v,v.data,it->samples*s.sample_bytes,0);
        if(it->cached!=~0u)DSP_FlushDataCache(v.data,it->samples*s.sample_bytes);
        v.wave.data_vaddr=v.data;v.wave.nsamples=it->samples;v.id=e.id;v.priority=it->priority;
        const float pan=e.x||e.y?std::max(-1.f,std::min(1.f,(e.x-cx-200)/400.f)):0;
        const float volume=0.6f*(512-distance)/512.f;
        float mix[12]={};mix[0]=volume*(pan>0?1-pan:1);mix[1]=volume*(pan<0?1+pan:1);
        ndspChnSetMix(slot+1,mix);
        if(it->cached!=~0u){ndspChnWaveBufAdd(slot+1,&v.wave);++s.counters.played;}
        else {v.offset=it->offset;v.bytes=it->samples*s.sample_bytes;v.filled=0;v.cancelled=false;v.pending.store(1,std::memory_order_release);}
        s.last_played[e.id]=now;
    }
}
AudioStats Audio::stats() const {
    const auto& s=*impl_;auto r=s.counters;r.music_underruns=s.underruns;r.io_errors=s.io_errors;
    r.played+=s.streamed.load();r.streamed=s.streamed.load();r.cache_bytes=s.resident;
    r.music_buffers=s.buffers;r.music_track=s.track;
    if(r.ready) {r.dsp_drops=ndspGetDroppedFrames();for(const auto& v:s.voices) if(v.wave.status==NDSP_WBUF_PLAYING || v.wave.status==NDSP_WBUF_QUEUED || v.tail.status==NDSP_WBUF_PLAYING || v.tail.status==NDSP_WBUF_QUEUED) ++r.voices;}
    return r;
}
bool Audio::playing(unsigned id) const {
 const auto& s=*impl_;if(!s.counters.ready)return false;
 for(const auto& v:s.voices)if(v.id==id && (v.wave.status==NDSP_WBUF_PLAYING || v.tail.status==NDSP_WBUF_PLAYING)){const auto now=osGetTime();if(!v.audible_clock.load())v.audible_clock=now;const auto onset=v.audible_sample.load();return onset!=~0u && now-v.audible_clock.load()>=u64(onset)*1000/22050;}return false;
}
bool Audio::contains(unsigned id)const{return id<audio_sound_limit && impl_->effect_index[id]>=0;}
const char* Audio::error() const {return impl_->message;}
}
