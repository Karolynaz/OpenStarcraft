#pragma once
#include "engine.h"
#include <citro2d.h>
#include <vector>
namespace port3ds {
class Graphics {
    struct Rect { unsigned page, x, y, width, height; };
    struct Sprite { std::uint32_t key; Rect rect; };
    C3D_Tex pages_[8]{};
    unsigned page_count_ = 0, width_ = 0, height_ = 0;
    std::vector<Rect> tiles_;
    std::vector<unsigned short> map_;
    std::vector<Sprite> sprites_;
    Rect minimap_{};
    bool loaded_ = false;
    char error_[160] = "Graphics not loaded";
    C2D_Image image(const Rect&, Tex3DS_SubTexture&);
    void draw(const Rect&, float x, float y, float scale_x = 1, float scale_y = 1, bool flip = false, bool shadow = false);
public:
    ~Graphics();
    bool load(const char* path, unsigned signature);
    void clear();
    bool ready() const { return loaded_; }
    const char* error() const { return error_; }
    unsigned texture_bytes() const;
    void terrain(int camera_x, int camera_y);
    unsigned sprites(const SpriteView* views, std::size_t count, int camera_x, int camera_y);
    void minimap(float x, float y, float width, float height);
};
}
