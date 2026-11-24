#include "engine.h"
#include "actions.h"
#include "graphics_format.h"
#include "battle_ai.h"
#include "terran_battle.h"
#include "creep_graphics.h"
#include "fog_memory.h"
#include "sound_ids.h"
#include <cmath>
#include "command_icons.h"
#include "campaign_runtime.h"
#include "campaign_map.h"
#include "campaign_ai.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace port3ds {
using namespace bwgame;
namespace {
constexpr unsigned first_selection_ring=unsigned(ImageTypes::IMAGEID_Selection_Circle_22pixels);
constexpr unsigned last_selection_ring=unsigned(ImageTypes::IMAGEID_Selection_Circle_Dashed_224pixels);
bool player_sees(const state_functions& f,const unit_t* unit) {
    return unit->owner==0 || ((unit->sprite->visibility_flags&1) && !f.unit_is_undetected(unit,0));
}
}
struct Engine::Impl {
    struct SoundFunctions : state_functions {
        std::array<SoundEvent,256> events{};
        unsigned count=0, dropped=0;bool fog=false, mute=false;
        detail::CampaignRuntime campaign;
        detail::CampaignAI campaign_ai;
        bool execute_trigger_action(execute_trigger_struct& ets,int owner,running_trigger& rt,running_trigger::action& ra,const trigger::action& a)override{
            // Melee/labs use adapter rules; map UMS triggers must not reset
            // lab funds or run campaign scripts in a custom melee match.
            if(!campaign.active)return true;
            bool handled=false;const bool result=campaign_action(ets,owner,rt,ra,a,handled);if(handled)return result;
            if(owner!=0)return state_functions::execute_trigger_action(ets,owner,rt,ra,a);
            auto camera=[&](){if(a.location>0 && unsigned(a.location)<=st.locations.size()){const auto& area=st.locations[a.location-1].area;campaign.camera=(area.from+area.to)/2;++campaign.camera_serial;}};
            switch(a.type){
            case 5:
                campaign.game_paused=campaign.honor_game_pause;
                campaign.cinematic=true;
                return true;
            case 6:
                campaign.game_paused=false;
                campaign.cinematic=false;
                return true;
            case 30:campaign.cinematic=true;return true;
            case 31:campaign.cinematic=false;return true;
            case 7:{
                if(!(ra.flags&1)){if(a.sound_index>0 && campaign_sound_base+unsigned(a.sound_index)<audio_sound_limit)play_sound(campaign_sound_base+a.sound_index,{},nullptr,false);campaign.message=a.string_index;campaign.message_until=st.current_frame+std::max(96,a.time_n*24/1000);}
                auto wait=a;wait.type=4;wait.time_n=std::max(1000,a.time_n);return state_functions::execute_trigger_action(ets,owner,rt,ra,wait);
            }
            case 9:campaign.message=a.string_index;campaign.message_until=st.current_frame+240;return true;
            case 10:camera();return true;
            case 12:campaign.objective=a.string_index;return true;
            case 8:if(a.sound_index>0 && campaign_sound_base+unsigned(a.sound_index)<audio_sound_limit)play_sound(campaign_sound_base+a.sound_index,{},nullptr,false);return true;
            case 17:case 32:return true; // leaderboard presentation uses the objective HUD
            default:return state_functions::execute_trigger_action(ets,owner,rt,ra,a);
            }
        }
        #include "campaign_triggers.h"
        explicit SoundFunctions(state& st) : state_functions(st) {}
        void play_sound(int id, xy pos, const unit_t* source,bool race) override {
            if(mute || id<0)return;
            // Frozen fog memories are silent. Global UI sounds have no map
            // position; native local effects require current player sight.
            if(fog){
                if(source){if(source->owner!=0 && (!source->sprite || !(source->sprite->visibility_flags&1)))return;}
                else if(pos.x>0 && pos.y>0 && pos.x<int(st.game->map_width) && pos.y<int(st.game->map_height) && !player_position_is_visible(0,pos))return;
            }
            if(count==events.size()) { ++dropped; return; }
            events[count++]={static_cast<unsigned>(id+(race?int(st.players[0].race):0)),static_cast<int>(pos.x),static_cast<int>(pos.y)};
        }
    };
    std::unique_ptr<game_player> player;
    std::unique_ptr<SoundFunctions> sound;
    char message[192] = "Not loaded";
    bool loaded = false,fog=false;
    savegame::Writer journal;std::string initial_map,initial_root;Scenario initial_scenario=Scenario::empty;MatchSetup initial_setup;
    std::unique_ptr<savegame::Reader> replay;
    savegame::Rules rules=savegame::Rules::current;
    static_vector<unit_id_32,192> selected;
    std::array<static_vector<unit_id_32,192>,4> groups;
    detail::FogMemory memory;
    std::array<int,1700> alert_hp;std::array<unsigned,1700> alert_ids{};unsigned alert_frame=0;xy alert{};
    detail::BattleAI ai;
    detail::TerranBattle battle;
    std::unique_ptr<detail::CreepGraphics> creep;
    std::unique_ptr<detail::CommandIcons> icons;
    std::array<grp_t,cursor_count> cursors;
    std::array<uint16_t,256> palette{};
    std::array<uint8_t,128> team{};
    std::array<std::array<uint16_t,256>,4> team_palette;
    std::array<std::array<uint16_t,256>,7> lights;
    UnitView unit_view(const unit_t* unit,bool selected) const;
    unit_t* first_selected() const {
        if (!loaded) return nullptr;
        for (auto id:selected) if (auto* u=player->funcs().get_unit(id))
            if (!player->funcs().unit_dead(u) && (!fog || player_sees(player->funcs(),u))) return u;
        return nullptr;
    }
    const char* command_message = "A select / B act";
};

namespace {
const char* unit_name(const unit_t* u) {
    switch(u->unit_type->id){
    case UnitTypes::Hero_Jim_Raynor_Vulture:case UnitTypes::Hero_Jim_Raynor_Marine:return "Jim Raynor";
    case UnitTypes::Hero_Fenix_Zealot:case UnitTypes::Hero_Fenix_Dragoon:return "Fenix";
    case UnitTypes::Hero_Zeratul:return "Zeratul";
    case UnitTypes::Hero_Samir_Duran:return "Samir Duran";
    case UnitTypes::Hero_Sarah_Kerrigan:return "Sarah Kerrigan";
    case UnitTypes::Hero_Infested_Kerrigan:return "Infested Kerrigan";
    case UnitTypes::Hero_Infested_Duran:return "Infested Duran";
    case UnitTypes::Hero_Alexei_Stukov:return "Alexei Stukov";
    case UnitTypes::Hero_Tassadar:return "Tassadar";
    case UnitTypes::Hero_Tassadar_Zeratul_Archon:return "Tassadar / Zeratul";
    case UnitTypes::Hero_Danimoth:return "Danimoth";
    case UnitTypes::Hero_Raszagal:return "Raszagal";
    case UnitTypes::Hero_Matriarch:return "Matriarch";
    case UnitTypes::Hero_Unclean_One:return "Unclean One";
    case UnitTypes::Hero_Gui_Montag:return "Gui Montag";
    case UnitTypes::Hero_Alan_Schezar:return "Alan Schezar";
    case UnitTypes::Hero_Tom_Kazansky:return "Tom Kazansky";
    case UnitTypes::Hero_Magellan:return "Magellan";
    case UnitTypes::Hero_Edmund_Duke_Tank_Mode:case UnitTypes::Hero_Edmund_Duke_Siege_Mode:return "Edmund Duke";
    case UnitTypes::Hero_Arcturus_Mengsk:return "Arcturus Mengsk";
    case UnitTypes::Hero_Hyperion:return "Hyperion";
    case UnitTypes::Hero_Norad_II:return "Norad II";
    case UnitTypes::Hero_Gerard_DuGalle:return "Gerard DuGalle";
    case UnitTypes::Hero_Torrasque:return "Torrasque";
    case UnitTypes::Hero_Devouring_One:return "Devouring One";
    case UnitTypes::Hero_Kukulza_Mutalisk:case UnitTypes::Hero_Kukulza_Guardian:return "Kukulza";
    case UnitTypes::Hero_Yggdrasill:return "Yggdrasill";
    case UnitTypes::Hero_Aldaris:return "Aldaris";
    case UnitTypes::Hero_Artanis:return "Artanis";
    case UnitTypes::Hero_Mojo:return "Mojo";
    case UnitTypes::Hero_Warbringer:return "Warbringer";
    case UnitTypes::Hero_Gantrithor:return "Gantrithor";
    case UnitTypes::Hero_Hunter_Killer:return "Hunter Killer";
    case UnitTypes::Hero_Dark_Templar:return "Dark Templar";
    default:break;
    }
    if(const char* n=zerg::name(u->unit_type->id))return n;
    if(const char* n=protoss::name(u->unit_type->id))return n;
    return terran::name(u->unit_type->id);
}
// Use the engine collision box with a small thumbstick tolerance. Prefer
// the nearest center when two boxes overlap; never select dead/hidden units.
unit_t* hit_unit(state_functions& funcs, int x, int y, bool own_only,bool fog=false,bool artefacts_only=false) {
    unit_t* best = nullptr;
    int distance = 0x7fffffff;
    for (auto* u : ptr(funcs.st.visible_units)) {
        if(artefacts_only && !funcs.is_campaign_powerup(u))continue;
        if ((fog && !player_sees(funcs,u)) || funcs.ut_turret(u) || (own_only && u->owner != 0) || funcs.unit_dead(u) ||
            (u->sprite->flags & sprite_t::flag_hidden)) continue;
        const auto box = funcs.unit_inner_bounding_box(u, u->sprite->position);
        if (x < box.from.x - 6 || y < box.from.y - 6 || x > box.to.x + 6 || y > box.to.y + 6) continue;
        const int dx = u->sprite->position.x - x, dy = u->sprite->position.y - y;
        if (dx * dx + dy * dy < distance || (dx * dx + dy * dy==distance && best && best->owner && !u->owner)) { best = u; distance = dx * dx + dy * dy; }
    }
    return best;
}
bool chk_path(const a_string& path){if(path.size()<4)return false;const auto n=path.size();return path[n-4]=='.' && (path[n-3]=='c' || path[n-3]=='C') && (path[n-2]=='h' || path[n-2]=='H') && (path[n-1]=='k' || path[n-1]=='K');}
void read_file(a_vector<uint8_t>& dst, const a_string& path) {
    data_loading::file_reader<> file(path);
    dst.resize(file.size());
    file.get_bytes(dst.data(), dst.size());
}
bool exists(const a_string& path) {
    auto* f = std::fopen(path.c_str(), "rb");
    if (!f) return false;
    std::fclose(f);
    return true;
}
// Reject large maps before region/pathfinding allocation. Accept only the
// first DIM section, and reject duplicate dimensions rather than trusting it.
void validate_map(const a_vector<uint8_t>& chk, unsigned& width, unsigned& height) {
    data_loading::data_reader_le r(chk.data(), chk.data() + chk.size());
    unsigned dims = 0;
    while (r.left()) {
        auto tag = r.get<std::array<char, 4>>();
        auto len = r.get<uint32_t>();
        const auto* section = r.get_n(len);
        if (std::memcmp(tag.data(), "DIM ", 4) == 0) {
            if (++dims != 1 || len != 4) throw std::runtime_error("Invalid DIM section");
            data_loading::data_reader_le d(section, section + len);
            width = d.get<uint16_t>(); height = d.get<uint16_t>();
        }
    }
    if (dims != 1 || width < 32 || height < 32 || width > 256 || height > 256)
        throw std::runtime_error("Map dimensions must be between 32 and 256 tiles");
}
}

bool Engine::preview_map(const char* data_root,const char* map_path,MapPreview& out) {
    out=MapPreview{};
    try {
        a_vector<uint8_t> chk;const a_string path(map_path);
        if(chk_path(path))read_file(chk,path);
        else {data_loading::mpq_file<> archive(path);archive(chk,"staredit/scenario.chk");}
        struct Chunk{const uint8_t* p=nullptr;size_t n=0;};Chunk dim,era,owners,units,sprp,strings,strx,terrain;
        size_t at=0;while(at+8<=chk.size()){
            const auto* tag=chk.data()+at;const size_t n=data_loading::value_at<uint32_t,true>(tag+4);at+=8;if(n>chk.size()-at)throw std::runtime_error("Truncated map chunk");
            auto keep=[&](const char* name,Chunk& dst){if(!std::memcmp(tag,name,4))dst={chk.data()+at,n};};
            keep("DIM ",dim);keep("ERA ",era);keep("OWNR",owners);keep("UNIT",units);keep("SPRP",sprp);keep("STR ",strings);keep("STRx",strx);keep("MTXM",terrain);at+=n;
        }
        auto word=[](const uint8_t* p){return unsigned(p[0]|(p[1]<<8));};
        if(dim.n!=4 || era.n<2)throw std::runtime_error("Missing map dimensions/tileset");
        out.width=word(dim.p);out.height=word(dim.p+2);out.tileset=word(era.p)&7;
        if(!out.width || !out.height || out.width>256 || out.height>256 || terrain.n<size_t(out.width)*out.height*2)throw std::runtime_error("Invalid preview dimensions");
        bool starts[8]{};for(size_t i=0;i+36<=units.n;i+=36)if(word(units.p+i+8)==214 && units.p[i+16]<8)starts[units.p[i+16]]=true;
        for(auto v:starts)out.players+=v;if(!out.players)for(size_t i=0;i<std::min<size_t>(8,owners.n);++i)if(owners.p[i]==5 || owners.p[i]==6)++out.players;
        auto string=[&](unsigned id,char* dst,size_t capacity){
            const auto c=strx.p?strx:strings;const size_t step=strx.p?4:2;if(!c.p || c.n<step || !id)return;
            const unsigned count=step==4?data_loading::value_at<uint32_t,true>(c.p):word(c.p);
            if(id>count || size_t(id)*step+step>c.n)return;
            const size_t off=step==4?data_loading::value_at<uint32_t,true>(c.p+id*step):word(c.p+id*step);size_t n=0;
            for(size_t i=off;i<c.n && c.p[i] && n+1<capacity;++i){const auto ch=c.p[i];if(ch=='\n' || ch>=32)dst[n++]=ch<127?char(ch):'?';}dst[n]=0;
        };
        if(sprp.n>=4){string(word(sprp.p),out.title,sizeof out.title);string(word(sprp.p+2),out.description,sizeof out.description);}
        if(!out.title[0]){const auto slash=path.find_last_of("/\\");std::snprintf(out.title,sizeof out.title,"%s",path.c_str()+(slash==a_string::npos?0:slash+1));}
        auto assets=data_loading::data_files_directory(data_root);a_vector<uint8_t> cv5,vx4,vr4,wpe;
        const char* names[]={"badlands","platform","install","ashworld","jungle","desert","ice","twilight"};const a_string name=a_string("tileset/")+names[out.tileset];
        assets(cv5,name+".cv5");assets(vx4,name+".vx4");assets(vr4,name+".vr4");assets(wpe,name+".wpe");if(wpe.size()!=1024)throw std::runtime_error("Invalid preview palette");
        for(unsigned y=0;y<128;++y)for(unsigned x=0;x<128;++x){
            const unsigned wx=(x*2+1)*out.width*32/256,wy=(y*2+1)*out.height*32/256;
            const unsigned tile=word(terrain.p+((wy/32)*out.width+wx/32)*2),entry=(tile>>4)*52+20+(tile&15)*2;
            if(entry+2>cv5.size())throw std::runtime_error("Preview CV5 bounds");const unsigned mega=word(cv5.data()+entry),off=mega*32+((wy%32/8)*4+(wx%32/8))*2;
            if(off+2>vx4.size())throw std::runtime_error("Preview VX4 bounds");const unsigned mini=word(vx4.data()+off),pixel=(mini/2)*64+(wy%8)*8+((mini&1)?7-wx%8:wx%8);
            if(pixel>=vr4.size())throw std::runtime_error("Preview VR4 bounds");const unsigned c=vr4[pixel]*4;
            out.pixels[y*128+x]=((wpe[c]>>3)<<11)|((wpe[c+1]>>3)<<6)|((wpe[c+2]>>3)<<1)|1;
        }
        // Resource/start markers remain useful on small terrain previews.
        for(size_t i=0;i+36<=units.n;i+=36){unsigned type=word(units.p+i+8),owner=units.p[i+16];if(type!=214 && type!=176 && type!=177 && type!=178 && type!=188)continue;
            unsigned x=word(units.p+i+4)*128/(out.width*32),y=word(units.p+i+6)*128/(out.height*32);const uint16_t color=type==214?(owner==0?0xf801:owner==1?0x003f:0x07c1):0x07ff;
            for(unsigned dy=0;dy<2;++dy)for(unsigned dx=0;dx<2;++dx)if(x+dx<128 && y+dy<128)out.pixels[(y+dy)*128+x+dx]=color;
        }
        out.valid=true;return true;
    }catch(const std::exception& e){std::snprintf(out.error,sizeof out.error,"%s",e.what());return false;}
}

