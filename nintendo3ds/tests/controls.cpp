#include "controls.h"
#include <cassert>
#include <cstdio>
int main() {
    port3ds::Controls c;
    c.map_size(32, 32);
    c.stick(20, -20, 1);
    assert(c.cursor_x == 200 && c.cursor_y == 120);
    c.stick(156, 0, 1);
    assert(c.cursor_x == 399 && c.camera_x > 0);
    const auto x = c.cursor_x, y = c.cursor_y;
    const auto camera = c.camera_x;
    assert(!c.minimap_touch(200, 40));
    assert(c.camera_x == camera && c.cursor_x == x && c.cursor_y == y);
    assert(c.minimap_touch(154, 152));
    assert(c.camera_x == 624 && c.camera_y == 784);
    assert(c.cursor_x == x && c.cursor_y == y); // Touch never controls the cursor.
    assert(c.minimap_touch(40, 45));
    assert(c.camera_x == 0 && c.camera_y == 0);
    assert(!c.minimap_touch(155, 153));
    c.focus(128, 128);
    assert(c.world_x() == 128 && c.world_y() == 128);
    assert(c.camera_x == 0 && c.camera_y == 8);
    c.focus(800, 800);
    assert(c.cursor_x == 200 && c.cursor_y == 120);
    assert(c.world_x() == 800 && c.world_y() == 800);
    c.cursor_x=73;c.cursor_y=82;
    c.center_camera(128,128);
    assert(c.camera_x==0 && c.camera_y==8 && c.cursor_x==73 && c.cursor_y==82);
    c.follow(800,800,1);
    assert(c.camera_x==600 && c.camera_y==680 && c.cursor_x==73 && c.cursor_y==82);
    std::puts("Stick cursor / edge scroll / minimap-only touch PASS");
}
