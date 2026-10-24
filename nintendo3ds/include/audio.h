#pragma once
#include "engine.h"
#include <memory>
namespace port3ds {
struct AudioStats {
    unsigned played=0, culled=0, missing=0, voice_drops=0, music_underruns=0, io_errors=0;
    unsigned streamed=0,cache_bytes=0;
    unsigned music_buffers=0, music_track=0, voices=0, dsp_drops=0;
    bool ready=false;
};
class Audio {
public:
    Audio(); ~Audio();
    bool load(const char* root, bool dsp_ready,const char* music_prefix="terran",const char* effects_path=nullptr);
    void update(const SoundEvent* events, unsigned count, int camera_x, int camera_y, bool paused);
    AudioStats stats() const;
    const char* error() const;
    void reset();
    void unload();
private:
    struct Impl; std::unique_ptr<Impl> impl_;
};
}
