#pragma once
#include <cstdint>
namespace port3ds {
enum class CursorKind : unsigned { arrow, own, neutral, enemy, target, target_neutral, target_enemy,
    up, down, left, right, up_left, up_right, down_left, down_right, busy, drag, count };
constexpr unsigned cursor_image_base=65400;
constexpr unsigned cursor_count=unsigned(CursorKind::count);
inline const char* cursor_file(unsigned kind) {
    static const char* names[]={"arrow","magg","magy","magr","targg","targy","targr",
        "scrollu","scrolld","scrolll","scrollr","scrollul","scrollur","scrolldl","scrolldr","time","drag"};
    return kind<cursor_count?names[kind]:nullptr;
}
// Show edge arrows only when the stick is actually requesting edge scrolling.
inline CursorKind edge_cursor(float x,float y,int dx,int dy) {
    const int horizontal=x<16 && dx< -20?-1:x>383 && dx>20?1:0;
    const int vertical=y<16 && dy>20?-1:y>223 && dy< -20?1:0;
    if(vertical<0)return horizontal<0?CursorKind::up_left:horizontal>0?CursorKind::up_right:CursorKind::up;
    if(vertical>0)return horizontal<0?CursorKind::down_left:horizontal>0?CursorKind::down_right:CursorKind::down;
    return horizontal<0?CursorKind::left:horizontal>0?CursorKind::right:CursorKind::arrow;
}
}