Engine::Engine() : impl_(new Impl) {}
Engine::~Engine() = default;
bool Engine::load(const char* data_root, const char* map_path, Scenario scenario,const MatchSetup* setup,const std::function<void()>& progress) {
    const bool scripted=campaign::scenario(unsigned(scenario)) || (setup && setup->enabled && setup->game_type==2);
    auto rules=scripted?savegame::Rules::preplaced_hangars:savegame::Rules::current;
    if(scenario==Scenario::bw_zerg_bonus)rules=savegame::Rules::dark_origin_pause;
    return load_with_rules(data_root,map_path,scenario,setup,progress,rules);
}
bool Engine::load_with_rules(const char* data_root,const char* map_path,Scenario scenario,const MatchSetup* setup,const std::function<void()>& progress,savegame::Rules rules) {
    impl_->replay.reset();
    impl_->rules=rules;
    impl_->journal.close();impl_->initial_root=data_root;impl_->alert_hp.fill(-1);impl_->alert_ids.fill(0);impl_->alert_frame=0;impl_->initial_map=map_path;impl_->initial_scenario=scenario;impl_->initial_setup=setup?*setup:MatchSetup{};
    const bool patriots_blood=scenario==Scenario::bw_terran_campaign_7;
    const bool campaign=campaign::scenario(unsigned(scenario));
    const bool custom=!campaign && setup && setup->enabled;
    const bool ums=custom && setup->game_type==2;
    const bool scripted=campaign || ums;
    const bool honor_game_pause=rules==savegame::Rules::dark_origin_pause;
    const bool load_preplaced_hangars=rules==savegame::Rules::preplaced_hangars || honor_game_pause;
    impl_->loaded = false;
    impl_->fog=scripted || scenario==Scenario::melee || scenario==Scenario::versus_ai || scenario==Scenario::protoss_versus_ai || scenario==Scenario::zerg_versus_ai;
    if(custom && setup->observer)impl_->fog=false;
    impl_->battle.reset(scenario==Scenario::terran_battle || scenario==Scenario::protoss_battle || scenario==Scenario::zerg_battle,scenario_race(scenario));
    impl_->selected.clear();
    impl_->memory.reset();
    for (auto& g:impl_->groups) g.clear();
    impl_->ai.reset(scenario==Scenario::ai_battle || scenario==Scenario::gas_battle || scenario==Scenario::protoss_ai_battle || scenario==Scenario::zerg_ai_battle,
        scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle || scenario==Scenario::protoss_ai_battle || scenario==Scenario::protoss_versus_ai || scenario==Scenario::zerg_ai_battle || scenario==Scenario::zerg_versus_ai,
        scenario==Scenario::gas_battle || scenario==Scenario::protoss_ai_battle || scenario==Scenario::protoss_versus_ai || scenario==Scenario::zerg_ai_battle || scenario==Scenario::zerg_versus_ai,scenario_race(scenario)==2,scenario_race(scenario)==0);
    if(ums)impl_->ai.reset(false,false);
    else if(custom)impl_->ai.configure(*setup);
    impl_->sound.reset();
    impl_->creep.reset();
    impl_->icons.reset();
    impl_->player.reset();
    for(auto& cursor:impl_->cursors)cursor=grp_t{};
    try {
        a_vector<uint8_t> chk;
        const a_string map(map_path);
        if (chk_path(map)) read_file(chk, map);
        else {
            data_loading::mpq_file<> archive(map);
            archive(chk, "staredit/scenario.chk");
        }
        unsigned width = 0, height = 0;
        validate_map(chk, width, height);
        if(ums){
            if(setup->races[0]<0 || setup->races[0]>2)throw std::runtime_error("Invalid player race");
            unsigned human=8;
            for(size_t at=0;at+8<=chk.size();){
                const auto length=campaign::get(chk.data()+at+4);
                if(!std::memcmp(chk.data()+at,"OWNR",4)){
                    if(length!=12)throw std::runtime_error("Invalid Use Map Settings player table");
                    for(unsigned i=0;i<8;++i)if(chk[at+8+i]==player_t::controller_open){human=i;break;}
                }
                at+=8+length;
            }
            if(human>=8)throw std::runtime_error("Use Map Settings needs an open player slot");
            campaign::normalize(chk,human);
        }
        std::array<unsigned,12> campaign_races{};campaign_races.fill(1);
        if(scripted)for(size_t at=0;at+8<=chk.size();){
            const auto length=campaign::get(chk.data()+at+4);
            if(!std::memcmp(chk.data()+at,"SIDE",4)){
                if(length!=12)throw std::runtime_error("Invalid map race table");
                for(unsigned i=0;i<12;++i){
                    const unsigned race=chk[at+8+i];
                    campaign_races[i]=race<3?race:ums?unsigned(setup->races[0]):0;
                }
            }
            at+=8+length;
        }
        if(custom && !ums){
            if(setup->races[0]<0 || setup->races[0]>2)throw std::runtime_error("Invalid player race");
            for(unsigned i=1;i<3;++i)if(setup->races[i]<-1 || setup->races[i]>2)throw std::runtime_error("Invalid AI race");
            if(setup->game_type>2)throw std::runtime_error("Invalid custom game type");
            // Compact sparse/native start owners into the three active UI slots.
            // Modify start markers only; resources retain their neutral ownership.
            std::array<int,8> remap;remap.fill(-1);unsigned starts=0;
            size_t at=0;while(at+8<=chk.size()){
                const auto n=data_loading::value_at<uint32_t,true>(chk.data()+at+4);auto* tag=chk.data()+at;at+=8;
                if(n>chk.size()-at)throw std::runtime_error("Truncated map section");
                if(!std::memcmp(tag,"UNIT",4))for(size_t j=at;j+36<=at+n;j+=36){auto* u=chk.data()+j;
                    if(data_loading::value_at<uint16_t,true>(u+8)==214){const auto owner=u[16];
                        if(owner>=8 || remap[owner]>=0)throw std::runtime_error("Invalid or duplicate start location");remap[owner]=int(starts++);
                    }
                }at+=n;
            }
            unsigned required=1;for(unsigned i=1;i<3;++i)if(setup->races[i]>=0)required=i+1;
            if(starts<required)throw std::runtime_error("Map has too few start locations for these AI slots");
            at=0;while(at+8<=chk.size()){const auto n=data_loading::value_at<uint32_t,true>(chk.data()+at+4);const bool units=!std::memcmp(chk.data()+at,"UNIT",4);at+=8;
                if(units)for(size_t j=at;j+36<=at+n;j+=36){auto* u=chk.data()+j;if(data_loading::value_at<uint16_t,true>(u+8)==214)u[16]=uint8_t(remap[u[16]]);}at+=n;
            }
        }
        a_string root(data_root);
        if (!root.empty() && root.back() != '/') root += '/';
        impl_->player.reset(new game_player);
        auto& player = *impl_->player;
        // Loose classic assets also allow local extraction from a user's CASC
        // installation. The 3DS does not need to implement the CASC filesystem.
        if (exists(root + "arr/units.dat")) {
            player.init([&](a_vector<uint8_t>& dst, const a_string& name) {
                a_string path = name;
                for (auto& c : path) if (c == '\\') c = '/';
                read_file(dst, root + path);if(progress)progress();
            });
        } else {
            auto assets = data_loading::data_files_directory(root);
            player.init([&](a_vector<uint8_t>& dst,const a_string& name){assets(dst,name);if(progress)progress();});
        }
        game_load_functions loader(player.st());
        loader.load_map_data(chk.data(), chk.size(), [&]() {
            const bool melee=scenario==Scenario::melee || scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle || scenario==Scenario::protoss_ai_battle || scenario==Scenario::protoss_versus_ai || scenario==Scenario::zerg_ai_battle || scenario==Scenario::zerg_versus_ai;
            if(scripted){
                loader.setup_info.create_no_units=false;loader.setup_info.victory_condition=0;loader.setup_info.starting_units=0;loader.setup_info.resource_type=0;
                loader.setup_info.load_preplaced_hangars=load_preplaced_hangars;
                // UMS computer slots begin allied across Force boundaries. Preserve
                // that relationship before replacing the retail AI controller;
                // original alliance/AI triggers may change it during the mission.
                std::array<bool,8> computers{};
                for(unsigned i=0;i<8;++i)computers[i]=player.st().players[i].controller==player_t::controller_computer;
                for(unsigned i=0;i<8;++i)for(unsigned j=0;j<8;++j)if(i!=j){
                    const auto force=player.st().players[i].force;
                    if((campaign && computers[i] && computers[j]) ||
                       (ums && force>=1 && force<=4 && force==player.st().players[j].force && (player.st().game->forces[force-1].flags&2)))
                        player.st().alliances[i][j]=2;
                }
                for(unsigned i=0;i<12;++i){auto& p=player.st().players[i];p.race=static_cast<race_t>(campaign_races[i]);if(p.controller==player_t::controller_open)p.controller=ums && i!=0?player_t::controller_closed:player_t::controller_occupied;else if(p.controller==player_t::controller_computer)p.controller=player_t::controller_occupied;}
                return;
            }
            loader.setup_info.create_no_units = !melee;
            loader.setup_info.victory_condition = 0;
            loader.setup_info.starting_units = custom?1:0;
            loader.setup_info.resource_type = 1;
            loader.setup_info.starting_minerals = 50;
            for (unsigned i = 0; i != 12; ++i) {
                auto& p = player.st().players[i];
                const bool active=custom?i<3 && setup->races[i]>=0:i<2;
                p.controller = active ? player_t::controller_occupied : player_t::controller_inactive;
                p.race = static_cast<race_t>(custom && i<3 && setup->races[i]>=0?setup->races[i]:int(scenario_race(scenario)));
                loader.setup_info.create_melee_units_for_player[i] = active && melee;
            }
        });
        // Maps without an authored preplaced hangar still use the previous
        // replay profile, so both executable versions can open their saves.
        if(rules==savegame::Rules::preplaced_hangars && !loader.has_preplaced_hangars)
            impl_->rules=savegame::Rules::authored_hangars;
        if(custom && !ums)for(unsigned p=0;p<3;++p)for(unsigned q=0;q<3;++q)if(p!=q)player.st().alliances[p][q]=setup->teams[p]==setup->teams[q]?2:0;
        if(scripted)for(const auto& trigger:player.st().game->triggers){
            for(const auto& c:trigger.conditions){if(!c.type)break;if(c.flags&2)continue;if(c.type!=1 && c.type!=5 && c.type!=6 && c.type!=7 && c.type!=16 && c.type!=17 && c.type!=23 && c.type!=2 && c.type!=3 && c.type!=4 && c.type!=11 && c.type!=12 && c.type!=14 && c.type!=15 && c.type!=21 && c.type!=22)throw std::runtime_error("Unsupported campaign trigger condition "+std::to_string(c.type));}
            for(const auto& a:trigger.actions){if(a.flags&2)continue;if(!a.type)break;if(a.type!=1 && a.type!=2 && a.type!=3 && a.type!=4 && a.type!=5 && a.type!=6 && a.type!=7 && a.type!=8 && a.type!=9 && a.type!=10 && a.type!=11 && a.type!=12 && a.type!=13 && a.type!=14 && a.type!=45 && a.type!=54 && a.type!=55 && a.type!=15 && a.type!=16 && a.type!=17 && a.type!=19 && a.type!=21 && a.type!=22 && a.type!=23 && a.type!=24 && a.type!=25 && a.type!=26 && a.type!=27 && a.type!=28 && a.type!=30 && a.type!=31 && a.type!=32 && a.type!=36 && a.type!=37 && a.type!=38 && a.type!=39 && a.type!=41 && a.type!=42 && a.type!=43 && a.type!=44 && a.type!=46 && a.type!=47 && a.type!=48 && a.type!=49 && a.type!=50 && a.type!=51 && a.type!=52 && a.type!=57)throw std::runtime_error("Unsupported campaign trigger action "+std::to_string(a.type));}
        }
        // The deterministic controller runs after loading; native player slots stay human.
        if (scenario == Scenario::two_units) {
            auto* type = loader.get_unit_type(UnitTypes::Terran_Marine);
            if (!loader.create_initial_unit(type, {128, 128}, 0) ||
                !loader.create_initial_unit(type, {512, 128}, 1))
                throw std::runtime_error("Could not place the two probe units");
        }
        if(!ums && (scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle || scenario==Scenario::protoss_ai_battle || scenario==Scenario::protoss_versus_ai || scenario==Scenario::zerg_ai_battle || scenario==Scenario::zerg_versus_ai)) {
            unit_t* bases[3]={nullptr,nullptr,nullptr};
            for(unsigned p=0;p!=(custom?3u:2u);++p) for(auto* u:ptr(player.st().player_units[p]))
                if(loader.unit_is(u,(custom?setup->races[p]:int(scenario_race(scenario)))==0?UnitTypes::Zerg_Hatchery:(custom?setup->races[p]:int(scenario_race(scenario)))==2?UnitTypes::Protoss_Nexus:UnitTypes::Terran_Command_Center)) bases[p]=u;
            if(custom)for(unsigned p=0;p<3;++p)if(setup->races[p]>=0 && !bases[p])throw std::runtime_error("Cannot create a base at the map start location");
            if(!custom && (!bases[0] || !bases[1] || !loader.is_reachable(bases[0],bases[1])))
                throw std::runtime_error("AI ground battle needs connected start locations");
        }
        if(arena_scenario(scenario))impl_->battle.setup(loader,scenario==Scenario::terran_lab || scenario==Scenario::protoss_lab || scenario==Scenario::zerg_lab);
        // Normal player skirmishes retain native exploration. Prepared labs
        // and observer fixtures keep their historical fully explored terrain.
        if(!impl_->fog)for(auto& tile:player.st().tiles)tile.explored&=~3;
        // GRPs are already resident because native iscript/collision requires
        // them. Load only palette tables, then expand frames on GPU-cache miss.
        const bool loose_assets=exists(root+"arr/units.dat");
        // Reuse the archive tables and page caches across presentation reads.
        // Keep this separate from core initialization so their temporary
        // allocations still have disjoint lifetimes on original hardware.
        std::unique_ptr<data_loading::data_files_loader<>> presentation_assets;
        if(!loose_assets)
            presentation_assets.reset(new data_loading::data_files_loader<>(data_loading::data_files_directory(root)));
        auto load_asset=[&](a_vector<uint8_t>& dst,const a_string& name) {
            if(loose_assets)read_file(dst,root+name);
            else (*presentation_assets)(dst,name);
        };
        auto pcx=[&](const a_string& name) {
            a_vector<uint8_t> src;load_asset(src,name);
            if(src.size()<128 || src[3]!=8 || src[65]!=1)throw std::runtime_error("Invalid palette PCX");
            const unsigned row=src[66]|(src[67]<<8),h=(src[10]|(src[11]<<8))-(src[6]|(src[7]<<8))+1;
            if(!row || row>1024 || !h || h>1024)throw std::runtime_error("Palette PCX bounds");
            a_vector<uint8_t> out;out.reserve(row*h);size_t at=128;
            while(out.size()<row*h){if(at>=src.size())throw std::runtime_error("Truncated palette PCX");unsigned c=src[at++],n=1;if((c&192)==192){n=c&63;if(at>=src.size())throw std::runtime_error("PCX run");c=src[at++];}if(!n || n>row*h-out.size())throw std::runtime_error("PCX bounds");while(n--)out.push_back(c);}
            return out;
        };
        const char* tilesets[]={"badlands","platform","install","ashworld","jungle","desert","ice","twilight"};
        const auto terrain=a_string("tileset/")+tilesets[player.st().game->tileset_index];a_vector<uint8_t> wpe;load_asset(wpe,terrain+".wpe");
        if(wpe.size()!=1024)throw std::runtime_error("Palette bounds");
        for(unsigned i=0;i<256;++i)impl_->palette[i]=((wpe[i*4]>>3)<<11)|((wpe[i*4+1]>>3)<<6)|((wpe[i*4+2]>>3)<<1)|1;
        auto team=pcx("game/tunit.pcx");if(team.size()<128)throw std::runtime_error("Team palette bounds");std::copy_n(team.begin(),128,impl_->team.begin());
        const char* light_names[]={"ofire","gfire","bfire","bexpl","trans50","red","green"};
        for(unsigned owner=0;owner<4;++owner) {
            impl_->team_palette[owner]=impl_->palette;
            const unsigned team=owner<3?owner:11;
            for(unsigned c=8;c<16;++c)impl_->team_palette[owner][c]=impl_->palette[impl_->team[team*8+c-8]];
        }
        // The renderer samples one fixed column of these remap tables. Resolve
        // the palette once and release each large PCX before reading the next.
        for(unsigned i=0;i<7;++i) {
            const auto table=pcx(terrain+"/"+light_names[i]+".pcx");
            impl_->lights[i][0]=0;
            for(unsigned c=1;c<256;++c) {
                const unsigned at=256*(c-1);
                impl_->lights[i][c]=at<table.size()?impl_->palette[table[at]]:0;
            }
            if(progress)progress();
        }
        // Original cursor canvases are 128x128 with the pointer hotspot at
        // (64,64). Keep the native frame offsets; do not center cropped frames.
        auto load_cursors=[&](auto&& read){for(unsigned i=0;i<cursor_count;++i){
            a_vector<uint8_t> bytes;read(bytes,a_string("cursor/")+cursor_file(i)+".grp");
            auto grp=read_grp(data_loading::data_reader_le(bytes.data(),bytes.data()+bytes.size()));
            if(grp.width!=128 || grp.height!=128 || grp.frames.empty() || grp.frames.size()>16)throw std::runtime_error("Cursor GRP bounds");
            for(const auto& frame:grp.frames)if(frame.size.x>64 || frame.size.y>64 || frame.offset.x+frame.size.x>128 || frame.offset.y+frame.size.y>128)throw std::runtime_error("Cursor frame bounds");
            impl_->cursors[i]=std::move(grp);
        }};
        load_cursors(load_asset);
        if(scripted || scenario_race(scenario)==0 || (custom && (setup->races[1]==0 || setup->races[2]==0))){impl_->creep.reset(new detail::CreepGraphics);impl_->creep->load(player.st(),terrain,impl_->palette,load_asset);}
        // UI art is optional for loose-asset installations; core gameplay can
        // retain its text menu if the original command assets were not supplied.
        try {impl_->icons.reset(new detail::CommandIcons);impl_->icons->load(load_asset,pcx,ums?unsigned(player.st().players[0].race):scenario_race(scenario));}
        catch(const std::exception&){impl_->icons.reset();}
        impl_->sound.reset(new Impl::SoundFunctions(player.st()));
        impl_->sound->fog=impl_->fog;
        impl_->sound->campaign.use_authored_hangars=rules==savegame::Rules::authored_hangars ||
                                                  load_preplaced_hangars;
        impl_->sound->campaign.honor_game_pause=honor_game_pause;
        impl_->sound->campaign.active=scripted;impl_->sound->campaign.mission=campaign::index(unsigned(scenario));
        if(scripted)for(size_t at=0;at+8<=chk.size();){unsigned n=data_loading::value_at<uint32_t,true>(chk.data()+at+4);if(!std::memcmp(chk.data()+at,"UPRP",4)){if(n>64*20 || n%20)throw std::runtime_error("Campaign unit property bounds");for(unsigned i=0;i<n/20;++i)std::copy_n(chk.data()+at+8+i*20,20,impl_->sound->campaign.properties[i].begin());}at+=8+n;}
        if(scripted){
            impl_->sound->campaign_ai.load(load_asset);
            impl_->sound->campaign_ai.use_valued_regions=rules!=savegame::Rules::early_bw_zerg;
            impl_->sound->campaign_ai.use_location_alliances=patriots_blood;
        }
        presentation_assets.reset();
        if(progress)progress();
        if(campaign){impl_->sound->campaign.camera=player.st().game->start_locations[0];impl_->sound->campaign.camera_serial=1;}
        // User-requested shorter Reign of Fire intro: only the four authored
        // generators start damaged; the Psi Disrupter and other missions keep
        // their original HP and native destruction / victory triggers.
        if(scenario==Scenario::bw_zerg_campaign_2)for(auto* u:ptr(player.st().player_units[7]))if(loader.unit_is(u,UnitTypes::Special_Power_Generator) && u->hp>fp8::integer(150))loader.set_unit_hp(u,fp8::integer(150));
        if(scenario==Scenario::bw_zerg_campaign && rules!=savegame::Rules::early_bw_zerg)detail::CampaignRuntime::prepare_burrowed_ambushers(loader);
        impl_->loaded = true;
        std::snprintf(impl_->message, sizeof impl_->message, "Loaded %ux%u tiles", width, height);
        return true;
    } catch (const std::exception& e) {
        std::snprintf(impl_->message, sizeof impl_->message, "%s", e.what());
        impl_->sound.reset();
        impl_->player.reset();
        return false;
    }
}
// A private map-local tile bank is built while audio is unloaded. GPU terrain
// uploads then read only the viewport's tiles, independent of map variety.
bool Engine::write_terrain(const char* data_root,const char* output){
    if(!ready())return false;
    FILE* out=nullptr;
    bool opened=false;
    auto io_error=[](const char* stage){
        const int code=errno;
        return std::runtime_error(std::string(stage)+" (SD error "+std::to_string(code)+")");
    };
    try{
        const auto& st=impl_->player->st();const auto& game=*st.game;
        const char* names[]={"badlands","platform","install","ashworld","jungle","desert","ice","twilight"};
        a_vector<uint8_t> vx4,vr4;
        auto read=[&](a_vector<uint8_t>& dst,const a_string& name){a_string root(data_root);if(!root.empty() && root.back()!='/')root+='/';if(exists(root+"arr/units.dat"))read_file(dst,root+name);else{auto assets=data_loading::data_files_directory(root);assets(dst,name);}};
        read(vx4,a_string("tileset/")+names[game.tileset_index]+".vx4");
        read(vr4,a_string("tileset/")+names[game.tileset_index]+".vr4");
        auto pixel=[&](unsigned id,unsigned x,unsigned y){const auto off=id*32+((y/8)*4+x/8)*2;if(off+2>vx4.size())throw std::runtime_error("Terrain VX4 bounds");const unsigned mini=vx4[off]|vx4[off+1]<<8,src=mini/2*64+(y%8)*8+((mini&1)?7-x%8:x%8);if(src>=vr4.size())throw std::runtime_error("Terrain VR4 bounds");return impl_->palette[vr4[src]];};
        std::vector<unsigned short> ids(st.tiles_mega_tile_index.begin(),st.tiles_mega_tile_index.end());
        std::sort(ids.begin(),ids.end());ids.erase(std::unique(ids.begin(),ids.end()),ids.end());
        // This cache is disposable. Graphics validates the signature and exact
        // file size before use and rebuilds an interrupted write on next launch.
        out=std::fopen(output,"wb");
        if(!out)throw io_error("Cannot open terrain cache");
        opened=true;
        auto put=[&](unsigned v){unsigned char b[4]={uint8_t(v),uint8_t(v>>8),uint8_t(v>>16),uint8_t(v>>24)};if(std::fwrite(b,1,4,out)!=4)throw io_error("Cannot write terrain cache");};
        if(std::fwrite("OB3DTRN1",1,8,out)!=8)throw io_error("Cannot write terrain cache");
        put(game.map_tile_width);put(game.map_tile_height);put(terrain_signature());put(ids.size());
        for(auto id:st.tiles_mega_tile_index)put(std::lower_bound(ids.begin(),ids.end(),id)-ids.begin());
        // Minimap colors sample native map terrain, without UI start markers.
        for(unsigned y=0;y<128;++y)for(unsigned x=0;x<128;++x){const unsigned wx=(x*2+1)*game.map_width/256,wy=(y*2+1)*game.map_height/256;const auto value=pixel(st.tiles_mega_tile_index[wy/32*game.map_tile_width+wx/32],wx%32,wy%32);unsigned char b[2]={uint8_t(value),uint8_t(value>>8)};if(std::fwrite(b,1,2,out)!=2)throw io_error("Cannot write minimap cache");}
        for(auto id:ids){uint16_t tile[1024];for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x)tile[y*32+x]=pixel(id,x,y);if(std::fwrite(tile,2,1024,out)!=1024)throw io_error("Cannot write terrain cache");}
        if(std::fflush(out))throw io_error("Cannot flush terrain cache");
        const int closed=std::fclose(out);
        out=nullptr;
        if(closed)throw io_error("Cannot close terrain cache");
        return true;
    }catch(const std::exception& e){
        if(out)std::fclose(out);
        if(opened)std::remove(output);
        std::snprintf(impl_->message,sizeof impl_->message,"%s",e.what());
        return false;
    }
}

