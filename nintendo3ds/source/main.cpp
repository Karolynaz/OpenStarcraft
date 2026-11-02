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
#include "pause_menu.h"
#include "result_draw.h"
#include "save_slots.h"
#include "selection_draw.h"
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
constexpr const char* userdata = "sdmc:/3ds/openbw/";
const char* root = userdata;
constexpr const char* session_path="sdmc:/3ds/openbw/saves/session.ob3";
constexpr const char* restore_map="sdmc:/3ds/openbw/saves/restored-map";
bool find_save(unsigned slot,std::string& path,port3ds::SaveInfo& info,char* error,size_t size){
 path=port3ds::SaveSlots::path(userdata,slot);if(port3ds::Engine::inspect_save(path.c_str(),info,error,size))return true;
 char first[192];std::snprintf(first,sizeof first,"%s",error);
 path+=".bak";if(port3ds::Engine::inspect_save(path.c_str(),info,error,size))return true;std::snprintf(error,size,"%s",first);path.clear();return false;
}

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
    static const char* names[] = {"Empty map", "Two marines", "Manual base", "AI vs AI", "Player vs AI", "Gas AI vs AI", "All Terran battle", "Terran laboratory", "All Protoss battle", "Protoss laboratory", "All Zerg battle", "Zerg laboratory", "Protoss AI vs AI", "Protoss player vs AI", "Zerg AI vs AI", "Zerg player vs AI", "Terran 1: Wasteland", "Zerg 1: Among the Ruins", "Protoss 1: First Strike", "BW Protoss 1: Escape from Aiur", "BW Terran 1: First Strike", "BW Zerg 1: Vile Disruption"};
    return s<sizeof(names)/sizeof(names[0])?names[s]:"Unknown scenario";
}
}

