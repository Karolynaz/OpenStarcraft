#include <3ds.h>
#include <citro2d.h>
#include "controls.h"
#include "engine.h"
#include "graphics.h"
#include "scheduler.h"
#include "frame_pacer.h"
#include "input.h"
#include "ability_controls.h"
#include "command_grid.h"
#include "frontend_draw.h"
#include "source_id.h"
#include "audio.h"
#include "csv_log.h"
#include <cstdio>
#include <cstring>
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
constexpr const char* arena = "sdmc:/3ds/openbw/maps/arena.chk";
bool restored = false;
void lifecycle(APT_HookType hook, void*) {
    if (hook == APTHOOK_ONRESTORE || hook == APTHOOK_ONWAKEUP) restored = true;
}
u64 now_us() {
    const u64 ticks = svcGetSystemTick();
    return ticks / SYSCLOCK_ARM11 * 1000000 + ticks % SYSCLOCK_ARM11 * 1000000 / SYSCLOCK_ARM11;
}
void load_stage(unsigned scenario,const char* stage,const char* detail="") {
    // Close each record before the next loading stage, including fatal failures.
    mkdir("sdmc:/3ds/openbw/logs",0777);
    FILE* f=std::fopen("sdmc:/3ds/openbw/logs/scenario-load.log","a");
    if(!f)return;
    const auto heap=mallinfo();
    std::fprintf(f,"%llu build=%s scenario=%u stage=%s heap_used=%d heap_free=%d linear_free=%lu %s\n",
        static_cast<unsigned long long>(osGetTime()),port3ds::source_id,scenario,stage,heap.uordblks,heap.fordblks,
        static_cast<unsigned long>(linearSpaceFree()),detail);
    std::fclose(f);
}
const char* scenario_name(unsigned s) {
    static const char* names[] = {"Empty map", "Two marines", "Manual base", "AI vs AI", "Player vs AI", "Gas AI vs AI", "All Terran battle", "Terran laboratory", "All Protoss battle", "Protoss laboratory", "All Zerg battle", "Zerg laboratory", "Protoss AI vs AI", "Protoss player vs AI", "Zerg AI vs AI", "Zerg player vs AI"};
    return names[s % 16];
}
}