bool Engine::surrender(){savegame::Writer::Guard record(impl_->journal,{frame(),savegame::surrender});if(ready() && impl_->sound->campaign.active){impl_->sound->st.players[0].victory_state=2;return true;}return ready() && !impl_->battle.enabled() && impl_->ai.surrender();}
bool Engine::tick() {
    if(impl_->replay)return replay_step();
    return simulate_frame();
}
bool Engine::simulate_frame() {
    if (!impl_->loaded) return false;
    try {
        if(impl_->sound->campaign.game_paused){
            // Trigger waits and transmissions advance during the authored pause.
            ++impl_->sound->st.current_frame;
            impl_->sound->process_triggers();
            return true;
        }
        if(impl_->sound->campaign.active){impl_->sound->campaign.advance();impl_->sound->campaign.rescue(*impl_->sound);}
        impl_->sound->next_frame();
        if(impl_->sound->campaign.game_paused)return true;
        if(impl_->sound->campaign.active)impl_->sound->campaign_ai.step(*impl_->sound);
        if(impl_->initial_scenario==Scenario::bw_zerg_campaign_5)
            impl_->sound->campaign.true_colors_opening(*impl_->sound);
        if(impl_->initial_scenario==Scenario::bw_zerg_campaign_2 && !count_units(0,7,false)){
            auto& f=*impl_->sound;action_state state;action_functions commands(f.st,state);
            for(auto* u:ptr(f.st.visible_units))if((u->owner==1 || u->owner==2) && (f.unit_is(u,UnitTypes::Terran_Marine) || f.unit_is(u,UnitTypes::Hero_Jim_Raynor_Marine)) && u->order_type->id==Orders::AttackMove){
                unit_t* target=nullptr;int nearest=512*512;
                for(auto* v:ptr(f.st.visible_units))if(f.unit_is(v,UnitTypes::Special_Power_Generator) && !f.unit_dead(v) && !f.u_invincible(v) && f.unit_can_attack_target(u,v)){const auto d=v->sprite->position-u->sprite->position;const int distance=d.x*d.x+d.y*d.y;if(distance<nearest){target=v;nearest=distance;}}
                if(target){commands.action_select(u->owner,u);commands.action_order(u->owner,f.get_order_type(Orders::AttackUnit),target->sprite->position,target,nullptr,false);}
            }
        }
        impl_->ai.step(*impl_->player);
        impl_->battle.step(*impl_->player);
        for(auto* u:ptr(impl_->player->st().player_units[0]))if(u->index<1700){unsigned id=impl_->sound->get_unit_id_32(u).raw_value;int hp=u->hp.raw_value;if(impl_->alert_ids[u->index]==id && impl_->alert_hp[u->index]>hp && u->last_attacking_player!=0){impl_->alert_frame=frame();impl_->alert=u->sprite->position;}impl_->alert_hp[u->index]=hp;impl_->alert_ids[u->index]=id;}
        if(impl_->fog)impl_->memory.update(impl_->player->funcs());
        return true;
    } catch (const std::exception& e) {
        std::snprintf(impl_->message, sizeof impl_->message, "%s", e.what());
        impl_->loaded = false;
        return false;
    }
}
std::size_t Engine::drain_sounds(SoundEvent* out, std::size_t capacity) {
    if(!impl_->sound) return 0;
    auto& s=*impl_->sound;
    const auto n=std::min<std::size_t>(s.count,capacity);
    std::copy_n(s.events.begin(),n,out);
    s.dropped+=s.count-n;s.count=0;return n;
}
unsigned Engine::dropped_sounds() const { return impl_->sound?impl_->sound->dropped:0; }
void Engine::advance_wall_time(std::uint64_t us) {if(ready() && impl_->battle.enabled())impl_->journal.append({frame(),savegame::elapsed,int(uint32_t(us)),int(uint32_t(us>>32))});if(ready())impl_->battle.elapsed(impl_->player->funcs(),us);}
Snapshot Engine::snapshot() const {
    Snapshot s;
    if (!impl_->loaded) return s;
    auto& st = impl_->player->st();
    s.alert_frame=impl_->alert_frame;s.alert_x=impl_->alert.x;s.alert_y=impl_->alert.y;if(impl_->sound->campaign.ping_frame>s.alert_frame){s.alert_frame=impl_->sound->campaign.ping_frame;s.alert_x=impl_->sound->campaign.ping.x;s.alert_y=impl_->sound->campaign.ping.y;}
    s.frame = st.current_frame;
    s.width = st.game->map_tile_width; s.height = st.game->map_tile_height;
    s.minerals = st.current_minerals[0]; s.gas = st.current_gas[0];
    const auto race = static_cast<unsigned>(st.players[0].race);
    s.human_race=race;
    s.units_completed=std::max(0,st.total_non_buildings_ever_completed[0]);
    s.buildings_completed=std::max(0,st.total_buildings_ever_completed[0]);
    s.minerals_gathered=std::max(0,st.total_minerals_gathered[0]);
    s.gas_gathered=std::max(0,st.total_gas_gathered[0]);
    s.supply = st.supply_used[0][race].integer_part();
    s.supply_max = std::min(200,st.supply_available[0][race].integer_part());
    const auto& funcs = impl_->player->funcs();
    if (const auto* u = impl_->first_selected()) {
        s.selected_name = unit_name(u);s.selected_building=funcs.ut_building(u);s.selected_larva=funcs.unit_is(u,UnitTypes::Zerg_Larva);
        s.selected_shields=u->unit_type->has_shield?u->shield_points.integer_part():0;s.selected_max_shields=u->unit_type->has_shield?u->unit_type->shield_points:0;
        s.selected_hp = u->hp.integer_part(); s.selected_max_hp = u->unit_type->hitpoints.integer_part();
        s.queue = u->build_queue.size();
        if(funcs.ut_has_energy(u)){s.selected_energy=u->energy.integer_part();s.selected_max_energy=funcs.unit_max_energy(u).integer_part();}
        if(funcs.ut_building(u) && (funcs.unit_is_researching(u) || funcs.unit_is_upgrading(u)))s.work_frames=u->building.upgrade_research_time;
        s.selection=funcs.ut_worker(u) ? SelectionKind::workers : funcs.unit_is(u,UnitTypes::Terran_Command_Center) ?
            SelectionKind::command_center : funcs.unit_is(u,UnitTypes::Terran_Barracks) ? SelectionKind::barracks : SelectionKind::other;
    }
    for(auto id:impl_->selected) if(auto* u=funcs.get_unit(id)) if(!funcs.unit_dead(u) && (!impl_->fog || player_sees(funcs,u))) ++s.selected;
    s.workers_only=s.selected>0;for(auto id:impl_->selected)if(auto* u=funcs.get_unit(id))if(!funcs.unit_dead(u) && !funcs.ut_worker(u))s.workers_only=false;
    s.campaign=campaign::scenario(unsigned(impl_->initial_scenario));s.scripted=impl_->sound->campaign.active;s.countdown_active=s.scripted && impl_->sound->campaign.countdown_active;s.countdown_seconds=impl_->sound->campaign.countdown();


    if(s.scripted){const auto& c=impl_->sound->campaign;s.next_scenario=c.text(impl_->player->st(),c.next_scenario);s.objectives=c.text(impl_->player->st(),c.objective);s.transmission=frame()<c.message_until?c.text(impl_->player->st(),c.message):"";s.mission_camera_serial=c.camera_serial;s.mission_camera_x=c.camera.x;s.mission_camera_y=c.camera.y;}
    s.observer=impl_->ai.observer() || impl_->battle.enabled(); s.winner=impl_->battle.enabled()?impl_->battle.winner():impl_->ai.winner();if(s.scripted){int result=impl_->player->st().players[0].victory_state;s.winner=result>=3?0:result==2?1:-1;}
    s.benchmark=impl_->battle.enabled();s.battle_seconds=impl_->battle.seconds();s.end_reason=impl_->battle.reason();
    s.score[0]=impl_->battle.score(0);s.score[1]=impl_->battle.score(1);
    for(unsigned p=0;p!=3;++p) {
        auto& v=s.players[p];
        v.scouts=impl_->ai.tactics(p).scouts();v.casts=impl_->ai.tactics(p).casts();v.drop_units=impl_->ai.tactics(p).drops();v.difficulty=impl_->ai.difficulty(p);
        v.minerals=st.current_minerals[p]; v.gas=st.current_gas[p];
        const auto player_race=static_cast<unsigned>(st.players[p].race);
        v.supply=st.supply_used[p][player_race].integer_part(); v.supply_max=std::min(200,st.supply_available[p][player_race].integer_part());
        v.ai=impl_->ai.enabled(p) || (p<2 && impl_->battle.enabled()); v.trained=impl_->ai.trained(p); v.built=impl_->ai.built(p); v.waves=impl_->ai.waves(p); v.lost=p<2 && impl_->battle.enabled()?impl_->battle.lost(p):impl_->ai.lost(p);
        for(const auto& u:st.player_units[p]) if(!funcs.unit_dead(&u) && !funcs.ut_turret(&u)) {
            ++v.live;
            if(funcs.ut_worker(&u)) ++v.workers;
            else if(funcs.ut_building(&u)) ++v.buildings;
            else ++v.army;
        }
    }
    for (unsigned p = 0; p != 12; ++p)
        for (const auto& u : st.player_units[p]) { (void)u; ++s.units; }
    return s;
}
SelectionInfo Engine::selection_info()const {
    SelectionInfo out;if(!ready())return out;const auto* u=impl_->first_selected();if(!u)return out;
    const auto& f=impl_->player->funcs();const auto* t=u->unit_type;const auto snap=snapshot();
    out.valid=true;out.count=snap.selected;out.own=u->owner==0;out.type=unsigned(t->id);out.name=unit_name(u);
    if(f.ut_worker(u) && u->worker.powerup)out.carried_powerup=unsigned(u->worker.powerup->unit_type->id);
    out.building=f.ut_building(u);out.completed=f.u_completed(u);out.hp=snap.selected_hp;out.max_hp=snap.selected_max_hp;
    out.shields=snap.selected_shields;out.max_shields=snap.selected_max_shields;out.energy=snap.selected_energy;out.max_energy=snap.selected_max_energy;
    out.armor=f.unit_armor(u).integer_part();out.kills=u->kill_count;out.armor_level=f.player_upgrade_level(u->owner,t->armor_upgrade);if(t->has_shield)out.shield_level=f.player_upgrade_level(u->owner,UpgradeTypes::Protoss_Plasma_Shields);
    auto add_icon=[&](int icon,int level){
        if(icon<=0)return;
        for(unsigned i=0;i<out.upgrade_count;++i)if(out.upgrades[i].icon==unsigned(icon))return;
        if(out.upgrade_count<out.upgrades.size())out.upgrades[out.upgrade_count++]={unsigned(icon),level};
    };
    auto add_upgrade=[&](UpgradeTypes id,int level){if(id!=UpgradeTypes::None)add_icon(f.get_upgrade_type(id)->icon,level);};
    UpgradeTypes armor_icon=t->armor_upgrade;
    // Static building armor uses a race icon even when no upgrade applies.
    if(out.building && (armor_icon==UpgradeTypes::None || f.get_upgrade_type(armor_icon)->icon<=0)){
        const auto race=f.unit_race(u);
        if(race==race_t::terran)armor_icon=UpgradeTypes::Terran_Vehicle_Plating;
        if(race==race_t::zerg)armor_icon=UpgradeTypes::Zerg_Carapace;
        if(race==race_t::protoss)armor_icon=UpgradeTypes::Protoss_Ground_Armor;
    }
    add_upgrade(armor_icon,out.armor_level);if(t->has_shield)add_upgrade(UpgradeTypes::Protoss_Plasma_Shields,out.shield_level);
    if(const auto* w=f.unit_or_subunit_ground_weapon(u)){out.ground_weapon=true;out.ground_damage=f.weapon_damage_amount(w,u->owner).integer_part();out.ground_level=f.player_upgrade_level(u->owner,w->damage_upgrade);add_icon(w->icon,out.ground_level);}
    if(const auto* w=f.unit_or_subunit_air_weapon(u)){out.air_weapon=true;out.air_damage=f.weapon_damage_amount(w,u->owner).integer_part();out.air_level=f.player_upgrade_level(u->owner,w->damage_upgrade);add_icon(w->icon,out.air_level);}
    out.supply_used=snap.players[u->owner<3?u->owner:0].supply;out.supply_total=snap.players[u->owner<3?u->owner:0].supply_max;out.supply_provided=t->supply_provided.integer_part();
    out.queue_count=std::min<unsigned>(5,u->build_queue.size());for(unsigned i=0;i<out.queue_count;++i)out.queue[i]=unsigned(u->build_queue[i]->id);
    auto unit_work=[&](WorkKind kind,const unit_type_t* type,int remaining){out.work=kind;out.work_icon=unsigned(type->id);out.work_name=zerg::name(type->id);if(!out.work_name)out.work_name=protoss::name(type->id);if(!out.work_name)out.work_name=terran::name(type->id);out.total=type->build_time;out.remaining=remaining;};
    if(f.unit_is_morphing_building(u) || (!out.completed && f.unit_race(u)==race_t::zerg && !out.building)){
        unit_work(WorkKind::morph,u->build_queue.empty()?t:u->build_queue.front(),u->remaining_build_time);
    }else if(!out.completed){unit_work(WorkKind::construction,t,u->remaining_build_time);}
    else if(out.building && f.unit_is_researching(u)){const auto* tech=u->building.researching_type;out.work=WorkKind::research;out.work_icon=tech->icon;out.work_name="Researching";catalogue::techs([&](const terran::Research& entry){if(entry.type==tech->id)out.work_name=entry.name;});out.total=tech->research_time;out.remaining=u->building.upgrade_research_time;}
    else if(out.building && f.unit_is_upgrading(u)){const auto* upgrade=u->building.upgrading_type;out.work=WorkKind::upgrade;out.work_icon=upgrade->icon;out.work_name="Upgrading";catalogue::upgrades([&](const terran::Upgrade& entry){if(entry.type==upgrade->id)out.work_name=entry.name;});out.total=f.upgrade_time_cost(u->owner,upgrade);out.remaining=u->building.upgrade_research_time;}
    else if(f.unit_is_building_addon(u) && u->current_build_unit){unit_work(WorkKind::addon,u->current_build_unit->unit_type,u->current_build_unit->remaining_build_time);}
    else if(out.queue_count){const auto* type=u->build_queue.front();unit_work(WorkKind::training,type,u->current_build_unit && !f.unit_dying(u->current_build_unit)?u->current_build_unit->remaining_build_time:type->build_time);}
    return out;
}
bool Engine::cancel_selected_work(unsigned slot){savegame::Writer::Guard record(impl_->journal,{frame(),savegame::cancel,int(slot)});
    const auto info=selection_info();if(!info.valid || info.count!=1 || !info.own || info.work==WorkKind::none || slot>=5)return false;
    auto* u=impl_->first_selected();action_state state;action_functions a(impl_->player->st(),state);a.action_select(0,u);bool ok=false;
    if(info.work==WorkKind::training){if(slot>=info.queue_count)return false;
        // Native training clears a cancelled head's current_build_unit on the
        // next simulation tick. A second head cancellation before that tick
        // must not erase another paid item without refunding it.
        if(slot==0 && u->current_build_unit && impl_->player->funcs().unit_dying(u->current_build_unit)){impl_->command_message="Cancellation pending next simulation tick";return false;}
        ok=a.action_cancel_build_queue(0,slot);}
    else if(slot)return false;
    else if(info.work==WorkKind::research)ok=a.action_cancel_research(0);
    else if(info.work==WorkKind::upgrade)ok=a.action_cancel_upgrade(0);
    else if(info.work==WorkKind::morph)ok=a.action_cancel_morph(0);
    else if(info.work==WorkKind::addon)ok=a.action_cancel_addon(0);
    else if(info.work==WorkKind::construction)ok=a.action_cancel_building_unit(0);
    impl_->command_message=ok?"Production cancelled":"Cannot cancel this production";return ok;
}
SpriteView Engine::selection_wireframe()const {
    const auto info=selection_info();
    return info.valid?unit_wireframe(info.type,info.hp,info.max_hp,info.shields):SpriteView{};
}
SpriteView Engine::unit_wireframe(unsigned type,int hp,int max_hp,int shields,bool group)const {
    if(type>=228 || !impl_->icons)return {};
    const unsigned shield=shields>0?1:0;
    if(group && type<impl_->icons->group_wireframes.frames.size()){
        const auto& tile=impl_->icons->group_wireframes.frames[type];
        if(tile.size.x && tile.size.y){
            const unsigned damage=hp<max_hp?std::min(8,1+(max_hp-std::max(0,hp))*8/max_hp):0;
            return {0,0,group_wireframe_image,type+228*(damage+9*shield),11,false,false,0,255};
        }
    }
    if(!impl_->icons->has_wireframes)return {};
    const unsigned damage=hp>=max_hp?0:hp>max_hp/3?1:2;
    return {0,0,detail::wireframe_image,type+228*(damage+3*shield),11,false,false,0,255};
}
const char* Engine::error() const { return impl_->message; }
bool Engine::ready() const { return impl_->loaded; }
// Reign of Fire's authored movie was framed for 640x480. Follow its actual
// SCV transport / attacked generators until the rescued Drones begin normal gameplay.
bool Engine::cinematic_focus(int& x,int& y)const{
 if(!ready() || impl_->initial_scenario!=Scenario::bw_zerg_campaign_2 || count_units(0,41,false))return false;
 const auto& f=impl_->player->funcs();
 for(auto* u:ptr(f.st.visible_units))if(!f.unit_dead(u) && f.unit_is(u,UnitTypes::Terran_Dropship))for(auto* passenger:f.loaded_units(u))if(f.unit_is(passenger,UnitTypes::Terran_SCV)){x=u->sprite->position.x;y=u->sprite->position.y;return true;}
 // The retail trigger movie moves the rescued SCV separately from its
 // transport. After the generators fall, follow the delivery Dropship even
 // when it has no native cargo slot; ownership transfers only at touchdown.
 if(!count_units(0,7,false) && !count_units(7,200,false)){
  unit_t* delivery=nullptr;int best=0x7fffffff;
  for(auto* u:ptr(f.st.visible_units))if(!f.unit_dead(u) && f.unit_is(u,UnitTypes::Terran_Dropship)){const auto p=u->sprite->position;const int d=(p.x-1456)*(p.x-1456)+(p.y-3504)*(p.y-3504);if(d<best){best=d;delivery=u;}}
  if(delivery){x=delivery->sprite->position.x;y=delivery->sprite->position.y;return true;}
 }
 for(auto* u:ptr(f.st.visible_units))if(!f.unit_dead(u) && (f.unit_is(u,UnitTypes::Terran_Marine) || f.unit_is(u,UnitTypes::Hero_Jim_Raynor_Marine))){auto* target=u->order_target.unit;if(target && f.unit_is(target,UnitTypes::Special_Power_Generator) && !f.unit_dead(target)){x=target->sprite->position.x;y=target->sprite->position.y;return true;}}
 return false;
}