int main() {
    // CIA/CCI bundle assets are read directly; saves/profiles/cache stay on SD.
    struct Bundle {
        bool mounted=false;
        Bundle(){if(R_SUCCEEDED(romfsInit())){if(auto* file=std::fopen("romfs:/openbw/StarDat.mpq","rb")){std::fclose(file);mounted=true;root="romfs:/openbw/";}else romfsExit();}}
        ~Bundle(){if(mounted)romfsExit();}
    } bundle;
    mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/openbw",0777);
    mkdir("sdmc:/3ds/openbw/campaign",0777);mkdir("sdmc:/3ds/openbw/saves",0777);
    const std::string map=std::string(root)+"maps/probe.scx",arena=std::string(root)+"maps/arena.chk";
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
    port3ds::Frontend front;port3ds::BriefingArt presentation_art;port3ds::SaveSlots save_slots;
    auto scan_slots=[&](){save_slots.scan(userdata,port3ds::Engine::inspect_save);};
    auto loading_screen=[&](){
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(text_buf);
        C2D_TargetClear(top,C2D_Color32(0,0,0,255));C2D_TargetClear(bottom,C2D_Color32(0,0,0,255));
        if(front.briefing_loading){C2D_SceneBegin(top);port3ds::draw_campaign_top(front,presentation_art,text_buf,true);}
        C2D_SceneBegin(bottom);
        const char* dots[]={"Loading.","Loading..","Loading..."};port3ds::ui_centered(text_buf,dots[(now_us()/375000)%3],{0,0,320,240},.65f,C2D_Color32(80,255,80,255));C3D_FrameEnd(0);
    };
    aptHookCookie cookie;
    aptHook(&cookie, lifecycle, nullptr);
    const bool self_test = port3ds::core_self_test();
    const Result audio = ndspInit();
    if (R_SUCCEEDED(audio)) ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    front.init(root,userdata);
    while(!front.quit){
    int launch_scenario=-1;port3ds::MatchSetup match_setup;
    unsigned continue_campaign=~0u;
    std::string launch_map,restore_path;bool return_custom=false,pending_restore=false;port3ds::SaveInfo restore_info;
    front.play=front.load=false;

    {
        port3ds::UiArt art;port3ds::MenuAnimation animation;const bool art_ok=art.load((std::string(root)+"ui/menu.bin").c_str()) && animation.load((std::string(root)+"ui/animations.bin").c_str());
        port3ds::Audio briefing_audio;unsigned spoken_mission=~0u,spoken_page=~0u,spoken_voice=~0u,spoken_serial=~0u,spoken_group=~0u;int touch_y=-1;std::string animation_path=std::string(root)+"ui/animations.bin";
        C2D_Prepare();u64 last=now_us();
        while(aptMainLoop()){
            hidScanInput();const u32 down=hidKeysDown();circlePosition stick{};hidCircleRead(&stick);
            const auto now=now_us();const float menu_dt=std::min<u64>(now-last,50000)/1000000.f;front.stick(stick.dx,stick.dy,menu_dt);last=now;
            if(front.dialog==port3ds::Frontend::Dialog::story && (hidKeysHeld()&KEY_TOUCH)){touchPosition t{};hidTouchRead(&t);if(touch_y>=0)front.scroll_story(touch_y-int(t.py));touch_y=t.py;}else touch_y=-1;
            if(down&KEY_TOUCH){touchPosition t{};hidTouchRead(&t);front.x=t.px;front.y=t.py;front.hover=front.hit(t.px,t.py);front.activate(front.hover);}
            if(down&KEY_A)front.confirm();
            if((down&KEY_START) && front.rich_briefing && front.dialog==port3ds::Frontend::Dialog::briefing)front.activate(116);
            if(down&KEY_Y)front.replay_briefing();
            if(down&KEY_B)front.back();
            if(front.dialog==port3ds::Frontend::Dialog::briefing && !front.briefing_paginated){
                if(auto measure=C2D_TextBufNew(160)){front.paginate_briefing([&](const char* line){return port3ds::ui_width(measure,line,port3ds::Frontend::briefing_font);},port3ds::Frontend::briefing_width,port3ds::Frontend::briefing_rows);C2D_TextBufDelete(measure);}
            }
            if(front.dialog==port3ds::Frontend::Dialog::story && front.story_lines.empty()){
                if(auto measure=C2D_TextBufNew(160)){front.layout_story([&](const char* line){return port3ds::ui_width(measure,line,port3ds::Frontend::story_font);},port3ds::Frontend::story_width);C2D_TextBufDelete(measure);}
            }
            const std::string wanted_animation=front.rich_briefing && front.presentation()?(front.dialog==port3ds::Frontend::Dialog::story?"":std::string(root)+"campaign/"+port3ds::campaign::missions[front.mission].key+"-animations.bin"):std::string(root)+"ui/animations.bin";
            if(animation_path!=wanted_animation){animation.clear();animation_path=wanted_animation;if(!animation_path.empty() && !animation.load(animation_path.c_str())){std::snprintf(front.error,sizeof front.error,"Install the updated briefing animation package.");front.dialog=port3ds::Frontend::Dialog::error;front.play=false;}}
            if(!presentation_art.sync(front,root)){std::snprintf(front.error,sizeof front.error,"Install the updated private campaign presentation assets.");front.dialog=port3ds::Frontend::Dialog::error;front.play=false;front.briefing_loading=false;}
            // Briefing audio is owned by this menu scope, and fully released
            // before the world/terrain/audio allocations of a mission begin.
            if(front.dialog==port3ds::Frontend::Dialog::briefing && front.briefing_ready() && !front.briefing_loading){
                if(spoken_mission!=front.mission || spoken_serial!=front.briefing_serial){if(spoken_mission!=front.mission)briefing_audio.unload();spoken_mission=front.mission;spoken_serial=front.briefing_serial;spoken_page=spoken_voice=spoken_group=~0u;briefing_audio.reset();}
                const auto voice=front.briefing_sound();
                const auto group=front.briefing_page<front.briefing_groups.size()?front.briefing_groups[front.briefing_page]:front.briefing_page;
                if(voice && !briefing_audio.stats().ready && spoken_page!=front.briefing_page){
                    const auto& mission=port3ds::campaign::missions[front.mission];
                    const auto bank=std::string(root)+"campaign/"+mission.key+"-effects.bin";
                    const char* prefix=mission.race==0?"zerg":mission.race==2?"protoss":"terran";
                    const auto audio_root=std::string(root)+"audio"+(mission.race==0?"/zerg":mission.race==2?"/protoss":"");
                    briefing_audio.load(audio_root.c_str(),R_SUCCEEDED(audio),prefix,bank.c_str());last=now_us();
                    load_stage(mission.scenario,briefing_audio.stats().ready?"briefing-audio-ready":"briefing-audio-off",briefing_audio.error());
                }
                if(spoken_page!=front.briefing_page && (spoken_voice!=voice || spoken_group!=group)){briefing_audio.reset();port3ds::SoundEvent event{voice,0,0};briefing_audio.update(&event,voice?1:0,0,0,false);spoken_voice=voice;}
                else briefing_audio.update(nullptr,0,0,0,false);
                spoken_page=front.briefing_page;spoken_group=group;
            }else if(spoken_mission!=~0u){const auto stats=briefing_audio.stats();char detail[96];std::snprintf(detail,sizeof detail,"played=%u missing=%u IO=%u",stats.played,stats.missing,stats.io_errors);load_stage(port3ds::campaign::missions[spoken_mission].scenario,"briefing-audio-done",detail);briefing_audio.unload();spoken_mission=spoken_page=spoken_voice=~0u;}
            front.briefing_speaking=briefing_audio.playing(front.briefing_sound());
            const auto speech=briefing_audio.stats();const bool fallback=!front.briefing_sound() || !speech.ready || speech.missing || speech.io_errors;
            if(fallback)front.briefing_started=true;front.update_presentation(menu_dt,front.briefing_speaking);
            if(front.quit)break;
            if(front.screen==port3ds::Frontend::Screen::custom && front.preview_dirty){front.preview_dirty=false;port3ds::Engine::preview_map(root,front.map_path().c_str(),front.preview);if(front.preview.valid)art.preview(front.preview.pixels.data());last=now_us();}
            if(down&KEY_DUP)front.direction(0);if(down&KEY_DRIGHT)front.direction(1);if(down&KEY_DDOWN)front.direction(2);if(down&KEY_DLEFT)front.direction(3);
            if(front.load){front.load=false;scan_slots();for(unsigned i=0;i<5;++i){front.load_labels[i]=save_slots.slots[i].label;front.load_short_labels[i]=save_slots.slots[i].short_label;front.load_enabled[i]=save_slots.slots[i].available;}front.dialog=port3ds::Frontend::Dialog::load_slots;front.x=160;front.y=72;front.hover=120;}
            if(front.load_requested_slot<5){const auto slot=front.load_requested_slot;front.load_requested_slot=~0u;if(find_save(slot,restore_path,restore_info,front.error,sizeof front.error)){launch_scenario=int(restore_info.scenario);match_setup=restore_info.setup;pending_restore=true;front.dialog=port3ds::Frontend::Dialog::none;break;}front.dialog=port3ds::Frontend::Dialog::error;}
            if(front.play){if(front.briefing_loading)presentation_art.release_room();launch_scenario=int(front.scenario());match_setup=front.match(unsigned(now));launch_map=front.map_path();front.observer_test=~0u;break;}
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(text_buf);
            C2D_TargetClear(top,C2D_Color32(0,0,0,255));C2D_SceneBegin(top);C3D_DepthTest(false,GPU_ALWAYS,GPU_WRITE_COLOR);
            if(front.rich_briefing && front.presentation())port3ds::draw_campaign_top(front,presentation_art,text_buf);else port3ds::draw_frontend_top(front,art,text_buf);
            C2D_TargetClear(bottom,C2D_Color32(0,0,0,255));C2D_SceneBegin(bottom);
            if(art_ok){if(front.rich_briefing && front.presentation())port3ds::draw_campaign_bottom(front,presentation_art,art,text_buf,animation,unsigned(now/125000));else port3ds::draw_frontend(front,art,animation,text_buf,unsigned(now/125000));}
            else {port3ds::ui_text(text_buf,"Missing menu art / animations",160,90,.55f,C2D_Color32(255,200,180,255),true);port3ds::ui_text(text_buf,"Install the complete SD package.",160,120,.40f,C2D_Color32(255,255,255,255),true);if(down&KEY_START)break;}
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
        port3ds::ResultArt result_art;
        port3ds::UiArt hud_art;hud_art.load((std::string(root)+"ui/hud.bin").c_str());
        auto hud_buf=C2D_TextBufNew(4096);
        port3ds::InputRouter input;port3ds::UnitDoubleClick double_click;
        using Menu=port3ds::PauseMenu::Page;
        auto overlay=[&](Menu p){return port3ds::PauseMenu::overlay(p);};
        float menu_x=160,menu_y=60;
        const char* notice_title="Save Game";char notice_message[192]="";
        port3ds::OptionPage option_page=port3ds::OptionPage::primary;
        port3ds::MenuOption option_rows[9]{};unsigned option_count=0;
        Menu menu=Menu::none; unsigned choice=0;
        bool result_shown=false;
        unsigned mission_camera_serial=0;
        bool initial_load=true, auto_camera=true, attacking=false, warming=false;
        u64 camera_manual_until=0;
        bool paused = true;
        bool pending_load = false;
        bool placing = false;
        unsigned ability_page=0,catalogue_page=0;
        bool drag_pending=false,drag_box=false;int drag_x=0,drag_y=0;
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
        C2D_Text diagnostics{}, commands{}, message{}, option_hint{};
        C2D_Text menu_rows[9]{},resource_values[3]{};
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
                loading_screen();double_click.reset();
                result_art.clear();
                placing = false; attacking=false;drag_pending=drag_box=false;ability_page=catalogue_page=0;menu=Menu::none;result_shown=false;
                // Release resident PCM and sprite scratch before the core's
                // temporary MPQ/GRP initialization peak on original hardware.
                load_stage(scenario,"release-start");
                sound.unload();graphics.clear();
                load_stage(scenario,"core-start");
                const auto mode=static_cast<port3ds::Scenario>(scenario);
                const bool arena_mode=port3ds::arena_scenario(mode);
                const unsigned next_race=port3ds::scenario_race(mode);
                loaded_scenario = scenario;
                if(scenario==unsigned(launch_scenario)){char setup_info[80];std::snprintf(setup_info,sizeof setup_info,"human=%d ai1=%d ai2=%d",match_setup.races[0],match_setup.races[1],match_setup.races[2]);load_stage(scenario,"match-setup",setup_info);}
                const bool campaign_mode=port3ds::campaign::scenario(scenario);
                const bool custom_match=scenario==unsigned(launch_scenario) && match_setup.enabled;
                const bool restoring=pending_restore;port3ds::SaveView restored_view;
                auto progress=[&](unsigned done,unsigned total){
                    if(!aptMainLoop())return false;hidScanInput();if(hidKeysDown()&KEY_B)return false;
                    loading_screen();return true;
                };
                mkdir("sdmc:/3ds/openbw/saves",0777);
                u64 last_load_draw=now_us();auto animate_loading=[&](){const auto t=now_us();if(t-last_load_draw>=375000){loading_screen();last_load_draw=t;}};
                const bool loaded_ok=restoring?engine.load_save(root,restore_path.c_str(),session_path,restore_map,restored_view,progress):engine.load(root, (custom_match || campaign_mode)?launch_map.c_str():arena_mode?arena.c_str():map.c_str(), static_cast<port3ds::Scenario>(scenario),scenario==unsigned(launch_scenario)?&match_setup:nullptr,animate_loading);
                paused=!loaded_ok;
                pending_restore=false;
                if(!restoring && engine.ready())engine.start_recording(session_path);
                if(restoring && !loaded_ok){std::snprintf(front.error,sizeof front.error,"%s",engine.error());front.dialog=port3ds::Frontend::Dialog::error;return_custom=true;break;}
                if(restoring)launch_map=std::string(restore_map)+(restore_info.raw_map?".chk":".scx");
                next_text=0;input=port3ds::InputRouter{};
                load_stage(scenario,engine.ready()?"core-ready":"core-failed",engine.ready()?"":engine.error());
                // Reserve the contiguous sprite scratch before segmented PCM
                // fills smaller holes left by the native core initialization.
                if(custom_match && !engine.ready()){std::snprintf(front.error,sizeof front.error,"%s",engine.error());front.dialog=port3ds::Frontend::Dialog::error;return_custom=true;break;}
                // Release before atlas allocation as well: otherwise the early
                // photo splits the contiguous block later required by ndsp.
                // The already presented objectives remain on screen during IO.
                const bool loading_photo=front.briefing_loading;
                if(loading_photo)presentation_art.clear();
                bool terrain_ok=true;
                if(custom_match || campaign_mode || restoring){load_stage(scenario,"terrain-cache-start",launch_map.c_str());terrain_ok=engine.write_terrain(root,"sdmc:/3ds/openbw/map-terrain.bin");if(terrain_ok)graphics.load("sdmc:/3ds/openbw/map-terrain.bin",engine.terrain_signature());}
                else if (engine.ready()) graphics.load((std::string(root)+(arena_mode?"graphics-arena.bin":"graphics.bin")).c_str(), engine.terrain_signature());
                else graphics.clear();
                load_stage(scenario,graphics.ready()?"graphics-ready":"graphics-off",graphics.ready()?"":graphics.error());
                if((custom_match || campaign_mode) && (!terrain_ok || !graphics.ready())){std::snprintf(front.error,sizeof front.error,"%s",terrain_ok?graphics.error():engine.error());front.dialog=port3ds::Frontend::Dialog::error;return_custom=true;break;}
                std::string effect_path=std::string(root)+"audio/effects-all.bin";if(campaign_mode)effect_path=(std::string(root)+"campaign/")+port3ds::campaign::missions[port3ds::campaign::index(scenario)].key+"-effects.bin";
                if(engine.ready())sound.load((std::string(root)+(next_race==0?"audio/zerg":next_race==2?"audio/protoss":"audio")).c_str(),R_SUCCEEDED(audio),next_race==0?"zerg":next_race==2?"protoss":"terran",effect_path.c_str(),engine.snapshot().width*engine.snapshot().height>128*128?2*1024*1024:4*1024*1024);
                if(loading_photo && !presentation_art.load_shot(front,root))front.briefing_loading=false;
                load_stage(scenario,sound.stats().ready?"audio-ready":"audio-off",sound.error());
                // Atlas descriptors keep their addresses across reloads, while
                // their GPU buffers can move. Invalidate Citro2D's texture cache.
                C2D_Prepare();
                scheduler.reset();warming=true;
                controls = port3ds::Controls{};
                const auto state = engine.snapshot();
                mission_camera_serial=0;
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
                if(restoring){controls.camera_x=restored_view.camera_x;controls.camera_y=restored_view.camera_y;controls.cursor_x=restored_view.cursor_x;controls.cursor_y=restored_view.cursor_y;auto_camera=restored_view.auto_camera;mission_camera_serial=state.mission_camera_serial;}
                camera_manual_until=0;
                if(!log) toggle_log();
                action = engine.ready() ? (auto_camera ? "AI vs AI: automatic camera / START menu" : "Circle Pad cursor / A action / B cancel") : engine.error();
            }
            circlePosition circle{}; hidCircleRead(&circle);
            const float ui_seconds=std::min<u64>(elapsed,50000)/1000000.0f;
            if(overlay(menu)) {
                auto axis=[](int v){return std::abs(v)<=20?0.f:std::copysign(std::min(1.f,(std::abs(v)-20)/136.f),float(v));};
                menu_x=std::max(0.f,std::min(319.f,menu_x+axis(circle.dx)*190*ui_seconds));
                menu_y=std::max(0.f,std::min(239.f,menu_y-axis(circle.dy)*190*ui_seconds));
                if(std::abs(circle.dx)>20 || std::abs(circle.dy)>20){const int hit=(menu==Menu::result?port3ds::ResultScreen::hit(int(menu_x),int(menu_y)):port3ds::PauseMenu::hit(int(menu_x),int(menu_y),port3ds::PauseMenu::count(menu)));if(hit>=0)choice=unsigned(hit);}
            }else controls.stick(circle.dx,circle.dy,ui_seconds);
            if(std::abs(circle.dx)>20 || std::abs(circle.dy)>20) camera_manual_until=now+8000000;
            auto command=[&](port3ds::Command c) {
                engine.command(c,controls.world_x(),controls.world_y()); action=engine.command_status();
            };
            auto menu_count=[&]() -> unsigned {return menu==Menu::catalogue?option_count:port3ds::PauseMenu::count(menu);};
            auto refresh_catalogue=[&](){
                port3ds::MenuOption all[64];unsigned total=engine.options(option_page,all,64);unsigned per_page=total>9?8:9,pages=std::max(1u,(total+per_page-1)/per_page);catalogue_page%=pages;
                option_count=0;for(unsigned i=catalogue_page*per_page;i<total && option_count<per_page;++i)option_rows[option_count++]=all[i];
                if(pages>1){auto& next=option_rows[8];next={};next.code=0xffff0004;next.enabled=true;next.icon=229;std::snprintf(next.label,sizeof next.label,"Next page");option_count=9;}
            };
            auto open_options=[&](port3ds::OptionPage page){option_page=page;catalogue_page=0;refresh_catalogue();choice=0;menu=option_count?Menu::catalogue:Menu::none;};
            auto refresh_world=[&](){
                for(auto& row:option_rows)row={};option_count=9;const auto selected=engine.snapshot();engine.movement_options(option_rows);
                if(!selected.selected)return;
                if(selected.workers_only || selected.selected_building || selected.selected_larva){
                    option_rows[4].code=0xffff0001;option_rows[4].enabled=true;option_rows[4].icon=selected.workers_only?228:229;
                    option_rows[5].code=0xffff0002;option_rows[5].enabled=true;option_rows[5].icon=230;
                    std::snprintf(option_rows[4].label,64,"%s",selected.workers_only?"Build":"Production / actions");
                    std::snprintf(option_rows[5].label,64,"%s",selected.workers_only?"Advanced build":"Other actions");
                }else{
                    port3ds::MenuOption abilities[64];unsigned total=engine.abilities(abilities,64),per_page=total>5?4:5,pages=std::max(1u,(total+per_page-1)/per_page);ability_page%=pages;
                    for(unsigned i=0;i<per_page && ability_page*per_page+i<total;++i)option_rows[4+i]=abilities[ability_page*per_page+i];
                    if(pages>1){option_rows[8].code=0xffff0003;option_rows[8].enabled=true;option_rows[8].icon=229;std::snprintf(option_rows[8].label,64,"More abilities");}
                }
            };
            if(menu==Menu::catalogue)refresh_catalogue();
            bool exit_requested=false;
            auto use_option=[&](const port3ds::MenuOption& option) {
                if(option.code==0xffff0001 || option.code==0xffff0002){const bool worker=engine.snapshot().workers_only;open_options(worker?(option.code==0xffff0001?port3ds::OptionPage::basic:port3ds::OptionPage::advanced):(option.code==0xffff0001?port3ds::OptionPage::primary:port3ds::OptionPage::secondary));return;}
                if(option.code==0xffff0003){++ability_page;refresh_world();return;}
                if(option.code==0xffff0004){++catalogue_page;refresh_catalogue();return;}
                if(!option.enabled){action="Unavailable: research, energy, resources or queue";return;}
                const auto kind=static_cast<port3ds::OptionKind>(option.code>>16);
                if(kind==port3ds::OptionKind::build) {placing=true;attacking=false;placing_code=option.code;action="A place building / B cancel";}
                else if(option.target) {attacking=true;placing=false;target_code=option.code;std::snprintf(target_help,sizeof target_help,"%.64s: A target / B cancel",option.label);action=target_help;}
                else {engine.perform(option.code,controls.world_x(),controls.world_y());action=engine.command_status();}
                menu=Menu::none;
            };
            auto activate_menu=[&]() {
                if(menu==Menu::result){const auto ended=engine.snapshot();continue_campaign=port3ds::ResultScreen::next_mission(scenario,ended.campaign && ended.winner==0,ended.next_scenario);return_custom=true;exit_requested=true;return;}
                if(menu==Menu::system) {
                    if(choice<2){scan_slots();menu=choice==0?Menu::save_slots:Menu::load_slots;choice=0;return;}
                    if(choice==2){menu=Menu::end_confirm;choice=1;return;}
                    if(choice==3){menu=Menu::options;choice=0;return;}
                    menu=Menu::none;return;
                } else if(menu==Menu::save_slots){
                    const auto slot=choice;port3ds::SaveView view;view.camera_x=controls.camera_x;view.camera_y=controls.camera_y;view.cursor_x=controls.cursor_x;view.cursor_y=controls.cursor_y;view.auto_camera=auto_camera;
                    notice_title="Save Game";if(engine.save_game(port3ds::SaveSlots::path(userdata,slot).c_str(),view,osGetTime()/1000-2208988800ULL))std::snprintf(notice_message,sizeof notice_message,"Game saved to slot %u.",slot+1);else std::snprintf(notice_message,sizeof notice_message,"%s",engine.error());menu=Menu::notice;choice=0;return;
                } else if(menu==Menu::load_slots){
                    if(!save_slots.slots[choice].available)return;
                    if(find_save(choice,restore_path,restore_info,notice_message,sizeof notice_message)){scenario=restore_info.scenario;launch_scenario=int(scenario);match_setup=restore_info.setup;pending_restore=pending_load=true;menu=Menu::none;}else {notice_title="Load Game";menu=Menu::notice;choice=0;}return;
                } else if(menu==Menu::notice){menu=Menu::system;choice=0;return;}
                else if(menu==Menu::end_confirm){
                    if(choice==0){return_custom=true;exit_requested=true;}else {menu=Menu::system;choice=2;}return;
                } else if(menu==Menu::options) {
                    if(choice==0){menu=Menu::scenarios;choice=0;return;}
                    if(choice==1){auto_camera=!auto_camera;action=auto_camera?"Automatic camera on":"Automatic camera off";}
                    if(choice==2)toggle_log();
                    if(choice==3){pending_load=true;action="Loading scenario and graphics...";menu=Menu::none;return;}
                    if(choice==4){engine.set_fog_enabled(!engine.fog_enabled());action=engine.fog_enabled()?"Fog of war: player vision":"Fog of war: full-map observer";}
                    if(choice==5)exit_requested=true;
                    if(choice==6){menu=Menu::system;choice=3;}return;
                } else if(menu==Menu::scenarios) {
                    if(choice==7){menu=Menu::legacy;choice=0;return;}
                    static const unsigned modes[]={10,11,8,9,6,7,4};scenario=modes[choice];pending_load=true;action="Loading scenario and graphics...";
                } else if(menu==Menu::legacy){static const unsigned modes[]={14,15,12,13,5,3,1,2};scenario=modes[choice];pending_load=true;action="Loading skirmish...";} else if(menu==Menu::catalogue) {refresh_catalogue();if(choice<option_count)use_option(option_rows[choice]);else {menu=Menu::none;action="Selection has no available commands";}return;}
                menu=Menu::none;
            };
            auto click_unit=[&](bool select_only){const int x=controls.world_x(),y=controls.world_y();
                if(double_click.tap(engine.own_unit_at(x,y),now))engine.select_similar(x,y,int(controls.camera_x),int(controls.camera_y));
                else if(select_only)engine.select(x,y);else engine.activate(x,y);
                action=engine.command_status();
            };
            port3ds::InputFrame frame;
            frame.a=down&KEY_A; frame.b=down&KEY_B; frame.x=down&KEY_X; frame.y=down&KEY_Y;
            if(overlay(menu)){frame.x=frame.y=false;}
            frame.start=down&KEY_START; frame.select=down&KEY_SELECT;if(result_shown)frame.select=false; frame.l=held&KEY_L; frame.r=held&KEY_R;frame.l_pressed=down&KEY_L;frame.r_pressed=down&KEY_R; if(overlay(menu))frame.r=frame.l=false; frame.time=now;
            if(down&KEY_DUP) frame.direction=0; else if(down&KEY_DRIGHT) frame.direction=1;
            else if(down&KEY_DDOWN) frame.direction=2; else if(down&KEY_DLEFT) frame.direction=3;
                        if(menu==Menu::none && !placing && !attacking && !frame.l && !frame.r && !engine.snapshot().selected && (down&KEY_A)){drag_pending=true;drag_box=false;drag_x=controls.world_x();drag_y=controls.world_y();frame.a=false;}
            if(drag_pending){frame.a=false;if(std::abs(controls.world_x()-drag_x)+std::abs(controls.world_y()-drag_y)>7)drag_box=true;if(hidKeysUp()&KEY_A){if(drag_box)engine.select_area(std::min(drag_x,controls.world_x()),std::min(drag_y,controls.world_y()),std::abs(drag_x-controls.world_x()),std::abs(drag_y-controls.world_y()));else click_unit(true);drag_pending=drag_box=false;}}
            const auto event=input.read(frame,menu!=Menu::none,menu==Menu::catalogue);
            using I=port3ds::InputAction;
            if(event.action!=I::none && event.action!=I::confirm){drag_pending=drag_box=false;double_click.reset();}
            const bool was_overlay=overlay(menu);
            switch(event.action) {
            case I::system:
                if(result_shown){menu=Menu::result;choice=0;break;}
                menu=overlay(menu)?Menu::none:Menu::system;choice=0;menu_x=160;menu_y=60;break;
            case I::cancel:
                if(result_shown){return_custom=true;exit_requested=true;break;}
                if(overlay(menu)){menu=port3ds::PauseMenu::back(menu);choice=0;break;}
                if(menu!=Menu::none || placing || attacking) { menu=Menu::none;placing=false;attacking=false;action="Action cancelled"; }
                else { engine.clear_selection();action="Selection cleared"; }
                break;
            case I::army: placing=false;attacking=false;engine.select_army(controls.camera_x,controls.camera_y);action="Military units on screen selected";menu=Menu::none;break;
            case I::workers: placing=false;attacking=false;engine.select_workers(int(controls.camera_x),int(controls.camera_y));action="Workers on screen selected";menu=Menu::none;break;
            case I::all:engine.select_area(int(controls.camera_x),int(controls.camera_y),400,240);menu=Menu::none;placing=attacking=false;break;
            case I::patrol:case I::stop:case I::attack:{port3ds::MenuOption movement[4];engine.movement_options(movement);use_option(movement[event.action==I::patrol?1:event.action==I::stop?2:3]);break;}
            case I::hotkey:if(menu==Menu::catalogue && event.slot<option_count)use_option(option_rows[event.slot]);break;
            case I::power:placing=attacking=false;refresh_world();if(event.slot<5 && option_rows[4+event.slot].label[0])use_option(option_rows[4+event.slot]);break;
            case I::confirm:
                if(menu!=Menu::none)activate_menu();
                else if(placing){if(engine.perform(placing_code,controls.world_x(),controls.world_y()))placing=false;action=engine.command_status();}
                else if(attacking){if(engine.perform(target_code,controls.world_x(),controls.world_y()))attacking=false;action=engine.command_status();}
                else click_unit(false);
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
                if(overlay(menu)){const int hit=(menu==Menu::result?port3ds::ResultScreen::hit(t.px,t.py):port3ds::PauseMenu::hit(t.px,t.py,menu_count()));if(hit>=0){choice=unsigned(hit);activate_menu();}}
                else if(port3ds::UiRect{255,156,55,22}.hit(t.px,t.py)){menu=Menu::system;choice=0;menu_x=160;menu_y=60;}
                else if(menu==Menu::none || menu==Menu::catalogue){
                    if(menu==Menu::none)refresh_world();
                    const int hit=port3ds::CommandGrid::hit(t.px,t.py,9);
                    if(hit>=0 && hit<int(option_count)){choice=unsigned(hit);use_option(option_rows[choice]);}
                    else {
                        const auto info=engine.selection_info();const int production=port3ds::ProductionPanel::hit(t.px,t.py,info);
                        if(production>=0){engine.cancel_selected_work(unsigned(production));action=engine.command_status();}
                        else if(info.count>1){
                        unsigned slot=0;const auto selectable=engine.visible_units(units,unit_view_capacity);
                        for(std::size_t i=0;i<selectable && slot<12;++i)if(units[i].selected){
                            const auto rect=port3ds::UiRect{13+int(slot%6)*37,157+int(slot/6)*37,35,35};
                            if(rect.hit(t.px,t.py)){engine.select(units[i].x,units[i].y);break;}++slot;
                        }
                        }
                    }
                }
            }
            if((held&KEY_TOUCH) && !overlay(menu)){touchPosition touch{};hidTouchRead(&touch);const auto current=engine.snapshot();const auto maprect=controls.minimap_bounds();bool on_alert=current.alert_frame && current.frame-current.alert_frame<24*10 && std::abs(touch.px-(maprect.x+current.alert_x*maprect.w/controls.map_width()))<7 && std::abs(touch.py-(maprect.y+current.alert_y*maprect.h/controls.map_height()))<7;
                if(controls.minimap_touch(touch.px,touch.py)){if(on_alert)controls.center_camera(current.alert_x,current.alert_y);camera_manual_until=now+8000000;}}
            if(was_overlay!=overlay(menu)){paused=overlay(menu) || !engine.ready();scheduler.reset();previous=now_us();}
            // Menus own all simulation and camera input until dismissed.
            if(overlay(menu))paused=true;
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
            if(!paused && state.observer && auto_camera && now>=camera_manual_until) {
                int x=0,y=0; if(engine.observer_focus(x,y)) controls.follow(x,y,ui_seconds);
            }
            if(state.campaign && state.mission_camera_serial!=mission_camera_serial){controls.focus(state.mission_camera_x,state.mission_camera_y);mission_camera_serial=state.mission_camera_serial;}
            if(state.winner>=0 && !result_shown) {
                if(state.campaign && state.winner==0)front.complete_mission(scenario,state.next_scenario);
                result_shown=true;paused=true;scheduler.reset();menu=Menu::result;choice=0;action=state.observer?(state.winner==0?"Red wins":state.winner==1?"Blue wins":state.winner==2?"Green wins":"Battle finished"):(state.winner==0?"Victory!":state.winner==3?"Draw":"Defeat");
                // Result pages replace the world: free its linear textures and
                // PCM before allocating the two native-resolution result pages.
                sound.unload();graphics.clear();hud_art.clear();presentation_art.clear();
                const bool result_ok=result_art.load(root,state.human_race,state.winner==0);
                C2D_Prepare();
                load_stage(scenario,result_ok?"result-art-ready":"result-art-off");
                menu_x=160;menu_y=218;next_text=0;
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
                char status[900], help[800], error[220];
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
                else if(menu==Menu::none) std::snprintf(help,sizeof help,"A action / B cancel\nSELECT all / R+X army / R+Y workers\nX/Y powers / L+X/L+Y more\nL+D-pad: save group\nR+D-pad: recall group\nTwice: center camera\nSTART pause/menu\nSTART menu");
                else {
                    const char* results[]={"Ok"};
                    const char* system[]={"Save Game","Load Game","End Game","Options","Return to Game (B)"};
                    const char* options[]={"Scenarios",auto_camera?"Auto camera: ON":"Auto camera: OFF",log?"CSV: ON":"CSV: OFF","Reload current",engine.fog_enabled()?"Fog: ON (player)":"Fog: OFF (observer)","Exit application","Back (B)"};
                    const char* ending[]={"End Game","Cancel (B)"};
                    const char* notice[]={"OK (B)"};
                    const char* modes[]={"All Zerg battle","Zerg laboratory","All Protoss battle","Protoss laboratory","All Terran battle","Terran laboratory","Player vs AI","More scenarios..."};
                    const char* legacy[]={"Zerg AI vs AI","Zerg player vs AI","Protoss AI vs AI","Protoss player vs AI","Gas AI vs AI","Marines AI vs AI","Two Marines","Manual base"};
                    if(menu==Menu::catalogue){option_count=engine.options(option_page,option_rows,9);choice=std::min(choice,option_count?option_count-1:0u);}
                    unsigned used=0;
                    for(unsigned i=0;i<menu_count();++i)used+=std::snprintf(help+used,sizeof help-used,"%s\n",menu==Menu::save_slots || menu==Menu::load_slots?save_slots.slots[i].label.c_str():menu==Menu::result?results[i]:menu==Menu::system?system[i]:menu==Menu::options?options[i]:menu==Menu::notice?notice[i]:menu==Menu::end_confirm?ending[i]:menu==Menu::scenarios?modes[i]:menu==Menu::legacy?legacy[i]:option_rows[i].label);
                }

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
                char values[3][32];
                std::snprintf(values[0],sizeof values[0],"%d",state.minerals);
                std::snprintf(values[1],sizeof values[1],"%d",state.gas);
                std::snprintf(values[2],sizeof values[2],"%d/%d",state.supply,state.supply_max);
                for(unsigned i=0;i<3;++i){C2D_TextParse(&resource_values[i],text_buf,values[i]);C2D_TextOptimize(&resource_values[i]);}
                C2D_TextParse(&option_hint,text_buf,menu==Menu::catalogue && option_count?option_rows[choice].label:"");C2D_TextOptimize(&option_hint);
                next_text = sample_time + 250000;
            }
            const u64 render_begin=now_us();
            const auto count = engine.visible_units(units, unit_view_capacity);
            const u32 bg = C2D_Color32(8, 16, 24, 255), grid = C2D_Color32(25, 49, 57, 255);
            const u32 green = C2D_Color32(100, 240, 180, 255), white = C2D_Color32(215, 230, 235, 255);
            const u64 wait_begin=now_us();
            C3D_FrameBegin(frame_pacer.needs_sync(C3D_FrameCounter(0),C3D_FrameCounter(1))?C3D_FRAME_SYNCDRAW:0);C2D_TextBufClear(hud_buf);
            frame_pacer.began(C3D_FrameCounter(0),C3D_FrameCounter(1));
            const u64 waited=now_us()-wait_begin;
            gpu_sum+=C3D_GetDrawingTime()*1000;
            if(menu==Menu::result){
                C2D_TargetClear(top,bg);C2D_SceneBegin(top);
                port3ds::draw_result_top(result_art,state,action,hud_buf);
                C2D_TargetClear(bottom,bg);C2D_SceneBegin(bottom);
                port3ds::draw_result_bottom(result_art,hud_buf);
            }else{
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
            if(drag_pending && drag_box)port3ds::ui_border({int(std::min(drag_x,controls.world_x())-controls.camera_x),int(std::min(drag_y,controls.world_y())-controls.camera_y),std::abs(drag_x-controls.world_x()),std::abs(drag_y-controls.world_y())},green);
            const float cx = controls.cursor_x, cy = controls.cursor_y;
            // Fixed, separate resource groups, as in the original HUD.
            // Only unusually large values shrink; normal amounts stay readable.
            constexpr float resource_column=104,resource_gap=16,resource_icon=21;
            constexpr float resource_start=400-8-3*resource_column-2*resource_gap;
            for(unsigned i=0;i<3;++i){
                const float x=resource_start+i*(resource_column+resource_gap);
                float width,height;C2D_TextGetDimensions(&resource_values[i],.72f,.72f,&width,&height);
                const float size=width>78?.72f*78/width:.72f;
                graphics.ui_image(engine.resource_icon(i),x,3,resource_icon/14,engine);
                C2D_DrawText(&resource_values[i],C2D_WithColor,x+27,3,0,size,size,C2D_Color32(0,0,0,190));
                C2D_DrawText(&resource_values[i],C2D_WithColor,x+26,2,0,size,size,C2D_Color32(80,255,80,255));
            }
            if(state.campaign){
                if(!state.transmission.empty()){C2D_DrawRectSolid(3,145,0,394,92,C2D_Color32(3,8,15,235));port3ds::ui_limited(hud_buf,state.transmission.c_str(),9,149,.48f,382,6,white);}
            }
            if(placing || attacking || pending_load || !engine.ready())C2D_DrawText(&message,C2D_WithColor|C2D_WordWrap,8,204,0,.46f,.46f,white,384.f);
            auto cursor=engine.cursor_kind(controls.world_x(),controls.world_y(),attacking);
            if(menu!=Menu::none)cursor=port3ds::CursorKind::arrow;
            else if(placing){const auto site=engine.placement_unit(placing_code&65535,controls.world_x(),controls.world_y());cursor=site.valid?port3ds::CursorKind::target:port3ds::CursorKind::target_enemy;}
            else {const auto edge=port3ds::edge_cursor(cx,cy,circle.dx,circle.dy);if(edge!=port3ds::CursorKind::arrow)cursor=edge;}
            if(cursor!=shown_cursor){shown_cursor=cursor;cursor_since=now;}
            if(!overlay(menu) && !graphics.cursor(engine.cursor_image(cursor,now-cursor_since),cx,cy,engine)){
                C2D_DrawLine(cx-6,cy,green,cx+6,cy,green,2,0);C2D_DrawLine(cx,cy-6,green,cx,cy+6,green,2,0);
            }
            if(overlay(menu)){
                C2D_DrawRectSolid(0,0,0,400,240,C2D_Color32(0,0,0,state.campaign?255:155));
                if(state.campaign){port3ds::ui_text(hud_buf,"Mission objectives",200,26,.65f,white,true);port3ds::ui_limited(hud_buf,state.objectives.c_str(),24,63,.62f,352,9,white);}
            }
            C2D_TargetClear(bottom, bg); C2D_SceneBegin(bottom);
            hud_art.draw("hud",0,0);
            char clock[20];const unsigned seconds=state.countdown_active?state.countdown_seconds:state.frame/24;std::snprintf(clock,sizeof clock,"%s%02lu:%02lu",state.countdown_active?"T-":"",static_cast<unsigned long>(seconds/60),static_cast<unsigned long>(seconds%60));
            port3ds::ui_text(hud_buf,clock,31,16,.48f,white,true);
            port3ds::ui_text(hud_buf,"MENU",283,161,.48f,white,true);
            if(menu==Menu::none)refresh_world();
            if(menu==Menu::none || menu==Menu::catalogue){
                for(unsigned i=0;i<option_count;++i){if(!option_rows[i].label[0])continue;auto cell=port3ds::CommandGrid::cell(i);
                    graphics.ui_image(engine.command_icon(option_rows[i].icon,option_rows[i].enabled),cell.x,cell.y,35.f/36,engine);
                    if(menu==Menu::catalogue && i==choice)port3ds::ui_border({cell.x,cell.y,35,35},green);
                    const char* keys=menu==Menu::catalogue?port3ds::CommandGrid::key(i):i<4?(i==0?"A":i==1?"R+A":i==2?"R+B":"L+A"):(i==4?"X":i==5?"Y":i==6?"L+X":i==7?"L+Y":"L+R");
                    char badge[24];std::snprintf(badge,sizeof badge,"key_%s",keys);unsigned bw=0,bh=0;
                    if(hud_art.size(badge,bw,bh))hud_art.draw(badge,cell.x+(35-int(bw))/2,cell.y+27);

                }
            }
            const auto selected_info=engine.selection_info();
            if(selected_info.count==1)port3ds::draw_selection_info(selected_info,graphics,engine,hud_art);
            if(state.observer && !selected_info.count){
                for(unsigned p=0;p<3;++p)if(state.players[p].ai){const auto& side=state.players[p];char row[80];std::snprintf(row,sizeof row,"%u %s  Scout %u Cast %u Drop %u",p+1,side.difficulty==port3ds::AiDifficulty::medium?"M":"E",side.scouts,side.casts,side.drop_units);port3ds::ui_text(hud_buf,row,15,169+p*19,.40f,white);}
            }
            unsigned selected_slot=0;
            for(std::size_t i=0;i<count && selected_slot<12;++i)if(units[i].selected){const auto& u=units[i];
                const int x=13+int(selected_slot%6)*37,y=157+int(selected_slot/6)*37;
                if(selected_info.count>1)graphics.ui_image(engine.command_icon(u.type),x+1,y+1,.82f,engine);
                if(selected_info.count>1 && u.max_hp>0)C2D_DrawRectSolid(x,y+29,0,32.f*u.hp/u.max_hp,2,green);

                ++selected_slot;
            }
            graphics.ui_image(engine.portrait(),259,181,1,engine);
            const auto mini=controls.minimap_bounds();
            C2D_DrawRectSolid(mini.x, mini.y, 0, mini.w, mini.h, grid);
            graphics.minimap(mini.x,mini.y,mini.w,mini.h);
            const float sx = mini.w / controls.map_width(), sy = mini.h / controls.map_height();
            for (std::size_t i = 0; i != count; ++i) {
                const u32 color=units[i].selected?green:units[i].owner==0?
                    (state.observer?C2D_Color32(240,110,100,255):green):units[i].owner==1?
                    C2D_Color32(100,150,255,255):units[i].owner==2?C2D_Color32(90,220,80,255):C2D_Color32(155,170,180,255);
                C2D_DrawRectSolid(mini.x + units[i].x * sx, mini.y + units[i].y * sy, 0, 2, 2,color);
            }
            port3ds::UnitView memories[64];const auto remembered=engine.remembered_units(memories,64);
            for(std::size_t i=0;i<remembered;++i)C2D_DrawRectSolid(mini.x+memories[i].x*sx,mini.y+memories[i].y*sy,0,2,2,memories[i].owner==1?C2D_Color32(100,150,255,255):C2D_Color32(155,170,180,255));
            graphics.fog_minimap(engine,mini.x,mini.y,mini.w,mini.h);
            const float mx = mini.x + controls.camera_x * sx, my = mini.y + controls.camera_y * sy;
            C2D_DrawLine(mx, my, green, mx + 400 * sx, my, green, 1, 0);
            C2D_DrawLine(mx, my, green, mx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx + 400 * sx, my, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            C2D_DrawLine(mx, my + 240 * sy, green, mx + 400 * sx, my + 240 * sy, green, 1, 0);
            if(state.alert_frame && state.frame-state.alert_frame<24*10 && (state.frame/6)%2)C2D_DrawCircleSolid(mini.x+state.alert_x*sx,mini.y+state.alert_y*sy,0,3,C2D_Color32(255,75,45,255));
            if(overlay(menu)){
                C2D_DrawRectSolid(0,0,0,320,240,C2D_Color32(0,0,0,155));
                C2D_DrawRectSolid(24,14,0,272,216,C2D_Color32(9,16,30,245));
                port3ds::ui_border({24,14,272,216},C2D_Color32(88,102,130,255));
                port3ds::ui_border({27,17,266,210},C2D_Color32(36,58,170,255));
                const char* heading=menu==Menu::save_slots?"Save Game":menu==Menu::load_slots?"Load Game":menu==Menu::system?"Game Menu":menu==Menu::options?"Options":menu==Menu::scenarios?"Test scenarios":menu==Menu::legacy?"More scenarios":menu==Menu::notice?notice_title:menu==Menu::end_confirm?"End this game?":action;
                port3ds::ui_text(hud_buf,heading,160,23,.63f,white,true);
                if(menu==Menu::notice){
                    port3ds::ui_limited(hud_buf,notice_message,40,100,.49f,240,4,white);
                }
                for(unsigned i=0;i<menu_count();++i){const auto r=port3ds::PauseMenu::row(i,menu_count());
                    C2D_DrawRectSolid(r.x,r.y,0,r.w,r.h,C2D_Color32(8,16,35,240));
                    port3ds::ui_border(r,i==choice?C2D_Color32(88,130,255,255):C2D_Color32(30,55,125,255));
                    const bool slots=menu==Menu::save_slots || menu==Menu::load_slots;const float size=slots?.43f:menu_count()>6?.49f:.55f;float w,h;C2D_TextGetDimensions(&menu_rows[i],size,size,&w,&h);
                    if(slots){const auto& slot=save_slots.slots[i];const auto& label=port3ds::ui_fits(slot.label.c_str(),size,r.w-12)?slot.label:slot.short_label;port3ds::ui_limited(hud_buf,label.c_str(),r.x+6,r.y+5,size,r.w-12,1,menu==Menu::load_slots && !slot.available?C2D_Color32(90,100,115,255):i==choice?C2D_Color32(245,240,110,255):white);}
                    else C2D_DrawText(&menu_rows[i],C2D_WithColor,r.x+(r.w-w)/2,r.y+(r.h-h)/2,0,size,size,i==choice?C2D_Color32(245,240,110,255):white);
                }
                graphics.cursor(engine.cursor_image(port3ds::CursorKind::arrow,now-cursor_since),menu_x,menu_y,engine);
            }
            } // World and pause UI are replaced by the result screens.
            if(pending_load || warming){
                C2D_SceneBegin(top);C2D_DrawRectSolid(0,0,0,400,240,C2D_Color32(0,0,0,255));
                if(front.briefing_loading)port3ds::draw_campaign_top(front,presentation_art,text_buf,true);
                C2D_SceneBegin(bottom);C2D_DrawRectSolid(0,0,0,320,240,C2D_Color32(0,0,0,255));
                const char* dots[]={"Loading.","Loading..","Loading..."};port3ds::ui_centered(text_buf,dots[(now_us()/375000)%3],{0,0,320,240},.65f,C2D_Color32(80,255,80,255));
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
                presentation_art.clear();front.briefing_loading=false;front.dialog=port3ds::Frontend::Dialog::none;
                warming=false;scheduler.reset();previous=interval=now_us();
                renders=ticks=0;tick_sum=tick_max=0;
                render_sum=wait_sum=collect_sum=sprite_sum=end_sum=gpu_sum=0;
            }
        }
        log.close();
        if(hud_buf)C2D_TextBufDelete(hud_buf);
    }
    if(!return_custom)break;
    front.screen=(match_setup.observer || port3ds::campaign::scenario(unsigned(launch_scenario)))?port3ds::Frontend::Screen::campaign:port3ds::Frontend::Screen::custom;front.transition=0;front.preview_dirty=true;front.x=240;front.y=218;
    front.dialog=port3ds::Frontend::Dialog::none;
    if(continue_campaign<port3ds::campaign::count){front.brood=port3ds::campaign::missions[continue_campaign].brood;front.open_briefing(continue_campaign);}
    C2D_Prepare();
    }
    presentation_art.clear();
    if (R_SUCCEEDED(audio)) ndspExit();
    aptUnhook(&cookie);
    C2D_TextBufDelete(text_buf);
    C2D_Fini(); C3D_Fini(); gfxExit();
    return 0;
}
