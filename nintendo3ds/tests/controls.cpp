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
    assert(c.minimap_touch(127, 231));
    assert(c.camera_x == 624 && c.camera_y == 784);
    assert(c.cursor_x == x && c.cursor_y == y); // Touch never controls the cursor.
    assert(c.minimap_touch(8, 144));
    assert(c.camera_x == 0 && c.camera_y == 0);
    assert(!c.minimap_touch(128, 232));
    std::puts("Stick cursor / edge scroll / minimap-only touch PASS");
}
