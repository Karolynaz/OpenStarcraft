#include <3ds.h>
#include <citro2d.h>
#include "controls.h"
#include "engine.h"
#include "scheduler.h"
#include <cstdio>
#include <malloc.h>
#include <sys/stat.h>

extern "C" {
u32 __ctru_heap_size = 40 * 1024 * 1024;
u32 __ctru_linear_heap_size = 4 * 1024 * 1024;
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
        port3ds::Scheduler scheduler;
        port3ds::Controls controls;
        unsigned scenario = 0, loaded_scenario = 0;
        bool paused = true;
        u64 previous = now_us(), interval = previous, next_text = 0;
        unsigned renders = 0, ticks = 0;
        double tick_sum = 0, tick_max = 0, fps = 0, tps = 0, mean = 0, worst = 0;
        FILE* log = nullptr;
        const char* action = "L loads your SD game files";
        C2D_Text diagnostics{}, commands{}, message{};
        port3ds::UnitView units[128];
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
            if ((down & KEY_L) && self_test) {
                loaded_scenario = scenario;
                paused = !engine.load(root, map, static_cast<port3ds::Scenario>(scenario));
                scheduler.reset();
                controls = port3ds::Controls{};
                const auto state = engine.snapshot();
                if (engine.ready()) controls.map_size(state.width, state.height);
                previous = interval = now_us();
                renders = ticks = 0; tick_sum = tick_max = 0;
                fps = tps = mean = worst = 0;
                action = engine.ready() ? "A select / B move to cursor" : "Load failed; check SD files";
            }
            if (down & KEY_SELECT) { paused = !paused; scheduler.reset(); }
            if (down & KEY_X) {
                if (log) { std::fclose(log); log = nullptr; action = "CSV logging closed"; }
                else {
                    mkdir("sdmc:/3ds/openbw/logs", 0777);
                    log = std::fopen("sdmc:/3ds/openbw/logs/benchmark.csv", "a");
                    if (log) std::fprintf(log, "scenario,frame,units,fps,tps,mean_tick_us,max_tick_us,heap_bytes,linear_used,backlog\n");
                    action = log ? "CSV logging enabled" : "Cannot open logs/benchmark.csv";
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
            if (down & KEY_A)
                action = !engine.ready() ? "Load game with L first" :
                    engine.select(controls.world_x(), controls.world_y()) ? "Own unit selected" : "Selection cleared";
            if (down & KEY_B)
                action = !engine.ready() ? "Load game with L first" :
                    engine.move_selected(controls.world_x(), controls.world_y()) ? "Move order accepted" : "Select a movable unit with A";
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
                    std::fprintf(log, "%u,%lu,%u,%.2f,%.2f,%.1f,%.1f,%d,%lu,%llu\n",
                        loaded_scenario, static_cast<unsigned long>(state.frame), state.units,
                        fps, tps, mean, worst, heap.uordblks, static_cast<unsigned long>(linear_used),
                        static_cast<unsigned long long>(scheduler.backlog()));
                    std::fflush(log);
                }
                renders = ticks = 0; tick_sum = tick_max = 0; interval = sample_time;
            }
            if (sample_time >= next_text || down) {
                char status[900], help[350], error[220];
                std::snprintf(status, sizeof status,
                    "OpenBW / ORIGINAL 3DS\nCore DCL/endian: %s   DSP: %s\n"
                    "%s  Frame %lu  Units %u\nFPS %.1f   TPS %.1f / 24\n"
                    "Tick avg %.1f / max %.1f us\nHeap %d KiB  Linear %lu KiB\n"
                    "Debt %llu  Cursor %.0f,%.0f\nCamera %.0f,%.0f  CSV %s\n"
                    "Next load: %s",
                    self_test ? "PASS" : "FAIL", R_SUCCEEDED(audio) ? "ready" : "off",
                    !engine.ready() ? "NOT LOADED" : paused ? "PAUSED" : "RUNNING",
                    static_cast<unsigned long>(state.frame), state.units, fps, tps, mean, worst,
                    heap.uordblks / 1024, static_cast<unsigned long>(linear_used / 1024),
                    static_cast<unsigned long long>(scheduler.backlog()), controls.cursor_x, controls.cursor_y,
                    controls.camera_x, controls.camera_y, log ? "ON" : "OFF", scenario_name(scenario));
                std::snprintf(help, sizeof help,
                    "Stick: cursor\nA: select\nB: move\nL: load  R: scenario\nSELECT: pause\nY: step  X: CSV\nSTART: exit");
                std::snprintf(error, sizeof error, "%s\n%.100s", action, engine.error());
                C2D_TextBufClear(text_buf);
                C2D_TextParse(&diagnostics, text_buf, status); C2D_TextOptimize(&diagnostics);
                C2D_TextParse(&commands, text_buf, help); C2D_TextOptimize(&commands);
                C2D_TextParse(&message, text_buf, error); C2D_TextOptimize(&message);
                next_text = sample_time + 250000;
            }
            const auto count = engine.visible_units(units, 128);
            const u32 bg = C2D_Color32(8, 16, 24, 255), grid = C2D_Color32(25, 49, 57, 255);
            const u32 green = C2D_Color32(100, 240, 180, 255), white = C2D_Color32(215, 230, 235, 255);
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(top, bg); C2D_SceneBegin(top);
            for (int x = -static_cast<int>(controls.camera_x) % 32; x < 400; x += 32)
                C2D_DrawLine(x, 0, grid, x, 240, grid, 1, 0);
            for (int y = -static_cast<int>(controls.camera_y) % 32; y < 240; y += 32)
                C2D_DrawLine(0, y, grid, 400, y, grid, 1, 0);
            // Diagnostic unit markers, not a terrain/sprite renderer. They
            // intentionally show all visible-list units, without fog rendering.
            for (std::size_t i = 0; i != count; ++i) {
                const auto& u = units[i];
                const float x = u.x - controls.camera_x, y = u.y - controls.camera_y;
                if (x < -8 || x > 408 || y < -8 || y > 248) continue;
                if (u.selected) C2D_DrawRectSolid(x - 7, y - 7, 0, 14, 14, white);
                C2D_DrawRectSolid(x - 4, y - 4, 0, 8, 8, u.owner == 0 ? green : C2D_Color32(240, 110, 100, 255));
            }
            const float cx = controls.cursor_x, cy = controls.cursor_y;
            C2D_DrawLine(cx - 6, cy, green, cx + 6, cy, green, 2, 0);
            C2D_DrawLine(cx, cy - 6, green, cx, cy + 6, green, 2, 0);
            C2D_DrawText(&message, C2D_WithColor | C2D_WordWrap, 8, 206, 0, 0.38f, 0.38f, white, 384.0f);
            C2D_TargetClear(bottom, bg); C2D_SceneBegin(bottom);
            C2D_DrawText(&diagnostics, C2D_WithColor, 8, 4, 0, 0.42f, 0.42f, white);
            C2D_DrawText(&commands, C2D_WithColor, 140, 144, 0, 0.40f, 0.40f, white);
            using C = port3ds::Controls;
            C2D_DrawRectSolid(C::minimap_x, C::minimap_y, 0, C::minimap_w, C::minimap_h, grid);
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
