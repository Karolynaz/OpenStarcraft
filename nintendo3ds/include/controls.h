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
    // Camera touch bounds match the supplied bottom-screen HUD.
    static constexpr int minimap_x = 14, minimap_y = 35, minimap_w = 80, minimap_h = 100;
    void map_size(unsigned tile_width, unsigned tile_height) {
        width_ = std::max(400.0f, tile_width * 32.0f);
        height_ = std::max(240.0f, tile_height * 32.0f);
        clamp_camera();
    }
    void focus(int world_x, int world_y) {
        camera_x = world_x - 200.0f;
        camera_y = world_y - 120.0f;
        clamp_camera();
        cursor_x = std::max(0.0f, std::min(399.0f, world_x - camera_x));
        cursor_y = std::max(0.0f, std::min(239.0f, world_y - camera_y));
    }
    void center_camera(int world_x,int world_y) {
        camera_x=world_x-200.0f; camera_y=world_y-120.0f; clamp_camera();
    }
    void follow(int world_x,int world_y,float seconds) {
        const float amount=std::min(1.0f,seconds*3);
        camera_x+=(world_x-200.0f-camera_x)*amount;
        camera_y+=(world_y-120.0f-camera_y)*amount;
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
    struct MapRect{float x,y,w,h;};
    MapRect minimap_bounds()const{float scale=std::min(minimap_w/width_,minimap_h/height_);float w=width_*scale,h=height_*scale;return {minimap_x+(minimap_w-w)/2,minimap_y+(minimap_h-h)/2,w,h};}
    bool minimap_touch(int x,int y){const auto m=minimap_bounds();if(x<m.x || x>=m.x+m.w || y<m.y || y>=m.y+m.h)return false;camera_x=(x-m.x)*width_/m.w-200;camera_y=(y-m.y)*height_/m.h-120;clamp_camera();return true;}
    int world_x() const { return static_cast<int>(camera_x + cursor_x); }
    int world_y() const { return static_cast<int>(camera_y + cursor_y); }
    float map_width() const { return width_; }
    float map_height() const { return height_; }
};
}
