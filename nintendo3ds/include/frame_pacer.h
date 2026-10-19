#pragma once
#include <cstdint>
namespace port3ds {
// Limit submission to the display cadence without waiting an extra refresh
// when simulation/render preparation has already crossed both VBlanks.
class FramePacer {
    std::uint32_t top_=0,bottom_=0;
    bool started_=false;
public:
    bool needs_sync(std::uint32_t top,std::uint32_t bottom) const {
        return !started_ || top==top_ || bottom==bottom_;
    }
    void began(std::uint32_t top,std::uint32_t bottom) {
        top_=top;bottom_=bottom;started_=true;
    }
};
}
