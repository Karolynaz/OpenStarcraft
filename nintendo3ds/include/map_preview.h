#pragma once
#include <array>
#include <cstdint>
namespace port3ds {
struct MapPreview {
 char title[128]{},description[768]{},error[128]{};
 unsigned width=0,height=0,players=0,tileset=0;bool valid=false;
 std::array<std::uint16_t,128*128> pixels{};
};
}
