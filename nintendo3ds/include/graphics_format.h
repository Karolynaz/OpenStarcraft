#pragma once
#include <cstdint>
namespace port3ds {
constexpr unsigned atlas_side = 512;
constexpr unsigned atlas_max_pages = 8;
constexpr unsigned atlas_page_bytes = atlas_side * atlas_side * 2;
inline std::uint32_t terrain_hash_step(std::uint32_t hash, unsigned value) {
    hash = (hash ^ (value & 255)) * 16777619u;
    return (hash ^ ((value >> 8) & 255)) * 16777619u;
}
inline std::uint32_t sprite_key(unsigned image, unsigned frame, unsigned owner) {
    return (image << 16) | (frame << 2) | (owner < 3 ? owner : 3);
}
}
