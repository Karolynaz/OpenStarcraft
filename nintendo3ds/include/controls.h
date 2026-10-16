#pragma once
#include <algorithm>
#include <cmath>

namespace port3ds {
class Controls {
    float width_ = 1024, height_ = 1024;
    void clamp_camera() {
        camera_x = std::max(0.0f, std::min(camera_x, width_ - 400));
        camera_y = std::max(0.0f, std::min(camera_y, height_ - 240));
    }
public:
    float cursor_x = 200, cursor_y = 120;
    float camera_x = 0, camera_y = 0;
    // The only touch-sensitive area, drawn identically by the bottom renderer.
    static constexpr int minimap_x = 8, minimap_y = 144, minimap_w = 120, minimap_h = 88;
    void map_size(unsigned tile_width, unsigned tile_height) {
        width_ = std::max(400.0f, tile_width * 32.0f);
        height_ = std::max(240.0f, tile_height * 32.0f);
        clamp_camera();
    }
    void stick(int dx, int dy, float seconds) {
        auto axis = [](int value) {
            const int magnitude = std::abs(value);
            if (magnitude <= 20) return 0.0f;
            return (value < 0 ? -1.0f : 1.0f) * std::min(1.0f, (magnitude - 20) / 136.0f);
        };
        const float x = axis(dx), y = -axis(dy);
        cursor_x = std::max(0.0f, std::min(399.0f, cursor_x + x * 220 * seconds));
        cursor_y = std::max(0.0f, std::min(239.0f, cursor_y + y * 220 * seconds));
        if ((cursor_x < 16 && x < 0) || (cursor_x > 383 && x > 0)) camera_x += x * 180 * seconds;
        if ((cursor_y < 16 && y < 0) || (cursor_y > 223 && y > 0)) camera_y += y * 180 * seconds;
        clamp_camera();
    }
    bool minimap_touch(int x, int y) {
        if (x < minimap_x || x >= minimap_x + minimap_w || y < minimap_y || y >= minimap_y + minimap_h) return false;
        camera_x = (x - minimap_x) * width_ / (minimap_w - 1) - 200;
        camera_y = (y - minimap_y) * height_ / (minimap_h - 1) - 120;
        clamp_camera();
        return true;
    }
    int world_x() const { return static_cast<int>(camera_x + cursor_x); }
    int world_y() const { return static_cast<int>(camera_y + cursor_y); }
    float map_width() const { return width_; }
    float map_height() const { return height_; }
};
}
