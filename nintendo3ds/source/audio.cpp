#include "audio.h"
#include <3ds.h>
#include <array>
#include <atomic>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <algorithm>
namespace port3ds {
namespace {
constexpr unsigned voice_count=8, voice_bytes=128*1024, chunk_bytes=32768;
unsigned read32(FILE* f) { unsigned char b[4]; if(std::fread(b,1,4,f)!=4) return ~0u; return b[0]|b[1]<<8|b[2]<<16|unsigned(b[3])<<24; }
}
struct Audio::Impl {
    struct Effect { unsigned id,offset,samples,priority,flags; };
    struct Voice { ndspWaveBuf wave{}; unsigned id=~0u,priority=0,capacity=0; char* data=nullptr; };
    std::vector<Effect> effects;
    std::vector<char> pcm;
    std::array<Voice,voice_count> voices{};
    ndspWaveBuf music[3]{};
    char* linear=nullptr;
    std::string root;
    FILE* file=nullptr;
    Thread thread=nullptr;
    std::atomic<bool> running{false}, paused{false};
    std::atomic<unsigned> underruns{0}, io_errors{0}, buffers{0}, track{0};
    AudioStats counters;
    char message[100]="Audio not loaded";
    u64 last_played[1200]{};
    unsigned next_track=1,sample_bytes=2;bool long_voice=false;
    bool open_track() {
        if(file) std::fclose(file);
        const auto path=root+"/terran"+std::to_string(next_track)+".pcm";
        file=std::fopen(path.c_str(),"rb");
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
        pcm.clear();effects.clear();counters.ready=false;
    }
    ~Impl() {close();}
};
Audio::Audio():impl_(new Impl) {}
Audio::~Audio()=default;
bool Audio::load(const char* root,bool dsp_ready) {
    auto& s=*impl_;s.close();s.counters={};s.underruns=0;s.io_errors=0;s.buffers=0;s.track=0;s.next_track=1;
    std::fill(std::begin(s.last_played),std::end(s.last_played),0);
    auto fail=[&](const char* m) {s.close();std::snprintf(s.message,sizeof s.message,"%s",m);return false;};
    if(!dsp_ready) return fail("DSP unavailable: check /3ds/dspfirm.cdc");
    s.root=root;
    FILE* f=std::fopen((s.root+"/effects.bin").c_str(),"rb");
    if(!f) return fail("Missing audio/effects.bin");
    char magic[8];bool valid=std::fread(magic,1,8,f)==8;
    const bool v2=valid && !std::memcmp(magic,"OBWAUD2\0",8);valid=valid && (v2 || !std::memcmp(magic,"OBWAUD1\0",8));
    const auto count=read32(f),size=read32(f);s.sample_bytes=v2?read32(f):2;
    valid=valid && count>0 && count<=1200 && size>0 && size<=8*1024*1024 && (s.sample_bytes==1 || s.sample_bytes==2);
    if(!valid) {std::fclose(f);return fail("Invalid audio header");}
    s.effects.reserve(count);s.long_voice=false;
    for(unsigned n=0;n<count;++n) {
        Impl::Effect e{read32(f),read32(f),read32(f),read32(f),read32(f)};
        if(e.id>=1200 || e.offset>size || !e.samples || e.samples>(size-e.offset)/s.sample_bytes || e.samples>voice_bytes*2/s.sample_bytes) valid=false;
        if(e.samples*s.sample_bytes>voice_bytes)s.long_voice=true;
        s.effects.push_back(e);
    }
    if(valid) {s.pcm.resize(size);valid=std::fread(s.pcm.data(),1,size,f)==size && std::fgetc(f)==EOF;}
    std::fclose(f);if(!valid) return fail("Invalid audio effect bounds");
    s.linear=(char*)linearAlloc((voice_count+(s.long_voice?1:0))*voice_bytes+3*chunk_bytes);
    if(!s.linear) return fail("Not enough linear memory for audio");
    for(unsigned i=0;i<=voice_count;++i) {
        ndspChnReset(i);ndspChnSetRate(i,22050);ndspChnSetInterp(i,NDSP_INTERP_LINEAR);
        ndspChnSetFormat(i,i?(s.sample_bytes==1?NDSP_FORMAT_MONO_PCM8:NDSP_FORMAT_MONO_PCM16):NDSP_FORMAT_STEREO_PCM16);
        float mix[12]={};mix[0]=mix[1]=i?0.5f:0.25f;ndspChnSetMix(i,mix);
    }
    for(unsigned i=0;i<voice_count;++i) {s.voices[i]={};s.voices[i].data=s.linear+(i?i+(s.long_voice?1:0):0)*voice_bytes;s.voices[i].capacity=voice_bytes*((!i && s.long_voice)?2:1);}
    for(unsigned i=0;i<3;++i) {
        s.music[i]={};s.music[i].data_vaddr=s.linear+(voice_count+(s.long_voice?1:0))*voice_bytes+i*chunk_bytes;
        if(!s.fill(s.music[i])) return fail("Cannot stream Terran music");
    }
    s.paused=false;s.running=true;
    // Use the application core, including on original 3DS. No extra CPU mode.
    s.thread=threadCreate(Impl::stream,&s,16*1024,0x31,0,false);
    if(!s.thread) return fail("Cannot create music streaming thread");
    s.counters.ready=true;std::snprintf(s.message,sizeof s.message,"Terran music + 8 DSP effect voices");return true;
}
void Audio::reset() {
    auto& s=*impl_;if(!s.counters.ready) return;
    for(unsigned i=0;i<voice_count;++i) {ndspChnWaveBufClear(i+1);s.voices[i].wave={};}
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
        auto it=std::find_if(s.effects.begin(),s.effects.end(),[&](const Impl::Effect& a){return a.id==e.id;});
        if(it==s.effects.end()) {++s.counters.missing;continue;}
        if(now-s.last_played[e.id]<80) continue;
        unsigned slot=voice_count,priority=it->priority;bool duplicate=false;
        for(unsigned i=0;i<voice_count;++i) {
            auto& v=s.voices[i];if(it->samples*s.sample_bytes>v.capacity)continue;const bool busy=v.wave.status==NDSP_WBUF_QUEUED || v.wave.status==NDSP_WBUF_PLAYING;
            if(busy && v.id==e.id && (it->flags&0x10)) duplicate=true;
            if(!busy) {slot=i;priority=0;}
            else if(v.priority<priority) {slot=i;priority=v.priority;}
        }
        if(duplicate) continue;
        if(slot==voice_count) {++s.counters.voice_drops;continue;}
        auto& v=s.voices[slot];ndspChnWaveBufClear(slot+1);v.wave={};
        std::memcpy(v.data,s.pcm.data()+it->offset,it->samples*s.sample_bytes);
        DSP_FlushDataCache(v.data,it->samples*s.sample_bytes);
        v.wave.data_vaddr=v.data;v.wave.nsamples=it->samples;v.id=e.id;v.priority=it->priority;
        const float pan=e.x||e.y?std::max(-1.f,std::min(1.f,(e.x-cx-200)/400.f)):0;
        const float volume=0.6f*(512-distance)/512.f;
        float mix[12]={};mix[0]=volume*(pan>0?1-pan:1);mix[1]=volume*(pan<0?1+pan:1);
        ndspChnSetMix(slot+1,mix);ndspChnWaveBufAdd(slot+1,&v.wave);
        s.last_played[e.id]=now;++s.counters.played;
    }
}
AudioStats Audio::stats() const {
    const auto& s=*impl_;auto r=s.counters;r.music_underruns=s.underruns;r.io_errors=s.io_errors;
    r.music_buffers=s.buffers;r.music_track=s.track;
    if(r.ready) {r.dsp_drops=ndspGetDroppedFrames();for(const auto& v:s.voices) if(v.wave.status==NDSP_WBUF_PLAYING || v.wave.status==NDSP_WBUF_QUEUED) ++r.voices;}
    return r;
}
const char* Audio::error() const {return impl_->message;}
}
