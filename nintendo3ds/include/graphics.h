#pragma once
#include "engine.h"
#include <citro2d.h>
#include <vector>
#include <unordered_map>
namespace port3ds {
class Graphics {
 struct Rect {unsigned page,x,y,width,height;};
 struct Sprite {std::uint32_t key;Rect rect;};
 struct Cached {Rect rect;std::uint64_t age=0;};
 C3D_Tex pages_[8]{};
 unsigned page_count_=0,terrain_pages_=0,width_=0,height_=0;
 std::vector<Rect> tiles_;
 std::vector<unsigned short> map_;
 std::vector<Sprite> sprites_;
 std::unordered_map<std::uint32_t,Cached> cache_;
 std::vector<uint16_t> scratch_;
 uint16_t occupied_[8][16]{};
 std::uint64_t frame_=0;
 unsigned cache_misses_=0,cache_overflows_=0,upload_pixels_=0;
 Rect minimap_{};bool loaded_=false;
 char error_[160]="Graphics not loaded";
 C2D_Image image(const Rect&,Tex3DS_SubTexture&);
 void draw(const Rect&,float,float,float sx=1,float sy=1,bool flip=false,bool shadow=false,unsigned opacity=255);
 bool allocate(unsigned w,unsigned h,Rect& result);
 void release(const Rect&);
 bool cache_frame(const SpriteView&,const Engine&,bool dirty[8]);
public:
 ~Graphics();bool load(const char*,unsigned signature);void clear();
 bool ready()const{return loaded_;}const char* error()const{return error_;}
 unsigned texture_bytes()const;
 unsigned cache_misses()const{return cache_misses_;}unsigned cache_overflows()const{return cache_overflows_;}unsigned upload_pixels()const{return upload_pixels_;}
 void terrain(int,int);
 unsigned sprites(const SpriteView*,std::size_t,int,int,const Engine&);
 void minimap(float,float,float,float);
};
}
