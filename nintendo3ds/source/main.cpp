#include <3ds.h>
#include <citro2d.h>
#include "controls.h"
#include "engine.h"
#include "graphics.h"
#include "scheduler.h"
#include <cstdio>
#include <malloc.h>
#include <sys/stat.h>

extern "C" {
u32 __ctru_heap_size = 36 * 1024 * 1024;
u32 __ctru_linear_heap_size = 8 * 1024 * 1024;
u32 __stacksize__ = 256 * 1024;
}
namespace {
constexpr const char* root = "sdmc:/3ds/openbw/";
constexpr const char* map = "sdmc:/3ds/openbw/maps/probe.scx";
bool restored = false;
void lifecycle(APT_HookType hook, void*) {
    if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP) restored = true;
}
u64 now_us() {
    const u64 ticks = svcGetSystemTick();
    return ticks / SYSCLOCK_ARM11 * 1000000 + ticks % SYSCLOCK_ARM11 * 1000000 / SYSCLOCK_ARM11;
}
const char* scenario_name(unsigned s) {
    static const char* names[] = {"Empty map", "Two marines", "1v1 melee (idle)"};
    return names[s % 3];
}
}

int main() {
    gfxInitDefault();
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) { gfxExit(); return 1; }
    if (!C2D_Init(2048)) { C3D_Fini(); gfxExit(); return 1; }
    C2D_Prepare();
    auto* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    auto* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    auto text_buf = C2D_TextBufNew(2048);
    if (!top || !bottom || !text_buf) {
        if (text_buf) C2D_TextBufDelete(text_buf);
        C2D_Fini(); C3D_Fini(); gfxExit(); return 1;
    }
    aptHookCookie cookie;
    aptHook(&cookie, lifecycle, nullptr);
    const bool self_test = port3ds::core_self_test();
    const Result audio = ndspInit();
    if (R_SUCCEEDED(audio)) ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    {
        port3ds::Engine engine;
        port3ds::Graphics graphics;
        port3ds::Scheduler scheduler;
        port3ds::Controls controls;
        unsigned scenario = 1, loaded_scenario = 1;
        bool paused = true;
        bool pending_load = false;
        bool placing = false;
        auto building = port3ds::Command::depot;
        u64 previous = now_us(), interval = previous, next_text = 0;
        unsigned renders = 0, ticks = 0;
        double tick_sum = 0, tick_max = 0, fps = 0, tps = 0, mean = 0, worst = 0;
        FILE* log = nullptr;
        const char* action = "L loads your SD game files";
        C2D_Text diagnostics{}, commands{}, message{}, resources{};
        port3ds::UnitView units[128];
        port3ds::SpriteView images[512];
        unsigned missing_images = 0;
        while (aptMainLoop()) {
            hidScanInput();
            const u32 down = hidKeysDown();
            if (down & KEY_START) break;
            const u64 now = now_us();
            if (restored) { previous = now; scheduler.reset(); restored = false; }
            const u64 elapsed = now - previous;
            if (!paused && engine.ready()) scheduler.add(elapsed);
            previous = now;
            if (down & KEY_R) scenario = (scenario + 1) % 3;
            const bool load_now = pending_load;
            pending_load = false;
            if ((down & KEY_L) && self_test) {
                pending_load = true;
                action = "Loading map, game data and graphics...";
                next_text = 0;
            }
            // Present the loading message for one frame before blocking SD IO.
            if (load_now) {
                placing = false;
                loaded_scenario = scenario;
                paused = !engine.load(root, map, static_cast<port3ds::Scenario>(scenario));
                if (engine.ready()) graphics.load("sdmc:/3ds/openbw/graphics.bin", engine.terrain_signature());
                else graphics.clear();
                // Atlas descriptors keep their addresses across reloads, while
                // their GPU buffers can move. Invalidate Citro2D's texture cache.
                C2D_Prepare();
                scheduler.reset();
                controls = port3ds::Controls{};
                const auto state = engine.snapshot();
                if (engine.ready()) {
                    controls.map_size(state.width, state.height);
                    const auto count = engine.visible_units(units, 128);
                    for (std::size_t i = 0; i != count; ++i) if (units[i].owner == 0) {
                        controls.focus(units[i].x, units[i].y);
                        break;
                    }
                }
                previous = interval = now_us();
                renders = ticks = 0; tick_sum = tick_max = 0;
                fps = tps = mean = worst = 0;
                action = engine.ready() ? "A select / B act / D-pad build or train" : "Load failed; check SD files";
            }
            if (down & KEY_SELECT) { paused = !paused; scheduler.reset(); }
            if ((down & KEY_X) && paused) {
                if (log) { std::fclose(log); log = nullptr; action = "CSV logging closed"; }
                else {
                    mkdir("sdmc:/3ds/openbw/logs", 0777);
                    log = std::fopen("sdmc:/3ds/openbw/logs/renderer-benchmark.csv", "a");
                    if (log) std::fprintf(log, "scenario,frame,units,fps,tps,mean_tick_us,max_tick_us,heap_bytes,linear_used,backlog,atlas_bytes,missing_images\n");
                    action = log ? "CSV logging enabled" : "Cannot open logs/renderer-benchmark.csv";
                }
            }
            circlePosition circle{};
            hidCircleRead(&circle);
            // UI motion is capped after a long loading/stalled frame. The
            // simulation accumulator above retains every scheduled tick.
            controls.stick(circle.dx, circle.dy, std::min<u64>(elapsed, 50000) / 1000000.0f);
            if (hidKeysHeld() & KEY_TOUCH) {
                touchPosition touch{}; hidTouchRead(&touch);
                controls.minimap_touch(touch.px, touch.py);
            }
            if ((down & KEY_A) && placing) { placing=false; action="Building placement cancelled"; }
            else if (down & KEY_A)
                action = !engine.ready() ? "Load game with L first" :
                    engine.select(controls.world_x(), controls.world_y()) ? "Own unit selected" : "Selection cleared";
            auto command = [&](port3ds::Command c) {
                engine.command(c, controls.world_x(), controls.world_y());
                action = engine.command_status();
            };
            if (down & KEY_B) {
                if (placing) {
                    if (engine.command(building,controls.world_x(),controls.world_y())) placing=false;
                    action=engine.command_status();
                } else command(port3ds::Command::context);
            }
            if ((down & KEY_X) && !paused) command(port3ds::Command::attack);
            if ((down & KEY_Y) && !paused) command(port3ds::Command::stop);
            if (down & KEY_DUP) command(port3ds::Command::train);
            if (down & (KEY_DRIGHT|KEY_DDOWN)) {
                building = (down & KEY_DRIGHT) ? port3ds::Command::barracks : port3ds::Command::depot;
                placing=true;
                action = building == port3ds::Command::depot ? "Depot: 100 minerals. B place / A cancel" : "Barracks: 150 minerals. B place / A cancel";
            }
            if (down & KEY_DLEFT) {
                if (placing) { placing=false; action="Building placement cancelled"; }
                else command(port3ds::Command::cancel);
            }
            const bool step = (down & KEY_Y) && paused;
            for (unsigned n = 0; n != 2 && engine.ready(); ++n) {
                if (!(step && n == 0) && (paused || !scheduler.due())) break;
                const u64 begin = now_us();
                const bool ok = engine.tick();
                const double duration = static_cast<double>(now_us() - begin);
                if (!ok) { paused = true; break; }
                ++ticks; tick_sum += duration;
                if (duration > tick_max) tick_max = duration;
                if (!paused) scheduler.consume();
            }
            const auto state = engine.snapshot();
            const auto heap = mallinfo();
            const u32 linear_used = __ctru_linear_heap_size - linearSpaceFree();
            const u64 sample_time = now_us();
            if (sample_time - interval >= 1000000) {
                const double seconds = (sample_time - interval) / 1000000.0;
                fps = renders / seconds; tps = ticks / seconds;
                mean = ticks ? tick_sum / ticks : 0; worst = tick_max;
                if (log) {
                    std::fprintf(log, "%u,%lu,%u,%.2f,%.2f,%.1f,%.1f,%d,%lu,%llu,%u,%u\n",
                        loaded_scenario, static_cast<unsigned long>(state.frame), state.units,
                        fps, tps, mean, worst, heap.uordblks, static_cast<unsigned long>(linear_used),
                        static_cast<unsigned long long>(scheduler.backlog()), graphics.texture_bytes(), missing_images);
                    std::fflush(log);
                }
                renders = ticks = 0; tick_sum = tick_max = 0; interval = sample_time;
            }
            if (sample_time >= next_text || down) {
                char status[900], help[350], error[220], hud[200];
                std::snprintf(status, sizeof status,
                    "OpenBW / ORIGINAL 3DS\nCore DCL/endian: %s   DSP: %s\n"
                    "%s  Frame %lu  Units %u\nFPS %.1f   TPS %.1f / 24\n"
                    "Tick avg %.1f / max %.1f us\nHeap %d KiB  Linear %lu KiB\n"
                    "Debt %llu  Cursor %.0f,%.0f\nCamera %.0f,%.0f  CSV %s\n"
                    "Next load: %s\nAtlas %lu KiB  Missing sprites %u",
                    self_test ? "PASS" : "FAIL", R_SUCCEEDED(audio) ? "ready" : "off",
                    !engine.ready() ? "NOT LOADED" : paused ? "PAUSED" : "RUNNING",
                    static_cast<unsigned long>(state.frame), state.units, fps, tps, mean, worst,
                    heap.uordblks / 1024, static_cast<unsigned long>(linear_used / 1024),
                    static_cast<unsigned long long>(scheduler.backlog()), controls.cursor_x, controls.cursor_y,
                    controls.camera_x, controls.camera_y, log ? "ON" : "OFF", scenario_name(scenario),
                    static_cast<unsigned long>(graphics.texture_bytes()/1024), missing_images);
                std::snprintf(help, sizeof help,
                    "A select / B act\nX attack / Y stop\nD-up: train unit\nD-right: Barracks\nD-down: Depot\nD-left: cancel\nL load / R mode\nSELECT pause\nPaused: X CSV / Y step");
                std::snprintf(hud, sizeof hud, "Minerals %d   Gas %d   Supply %d/%d\n%s  HP %d/%d  Queue %u",
                    state.minerals, state.gas, state.supply, state.supply_max, state.selected_name,
                    state.selected_hp, state.selected_max_hp, state.queue);
                std::snprintf(error, sizeof error, "%s\n%.100s", action, graphics.ready() ? engine.error() : graphics.error());
                C2D_TextBufClear(text_buf);
                C2D_TextParse(&diagnostics, text_buf, status); C2D_TextOptimize(&diagnostics);
                C2D_TextParse(&commands, text_buf, help); C2D_TextOptimize(&commands);
                C2D_TextParse(&message, text_buf, error); C2D_TextOptimize(&message);
                C2D_TextParse(&resources, text_buf, hud); C2D_TextOptimize(&resources);
                next_text = sample_time + 250000;
            }
            const auto count = engine.visible_units(units, 128);
            const u32 bg = C2D_Color32(8, 16, 24, 255), grid = C2D_Color32(25, 49, 57, 255);
            const u32 green = C2D_Color32(100, 240, 180, 255), white = C2D_Color32(215, 230, 235, 255);
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(top, bg); C2D_SceneBegin(top);
            if (graphics.ready()) graphics.terrain(controls.camera_x,controls.camera_y);
            else {
                for (int x = -static_cast<int>(controls.camera_x) % 32; x < 400; x += 32)
                    C2D_DrawLine(x, 0, grid, x, 240, grid, 1, 0);
                for (int y = -static_cast<int>(controls.camera_y) % 32; y < 240; y += 32)
                    C2D_DrawLine(0, y, grid, 400, y, grid, 1, 0);
            }
            for (std::size_t i = 0; i != count; ++i) {
                const auto& u = units[i];
                const float x = u.x - controls.camera_x, y = u.y - controls.camera_y;
                if (x < -8 || x > 408 || y < -8 || y > 248) continue;
                if (u.selected) {
                    const float w = std::max(12, u.width/2), h = std::max(7, u.height/2);
                    C2D_DrawLine(x-w,y-h,green,x+w,y-h,green,1,0);
                    C2D_DrawLine(x-w,y+h,green,x+w,y+h,green,1,0);
                    C2D_DrawLine(x-w,y-h,green,x-w,y+h,green,1,0);
                    C2D_DrawLine(x+w,y-h,green,x+w,y+h,green,1,0);
                }
                if (!graphics.ready()) C2D_DrawRectSolid(x - 4, y - 4, 0, 8, 8, u.owner == 0 ? green : C2D_Color32(240, 110, 100, 255));
            }
            const auto image_count=engine.visible_images(images,512,controls.camera_x,controls.camera_y);
            missing_images=graphics.sprites(images,image_count,controls.camera_x,controls.camera_y);
            if (placing) {
                const auto site=engine.placement(building,controls.world_x(),controls.world_y());
                const u32 color=site.valid ? C2D_Color32(70,230,100,100) : C2D_Color32(240,60,60,100);
                C2D_DrawRectSolid(site.x-controls.camera_x,site.y-controls.camera_y,0,site.width,site.height,color);
            }
            for (std::size_t i = 0; i != count; ++i) if (units[i].selected && units[i].max_hp > 0) {
                const float x = units[i].x-controls.camera_x, y = units[i].y-controls.camera_y;
                const float w = std::max(24, units[i].width);
                C2D_DrawRectSolid(x-w/2,y-units[i].height/2-5,0,w,3,C2D_Color32(35,35,35,255));
                C2D_DrawRectSolid(x-w/2,y-units[i].height/2-5,0,w*units[i].hp/units[i].max_hp,3,green);
            }
            const float cx = controls.cursor_x, cy = controls.cursor_y;
            C2D_DrawLine(cx - 6, cy, green, cx + 6, cy, green, 2, 0);
            C2D_DrawLine(cx, cy - 6, green, cx, cy + 6, green, 2, 0);
            C2D_DrawRectSolid(0,0,0,400,29,C2D_Color32(8,16,24,210));
            C2D_DrawText(&resources, C2D_WithColor, 8, 1, 0, 0.40f, 0.40f, white);
            C2D_DrawText(&message, C2D_WithColor | C2D_WordWrap, 8, 206, 0, 0.38f, 0.38f, white, 384.0f);
            C2D_TargetClear(bottom, bg); C2D_SceneBegin(bottom);
            C2D_DrawText(&diagnostics, C2D_WithColor, 8, 4, 0, 0.42f, 0.42f, white);
            C2D_DrawText(&commands, C2D_WithColor, 140, 144, 0, 0.34f, 0.34f, white);
            using C = port3ds::Controls;
            C2D_DrawRectSolid(C::minimap_x, C::minimap_y, 0, C::minimap_w, C::minimap_h, grid);
            graphics.minimap(C::minimap_x,C::minimap_y,C::minimap_w,C::minimap_h);
            const float sx = C::minimap_w / controls.map_width(), sy = C::minimap_h / controls.map_height();
            for (std::size_t i = 0; i != count; ++i)
                C2D_DrawRectSolid(C::minimap_x + units[i].x * sx, C::minimap_y + units[i].y * sy, 0, 2, 2,
                                 units[i].owner == 0 ? green : C2D_Color32(240, 110, 100, 255));
            const float mx = C::minimap_x + controls.camera_x * sx, my = C::minimap_y + controls.camera_y * sy;
            C2D_DrawLine(mx, my, green, mx + 400 * sx, my, green, 1, 0);
            C2D_DrawLine(mx, my, green, mx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx + 400 * sx, my, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx, my + 240 * sy, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            C3D_FrameEnd(0);
            ++renders;
        }
        if (log) std::fclose(log);
    }
    if (R_SUCCEEDED(audio)) ndspExit();
    aptUnhook(&cookie);
    C2D_TextBufDelete(text_buf);
    C2D_Fini(); C3D_Fini(); gfxExit();
    return 0;
}
