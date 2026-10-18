#include <3ds.h>
#include "audio.h"
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};}
static void u32(std::ofstream& f,unsigned n) {for(unsigned i=0;i<4;++i) f.put(n>>(8*i));}
static void effects(const std::string& root,bool bad=false,bool eight=false) {
    std::ofstream f(root+"/effects.bin",std::ios::binary);f.write(eight?"OBWAUD2\0":"OBWAUD1\0",8);u32(f,1);u32(f,8);if(eight)u32(f,1);
    u32(f,69);u32(f,bad?9:0);u32(f,eight?8:4);u32(f,1);u32(f,0);f.write("12345678",8);
}
int main() {
    char path[]="/tmp/openbw-audio-test-XXXXXX";assert(mkdtemp(path));const std::string root=path;
    for(unsigned n=1;n<=3;++n) {std::ofstream f(root+"/terran"+std::to_string(n)+".pcm",std::ios::binary);f<<std::string(4096,'a'+n);}
    effects(root);
    {
        port3ds::Audio audio;assert(!audio.load(path,false));assert(!audio.stats().ready);
        assert(audio.load(path,true));assert(audio.stats().music_buffers==3);
        // Short tracks exercise EOF/next-track across a single streaming chunk.
        assert(audio.stats().music_track>=1 && audio.stats().music_track<=3);
        port3ds::SoundEvent events[]={{69,100,100},{69,2000,2000},{1199,100,100}};
        audio.update(events,3,0,0,false);
        auto s=audio.stats();assert(s.played==1 && s.culled==1 && s.missing==1 && s.voices==1);
        const auto queued=fake::queued.load();audio.update(events,1,0,0,true);
        assert(audio.stats().played==1 && fake::queued==queued && fake::pauses==9);
        audio.update(nullptr,0,0,0,false);assert(fake::pauses==18);
        audio.reset();assert(audio.stats().voices==0);
        effects(root,true);assert(!audio.load(path,true));assert(!audio.stats().ready && fake::allocations==0);
        effects(root);assert(audio.load(path,true));
        effects(root,false,true);assert(audio.load(path,true));audio.update(events,1,0,0,false);assert(audio.stats().played==1);
        // A missing/empty music file must stop cleanly, rather than spin at EOF.
        std::ofstream(root+"/terran1.pcm",std::ios::binary).close();
        assert(!audio.load(path,true));assert(fake::allocations==0 && !audio.stats().ready);
        std::remove((root+"/terran1.pcm").c_str());
        assert(!audio.load(path,true));assert(audio.stats().io_errors>0 && fake::allocations==0);
    }
    assert(fake::allocations==0 && fake::queued==0);
    for(unsigned n=1;n<=3;++n) std::remove((root+"/terran"+std::to_string(n)+".pcm").c_str());
    std::remove((root+"/effects.bin").c_str());rmdir(path);
    std::puts("Audio bounds / missing DSP / EOF track transitions / positional effects / pause / reload / DSP buffer lifetime PASS");
}