int main() {
    gfxInitDefault();
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) { gfxExit(); return 1; }
    if (!C2D_Init(4096)) { C3D_Fini(); gfxExit(); return 1; }
    C2D_Prepare();
    auto* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    auto* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    auto text_buf = C2D_TextBufNew(4096);
    if (!top || !bottom || !text_buf) {
        if (text_buf) C2D_TextBufDelete(text_buf);
        C2D_Fini(); C3D_Fini(); gfxExit(); return 1;
    }
    aptHookCookie cookie;
    aptHook(&cookie, lifecycle, nullptr);
    const bool self_test = port3ds::core_self_test();
    const Result audio = ndspInit();
    if (R_SUCCEEDED(audio)) ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    int launch_scenario=-1;
    {
        port3ds::Frontend front;front.init(root);
        port3ds::UiArt art;const bool art_ok=art.load("sdmc:/3ds/openbw/ui/menu.bin");
        C2D_Prepare();u64 last=now_us();
        while(aptMainLoop()){
            hidScanInput();const u32 down=hidKeysDown();circlePosition stick{};hidCircleRead(&stick);
            const auto now=now_us();front.stick(stick.dx,stick.dy,std::min<u64>(now-last,50000)/1000000.f);last=now;
            if(down&KEY_TOUCH){touchPosition t{};hidTouchRead(&t);front.x=t.px;front.y=t.py;front.hover=front.hit(t.px,t.py);front.activate(front.hover);}
            if(down&KEY_A)front.activate(front.hit(int(front.x),int(front.y)));
            if(down&KEY_B)front.back();
            if(front.quit)break;
            if(front.play){launch_scenario=int(front.scenario());break;}
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(text_buf);
            C2D_TargetClear(top,C2D_Color32(0,0,0,255));C2D_SceneBegin(top);C3D_DepthTest(false,GPU_ALWAYS,GPU_WRITE_COLOR);
            art.draw("title",0,0);
            C2D_TargetClear(bottom,C2D_Color32(0,0,0,255));C2D_SceneBegin(bottom);
            if(art_ok)port3ds::draw_frontend(front,art,text_buf,unsigned(now/125000));
            else {port3ds::ui_text(text_buf,"Missing ui/menu.bin",160,90,.55f,C2D_Color32(255,200,180,255),true);port3ds::ui_text(text_buf,"Install the complete SD package.",160,120,.40f,C2D_Color32(255,255,255,255),true);if(down&KEY_START)break;}
            C3D_FrameEnd(0);
        }
    }
    if(launch_scenario>=0) {
        port3ds::Engine engine;
        port3ds::Graphics graphics;
        port3ds::Audio sound;
        port3ds::Scheduler scheduler;
        port3ds::FramePacer frame_pacer;
        port3ds::Controls controls;
        unsigned scenario = unsigned(launch_scenario), loaded_scenario = scenario;
        port3ds::UiArt hud_art;hud_art.load("sdmc:/3ds/openbw/ui/hud.bin");
        auto hud_buf=C2D_TextBufNew(4096);
        port3ds::InputRouter input;
        enum class Menu { none, system, scenarios, legacy, catalogue };
        port3ds::OptionPage option_page=port3ds::OptionPage::primary;
        port3ds::MenuOption option_rows[8]{};unsigned option_count=0;
        Menu menu=Menu::none; unsigned choice=0;
        bool initial_load=true, auto_camera=true, attacking=false, warming=false;
        u64 camera_manual_until=0;
        bool paused = true;
        bool pending_load = false;
        bool placing = false;
        unsigned placing_code=0, target_code=0;
        char target_help[128]{};
        auto shown_cursor=port3ds::CursorKind::arrow;u64 cursor_since=now_us();
        u64 previous = now_us(), interval = previous, next_text = 0;
        unsigned renders = 0, ticks = 0;
        double tick_sum = 0, tick_max = 0, fps = 0, tps = 0, mean = 0, worst = 0;
        double render_sum=0,wait_sum=0,collect_sum=0,sprite_sum=0,end_sum=0,gpu_sum=0;
        port3ds::CsvLog log;
        bool audio_paused_last=true;
        u64 last_log_flush_us=0;
        const char* action = "Loading player vs AI...";
        C2D_Text diagnostics{}, commands{}, message{}, resources{}, option_hint{};
        C2D_Text menu_rows[8]{};
        // 40 controlled units per side plus the map's neutral resources.
        constexpr unsigned unit_view_capacity=512;
        port3ds::UnitView units[unit_view_capacity];
        port3ds::SpriteView images[2048];
        unsigned missing_images = 0;
        auto toggle_log=[&]() {
            if(log) { log.close(); action="CSV logging closed"; return; }
            mkdir("sdmc:/3ds/openbw/logs",0777);
            log.open("sdmc:/3ds/openbw/logs/ai-audio-benchmark.csv","scenario,frame,units,fps,tps,mean_tick_us,max_tick_us,heap_bytes,linear_used,backlog,atlas_bytes,missing_images,red_workers,red_army,red_buildings,red_minerals,red_lost,blue_workers,blue_army,blue_buildings,blue_minerals,blue_lost,winner,audio_ready,sfx_played,sfx_culled,sfx_missing,sfx_voice_drops,event_drops,music_buffers,music_underruns,audio_io_errors,music_track,dsp_drops,red_gas,blue_gas,csv_flush_us,battle_seconds,red_score,blue_score,end_reason,cache_misses,cache_overflows,upload_pixels,csv_dropped,csv_errors,render_cpu_us,frame_wait_us,sprite_collect_us,sprite_draw_us,frame_end_us,gpu_us,source_id\n");
            action=log?"CSV logging enabled":"Cannot open ai-audio-benchmark.csv";
        };
        while (aptMainLoop()) {
            hidScanInput();
            const u32 down = hidKeysDown();
            const u32 held=hidKeysHeld();
            const u64 now = now_us();
            if (restored) { previous = now; scheduler.reset(); restored = false; }
            const u64 elapsed = now - previous;
            if (!paused && engine.ready()) {scheduler.add(elapsed);engine.advance_wall_time(elapsed);}
            previous = now;
            const bool load_now = pending_load;
            pending_load = false;
            if(initial_load && self_test) { initial_load=false; pending_load=true; }
            // Present the loading message for one frame before blocking SD IO.
            if (load_now) {
                placing = false; attacking=false; menu=Menu::none;
                // Release resident PCM and sprite scratch before the core's
                // temporary MPQ/GRP initialization peak on original hardware.
                load_stage(scenario,"release-start");
                sound.unload();graphics.clear();
                load_stage(scenario,"core-start");
                const auto mode=static_cast<port3ds::Scenario>(scenario);
                const bool arena_mode=port3ds::arena_scenario(mode);
                const unsigned next_race=port3ds::scenario_race(mode);
                loaded_scenario = scenario;
                paused = !engine.load(root, arena_mode?arena:map, static_cast<port3ds::Scenario>(scenario));
                load_stage(scenario,engine.ready()?"core-ready":"core-failed",engine.ready()?"":engine.error());
                // Reserve the contiguous sprite scratch before segmented PCM
                // fills smaller holes left by the native core initialization.
                if (engine.ready()) graphics.load(arena_mode?"sdmc:/3ds/openbw/graphics-arena.bin":"sdmc:/3ds/openbw/graphics.bin", engine.terrain_signature());
                else graphics.clear();
                load_stage(scenario,graphics.ready()?"graphics-ready":"graphics-off",graphics.ready()?"":graphics.error());
                if(engine.ready())sound.load(next_race==0?"sdmc:/3ds/openbw/audio/zerg":next_race==2?"sdmc:/3ds/openbw/audio/protoss":"sdmc:/3ds/openbw/audio",R_SUCCEEDED(audio),next_race==0?"zerg":next_race==2?"protoss":"terran");
                load_stage(scenario,sound.stats().ready?"audio-ready":"audio-off",sound.error());
                // Atlas descriptors keep their addresses across reloads, while
                // their GPU buffers can move. Invalidate Citro2D's texture cache.
                C2D_Prepare();
                scheduler.reset();warming=true;
                controls = port3ds::Controls{};
                const auto state = engine.snapshot();
                if (engine.ready()) {
                    controls.map_size(state.width, state.height);
                    const auto count = engine.visible_units(units, unit_view_capacity);
                    for (std::size_t i = 0; i != count; ++i) if (units[i].owner == 0) {
                        controls.focus(units[i].x, units[i].y);
                        break;
                    }
                }
                previous = interval = now_us();
                renders = ticks = 0; tick_sum = tick_max = 0;
                fps = tps = mean = worst = 0;
                auto_camera=engine.snapshot().observer;
                camera_manual_until=0;
                if(!log) toggle_log();
                action = engine.ready() ? (auto_camera ? "AI vs AI: automatic camera / START menu" : "Circle Pad cursor / A action / B cancel") : engine.error();
            }
            circlePosition circle{}; hidCircleRead(&circle);
            const float ui_seconds=std::min<u64>(elapsed,50000)/1000000.0f;
            controls.stick(circle.dx,circle.dy,ui_seconds);
            if(std::abs(circle.dx)>20 || std::abs(circle.dy)>20) camera_manual_until=now+8000000;
            if(held & KEY_TOUCH) {
                touchPosition touch{};hidTouchRead(&touch);
                if(controls.minimap_touch(touch.px,touch.py)) camera_manual_until=now+8000000;
            }
            auto command=[&](port3ds::Command c) {
                engine.command(c,controls.world_x(),controls.world_y()); action=engine.command_status();
            };
            auto menu_count=[&]() -> unsigned {return menu==Menu::system?6:menu==Menu::scenarios?8:menu==Menu::legacy?8:menu==Menu::catalogue?option_count:0;};
            auto open_options=[&](port3ds::OptionPage page) {
                option_page=page;option_count=engine.options(page,option_rows,8);choice=0;
                menu=option_count?Menu::catalogue:Menu::none;
                if(!option_count)action="No special action for this selection";
            };
            bool exit_requested=false;
            auto use_option=[&](const port3ds::MenuOption& option) {
                if(!option.enabled){action="Unavailable: research, energy, resources or queue";return;}
                const auto kind=static_cast<port3ds::OptionKind>(option.code>>16);
                if(kind==port3ds::OptionKind::build) {placing=true;attacking=false;placing_code=option.code;action="A place building / B cancel";}
                else if(option.target) {attacking=true;placing=false;target_code=option.code;std::snprintf(target_help,sizeof target_help,"%.64s: A target / B cancel",option.label);action=target_help;}
                else {engine.perform(option.code,controls.world_x(),controls.world_y());action=engine.command_status();}
                menu=Menu::none;
            };
            auto activate_menu=[&]() {
                if(menu==Menu::system) {
                    if(choice==0) {menu=Menu::scenarios;choice=0;return;}
                    if(choice==1) {auto_camera=!auto_camera;action=auto_camera?"Automatic camera on":"Automatic camera off";}
                    if(choice==2)toggle_log();
                    if(choice==3){pending_load=true;action="Loading scenario and graphics...";}
                    if(choice==4){engine.set_fog_enabled(!engine.fog_enabled());action=engine.fog_enabled()?"Fog of war: player vision":"Fog of war: full-map observer";}
                    if(choice==5)exit_requested=true;
                } else if(menu==Menu::scenarios) {
                    if(choice==7){menu=Menu::legacy;choice=0;return;}
                    static const unsigned modes[]={10,11,8,9,6,7,4};scenario=modes[choice];pending_load=true;action="Loading scenario and graphics...";
                } else if(menu==Menu::legacy){static const unsigned modes[]={14,15,12,13,5,3,1,2};scenario=modes[choice];pending_load=true;action="Loading skirmish...";} else if(menu==Menu::catalogue) {option_count=engine.options(option_page,option_rows,8);if(choice<option_count)use_option(option_rows[choice]);else {menu=Menu::none;action="Selection has no available commands";}return;}
                menu=Menu::none;
            };
            port3ds::InputFrame frame;
            frame.a=down&KEY_A; frame.b=down&KEY_B; frame.x=down&KEY_X; frame.y=down&KEY_Y;
            frame.start=down&KEY_START; frame.select=down&KEY_SELECT; frame.l=held&KEY_L; frame.r=held&KEY_R; frame.time=now;
            if(down&KEY_DUP) frame.direction=0; else if(down&KEY_DRIGHT) frame.direction=1;
            else if(down&KEY_DDOWN) frame.direction=2; else if(down&KEY_DLEFT) frame.direction=3;
            const auto event=input.read(frame,menu!=Menu::none);
            using I=port3ds::InputAction;
            switch(event.action) {
            case I::system: menu=menu==Menu::system?Menu::none:Menu::system; choice=0; break;
            case I::pause: paused=!paused; scheduler.reset(); break;
            case I::cancel:
                if(menu!=Menu::none || placing || attacking) { menu=Menu::none;placing=false;attacking=false;action="Action cancelled"; }
                else { engine.clear_selection();action="Selection cleared"; }
                break;
            case I::army: placing=false;attacking=false;engine.select_army(controls.camera_x,controls.camera_y);action="Military units on screen selected";menu=Menu::none;break;
            case I::workers: placing=false;attacking=false;engine.select_workers();action="All own workers selected";menu=Menu::none;break;
            case I::primary:
            case I::secondary:
                placing=false;attacking=false;
                if(engine.snapshot().selection==port3ds::SelectionKind::workers)open_options(event.action==I::primary?port3ds::OptionPage::basic:port3ds::OptionPage::advanced);
                else {
                    const auto selected=engine.snapshot();
                    if(!selected.selected_building){
                        option_count=engine.options(port3ds::OptionPage::primary,option_rows,8);
                        const int shortcut=port3ds::ability_shortcut(option_count,event.action==I::secondary,selected.selected_larva);
                        if(shortcut>=0)use_option(option_rows[shortcut]);
                        else open_options(event.action==I::primary?port3ds::OptionPage::primary:port3ds::OptionPage::secondary);
                    }else {
                        open_options(event.action==I::primary?port3ds::OptionPage::primary:port3ds::OptionPage::secondary);
                        if(event.action==I::primary && option_count==1)use_option(option_rows[0]);
                    }
                }
                break;
            case I::confirm:
                if(menu!=Menu::none)activate_menu();
                else if(placing){if(engine.perform(placing_code,controls.world_x(),controls.world_y()))placing=false;action=engine.command_status();}
                else if(attacking){if(engine.perform(target_code,controls.world_x(),controls.world_y()))attacking=false;action=engine.command_status();}
                else {engine.activate(controls.world_x(),controls.world_y());action=engine.command_status();}
                break;
            case I::previous:case I::next:
                if(menu_count())choice=menu==Menu::catalogue?port3ds::CommandGrid::move(choice,event.slot,menu_count()):event.action==I::previous?(choice+menu_count()-1)%menu_count():(choice+1)%menu_count();
                break;
            case I::store: action=engine.store_group(event.slot)?"Group saved to D-pad":"Empty group saved"; break;
            case I::recall: {
                placing=false;attacking=false;
                int x=0,y=0;
                if(engine.recall_group(event.slot,&x,&y)) {
                    if(event.center) { controls.center_camera(x,y);camera_manual_until=now+8000000; }
                    action=event.center?"Group selected and camera centered":"Group selected";
                } else action="Group empty or temporarily inside building";
                break;
            }
            default: break;
            }
            if(down&KEY_TOUCH) {
                touchPosition t{};hidTouchRead(&t);
                if(port3ds::UiRect{244,161,66,21}.hit(t.px,t.py)){menu=menu==Menu::system?Menu::none:Menu::system;choice=0;}
                else if(menu==Menu::none || menu==Menu::catalogue){
                    if(menu==Menu::none){option_page=port3ds::OptionPage::primary;option_count=engine.options(option_page,option_rows,8);}
                    const int hit=port3ds::CommandGrid::hit(t.px,t.py,9);
                    if(hit>=0 && hit<int(option_count)){choice=unsigned(hit);use_option(option_rows[choice]);}
                    else if(hit==8){
                        const bool worker=engine.snapshot().selection==port3ds::SelectionKind::workers;
                        open_options(worker?(option_page==port3ds::OptionPage::basic?port3ds::OptionPage::advanced:port3ds::OptionPage::basic):(option_page==port3ds::OptionPage::primary?port3ds::OptionPage::secondary:port3ds::OptionPage::primary));
                    }else {
                        unsigned slot=0;const auto selectable=engine.visible_units(units,unit_view_capacity);
                        for(std::size_t i=0;i<selectable && slot<12;++i)if(units[i].selected){
                            const auto rect=port3ds::UiRect{15+int(slot%6)*34,166+int(slot/6)*34,32,32};
                            if(rect.hit(t.px,t.py)){engine.select(units[i].x,units[i].y);break;}++slot;
                        }
                    }
                }else if(t.px>=164 && t.px<306 && t.py>=46 && t.py<46+menu_count()*17){choice=(t.py-46)/17;activate_menu();}
            }
            if(exit_requested) break;
            const bool step = false;
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
            if(state.observer && auto_camera && now>=camera_manual_until) {
                int x=0,y=0; if(engine.observer_focus(x,y)) controls.follow(x,y,ui_seconds);
            }
            if(state.winner>=0 && !paused) {
                paused=true;scheduler.reset();action=state.winner==0?"Red wins / START to restart":state.winner==1?"Blue wins / START to restart":"Draw / START to restart";
            }
            port3ds::SoundEvent sound_events[256];
            const auto sound_count=engine.drain_sounds(sound_events,256);
            sound.update(sound_events,sound_count,controls.camera_x,controls.camera_y,paused);
            const auto sound_stats=sound.stats();
            const bool flush_pending=paused && !audio_paused_last;
            audio_paused_last=paused;
            const auto heap = mallinfo();
            const u32 linear_used = __ctru_linear_heap_size - linearSpaceFree();
            const u64 sample_time = now_us();
            if (sample_time - interval >= 1000000) {
                const double seconds = (sample_time - interval) / 1000000.0;
                fps = renders / seconds; tps = ticks / seconds;
                mean = ticks ? tick_sum / ticks : 0; worst = tick_max;
                if (log) {
                    char row[1400];std::snprintf(row,sizeof row, "%u,%lu,%u,%.2f,%.2f,%.1f,%.1f,%d,%lu,%llu,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%d,%u,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%llu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%s\n",
                        loaded_scenario, static_cast<unsigned long>(state.frame), state.units,
                        fps, tps, mean, worst, heap.uordblks, static_cast<unsigned long>(linear_used),
                        static_cast<unsigned long long>(scheduler.backlog()), graphics.texture_bytes(), missing_images,
                        state.players[0].workers,state.players[0].army,state.players[0].buildings,state.players[0].minerals,state.players[0].lost,
                        state.players[1].workers,state.players[1].army,state.players[1].buildings,state.players[1].minerals,state.players[1].lost,state.winner,
                        unsigned(sound_stats.ready),sound_stats.played,sound_stats.culled,sound_stats.missing,sound_stats.voice_drops,engine.dropped_sounds(),
                        sound_stats.music_buffers,sound_stats.music_underruns,sound_stats.io_errors,sound_stats.music_track,sound_stats.dsp_drops,state.players[0].gas,state.players[1].gas,static_cast<unsigned long long>(last_log_flush_us),state.battle_seconds,state.score[0],state.score[1],state.end_reason,graphics.cache_misses(),graphics.cache_overflows(),graphics.upload_pixels(),log.stats().dropped,log.stats().errors,render_sum/std::max(1u,renders),wait_sum/std::max(1u,renders),collect_sum/std::max(1u,renders),sprite_sum/std::max(1u,renders),end_sum/std::max(1u,renders),gpu_sum/std::max(1u,renders),port3ds::source_id);
                    log.write(row);
                }
                renders = ticks = 0; tick_sum = tick_max = 0; interval = sample_time;
                render_sum=wait_sum=collect_sum=sprite_sum=end_sum=gpu_sum=0;
            }
            if(flush_pending && log)log.flush();
            last_log_flush_us=log.stats().flush_us;
            if (sample_time >= next_text || down) {
                char status[900], help[800], error[220], hud[200];
                std::snprintf(status, sizeof status,
                    "OpenBW / ORIGINAL 3DS\n%s  Core: %s   DSP: %s\n"
                    "%s  Frame %lu  Units %u\nFPS %.1f   TPS %.1f / 24\n"
                    "Tick avg %.1f / max %.1f us\nHeap %d KiB  Linear %lu KiB\n"
                    "Debt %llu  Camera %.0f,%.0f  CSV %s\n"
                    "%s  Selected %u\nAtlas %lu KiB  Missing sprites %u\nAudio %s  FX %u  Gaps %u / IO %u",
                    port3ds::source_id,self_test ? "PASS" : "FAIL", R_SUCCEEDED(audio) ? "ready" : "off",
                    !engine.ready() ? "NOT LOADED" : paused ? "PAUSED" : "RUNNING",
                    static_cast<unsigned long>(state.frame), state.units, fps, tps, mean, worst,
                    heap.uordblks / 1024, static_cast<unsigned long>(linear_used / 1024),
                    static_cast<unsigned long long>(scheduler.backlog()),
                    controls.camera_x, controls.camera_y, log.stats().errors?"ERR":log ? "ON" : "OFF", scenario_name(loaded_scenario), state.selected,
                    static_cast<unsigned long>(graphics.texture_bytes()/1024), missing_images,
                    sound_stats.ready?"ON":"OFF",sound_stats.played,sound_stats.music_underruns,sound_stats.io_errors);
                if(menu==Menu::none && attacking)std::snprintf(help,sizeof help,"%.64s\nA confirm target\nB cancel targeting\nCircle Pad: aim",target_help);
                else if(menu==Menu::none) std::snprintf(help,sizeof help,"A action / B cancel\nX army / L+X workers\nY power1 / L+Y power2/menu\nR+D-pad: save group\nD-pad: recall group\nTwice: center camera\nSELECT pause\nSTART menu");
                else {
                    const char* system[]={"Scenarios",auto_camera?"Auto camera: ON":"Auto camera: OFF",log?"CSV: ON":"CSV: OFF","Reload current",engine.fog_enabled()?"Fog: ON (player)":"Fog: OFF (observer)","Exit"};
                    const char* modes[]={"All Zerg battle","Zerg laboratory","All Protoss battle","Protoss laboratory","All Terran battle","Terran laboratory","Player vs AI","More scenarios..."};
                    const char* legacy[]={"Zerg AI vs AI","Zerg player vs AI","Protoss AI vs AI","Protoss player vs AI","Gas AI vs AI","Marines AI vs AI","Two Marines","Manual base"};
                    if(menu==Menu::catalogue){option_count=engine.options(option_page,option_rows,8);choice=std::min(choice,option_count?option_count-1:0u);}
                    unsigned used=0;
                    for(unsigned i=0;i<menu_count();++i)used+=std::snprintf(help+used,sizeof help-used,"%s %s\n",menu==Menu::catalogue?"":i==choice?">":" ",menu==Menu::system?system[i]:menu==Menu::scenarios?modes[i]:menu==Menu::legacy?legacy[i]:option_rows[i].label);
                }
                std::snprintf(hud,sizeof hud,"Minerals %d   Gas %d   Supply %d/%d",state.minerals,state.gas,state.supply,state.supply_max);
                std::snprintf(error, sizeof error, "%s\n%.100s", action, !engine.ready()?engine.error():!sound_stats.ready ? sound.error() : graphics.ready() ? engine.error() : graphics.error());
                C2D_TextBufClear(text_buf);
                C2D_TextParse(&diagnostics, text_buf, status); C2D_TextOptimize(&diagnostics);
                C2D_TextParse(&commands, text_buf, help); C2D_TextOptimize(&commands);
                if(menu_count()) {
                    char* line=help;
                    for(unsigned i=0;i<menu_count();++i) {
                        char* end=std::strchr(line,'\n');if(end) *end=0;
                        C2D_TextParse(&menu_rows[i],text_buf,line);C2D_TextOptimize(&menu_rows[i]);
                        if(end) line=end+1;
                    }
                }
                C2D_TextParse(&message, text_buf, error); C2D_TextOptimize(&message);
                C2D_TextParse(&resources, text_buf, hud); C2D_TextOptimize(&resources);
                C2D_TextParse(&option_hint,text_buf,menu==Menu::catalogue && option_count?option_rows[choice].label:"");C2D_TextOptimize(&option_hint);
                next_text = sample_time + 250000;
            }
            const u64 render_begin=now_us();
            const auto count = engine.visible_units(units, unit_view_capacity);
            const u32 bg = C2D_Color32(8, 16, 24, 255), grid = C2D_Color32(25, 49, 57, 255);
            const u32 green = C2D_Color32(100, 240, 180, 255), white = C2D_Color32(215, 230, 235, 255);
            const u64 wait_begin=now_us();
            C3D_FrameBegin(frame_pacer.needs_sync(C3D_FrameCounter(0),C3D_FrameCounter(1))?C3D_FRAME_SYNCDRAW:0);
            frame_pacer.began(C3D_FrameCounter(0),C3D_FrameCounter(1));
            const u64 waited=now_us()-wait_begin;
            gpu_sum+=C3D_GetDrawingTime()*1000;
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
            const u64 collect_begin=now_us();
            const auto image_count=engine.visible_images(images,2048,controls.camera_x,controls.camera_y);
            const u64 sprites_begin=now_us();
            collect_sum+=sprites_begin-collect_begin;
            missing_images=graphics.sprites(images,image_count,controls.camera_x,controls.camera_y,engine);
            sprite_sum+=now_us()-sprites_begin;
            graphics.fog(engine,controls.camera_x,controls.camera_y);
            if (placing) {
                const auto site=engine.placement_unit(placing_code&65535,controls.world_x(),controls.world_y());
                const u32 color=site.valid ? C2D_Color32(70,230,100,100) : C2D_Color32(240,60,60,100);
                C2D_DrawRectSolid(site.x-controls.camera_x,site.y-controls.camera_y,0,site.width,site.height,color);
            }
            for (std::size_t i = 0; i != count; ++i) if (units[i].selected && units[i].max_hp > 0) {
                const float x = units[i].x-controls.camera_x, y = units[i].y-controls.camera_y;
                const float w = std::max(24, units[i].width);
                C2D_DrawRectSolid(x-w/2,y-units[i].height/2-5,0,w,3,C2D_Color32(35,35,35,255));
                C2D_DrawRectSolid(x-w/2,y-units[i].height/2-5,0,w*units[i].hp/units[i].max_hp,3,green);
                if(units[i].max_shields)C2D_DrawRectSolid(x-w/2,y-units[i].height/2-9,0,w*units[i].shields/units[i].max_shields,3,C2D_Color32(80,140,255,255));
            }
            const float cx = controls.cursor_x, cy = controls.cursor_y;
            float resource_width=0,resource_height=0;C2D_TextGetDimensions(&resources,.36f,.36f,&resource_width,&resource_height);
            C2D_DrawRectSolid(394-resource_width,0,0,resource_width+6,14,C2D_Color32(8,16,24,210));
            C2D_DrawText(&resources,C2D_WithColor,397-resource_width,1,0,.36f,.36f,white);
            if(placing || attacking || pending_load || !engine.ready())C2D_DrawText(&message,C2D_WithColor|C2D_WordWrap,8,211,0,.34f,.34f,white,384.f);
            auto cursor=engine.cursor_kind(controls.world_x(),controls.world_y(),attacking);
            if(menu!=Menu::none)cursor=port3ds::CursorKind::arrow;
            else if(placing){const auto site=engine.placement_unit(placing_code&65535,controls.world_x(),controls.world_y());cursor=site.valid?port3ds::CursorKind::target:port3ds::CursorKind::target_enemy;}
            else {const auto edge=port3ds::edge_cursor(cx,cy,circle.dx,circle.dy);if(edge!=port3ds::CursorKind::arrow)cursor=edge;}
            if(cursor!=shown_cursor){shown_cursor=cursor;cursor_since=now;}
            if(!graphics.cursor(engine.cursor_image(cursor,now-cursor_since),cx,cy,engine)){
                C2D_DrawLine(cx-6,cy,green,cx+6,cy,green,2,0);C2D_DrawLine(cx,cy-6,green,cx,cy+6,green,2,0);
            }
            C2D_TargetClear(bottom, bg); C2D_SceneBegin(bottom);
            hud_art.draw("hud",0,0);
            C2D_TextBufClear(hud_buf);
            char clock[20];std::snprintf(clock,sizeof clock,"%02lu:%02lu",static_cast<unsigned long>(state.frame/24/60),static_cast<unsigned long>(state.frame/24%60));
            port3ds::ui_text(hud_buf,clock,69,25,.34f,white,true);
            port3ds::ui_text(hud_buf,"Meniu",279,163,.38f,white,true);
            if(menu==Menu::none){option_page=port3ds::OptionPage::primary;option_count=engine.options(option_page,option_rows,8);}
            if(menu==Menu::none || menu==Menu::catalogue){
                using G=port3ds::CommandGrid;
                for(unsigned i=0;i<option_count;++i){
                    const float x=G::x+(i%G::columns)*G::cell_width,y=G::y+(i/G::columns)*G::cell_height;
                    graphics.ui_image(engine.command_icon(option_rows[i].icon,option_rows[i].enabled),x+1,y+1,.82f,engine);
                    if(menu==Menu::catalogue && i==choice)port3ds::ui_border({int(x),int(y),32,32},green);
                }
                port3ds::ui_text(hud_buf,"...",262,125,.48f,green,true);
                if(menu==Menu::catalogue && choice<option_count)port3ds::ui_text(hud_buf,option_rows[choice].label,220,32,.28f,white,true);
                else if(state.selected)port3ds::ui_text(hud_buf,state.selected_name,220,32,.32f,white,true);
            }
            unsigned selected_slot=0;
            for(std::size_t i=0;i<count && selected_slot<12;++i)if(units[i].selected){const auto& u=units[i];
                const int x=15+int(selected_slot%6)*34,y=166+int(selected_slot/6)*34;
                graphics.ui_image(engine.command_icon(u.type),x+1,y+1,.82f,engine);
                if(u.max_hp>0)C2D_DrawRectSolid(x,y+29,0,32.f*u.hp/u.max_hp,2,green);
                if(selected_slot==0)graphics.ui_image(engine.command_icon(u.type),255,193,1.1f,engine);
                ++selected_slot;
            }
            if(selected_slot){char hp[40];std::snprintf(hp,sizeof hp,"%d/%d",state.selected_hp,state.selected_max_hp);port3ds::ui_text(hud_buf,hp,275,230,.25f,green,true);}
            using C = port3ds::Controls;
            C2D_DrawRectSolid(C::minimap_x, C::minimap_y, 0, C::minimap_w, C::minimap_h, grid);
            graphics.minimap(C::minimap_x,C::minimap_y,C::minimap_w,C::minimap_h);
            const float sx = C::minimap_w / controls.map_width(), sy = C::minimap_h / controls.map_height();
            for (std::size_t i = 0; i != count; ++i) {
                const u32 color=units[i].selected?green:units[i].owner==0?
                    (state.observer?C2D_Color32(240,110,100,255):green):units[i].owner==1?
                    C2D_Color32(100,150,255,255):C2D_Color32(155,170,180,255);
                C2D_DrawRectSolid(C::minimap_x + units[i].x * sx, C::minimap_y + units[i].y * sy, 0, 2, 2,color);
            }
            port3ds::UnitView memories[64];const auto remembered=engine.remembered_units(memories,64);
            for(std::size_t i=0;i<remembered;++i)C2D_DrawRectSolid(C::minimap_x+memories[i].x*sx,C::minimap_y+memories[i].y*sy,0,2,2,memories[i].owner==1?C2D_Color32(100,150,255,255):C2D_Color32(155,170,180,255));
            graphics.fog_minimap(engine,C::minimap_x,C::minimap_y,C::minimap_w,C::minimap_h);
            const float mx = C::minimap_x + controls.camera_x * sx, my = C::minimap_y + controls.camera_y * sy;
            C2D_DrawLine(mx, my, green, mx + 400 * sx, my, green, 1, 0);
            C2D_DrawLine(mx, my, green, mx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx + 400 * sx, my, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx, my + 240 * sy, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            if(menu!=Menu::none && menu!=Menu::catalogue){
                C2D_DrawRectSolid(161,40,0,151,12+menu_count()*17,C2D_Color32(4,12,16,245));
                for(unsigned i=0;i<menu_count();++i){if(i==choice)C2D_DrawRectSolid(164,46+i*17,0,142,16,C2D_Color32(25,65,55,255));C2D_DrawText(&menu_rows[i],C2D_WithColor,167,47+i*17,0,.32f,.32f,white);}
            }
            const u64 end_begin=now_us();
            C3D_FrameEnd(0);
            const u64 render_end=now_us();
            end_sum+=render_end-end_begin;wait_sum+=waited;render_sum+=render_end-render_begin-waited;
            ++renders;
            if(warming) {
                // The first present can include GPU uploads and SD cache work.
                // Start the simulation clock only after that loading frame.
                C3D_FrameSync();
                warming=false;scheduler.reset();previous=interval=now_us();
                renders=ticks=0;tick_sum=tick_max=0;
                render_sum=wait_sum=collect_sum=sprite_sum=end_sum=gpu_sum=0;
            }
        }
        log.close();
        if(hud_buf)C2D_TextBufDelete(hud_buf);
    }
    if (R_SUCCEEDED(audio)) ndspExit();
    aptUnhook(&cookie);
    C2D_TextBufDelete(text_buf);
    C2D_Fini(); C3D_Fini(); gfxExit();
    return 0;
}