unsigned Engine::count_units(unsigned owner,unsigned type,bool completed) const {
    if(!ready() || owner>=12 || type>=228)return 0;unsigned n=0;const auto& f=impl_->player->funcs();
    for(auto* u:ptr(f.st.player_units[owner]))if(!f.unit_dead(u) && unsigned(u->unit_type->id)==type && (!completed || f.u_completed(u)))++n;
    return n;
}
bool Engine::researched(unsigned tech) const {return ready() && tech<44 && impl_->player->funcs().player_has_researched(0,static_cast<TechTypes>(tech));}
unsigned Engine::upgrade_level(unsigned type) const {return ready() && type<61?impl_->player->funcs().player_upgrade_level(0,static_cast<UpgradeTypes>(type)):0;}
bool Engine::select(int x, int y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::select,x,y});
    if (!ready()) return false;
    auto& funcs = impl_->player->funcs();
    auto* nearest = hit_unit(funcs,x,y,false,impl_->fog);
    impl_->selected.clear();
    if(nearest) impl_->selected.push_back(funcs.get_unit_id_32(nearest));
    if(!impl_->selected.empty()) {
        auto* u=impl_->first_selected();
        if(u && !u->owner && u->unit_type->first_what_sound) impl_->sound->play_sound(u->unit_type->first_what_sound,{},u,false);
    }
    return !impl_->selected.empty();
}
unsigned Engine::own_unit_at(int x,int y)const{
    if(!ready())return 0;auto& f=impl_->player->funcs();auto* u=hit_unit(f,x,y,true,impl_->fog);return u?f.get_unit_id_32(u).raw_value:0;
}
unsigned Engine::select_similar(int x,int y,int cx,int cy){savegame::Writer::Guard record(impl_->journal,{frame(),savegame::similar,x,y,cx,cy});
    if(!ready())return 0;auto& f=impl_->player->funcs();auto* target=hit_unit(f,x,y,true,impl_->fog);if(!target)return 0;
    if(f.ut_building(target)){select(x,y);return impl_->selected.size();}
    impl_->selected.clear();
    for(auto* u:ptr(f.st.player_units[0])){
        if(u->unit_type!=target->unit_type || f.unit_dead(u) || f.ut_turret(u) || f.us_hidden(u) || (impl_->fog && !player_sees(f,u)))continue;
        const auto pos=u->sprite->position;if(pos.x<cx || pos.y<cy || pos.x>=cx+400 || pos.y>=cy+240)continue;
        if(impl_->selected.size()==192)break;impl_->selected.push_back(f.get_unit_id_32(u));
    }
    impl_->command_message="Matching units on screen selected";return impl_->selected.size();
}
void Engine::clear_selection() {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::clear}); impl_->selected.clear(); }
void Engine::deselect_passengers() {
    if(!ready() || impl_->selected.empty())return;
    const auto& f=impl_->player->funcs();
    const auto end=std::remove_if(impl_->selected.begin(),impl_->selected.end(),[&](auto id){
        const auto* unit=f.get_unit(id);
        return unit && f.u_loaded(unit);
    });
    const auto size=end-impl_->selected.begin();
    if(size==impl_->selected.size())return;
    impl_->journal.append({frame(),savegame::deselect_passengers});
    impl_->selected.resize(size);
}
bool Engine::activate(int x,int y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::activate,x,y});
    if(!ready()) return false;
    if(impl_->ai.observer() || impl_->battle.enabled()) { const bool ok=select(x,y); impl_->command_message=ok?"Observer selection":"Selection cleared"; return ok; }
    auto& f=impl_->player->funcs();
    const auto* current=impl_->first_selected();
    auto* target=hit_unit(f,x,y,true,impl_->fog);
    // A remains context-sensitive for artefacts even if an original trigger
    // gave the item to the human slot. Do not replace the worker selection
    // with the immobile artefact when the player is trying to collect it.
    const auto* item=current && f.ut_worker(current)?hit_unit(f,x,y,false,impl_->fog,true):nullptr;
    const bool pickup=item && f.is_campaign_powerup(item);
    const bool loading=current && target && ((f.unit_provides_space(target) && f.unit_can_load_target(target,current)) || (f.unit_provides_space(current) && f.unit_can_load_target(current,target)));
    const bool repairing=current && target && f.unit_is(current,UnitTypes::Terran_SCV) && (f.ut_mechanical(target) || f.ut_building(target)) && (!f.u_completed(target) || target->hp<target->unit_type->hitpoints);
    const bool healing=current && target && f.unit_is(current,UnitTypes::Terran_Medic) && f.ut_organic(target) && target->hp<target->unit_type->hitpoints;
    const bool gas=current && target && f.ut_worker(current) && f.unit_is_refinery(target);
    const bool recharge=current && target && f.unit_can_use_shield_battery(current,target);
    const bool nydus=current && target && !f.ut_building(current) && f.unit_can_move(current) && f.unit_can_enter_nydus(current,target);
    if(target && !pickup && !loading && !repairing && !healing && !gas && !recharge && !nydus) {
        const bool ok=select(x,y);impl_->command_message=ok?"Unit selected":"Selection cleared";return ok;
    }
    if(!current || current->owner!=0) { const bool ok=select(x,y); impl_->command_message=ok?"Unit selected":"Select a unit first"; return ok; }
    return command(Command::context,x,y);
}
unsigned Engine::select_army(int cx,int cy) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::army,cx,cy});
    impl_->selected.clear();
    if(!ready()) return 0;
    const auto& f=impl_->player->funcs();
    for(const auto* u:ptr(impl_->player->st().visible_units)) {
        if(u->owner!=0 || f.unit_dead(u) || f.ut_turret(u) || !f.u_completed(u) || f.ut_worker(u) || f.ut_building(u) || f.us_hidden(u) || f.unit_is(u,UnitTypes::Zerg_Larva) || f.unit_is(u,UnitTypes::Zerg_Egg) || f.unit_is(u,UnitTypes::Zerg_Cocoon) || f.unit_is(u,UnitTypes::Zerg_Lurker_Egg)) continue;
        const auto p=u->sprite->position;
        if(p.x<cx || p.x>=cx+400 || p.y<cy || p.y>=cy+240) continue;
        if(impl_->selected.size()==192) break;
        impl_->selected.push_back(f.get_unit_id_32(u));
    }
    return impl_->selected.size();
}
unsigned Engine::select_workers(int camera_x,int camera_y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::workers,camera_x,camera_y});
    impl_->selected.clear();
    if(!ready()) return 0;
    const auto& f=impl_->player->funcs();
    for(const auto* u:ptr(impl_->player->st().visible_units)) {
        if(u->owner!=0 || f.unit_dead(u) || !f.u_completed(u) || !f.ut_worker(u) || f.us_hidden(u)) continue;
        const auto pos=u->sprite->position;if(camera_x>=0 && (pos.x<camera_x || pos.x>=camera_x+400 || pos.y<camera_y || pos.y>=camera_y+240))continue;
        if(impl_->selected.size()==192) break;
        impl_->selected.push_back(f.get_unit_id_32(u));
    }
    return impl_->selected.size();
}
bool Engine::store_group(unsigned slot) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::store,int(slot)});
    if(slot>=4 || !ready()) return false;
    impl_->groups[slot].clear();
    const auto& f=impl_->player->funcs();
    for(auto id:impl_->selected) if(auto* u=f.get_unit(id))
        if(!f.unit_dead(u) && (!impl_->fog || player_sees(f,u))) impl_->groups[slot].push_back(id);
    return !impl_->groups[slot].empty();
}
bool Engine::recall_group(unsigned slot,int* x,int* y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::recall,int(slot)});
    if(slot>=4 || !ready()) return false;
    impl_->selected.clear();
    const auto& f=impl_->player->funcs();
    int sx=0,sy=0;
    static_vector<unit_id_32,192> retained;
    for(auto id:impl_->groups[slot]) if(auto* u=f.get_unit(id)) {
        if(f.unit_dead(u) || (u->owner!=0 && !impl_->ai.observer() && !impl_->battle.enabled())) continue;
        retained.push_back(id);
        // Entering a refinery is temporary. Keep this live bookmark while the
        // worker is hidden, but do not select or command an unavailable unit.
        if(f.us_hidden(u) || (impl_->fog && !player_sees(f,u))) continue;
        impl_->selected.push_back(id); sx+=u->sprite->position.x; sy+=u->sprite->position.y;
    }
    impl_->groups[slot]=retained;
    if(impl_->selected.empty()) return false;
    if(x) *x=sx/static_cast<int>(impl_->selected.size());
    if(y) *y=sy/static_cast<int>(impl_->selected.size());
    return true;
}
bool Engine::observer_focus(int& x,int& y) const {
    if(!ready()) return false;
    const auto& st=impl_->player->st(); const auto& f=impl_->player->funcs();
    unsigned active[3]{},n=0;
    for(unsigned p=0;p<3;++p)if(!st.player_units[p].empty())active[n++]=p;
    if(!n)return false;
    // Visit every live base even during fighting, then spend one slot following
    // action/airlifts. Three-sided matches never omit the third base.
    const unsigned slot=(st.current_frame/(24*8))%(n+1);
    if(slot<n){const unit_t* base=nullptr;int best=0x7fffffff;const auto side=active[slot];
        for(const auto* u:ptr(st.player_units[side]))if(!f.unit_dead(u) && f.ut_resource_depot(u) && (!impl_->fog || player_sees(f,u))){auto d=u->sprite->position-st.game->start_locations[side];int dist=d.x*d.x+d.y*d.y;if(dist<best){best=dist;base=u;}}
        if(base){x=base->sprite->position.x;y=base->sprite->position.y;return true;}
    }
    const unit_t* moving=nullptr;
    for(unsigned i=0;i<n;++i){unsigned p=active[(i+st.current_frame/(24*8))%n];for(const auto* u:ptr(st.player_units[p])) {
        if(f.unit_dead(u) || f.ut_building(u) || f.ut_worker(u) || f.ut_turret(u) || f.us_hidden(u) || (impl_->fog && !player_sees(f,u)))continue;
        const auto* target=u->order_target.unit;
        if(target && target->owner<3 && !st.alliances[p][target->owner] && !f.unit_dead(target) && (!impl_->fog || player_sees(f,target))) {
            x=(u->sprite->position.x+target->sprite->position.x)/2;y=(u->sprite->position.y+target->sprite->position.y)/2;return true;
        }
        if(f.u_flying(u) && f.unit_provides_space(u) && !f.loaded_units(u).empty()){x=u->sprite->position.x;y=u->sprite->position.y;return true;}
        if(u->order_type->id==Orders::AttackMove)moving=u;
    }}
    if(moving){x=moving->sprite->position.x;y=moving->sprite->position.y;return true;}
    return false;
}
const char* Engine::command_status() const { return impl_->command_message; }
PlacementView Engine::placement_unit(unsigned id,int x,int y) const {
    PlacementView view;
    if(!ready() || id>=228)return view;
    const auto& f=impl_->player->funcs();const auto* type=f.get_unit_type(static_cast<UnitTypes>(id));
    if(!f.ut_building(type))return view;
    view.width=type->placement_size.x;view.height=type->placement_size.y;
    view.x=(x-view.width/2)/32*32;view.y=(y-view.height/2)/32*32;
    const auto* u=impl_->first_selected();
    if(f.unit_is_refinery(type)) {
        bool found=false;
        for(auto* g:ptr(impl_->player->st().visible_units))if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !f.unit_dead(g) && (!impl_->fog || player_sees(f,g))) {
            auto p=g->sprite->position;if(std::abs(x-p.x)<=64 && std::abs(y-p.y)<=48) {view.x=p.x-view.width/2;view.y=p.y-view.height/2;found=true;break;}
        }
        if(!found && impl_->fog){
            UnitView remembered[64];const auto n=impl_->memory.markers(remembered,64);
            for(std::size_t i=0;i<n;++i)if(remembered[i].type==unsigned(UnitTypes::Resource_Vespene_Geyser) && std::abs(x-remembered[i].x)<=64 && std::abs(y-remembered[i].y)<=48){view.x=remembered[i].x-view.width/2;view.y=remembered[i].y-view.height/2;found=true;break;}
        }
        if(!found)return view;
    }
    view.valid=!impl_->ai.enabled(0) && !impl_->battle.enabled() && u && u->owner==0 && !f.unit_dead(u) &&
        ((f.unit_is_nydus(u) && f.unit_is(type,UnitTypes::Zerg_Nydus_Canal) && !u->building.nydus.exit && f.u_completed(u)) || (f.unit_can_build(u,type) && f.has_available_resources_for(0,type))) && x>=view.width/2 && y>=view.height/2 &&
        f.can_place_building(u,0,type,{view.x+view.width/2,view.y+view.height/2},!impl_->fog,!impl_->fog);
    return view;
}
PlacementView Engine::placement(Command kind,int x,int y) const {
    if(kind!=Command::depot && kind!=Command::barracks && kind!=Command::refinery)return {};
    return placement_unit(static_cast<unsigned>(kind==Command::depot?UnitTypes::Terran_Supply_Depot:kind==Command::refinery?UnitTypes::Terran_Refinery:UnitTypes::Terran_Barracks),x,y);
}
bool Engine::command(Command command, int x, int y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::command,int(command),x,y});
    auto fail = [&](const char* why) { impl_->command_message = why; return false; };
    if (!ready()) return fail("Load game with L first");
    auto& player = *impl_->player;
    auto& funcs = player.funcs();
    auto* u = impl_->first_selected();
    if(impl_->ai.enabled(0) || impl_->battle.enabled()) return fail("Observer mode: AI controls both armies");
    if (!u || u->owner != 0 || funcs.unit_dead(u)) return fail("Select an own unit with A");
    action_state actions;
    action_functions commands(player.st(), actions);
    if (!commands.action_select(0, u)) return fail("Unit cannot receive commands");
    bool accepted = false;
    const char* message = "Command accepted";
    switch (command) {
    case Command::context:
    case Command::attack:
    case Command::stop:
    case Command::primary: {
        if (x < 0 || y < 0 || x >= static_cast<int>(player.st().game->map_width) ||
            y >= static_cast<int>(player.st().game->map_height)) return fail("Target outside map");
        auto* target = hit_unit(funcs, x, y, false,impl_->fog);
        // A friendly scout or escort may stand directly over the artefact.
        // For workers, the explicit item hit wins over Follow/selecting the
        // overlapping friendly unit; pickup still requires physical contact.
        if(funcs.ut_worker(u))if(auto* item=hit_unit(funcs,x,y,false,impl_->fog,true))target=item;
        if (command == Command::context) {
            const auto* order = funcs.get_default_order(funcs.default_action(u), u, {x,y}, target, nullptr);
            message = "Move / interact / rally";
            if (order) {
                switch (order->id) {
                case Orders::MoveToMinerals: message = "Gather minerals"; break;
                case Orders::MoveToGas: message = "Gather gas"; break;
                case Orders::AttackDefault: message = "Attack target"; break;
                default: break;
                }
            }
        }
        // Issue group actions in native batches of at most twelve. Buildings
        // are single-select; no upstream selection limits are changed.
        static_vector<unit_t*,12> batch;
        auto issue=[&]() {
            if(batch.empty()) return;
            commands.action_select(0,batch);
            if(command==Command::context) accepted=commands.action_default_order(0,{x,y},target,nullptr,false) || accepted;
            else if(command==Command::stop) accepted=commands.action_stop(0,false) || accepted;
            else if(command==Command::primary) {
                accepted=commands.action_siege(0,false) || accepted;
                accepted=commands.action_unsiege(0,false) || accepted;
            } else accepted=commands.action_order(0,commands.get_order_type(Orders::AttackMove),{x,y},nullptr,nullptr,false) || accepted;
            batch.clear();
        };
        for(auto id:impl_->selected) if(auto* member=funcs.get_unit(id)) {
            if(member->owner!=0 || funcs.unit_dead(member)) continue;
            batch.push_back(member); if(batch.size()==12) issue();
        }
        issue();
        if (command != Command::context)
            message=command==Command::stop ? "Stop" : command==Command::primary ? "Siege mode toggled" : "Attack-move to cursor";
        break;
    }
    case Command::train: {
        if (!funcs.unit_is(u, UnitTypes::Terran_Command_Center) && !funcs.unit_is(u, UnitTypes::Terran_Barracks))
            return fail("Select Command Center / Barracks");
        auto* type = funcs.get_unit_type(funcs.unit_is(u, UnitTypes::Terran_Command_Center) ? UnitTypes::Terran_SCV : UnitTypes::Terran_Marine);
        if (!funcs.unit_can_build(u, type)) return fail("Building incomplete / queue full");
        if (!funcs.has_available_resources_for(0, type)) return fail("Not enough minerals / gas");
        if (!funcs.has_available_supply_for(0, type)) return fail("Build more Supply Depots");
        accepted = commands.action_train(0, type); message = "Unit added to training queue"; break;
    }
    case Command::depot:
    case Command::barracks:
    case Command::refinery: {
        auto* type = funcs.get_unit_type(command == Command::depot ? UnitTypes::Terran_Supply_Depot : command == Command::refinery ? UnitTypes::Terran_Refinery : UnitTypes::Terran_Barracks);
        if (!funcs.unit_can_build(u, type)) return fail("Select an available SCV");
        if (!funcs.has_available_resources_for(0, type)) return fail("Not enough minerals / gas");
        const auto site = placement(command,x,y);
        if (!site.valid)
            return fail("Cannot build here: blocked / unexplored");
        accepted = commands.action_build(0, commands.get_order_type(Orders::PlaceBuilding), type,
                                         {static_cast<std::size_t>(site.x/32), static_cast<std::size_t>(site.y/32)});
        message = "SCV moving to construction site"; break;
    }
    case Command::cancel:
        if(funcs.unit_race(u)==race_t::zerg && (!funcs.u_completed(u) || funcs.unit_is_morphing_building(u) || u->order_type->id==Orders::ZergUnitMorph)) accepted=commands.action_cancel_morph(0);
        else if (!funcs.u_completed(u) && funcs.ut_building(u)) accepted = commands.action_cancel_building_unit(0);
        else accepted = commands.action_cancel_build_queue(0, 254);
        message = "Construction / last queued unit cancelled"; break;
    }
    if (!accepted) return fail("Command unavailable for this unit");
    if(u->unit_type->first_yes_sound && !funcs.ut_building(u)) impl_->sound->play_sound(u->unit_type->first_yes_sound,{},u,false);
    impl_->command_message = message;
    return true;
}
unsigned Engine::options(OptionPage page,MenuOption* output,unsigned capacity) const {
    if(!ready())return 0;const auto* u=impl_->first_selected();if(!u || u->owner!=0)return 0;
    const auto& f=impl_->player->funcs();unsigned n=0;
    auto add=[&](const char* name,OptionKind kind,unsigned id,bool target,bool enabled,int minerals=0,int gas=0) {
        if(n>=capacity)return;auto& o=output[n++];o.icon=~0u;o.code=option_code(kind,id);o.target=target;o.enabled=enabled;
        auto order_icon=[&](Orders order){const auto* type=f.get_order_type(order);if(type->highlight>=0)o.icon=unsigned(type->highlight);else if(unsigned(type->tech_type)<44)o.icon=unsigned(f.get_tech_type(type->tech_type)->icon);};
        switch(kind){
        case OptionKind::build:case OptionKind::addon:case OptionKind::train:case OptionKind::morph:o.icon=id;break;
        case OptionKind::research:o.icon=f.get_tech_type(TechTypes(id))->icon;break;
        case OptionKind::upgrade:o.icon=f.get_upgrade_type(UpgradeTypes(id))->icon;break;
        case OptionKind::order:order_icon(id==unsigned(Orders::MoveUnload)?Orders::Unload:Orders(id));if(id==unsigned(Orders::RechargeShieldsBattery))o.icon=unsigned(UnitTypes::Protoss_Shield_Battery);break;
        case OptionKind::stim:o.icon=f.get_tech_type(TechTypes::Stim_Packs)->icon;break;
        case OptionKind::siege:order_icon(f.unit_is(u,UnitTypes::Terran_Siege_Tank_Siege_Mode)?Orders::Unsieging:Orders::Sieging);break;
        case OptionKind::cloak:order_icon(f.u_cloaked(u)?Orders::Decloak:Orders::Cloak);break;
        case OptionKind::burrow:order_icon(f.u_burrowed(u)?Orders::Unburrowing:Orders::Burrowing);break;
        case OptionKind::liftoff:order_icon(Orders::BuildingLiftoff);break;
        case OptionKind::land:order_icon(Orders::BuildingLand);break;
        case OptionKind::unload:order_icon(Orders::Unload);break;
        case OptionKind::merge:o.icon=unsigned(f.get_tech_type(id?TechTypes::Dark_Archon_Meld:TechTypes::Archon_Warp)->icon);break;
        case OptionKind::larvae:o.icon=unsigned(UnitTypes::Zerg_Larva);break;
        case OptionKind::nydus:o.icon=unsigned(UnitTypes::Zerg_Nydus_Canal);break;
        default:break;
        }
        if(minerals || gas)std::snprintf(o.label,sizeof o.label,"%s %s %d/%d",enabled?"":"x",name,minerals,gas);
        else std::snprintf(o.label,sizeof o.label,"%s%s",enabled?"":"x ",name);
    };
    if(f.unit_race(u)==race_t::zerg && !f.u_completed(u)){add("Cancel morph / construction",OptionKind::cancel,0,false,true);return n;}
    if(page==OptionPage::basic || page==OptionPage::advanced) {
        unsigned index=0;catalogue::buildings([&](const terran::Product& b){
            if(b.parent!=u->unit_type->id)return;
            const auto* t=f.get_unit_type(b.type);if(f.ut_addon(t))return;
            if((page==OptionPage::basic)==(index++<9))add(b.name,OptionKind::build,unsigned(b.type),true,f.unit_can_build(u,t) && f.has_available_resources_for(0,t),t->mineral_cost,t->gas_cost);
        });if(page==OptionPage::advanced && f.unit_is(u,UnitTypes::Zerg_Drone))add(f.u_burrowed(u)?"Unburrow":"Burrow",OptionKind::burrow,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Burrowing)));return n;
    }
    if(f.unit_is_hatchery(u))add("Select Larva",OptionKind::larvae,0,false,true);
    if(f.ut_building(u) || f.unit_is_carrier(u) || f.unit_is_reaver(u)) {
        catalogue::units([&](const terran::Product& b){if(b.parent==u->unit_type->id && page==OptionPage::primary) {
            const auto* t=f.get_unit_type(b.type);add(b.name,OptionKind::train,unsigned(b.type),false,f.unit_can_build(u,t) && f.has_available_resources_for(0,t) && f.has_available_supply_for(0,t),t->mineral_cost,t->gas_cost);
        }
        });catalogue::buildings([&](const terran::Product& b){if(b.parent==u->unit_type->id && page==OptionPage::primary) {
            const auto* t=f.get_unit_type(b.type);add(b.name,f.unit_race(u)==race_t::zerg?OptionKind::morph:OptionKind::addon,unsigned(b.type),false,f.unit_can_build(u,t) && f.has_available_resources_for(0,t),t->mineral_cost,t->gas_cost);
        }
        });catalogue::techs([&](const terran::Research& t){if(t.parent==u->unit_type->id) {
            const auto* type=f.get_tech_type(t.type);add(t.name,OptionKind::research,unsigned(t.type),false,f.unit_can_research(u,type,0) && f.has_available_resources_for(0,type,false),type->mineral_cost,type->gas_cost);
        }
        });catalogue::upgrades([&](const terran::Upgrade& t){if(zerg::parent(u->unit_type->id,t.parent)) {
            const auto* type=f.get_upgrade_type(t.type);char label[40];std::snprintf(label,sizeof label,"%s L%d",t.name,f.player_upgrade_level(0,t.type)+1);
            add(label,OptionKind::upgrade,unsigned(t.type),false,f.unit_can_upgrade(u,type,0) && f.has_available_resources_for(0,type,false),f.upgrade_mineral_cost(0,type),f.upgrade_gas_cost(0,type));
        }
        });if(page==OptionPage::secondary) {
            if(u->unit_type->id==UnitTypes::Terran_Command_Center || u->unit_type->id==UnitTypes::Terran_Barracks || u->unit_type->id==UnitTypes::Terran_Factory || u->unit_type->id==UnitTypes::Terran_Starport || u->unit_type->id==UnitTypes::Terran_Science_Facility || u->unit_type->id==UnitTypes::Terran_Engineering_Bay)
                add(f.u_flying(u)?"Land at cursor":"Lift off",f.u_flying(u)?OptionKind::land:OptionKind::liftoff,0,f.u_flying(u),f.u_completed(u));
            if(f.unit_provides_space(u))add("Unload all",OptionKind::unload,0,false,!f.loaded_units(u).empty());
            add("Cancel work/queue",OptionKind::cancel,0,false,true);
            if(!f.ut_building(u)){add("Attack-move",OptionKind::order,unsigned(Orders::AttackMove),true,true);add("Stop",OptionKind::order,unsigned(Orders::Stop),false,true);}
        }
        if(f.unit_is(u,UnitTypes::Zerg_Nydus_Canal))add("Build Nydus exit",OptionKind::nydus,unsigned(UnitTypes::Zerg_Nydus_Canal),true,!u->building.nydus.exit && f.u_completed(u));
        if(f.unit_is(u,UnitTypes::Protoss_Shield_Battery))add("Recharge shields",OptionKind::order,unsigned(Orders::RechargeShieldsBattery),true,f.u_completed(u) && !f.unit_is_disabled(u) && u->energy>0_fp8);
        if(u->unit_type->id==UnitTypes::Terran_Comsat_Station)add("Scanner Sweep (50 energy)",OptionKind::order,unsigned(Orders::CastScannerSweep),true,f.u_completed(u) && !f.unit_is_disabled(u) && u->energy>=fp8::integer(50));
        return n;
    }
    unsigned product_index=0;
    catalogue::units([&](const terran::Product& b){if(b.parent!=u->unit_type->id)return;
        const unsigned index=product_index++;if((page==OptionPage::primary)!=(index<9))return;
        const auto* t=f.get_unit_type(b.type);add(b.name,OptionKind::morph,unsigned(b.type),false,f.unit_can_build(u,t) && f.has_available_resources_for(0,t) && f.has_available_supply_for(0,t),t->mineral_cost,t->gas_cost);
    });
    if(f.unit_is(u,UnitTypes::Zerg_Larva))return n;
    if(f.ut_can_burrow(u))add(f.u_burrowed(u)?"Unburrow":"Burrow",OptionKind::burrow,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Burrowing)));
    auto ability=[&](const char* name,Orders order,TechTypes tech) {
        const auto* type=f.get_tech_type(tech);char label[64];if(type->energy_cost)std::snprintf(label,sizeof label,"%s (%u energy)",name,unsigned(type->energy_cost));else if(tech==TechTypes::Spider_Mines)std::snprintf(label,sizeof label,"%s (%u left)",name,unsigned(f.unit_spider_mine_count(u)));else std::snprintf(label,sizeof label,"%s",name);
        bool enabled=false;
        for(auto id:impl_->selected)if(const auto* caster=f.get_unit(id))if(caster->owner==0 && caster->unit_type==u->unit_type && !f.unit_dead(caster) && f.unit_can_use_tech(caster,type) && caster->energy>=fp8::integer(type->energy_cost))enabled=true;
        const auto index=n;add(label,OptionKind::order,unsigned(order),true,enabled);if(n>index)output[index].icon=unsigned(type->icon);
    };
    auto transport_options=[&](){
        const bool can_unload=!f.loaded_units(u).empty() && f.unit_can_receive_order(u,f.get_order_type(Orders::Unload),0);
        add("Load unit",OptionKind::order,unsigned(Orders::PickupTransport),true,f.unit_provides_space(u));
        add("Unload all",OptionKind::unload,0,false,can_unload);
        add("Unload at target",OptionKind::order,unsigned(Orders::MoveUnload),true,can_unload && f.u_flying(u));
    };
    switch(u->unit_type->id) {
    case UnitTypes::Zerg_Queen:case UnitTypes::Hero_Matriarch:
        ability("Parasite",Orders::CastParasite,TechTypes::Parasite);ability("Spawn Broodlings",Orders::CastSpawnBroodlings,TechTypes::Spawn_Broodlings);ability("Ensnare",Orders::CastEnsnare,TechTypes::Ensnare);ability("Infest Command Center",Orders::CastInfestation,TechTypes::Infestation);break;
    case UnitTypes::Zerg_Defiler:case UnitTypes::Hero_Unclean_One:
        ability("Dark Swarm",Orders::CastDarkSwarm,TechTypes::Dark_Swarm);ability("Plague",Orders::CastPlague,TechTypes::Plague);ability("Consume",Orders::CastConsume,TechTypes::Consume);break;
    case UnitTypes::Zerg_Overlord:case UnitTypes::Hero_Yggdrasill:transport_options();break;
    case UnitTypes::Protoss_High_Templar:
        ability("Psionic Storm",Orders::CastPsionicStorm,TechTypes::Psionic_Storm);ability("Hallucination",Orders::CastHallucination,TechTypes::Hallucination);
        add("Merge nearest Templar",OptionKind::merge,0,false,true);break;
    case UnitTypes::Hero_Tassadar:
        ability("Psionic Storm",Orders::CastPsionicStorm,TechTypes::Psionic_Storm);ability("Hallucination",Orders::CastHallucination,TechTypes::Hallucination);break;
    case UnitTypes::Hero_Infested_Kerrigan:
        ability("Psionic Storm",Orders::CastPsionicStorm,TechTypes::Psionic_Storm);ability("Ensnare",Orders::CastEnsnare,TechTypes::Ensnare);ability("Consume",Orders::CastConsume,TechTypes::Consume);
        add("Toggle Cloak",OptionKind::cloak,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Personnel_Cloaking)));break;
    case UnitTypes::Protoss_Dark_Templar:add("Merge nearest Dark Templar",OptionKind::merge,1,false,true);break;
    case UnitTypes::Protoss_Dark_Archon:
        ability("Feedback",Orders::CastFeedback,TechTypes::Feedback);ability("Mind Control",Orders::CastMindControl,TechTypes::Mind_Control);ability("Maelstrom",Orders::CastMaelstrom,TechTypes::Maelstrom);break;
    case UnitTypes::Protoss_Arbiter:case UnitTypes::Hero_Danimoth:ability("Recall",Orders::CastRecall,TechTypes::Recall);ability("Stasis Field",Orders::CastStasisField,TechTypes::Stasis_Field);break;
    case UnitTypes::Protoss_Corsair:case UnitTypes::Hero_Raszagal:ability("Disruption Web",Orders::CastDisruptionWeb,TechTypes::Disruption_Web);break;
    case UnitTypes::Protoss_Shuttle:transport_options();break;
    case UnitTypes::Terran_Marine:case UnitTypes::Hero_Jim_Raynor_Marine:case UnitTypes::Terran_Firebat:case UnitTypes::Hero_Gui_Montag:add("Stim Packs",OptionKind::stim,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Stim_Packs)));break;
    case UnitTypes::Terran_Siege_Tank_Tank_Mode:case UnitTypes::Terran_Siege_Tank_Siege_Mode:case UnitTypes::Hero_Edmund_Duke_Tank_Mode:case UnitTypes::Hero_Edmund_Duke_Siege_Mode:add("Toggle Siege Mode",OptionKind::siege,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Tank_Siege_Mode)));break;
    case UnitTypes::Terran_Vulture:case UnitTypes::Hero_Jim_Raynor_Vulture:ability("Spider Mine",Orders::PlaceMine,TechTypes::Spider_Mines);break;
    case UnitTypes::Terran_Wraith:case UnitTypes::Hero_Tom_Kazansky:add("Toggle Cloak",OptionKind::cloak,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Cloaking_Field)));break;
    case UnitTypes::Terran_Ghost:case UnitTypes::Hero_Sarah_Kerrigan:case UnitTypes::Hero_Samir_Duran:case UnitTypes::Hero_Infested_Duran:case UnitTypes::Hero_Alexei_Stukov:
        add("Toggle Cloak",OptionKind::cloak,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Personnel_Cloaking)));
        ability("Lockdown",Orders::CastLockdown,TechTypes::Lockdown);add("Nuclear Strike",OptionKind::order,unsigned(Orders::NukePaint),true,true);if(f.unit_is(u,UnitTypes::Hero_Infested_Duran))ability("Consume",Orders::CastConsume,TechTypes::Consume);break;
    case UnitTypes::Terran_Medic:ability("Heal",Orders::MedicHeal,TechTypes::Healing);ability("Restoration",Orders::CastRestoration,TechTypes::Restoration);ability("Optical Flare",Orders::CastOpticalFlare,TechTypes::Optical_Flare);break;
    case UnitTypes::Terran_Science_Vessel:case UnitTypes::Hero_Magellan:ability("Defensive Matrix",Orders::CastDefensiveMatrix,TechTypes::Defensive_Matrix);ability("EMP Shockwave",Orders::CastEMPShockwave,TechTypes::EMP_Shockwave);ability("Irradiate",Orders::CastIrradiate,TechTypes::Irradiate);break;
    case UnitTypes::Terran_Battlecruiser:case UnitTypes::Hero_Hyperion:case UnitTypes::Hero_Norad_II:case UnitTypes::Hero_Gerard_DuGalle:ability("Yamato Gun",Orders::FireYamatoGun,TechTypes::Yamato_Gun);break;
    case UnitTypes::Terran_Dropship:transport_options();break;
    default:break;
    }
    if(page==OptionPage::secondary) {
        add("Attack-move",OptionKind::order,unsigned(Orders::AttackMove),true,true);
        add("Hold position",OptionKind::order,unsigned(Orders::HoldPosition),false,true);
        add("Stop",OptionKind::order,unsigned(Orders::Stop),false,true);
    }
    return n;
}
bool Engine::perform(unsigned code,int x,int y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::perform,int(code),x,y});
    auto fail=[&](const char* m){impl_->command_message=m;return false;};
    if(!ready())return fail("Load a scenario first");
    if(impl_->ai.enabled(0) || impl_->battle.enabled())return fail("Observer: AI controls both armies");
    const auto kind=static_cast<OptionKind>(code>>16);const unsigned id=code&65535;
    struct SelectionRestore{Impl* p;static_vector<unit_id_32,192> saved;bool restore=false;~SelectionRestore(){if(restore)p->selected=saved;}} selection{impl_.get(),impl_->selected,false};
    if(impl_->selected.size()>1 && (kind==OptionKind::merge || kind==OptionKind::morph || kind==OptionKind::train || kind==OptionKind::stim || kind==OptionKind::siege || kind==OptionKind::cloak || kind==OptionKind::order || kind==OptionKind::unload || kind==OptionKind::burrow)){
        unit_id_32 chosen{};for(auto uid:selection.saved){impl_->selected.clear();impl_->selected.push_back(uid);MenuOption rows[32];unsigned count=options(OptionPage::primary,rows,32);for(unsigned i=0;i<count;++i)if(rows[i].code==code && rows[i].enabled){chosen=uid;break;}if(chosen.raw_value)break;}
        impl_->selected=selection.saved;if(chosen.raw_value && !impl_->selected.empty() && impl_->selected.front().raw_value!=chosen.raw_value){auto it=std::find(impl_->selected.begin(),impl_->selected.end(),chosen);std::rotate(impl_->selected.begin(),it,it+1);selection.restore=true;}
    }
    auto* u=impl_->first_selected();if(!u || u->owner!=0)return fail("Select an own unit/building");
    auto& p=*impl_->player;auto& f=p.funcs();action_state state;action_functions a(p.st(),state);a.action_select(0,u);bool ok=false;
    if(kind==OptionKind::larvae) {
        if(!f.unit_is_hatchery(u))return fail("Select Hatchery / Lair / Hive");
        for(auto* larva:ptr(p.st().player_units[0]))if(f.unit_is(larva,UnitTypes::Zerg_Larva) && larva->connected_unit==u && !f.unit_dead(larva)) {impl_->selected.clear();impl_->selected.push_back(f.get_unit_id_32(larva));impl_->command_message="Larva selected: Y / L+Y morph menus";return true;}
        return fail("No available Larva");
    } else if(kind==OptionKind::morph) {
        const auto* type=f.get_unit_type(static_cast<UnitTypes>(id));bool allowed=false;
        catalogue::units([&](const terran::Product& r){if(unsigned(r.type)==id && r.parent==u->unit_type->id)allowed=true;});
        catalogue::buildings([&](const terran::Product& r){if(unsigned(r.type)==id && r.parent==u->unit_type->id)allowed=true;});
        if(!allowed || f.unit_race(u)!=race_t::zerg)return fail("Invalid Zerg morph recipe");
        if(!f.unit_can_build(u,type) || !f.has_available_resources_for(0,type) || !f.has_available_supply_for(0,type))return fail("Morph needs prerequisites, resources or supply");
        ok=f.ut_building(u)?a.action_morph_building(0,type):a.action_morph(0,type);
    } else if(kind==OptionKind::nydus){
        if(!f.unit_is_nydus(u))return fail("Select Nydus Canal");const auto place=placement_unit(unsigned(UnitTypes::Zerg_Nydus_Canal),x,y);
        if(!place.valid)return fail("Nydus exit needs free creep");ok=a.action_build(0,a.get_order_type(Orders::BuildNydusExit),u->unit_type,{size_t(place.x/32),size_t(place.y/32)});
    } else if(kind==OptionKind::build || kind==OptionKind::addon || kind==OptionKind::train) {
        const terran::Product* recipe=nullptr;
        catalogue::buildings([&](const terran::Product& r){if(unsigned(r.type)==id && r.parent==u->unit_type->id)recipe=&r;});
        if(kind==OptionKind::train) {recipe=nullptr;catalogue::units([&](const terran::Product& r){if(unsigned(r.type)==id && r.parent==u->unit_type->id)recipe=&r;});}
        if(!recipe)return fail("This unit cannot make that product");
        const auto* type=f.get_unit_type(recipe->type);
        if(!f.unit_can_build(u,type))return fail("Requires prerequisites, addon, or free queue");
        if(!f.has_available_resources_for(0,type))return fail("Not enough minerals / gas");
        if(kind==OptionKind::train) {
            if(!f.has_available_supply_for(0,type))return fail("Build more Supply Depots");
            ok=(f.unit_is_carrier(u) || f.unit_is_reaver(u))?a.action_train_fighter(0):a.action_train(0,type);
        } else if(kind==OptionKind::addon) {
            const auto pos=u->sprite->position-u->unit_type->placement_size/2+type->addon_position+type->placement_size/2;
            if(!f.can_place_building(u,0,type,pos,true,true))return fail("Addon area is blocked");
            const auto tile=(pos-type->placement_size/2)/32;
            const auto* order=a.get_order_type(Orders::PlaceAddon);
            if(!f.unit_build_order_valid(u,order,type,0))return fail("Addon cannot be started now");
            // The parent already occupies its ground footprint. Rechecking that
            // footprint via action_build rejects the SCV which just finished it.
            // Keep native addon placement/clearance and the native construction
            // order; only flying parents need action_build's relocation path.
            if(f.u_grounded_building(u)) {f.place_building(u,order,type,u->sprite->position);ok=true;}
            else ok=a.action_build(0,order,type,{size_t(tile.x),size_t(tile.y)});
        } else {
            const auto place=placement_unit(id,x,y);if(!place.valid)return fail("Invalid placement / occupied terrain");
            ok=a.action_build(0,a.get_order_type(f.unit_race(type)==race_t::protoss?Orders::PlaceProtossBuilding:f.unit_race(type)==race_t::zerg?Orders::DroneStartBuild:Orders::PlaceBuilding),type,{size_t(place.x/32),size_t(place.y/32)});
        }
    } else if(kind==OptionKind::research) {
        bool allowed=false;catalogue::techs([&](const terran::Research& t){if(unsigned(t.type)==id && t.parent==u->unit_type->id)allowed=true;});
        if(!allowed)return fail("This building cannot research that technology");
        ok=a.action_research(0,a.get_tech_type(static_cast<TechTypes>(id)));
    } else if(kind==OptionKind::upgrade) {
        bool allowed=false;catalogue::upgrades([&](const terran::Upgrade& t){if(unsigned(t.type)==id && zerg::parent(u->unit_type->id,t.parent))allowed=true;});
        if(!allowed)return fail("This building cannot perform that upgrade");
        ok=a.action_upgrade(0,a.get_upgrade_type(static_cast<UpgradeTypes>(id)));
    } else if(kind==OptionKind::merge) {
        const auto type=id?UnitTypes::Protoss_Dark_Templar:UnitTypes::Protoss_High_Templar;
        if(!f.unit_is(u,type))return fail("Select the matching Templar");
        unit_t* other=nullptr;int distance=0x7fffffff;
        for(auto* candidate:ptr(p.st().player_units[0]))if(candidate!=u && f.unit_is(candidate,type) && !f.unit_dead(candidate) && !f.us_hidden(candidate)) {
            const auto d=candidate->sprite->position-u->sprite->position;const int dd=d.x*d.x+d.y*d.y;
            if(dd<distance){distance=dd;other=candidate;}
        }
        if(!other)return fail("Requires two matching Templar");
        a.action_select(0,std::array<unit_t*,2>{{u,other}});
        ok=id?a.action_morph_dark_archon(0):a.action_morph_archon(0);
    } else if(kind==OptionKind::cancel) {
        if(f.unit_is_researching(u))ok=a.action_cancel_research(0);
        else if(f.unit_is_upgrading(u))ok=a.action_cancel_upgrade(0);
        else if(f.unit_is_building_addon(u))ok=a.action_cancel_addon(0);
        else return command(Command::cancel,x,y);
    } else if(kind==OptionKind::liftoff)ok=a.action_liftoff(0,u->sprite->position);
    else if(kind==OptionKind::land) {
        const auto* type=u->unit_type;const xy pos={(x-type->placement_size.x/2)/32*32+type->placement_size.x/2,(y-type->placement_size.y/2)/32*32+type->placement_size.y/2};
        if(!f.u_flying(u) || !f.can_place_building(u,0,type,pos,true,true))return fail("Cannot land here");
        f.issue_order(u,false,a.get_order_type(Orders::BuildingLand),{pos});ok=true;
    } else {
        // Apply immediate abilities to each native single-unit selection. Group
        // movement stays in the existing batches of twelve.
        for(auto uid:impl_->selected)if(auto* member=f.get_unit(uid)) {
            if(member->owner!=0 || f.unit_dead(member) || f.us_hidden(member))continue;
            a.action_select(0,member);
            if(kind==OptionKind::burrow)ok=(f.u_burrowed(member)?a.action_unburrow(0):a.action_burrow(0,false))||ok;
            else if(kind==OptionKind::stim)ok=a.action_stim_pack(0)||ok;
            else if(kind==OptionKind::siege)ok=(f.unit_is_sieged_tank(member)?a.action_unsiege(0,false):a.action_siege(0,false))||ok;
            else if(kind==OptionKind::cloak) {
                const auto tech=(f.unit_is_ghost(member) || f.unit_is(member,UnitTypes::Hero_Infested_Kerrigan))?TechTypes::Personnel_Cloaking:TechTypes::Cloaking_Field;
                if(!f.unit_can_use_tech(member,f.get_tech_type(tech)))continue;
                if(f.u_requires_detector(member))a.action_decloak(0);else if(member->energy>=fp8::integer(f.get_tech_type(tech)->energy_cost))a.action_cloak(0);else continue;
                ok=true; // Native action_cloak changes state but returns false.
            } else if(kind==OptionKind::unload)ok=a.action_unload_all(0,false)||ok;
            else if(kind==OptionKind::order && id<unsigned(Orders::None)) {
                const auto order=static_cast<Orders>(id);
                if(order==Orders::MoveUnload && (!f.u_flying(member) || f.loaded_units(member).empty()))continue;
                const bool ground=order==Orders::CastDarkSwarm || order==Orders::CastPlague || order==Orders::CastEnsnare || order==Orders::CastPsionicStorm || order==Orders::CastStasisField || order==Orders::CastRecall || order==Orders::CastMaelstrom || order==Orders::CastDisruptionWeb || order==Orders::PlaceMine || order==Orders::CastEMPShockwave || order==Orders::CastScannerSweep || order==Orders::NukePaint || order==Orders::AttackMove || order==Orders::Move || order==Orders::Patrol || order==Orders::MoveUnload;
                auto* target=ground?nullptr:hit_unit(f,x,y,false,impl_->fog);
                ok=a.action_order(0,a.get_order_type(static_cast<Orders>(id)),{x,y},target,nullptr,false)||ok;
            }
        }
    }
    if(!ok)return fail("Unavailable: prerequisites, resources, energy or target");
    impl_->command_message="Command accepted";return true;
}
bool Engine::move_selected(int x, int y) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::move,x,y});
    if (!ready() || impl_->ai.enabled(0) || impl_->battle.enabled()) return false;
    auto& player = *impl_->player;
    auto& funcs = player.funcs();
    auto* u = impl_->first_selected();
    if (!u || u->owner != 0 || funcs.unit_dead(u)) return false;
    if (x < 0 || y < 0 || x >= static_cast<int>(player.st().game->map_width) ||
        y >= static_cast<int>(player.st().game->map_height)) return false;
    action_state actions;
    action_functions commands(player.st(), actions);
    commands.action_select(0, u);
    return commands.action_order(0, commands.get_order_type(Orders::Move), {x, y}, nullptr, nullptr, false);
}
UnitView Engine::Impl::unit_view(const unit_t* u,bool is_selected) const {
        const auto& funcs=player->funcs();
        const auto* t = u->unit_type;
        unsigned cargo=0;for(auto* passenger:funcs.loaded_units(u)){(void)passenger;++cargo;}
        UnitView view ={u->sprite->position.x, u->sprite->position.y, static_cast<unsigned>(u->owner), is_selected,
                       static_cast<unsigned>(t->id), u->hp.integer_part(), t->hitpoints.integer_part(),
                       t->dimensions.from.x + t->dimensions.to.x + 1, t->dimensions.from.y + t->dimensions.to.y + 1,
                       funcs.u_completed(u), funcs.ut_worker(u), funcs.unit_is_mineral_field(u),funcs.unit_is(u,UnitTypes::Resource_Vespene_Geyser),funcs.unit_is_refinery(u),funcs.u_flying(u),cargo,t->has_shield?u->shield_points.integer_part():0,t->has_shield?int(t->shield_points):0,unsigned(funcs.unit_is_carrier(u)?funcs.unit_interceptor_count(u):funcs.unit_is_reaver(u)?funcs.unit_scarab_count(u):0),unsigned(u->order_type->id),u->energy.integer_part(),funcs.u_burrowed(u),funcs.ut_worker(u) && u->worker.powerup?unsigned(u->worker.powerup->unit_type->id):0,funcs.u_disabled(u)};
        if(is_selected){
            const auto* sprite=u->sprite->sprite_type;
            const unsigned ring=first_selection_ring+sprite->selection_circle;
            const auto& grps=player->st().global->image_grp;
            const int ring_height=ring<=last_selection_ring && grps[ring] && !grps[ring]->frames.empty()
                ?int(grps[ring]->frames[0].size.y):view.height;
            view.bar_width=std::max(19,sprite->health_bar_size);
            view.bar_y=u->sprite->position.y+sprite->selection_circle_vpos+ring_height/2+6;
            if(funcs.ut_has_energy(u))view.max_energy=funcs.unit_max_energy(u).integer_part();
            else if(funcs.u_hallucination(u) || funcs.unit_is(u,UnitTypes::Zerg_Broodling)){
                view.max_energy=funcs.default_remove_timer(u);
                view.energy=u->remove_timer;
            }
        }
        return view;
}
std::size_t Engine::visible_units(UnitView* output, std::size_t capacity) const {
    if(!ready())return 0;
    const auto& funcs=impl_->player->funcs();
    std::size_t n=0;
    for(const auto* unit:ptr(impl_->player->st().visible_units)){
        if(n==capacity)break;
        if(funcs.unit_dead(unit) || funcs.ut_turret(unit) || (impl_->fog && !player_sees(funcs,unit)))continue;
        const auto id=funcs.get_unit_id_32(unit);
        const bool selected=std::find(impl_->selected.begin(),impl_->selected.end(),id)!=impl_->selected.end();
        output[n++]=impl_->unit_view(unit,selected);
    }
    return n;
}
std::size_t Engine::selected_units(UnitView* output,std::size_t capacity) const {
    if(!ready())return 0;
    const auto& funcs=impl_->player->funcs();
    std::size_t n=0;
    for(auto id:impl_->selected){
        if(n==capacity)break;
        const auto* unit=funcs.get_unit(id);
        if(!unit || funcs.unit_dead(unit) || funcs.us_hidden(unit) || (impl_->fog && !player_sees(funcs,unit)))continue;
        output[n++]=impl_->unit_view(unit,true);
    }
    return n;
}
std::uint32_t Engine::terrain_signature() const {
    if (!ready()) return 0;
    const auto& st = impl_->player->st();
    auto hash = terrain_hash_step(2166136261u, st.game->map_tile_width);
    hash = terrain_hash_step(hash, st.game->map_tile_height);
    hash = terrain_hash_step(hash, st.game->tileset_index);
    for (auto tile : st.tiles_mega_tile_index) hash = terrain_hash_step(hash, tile);
    return hash;
}
std::uint32_t Engine::frame() const {return ready()?impl_->player->st().current_frame:0;}
bool Engine::fog_enabled() const {return ready() && impl_->fog;}
void Engine::set_fog_enabled(bool enabled) {savegame::Writer::Guard record(impl_->journal,{frame(),savegame::fog,int(enabled)});impl_->fog=enabled;if(impl_->sound)impl_->sound->fog=enabled;if(enabled && ready())impl_->memory.update(impl_->player->funcs());}
std::size_t Engine::remembered_units(UnitView* output,std::size_t capacity)const {return fog_enabled()?impl_->memory.markers(output,capacity):0;}
FogCell Engine::fog_cell(unsigned x,unsigned y) const {
    if(!ready())return FogCell::unexplored;
    const auto& st=impl_->player->st();if(x>=st.game->map_tile_width || y>=st.game->map_tile_height)return FogCell::unexplored;
    if(!impl_->fog)return FogCell::visible;
    const auto& tile=st.tiles[y*st.game->map_tile_width+x];
    return !(tile.visible&1)?FogCell::visible:!(tile.explored&1)?FogCell::explored:FogCell::unexplored;
}
CursorKind Engine::cursor_kind(int x,int y,bool targeting) const {
    if(!ready())return CursorKind::arrow;
    const auto* unit=hit_unit(impl_->player->funcs(),x,y,false,impl_->fog);
    if(!unit)return targeting?CursorKind::target:CursorKind::arrow;
    if(unit->owner==0)return targeting?CursorKind::target:CursorKind::own;
    if(unit->owner>0 && unit->owner<3)return targeting?CursorKind::target_enemy:CursorKind::enemy;
    return targeting?CursorKind::target_neutral:CursorKind::neutral;
}
SpriteView Engine::resource_icon(unsigned kind)const {
    if(!ready() || !impl_->icons || !impl_->icons->has_resources || kind>2)return {};
    const unsigned race=unsigned(impl_->player->st().players[0].race);
    if(race>2)return {};
    const unsigned frame=kind==0?0:kind==1?1+race:4+race;
    return {0,0,detail::resource_icon_image,frame,11,false,false,0,255};
}
SpriteView Engine::command_icon(unsigned icon,bool enabled)const {return ready() && impl_->icons?impl_->icons->view(icon,enabled):SpriteView{};}
SpriteView Engine::command_button()const {return ready() && impl_->icons?SpriteView{0,0,detail::command_button_image,0,11,false,false,0,255}:SpriteView{};}
SpriteView Engine::cursor_image(CursorKind kind,std::uint64_t elapsed_us) const {
    const auto index=unsigned(kind);if(!ready() || index>=cursor_count)return {};
    const auto& grp=impl_->cursors[index];if(grp.frames.empty())return {};
    const unsigned frame=(elapsed_us/100000)%grp.frames.size();const auto& f=grp.frames[frame];
    return {int(f.offset.x)-64,int(f.offset.y)-64,cursor_image_base+index,frame,11,false,false,0,255};
}
SpriteInfo Engine::sprite_info(unsigned image,unsigned frame) const {
    if(image==65384)return {60,56,frame<110*17};
    if(image==selection_bar_image)return {256,3,frame<6};
    if(!ready())return {};
    if(image==detail::wireframe_image)return impl_->icons && impl_->icons->has_wireframes && frame<228*6?SpriteInfo{64,64,true}:SpriteInfo{};
    if(image==group_wireframe_image)return impl_->icons && frame<228*18 &&
        frame%228<impl_->icons->group_wireframes.frames.size()?SpriteInfo{32,32,true}:SpriteInfo{};
    if(image==detail::resource_icon_image)return impl_->icons && impl_->icons->has_resources && frame<7?SpriteInfo{14,14,true}:SpriteInfo{};
    if(image==detail::command_icon_image)return impl_->icons?impl_->icons->info(frame):SpriteInfo{};
    if(image==detail::command_button_image)return impl_->icons && frame==0?SpriteInfo{48,40,true}:SpriteInfo{};
    if(image>=cursor_image_base && image<cursor_image_base+cursor_count){const auto& grp=impl_->cursors[image-cursor_image_base];if(frame>=grp.frames.size())return {};const auto& f=grp.frames[frame];return {unsigned(f.size.x),unsigned(f.size.y),true};}
    if(impl_->creep && image==detail::creep_tile_image)return frame<16?SpriteInfo{32,32,true}:SpriteInfo{};
    if(impl_->creep && image==detail::creep_edge_image){if(frame>=impl_->creep->edges.frames.size())return {};const auto& r=impl_->creep->edges.frames[frame];return {unsigned(r.size.x),unsigned(r.size.y),true};}
    if(image>=impl_->player->st().global->image_grp.size())return {};
    const auto& grp=impl_->player->st().global->image_grp[image];if(!grp || frame>=grp->frames.size())return {};
    const auto& f=grp->frames[frame];return {unsigned(f.size.x),unsigned(f.size.y),true};
}
bool Engine::sprite_pixels(unsigned image,unsigned frame,unsigned owner,uint16_t* out,std::size_t capacity) const {
    if(image==65384){
        constexpr unsigned portraits=110,pixels=60*56;
        if(capacity<pixels || frame>=portraits*17)return false;
        const auto directory=impl_->initial_root+(impl_->initial_root.empty() || impl_->initial_root.back()!='/'?"/":"")+"portraits/";
        const auto path=directory+std::to_string(frame%portraits);
        auto read=[&](const std::string& name,long offset){
            FILE* f=std::fopen(name.c_str(),"rb");
            if(!f)return false;
            const bool ok=!std::fseek(f,offset,SEEK_SET) && std::fread(out,2,pixels,f)==pixels;
            std::fclose(f);
            return ok;
        };
        if(frame>=portraits && read(path+"-anim.bin",long((frame/portraits-1)*pixels*2)))return true;
        return read(path+".bin",0);
    }

    const auto info=sprite_info(image,frame);if(!info.valid || info.width>512 || info.height>512 || size_t(info.width)*info.height>capacity)return false;
    if(image==selection_bar_image){
        constexpr unsigned colors[][3]={{73,79,84},{65,214,61},{229,189,55},{226,49,42},{65,130,245},{223,156,64}};
        for(unsigned y=0;y<info.height;++y)for(unsigned x=0;x<info.width;++x){
            const unsigned gain=x%3==2?27:y==0?120:y==1?100:63;
            const auto& c=colors[frame];
            out[y*info.width+x]=uint16_t((std::min(255u,c[0]*gain/100)>>3)<<11 |
                (std::min(255u,c[1]*gain/100)>>3)<<6 | (std::min(255u,c[2]*gain/100)>>3)<<1 | 1);
        }
        return true;
    }
    if(image==detail::wireframe_image)return impl_->icons && impl_->icons->wire_pixels(frame,out,capacity);
    if(image==group_wireframe_image)return impl_->icons && impl_->icons->wire_pixels(frame,out,capacity,true);
    if(image==detail::resource_icon_image){std::copy(impl_->icons->resources[frame].begin(),impl_->icons->resources[frame].end(),out);return true;}
    if(image==detail::command_icon_image)return impl_->icons && impl_->icons->pixels(frame,owner==0,out,capacity);
    if(image==detail::command_button_image){std::copy(impl_->icons->button.begin(),impl_->icons->button.end(),out);return true;}
    if(image==detail::creep_tile_image){std::copy(impl_->creep->tiles[frame].begin(),impl_->creep->tiles[frame].end(),out);return true;}
    const bool cursor=image>=cursor_image_base && image<cursor_image_base+cursor_count;
    const auto& st=impl_->player->st();const auto& source=cursor?impl_->cursors[image-cursor_image_base].frames[frame]:image==detail::creep_edge_image?impl_->creep->edges.frames[frame]:st.global->image_grp[image]->frames[frame];
    std::fill_n(out,size_t(info.width)*info.height,0);
    const bool edge=image==detail::creep_edge_image;
    const bool ordinary=!edge && !cursor;
    const auto& type=st.global->image_types.vec[ordinary?image:0];
    const uint16_t* colors=(edge || cursor)?impl_->palette.data():impl_->team_palette[owner<3?owner:3].data();
    const bool ring=image>=first_selection_ring && image<=last_selection_ring;
    const auto color=[&](unsigned index){
        if(ring && index<8)return uint16_t(owner==0?0x3611:owner==11?0xe681:0xd2c5);
        return colors[index];
    };
    if(ordinary && type.modifier==9 && type.color_shift!=5 && type.color_shift>=1 && type.color_shift<=7)colors=impl_->lights[type.color_shift-1].data();
    for(unsigned y=0;y<info.height;++y) {
        const auto* p=source.data_container.data()+source.line_data_offset[y];unsigned x=0;
        while(x<info.width) {
            unsigned token=*p++,n=token&127;if(token&128){x+=n;continue;}
            if(token&64){n=std::min(token&63,info.width-x);const auto c=color(*p++);std::fill_n(out+y*info.width+x,n,c);x+=n;}
            else {const unsigned end=x+std::min(n,info.width-x);while(x<end)out[y*info.width+x++]=color(*p++);}
        }
    }
    // Shadow GRP indices are a coverage mask, not palette colors. Baking
    // black into the cached mask also keeps it independent of GPU tint state.
    if(ordinary && !ring && type.modifier==10)for(size_t i=0;i<size_t(info.width)*info.height;++i)if(out[i]&1)out[i]=1;
    return true;
}
std::size_t Engine::image_types(unsigned* output,std::size_t capacity) const {
    if(!ready()) return 0;
    bool seen[1024]{};
    std::size_t n=0;
    for(const auto& line:impl_->player->st().sprites_on_tile_line) for(const auto& sprite:line)
        for(const auto& img:sprite.images) {
            if(img.modifier<0 || (img.modifier>7 && img.modifier!=9 && img.modifier!=10 && img.modifier!=12)) continue;
            const unsigned id=static_cast<unsigned>(img.image_type->id);
            if(id>=1024 || seen[id]) continue;
            seen[id]=true;
            if(n<capacity) output[n++]=id;
        }
    return n;
}
std::size_t Engine::visible_images(SpriteView* output, std::size_t capacity, int cx, int cy,bool selection_rings) const {
    if (!ready()) return 0;
    const auto& st = impl_->player->st();
    const auto& funcs = impl_->player->funcs();
    std::size_t n = 0;
    if(impl_->creep){
        const int w=st.game->map_tile_width,h=st.game->map_tile_height;
        const xy dirs[]={{1,1},{0,1},{-1,1},{1,0},{-1,0},{1,-1},{0,-1},{-1,-1}};
        auto has=[&](int x,int y){return x>=0 && y>=0 && x<w && y<h && (impl_->fog?impl_->memory.creep(x,y):(st.tiles[x+y*w].flags&tile_t::flag_has_creep));};
        for(int y=std::max(0,cy/32);y<std::min(h,(cy+271)/32);++y)for(int x=std::max(0,cx/32);x<std::min(w,(cx+431)/32);++x){
            if(n==capacity || (impl_->fog && fog_cell(x,y)==FogCell::unexplored))continue;
            if(has(x,y))output[n++]={x*32,y*32,detail::creep_tile_image,detail::creep_variant(x,y),11,false,false,0,255};
            else{unsigned mask=0;for(unsigned d=0;d<8;++d)if(has(x+dirs[d].x,y+dirs[d].y))mask|=1<<d;
                const unsigned frame=impl_->creep->edge_frames[mask];if(frame && frame<=impl_->creep->edges.frames.size()){const auto& r=impl_->creep->edges.frames[frame-1];output[n++]={x*32+int(r.offset.x),y*32+int(r.offset.y),detail::creep_edge_image,frame-1,11,false,false,0,255};}
            }
        }
    }
    if(selection_rings)for(auto id:impl_->selected){
        const auto* unit=funcs.get_unit(id);
        if(!unit || funcs.unit_dead(unit) || funcs.us_hidden(unit) || !unit->sprite ||
           (impl_->fog && !player_sees(funcs,unit)))continue;
        const auto& sprite=*unit->sprite;
        const unsigned image=first_selection_ring+sprite.sprite_type->selection_circle;
        if(image>last_selection_ring || !sprite_info(image,0).valid || n==capacity)continue;
        const auto& grp=*st.global->image_grp[image];
        const auto& frame=grp.frames[0];
        const int x=sprite.position.x+int(frame.offset.x)-int(grp.width/2);
        const int y=sprite.position.y+sprite.sprite_type->selection_circle_vpos+int(frame.offset.y)-int(grp.height/2);
        if(x>=cx+400 || y>=cy+240 || x+int(frame.size.x)<=cx || y+int(frame.size.y)<=cy)continue;
        output[n++]={x,y,image,0,unit->owner==0?0u:unit->owner==11?11u:1u,false,false,1,255};
    }
    // Detection is per unit; visual effects use the native sprite visibility.
    // The fixed native sprite pool bounds this scratch without frame allocation.
    bool undetected[2500];
    if(impl_->fog)std::fill_n(undetected,2500,false);
    if(impl_->fog)for(const auto* unit:ptr(st.visible_units))if(!player_sees(impl_->player->funcs(),unit)){
        if(unit->sprite->index<2500)undetected[unit->sprite->index]=true;
        if(unit->subunit && unit->subunit->sprite->index<2500)undetected[unit->subunit->sprite->index]=true;
    }
    for (const auto& line : st.sprites_on_tile_line) for (const auto& sprite : line) {
        if (sprite.flags & sprite_t::flag_hidden) continue;
        if(impl_->fog && (!(sprite.visibility_flags&1) || (sprite.index<2500 && undetected[sprite.index])))continue;
        unsigned layer = 0;
        for (const auto& img : reverse(sprite.images)) {
            if((img.flags & image_t::flag_hidden) && !(img.modifier>=2 && img.modifier<=7) && !(sprite.flags&sprite_t::flag_burrowed))continue;
            // Keep unsupported distortion effects out of the color pipeline.
            // Fire/glow tables use an additive RGB approximation; trans50
            // images retain source/team colors and blend at half opacity.
            if(img.modifier<0 || (img.modifier>7 && img.modifier!=9 && img.modifier!=10 && img.modifier!=12))continue;
            const auto p = funcs.get_image_map_position(&img);
            const auto& frame = img.grp->frames.at(img.frame_index);
            if (p.x >= cx + 400 || p.y >= cy + 240 || p.x + static_cast<int>(frame.size.x) <= cx ||
                p.y + static_cast<int>(frame.size.y) <= cy) continue;
            if (n == capacity) continue;
            // The Templar afterimage is a separate native iscript sprite,
            // not an extra selectable unit. Preserve its native frames and
            // positions, but soften it instead of drawing a second solid body.
            const bool trail=img.image_type->id==ImageTypes::IMAGEID_High_Templar_Glow;
            const bool glow=img.modifier==9 && img.image_type->color_shift!=5;
            output[n++] = {p.x, p.y, static_cast<unsigned>(img.image_type->id), static_cast<unsigned>(img.frame_index),
                           static_cast<unsigned>(sprite.owner), (img.flags & image_t::flag_horizontally_flipped) != 0,
                           img.modifier == 10,
                           ((static_cast<std::uint64_t>(sprite.elevation_level) * 8192 + sprite.position.y) * 4096 + sprite.index) * 64 + layer++,
                           ((img.modifier>=2 && img.modifier<=7) || (sprite.flags&sprite_t::flag_burrowed))?112u:img.modifier==12?96u:trail?128u:img.modifier==9?(glow?255u:128u):255u,glow};
        }
    }
    // The native pickup removes the artefact unit from the visible/finder
    // lists. Its logical pointer still drives original Bring/rescue triggers,
    // but no attached image was being drawn: the SCV appeared empty-handed.
    // Draw the original artefact image above its carrier without exposing a
    // second selectable unit, moving the hidden unit or mutating game state.
    for(const auto* carrier:ptr(st.visible_units)){
        if(!funcs.ut_worker(carrier) || funcs.unit_dead(carrier) || funcs.us_hidden(carrier) || (impl_->fog && !player_sees(funcs,carrier)))continue;
        const auto* item=carrier->worker.powerup;if(!item || !item->sprite || !funcs.is_campaign_powerup(item) || !funcs.us_hidden(item))continue;
        const auto& sprite=*carrier->sprite;const auto shift=sprite.position-item->sprite->position-xy(0,12);
        unsigned layer=32;
        for(const auto& img:reverse(item->sprite->images)){
            if(img.modifier==10 || img.modifier<0 || img.modifier>7)continue;
            const auto p=funcs.get_image_map_position(&img)+shift;const auto& frame=img.grp->frames.at(img.frame_index);
            if(p.x>=cx+400 || p.y>=cy+240 || p.x+int(frame.size.x)<=cx || p.y+int(frame.size.y)<=cy || n==capacity)continue;
            output[n++]={p.x,p.y,unsigned(img.image_type->id),unsigned(img.frame_index),unsigned(carrier->owner),(img.flags&image_t::flag_horizontally_flipped)!=0,false,((std::uint64_t(sprite.elevation_level)*8192+sprite.position.y)*4096+sprite.index)*64+layer++,255};
        }
    }
    if(impl_->fog && n<capacity)n+=impl_->memory.images(output+n,capacity-n,cx,cy,funcs);
    std::sort(output, output + n, [](const SpriteView& a, const SpriteView& b) { return a.order < b.order; });
    return n;
}
std::size_t state_size() { return sizeof(state); }
std::size_t unit_size() { return sizeof(unit_t); }
bool core_self_test() {
    // PKWARE DCL vector from zlib/contrib/blast/test.pk and test.txt.
    // Exercises the decoder whose uint_fast8_t promoted differently on ARM.
    uint8_t compressed[] = {0x00, 0x04, 0x82, 0x24, 0x25, 0x8f, 0x80, 0x7f};
    uint8_t output[13]{};
    try {
        data_loading::decompress(compressed, sizeof compressed, output, sizeof output);
        if (std::memcmp(output, "AIAIAIAIAIAIA", sizeof output) != 0) return false;
        uint8_t unaligned[] = {0, 0x78, 0x56, 0x34, 0x12};
        if (data_loading::value_at<uint32_t, true>(unaligned + 1) != 0x12345678) return false;
        return fp8::integer(3).raw_value == 768;
    } catch (...) { return false; }
}
#include "hud_engine.h"
#include "save_engine.h"

}
