#include "engine.h"
#include "actions.h"
#include "graphics_format.h"
#include "battle_ai.h"
#include "terran_battle.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace port3ds {
using namespace bwgame;
struct Engine::Impl {
    struct SoundFunctions : state_functions {
        std::array<SoundEvent,256> events{};
        unsigned count=0, dropped=0;
        explicit SoundFunctions(state& st) : state_functions(st) {}
        void play_sound(int id, xy pos, const unit_t*, bool race) override {
            if(id<0) return;
            if(count==events.size()) { ++dropped; return; }
            events[count++]={static_cast<unsigned>(id+(race?int(st.players[0].race):0)),static_cast<int>(pos.x),static_cast<int>(pos.y)};
        }
    };
    std::unique_ptr<game_player> player;
    std::unique_ptr<SoundFunctions> sound;
    char message[192] = "Not loaded";
    bool loaded = false;
    static_vector<unit_id_32,192> selected;
    std::array<static_vector<unit_id_32,192>,4> groups;
    detail::BattleAI ai;
    detail::TerranBattle battle;
    std::array<uint16_t,256> palette{};
    std::array<uint8_t,128> team{};
    std::array<a_vector<uint8_t>,7> lights;
    unit_t* first_selected() const {
        if (!loaded) return nullptr;
        for (auto id:selected) if (auto* u=player->funcs().get_unit(id))
            if (!player->funcs().unit_dead(u)) return u;
        return nullptr;
    }
    const char* command_message = "A select / B act";
};

namespace {
const char* unit_name(const unit_t* u) {if(const char* n=protoss::name(u->unit_type->id))return n;return terran::name(u->unit_type->id);}
// Use the engine collision box with a small thumbstick tolerance. Prefer
// the nearest center when two boxes overlap; never select dead/hidden units.
unit_t* hit_unit(state_functions& funcs, int x, int y, bool own_only) {
    unit_t* best = nullptr;
    int distance = 0x7fffffff;
    for (auto* u : ptr(funcs.st.visible_units)) {
        if (funcs.ut_turret(u) || (own_only && u->owner != 0) || funcs.unit_dead(u) ||
            (u->sprite->flags & sprite_t::flag_hidden)) continue;
        const auto box = funcs.unit_inner_bounding_box(u, u->sprite->position);
        if (x < box.from.x - 6 || y < box.from.y - 6 || x > box.to.x + 6 || y > box.to.y + 6) continue;
        const int dx = u->sprite->position.x - x, dy = u->sprite->position.y - y;
        if (dx * dx + dy * dy < distance) { best = u; distance = dx * dx + dy * dy; }
    }
    return best;
}
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
    if (dims != 1 || width < 32 || height < 32 || width > 64 || height > 64)
        throw std::runtime_error("Use a 32..64 tile map for the original 3DS probe");
}
}

Engine::Engine() : impl_(new Impl) {}
Engine::~Engine() = default;
bool Engine::load(const char* data_root, const char* map_path, Scenario scenario) {
    impl_->loaded = false;
    impl_->battle.reset(scenario==Scenario::terran_battle || scenario==Scenario::protoss_battle,scenario==Scenario::protoss_battle || scenario==Scenario::protoss_lab);
    impl_->selected.clear();
    for (auto& g:impl_->groups) g.clear();
    impl_->ai.reset(scenario==Scenario::ai_battle || scenario==Scenario::gas_battle,
        scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle,scenario==Scenario::gas_battle);
    impl_->sound.reset();
    impl_->player.reset();
    try {
        a_vector<uint8_t> chk;
        const a_string map(map_path);
        if (map.size() >= 4 && map.substr(map.size() - 4) == ".chk") read_file(chk, map);
        else {
            data_loading::mpq_file<> archive(map);
            archive(chk, "staredit/scenario.chk");
        }
        unsigned width = 0, height = 0;
        validate_map(chk, width, height);
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
                read_file(dst, root + path);
            });
        } else {
            auto assets = data_loading::data_files_directory(root);
            player.init(assets);
        }
        game_load_functions loader(player.st());
        loader.load_map_data(chk.data(), chk.size(), [&]() {
            const bool melee=scenario==Scenario::melee || scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle;
            loader.setup_info.create_no_units = !melee;
            loader.setup_info.victory_condition = 0;
            loader.setup_info.resource_type = 1;
            loader.setup_info.starting_minerals = 50;
            for (unsigned i = 0; i != 12; ++i) {
                auto& p = player.st().players[i];
                p.controller = i < 2 ? player_t::controller_occupied : player_t::controller_inactive;
                p.race = (scenario==Scenario::protoss_battle || scenario==Scenario::protoss_lab)?race_t::protoss:race_t::terran;
                loader.setup_info.create_melee_units_for_player[i] = i < 2 && melee;
            }
        });
        // Both slots are human/idle: this probe does not claim a functioning AI.
        if (scenario == Scenario::two_units) {
            auto* type = loader.get_unit_type(UnitTypes::Terran_Marine);
            if (!loader.create_initial_unit(type, {128, 128}, 0) ||
                !loader.create_initial_unit(type, {512, 128}, 1))
                throw std::runtime_error("Could not place the two probe units");
        }
        if(scenario==Scenario::ai_battle || scenario==Scenario::versus_ai || scenario==Scenario::gas_battle) {
            unit_t* bases[2]={nullptr,nullptr};
            for(unsigned p=0;p!=2;++p) for(auto* u:ptr(player.st().player_units[p]))
                if(loader.unit_is(u,UnitTypes::Terran_Command_Center)) bases[p]=u;
            if(!bases[0] || !bases[1] || !loader.is_reachable(bases[0],bases[1]))
                throw std::runtime_error("AI ground battle needs connected start locations");
        }
        if(scenario>=Scenario::terran_battle)impl_->battle.setup(loader,scenario==Scenario::terran_lab || scenario==Scenario::protoss_lab);
        // This no-fog prototype starts with terrain explored for both players.
        // Cloaking, detection and native combat sight ranges remain active.
        for(auto& tile:player.st().tiles)tile.explored&=~3;
        // GRPs are already resident because native iscript/collision requires
        // them. Load only palette tables, then expand frames on GPU-cache miss.
        auto load_asset=[&](a_vector<uint8_t>& dst,const a_string& name) {
            if(exists(root+"arr/units.dat"))read_file(dst,root+name);
            else {auto files=data_loading::data_files_directory(root);files(dst,name);}
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
        for(unsigned i=0;i<7;++i)impl_->lights[i]=pcx(terrain+"/"+light_names[i]+".pcx");
        impl_->sound.reset(new Impl::SoundFunctions(player.st()));
        impl_->loaded = true;
        std::snprintf(impl_->message, sizeof impl_->message, "Loaded %ux%u tiles", width, height);
        return true;
    } catch (const std::exception& e) {
        std::snprintf(impl_->message, sizeof impl_->message, "%s", e.what());
        impl_->player.reset();
        return false;
    }
}
bool Engine::tick() {
    if (!impl_->loaded) return false;
    try {
        impl_->sound->next_frame();
        impl_->ai.step(*impl_->player);
        impl_->battle.step(*impl_->player);
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
void Engine::advance_wall_time(std::uint64_t us) {if(ready())impl_->battle.elapsed(impl_->player->funcs(),us);}
Snapshot Engine::snapshot() const {
    Snapshot s;
    if (!impl_->loaded) return s;
    auto& st = impl_->player->st();
    s.frame = st.current_frame;
    s.width = st.game->map_tile_width; s.height = st.game->map_tile_height;
    s.minerals = st.current_minerals[0]; s.gas = st.current_gas[0];
    const auto race = static_cast<unsigned>(st.players[0].race);
    s.supply = st.supply_used[0][race].integer_part();
    s.supply_max = st.supply_available[0][race].integer_part();
    const auto& funcs = impl_->player->funcs();
    if (const auto* u = impl_->first_selected()) {
        s.selected_name = unit_name(u);
        s.selected_shields=u->shield_points.integer_part();s.selected_max_shields=u->unit_type->shield_points;
        s.selected_hp = u->hp.integer_part(); s.selected_max_hp = u->unit_type->hitpoints.integer_part();
        s.queue = u->build_queue.size();
        if(funcs.ut_has_energy(u)){s.selected_energy=u->energy.integer_part();s.selected_max_energy=funcs.unit_max_energy(u).integer_part();}
        if(funcs.ut_building(u) && (funcs.unit_is_researching(u) || funcs.unit_is_upgrading(u)))s.work_frames=u->building.upgrade_research_time;
        s.selection=funcs.ut_worker(u) ? SelectionKind::workers : funcs.unit_is(u,UnitTypes::Terran_Command_Center) ?
            SelectionKind::command_center : funcs.unit_is(u,UnitTypes::Terran_Barracks) ? SelectionKind::barracks : SelectionKind::other;
    }
    for(auto id:impl_->selected) if(auto* u=funcs.get_unit(id)) if(!funcs.unit_dead(u)) ++s.selected;
    s.observer=impl_->ai.observer() || impl_->battle.enabled(); s.winner=impl_->battle.enabled()?impl_->battle.winner():impl_->ai.winner();
    s.benchmark=impl_->battle.enabled();s.battle_seconds=impl_->battle.seconds();s.end_reason=impl_->battle.reason();
    s.score[0]=impl_->battle.score(0);s.score[1]=impl_->battle.score(1);
    for(unsigned p=0;p!=2;++p) {
        auto& v=s.players[p];
        v.minerals=st.current_minerals[p]; v.gas=st.current_gas[p];
        v.supply=st.supply_used[p][race].integer_part(); v.supply_max=st.supply_available[p][race].integer_part();
        v.ai=impl_->ai.enabled(p) || impl_->battle.enabled(); v.trained=impl_->ai.trained(p); v.built=impl_->ai.built(p); v.waves=impl_->ai.waves(p); v.lost=impl_->battle.enabled()?impl_->battle.lost(p):impl_->ai.lost(p);
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
const char* Engine::error() const { return impl_->message; }
bool Engine::ready() const { return impl_->loaded; }
unsigned Engine::count_units(unsigned owner,unsigned type,bool completed) const {
    if(!ready() || owner>=12 || type>=228)return 0;unsigned n=0;const auto& f=impl_->player->funcs();
    for(auto* u:ptr(f.st.player_units[owner]))if(!f.unit_dead(u) && unsigned(u->unit_type->id)==type && (!completed || f.u_completed(u)))++n;
    return n;
}
bool Engine::researched(unsigned tech) const {return ready() && tech<44 && impl_->player->funcs().player_has_researched(0,static_cast<TechTypes>(tech));}
unsigned Engine::upgrade_level(unsigned type) const {return ready() && type<61?impl_->player->funcs().player_upgrade_level(0,static_cast<UpgradeTypes>(type)):0;}
bool Engine::select(int x, int y) {
    if (!ready()) return false;
    auto& funcs = impl_->player->funcs();
    auto* nearest = hit_unit(funcs, x, y, !(impl_->ai.observer() || impl_->battle.enabled()));
    impl_->selected.clear();
    if(nearest && nearest->owner<2) impl_->selected.push_back(funcs.get_unit_id_32(nearest));
    if(!impl_->selected.empty()) {
        auto* u=impl_->first_selected();
        if(u && u->unit_type->first_what_sound) impl_->sound->play_sound(u->unit_type->first_what_sound,{},u,false);
    }
    return !impl_->selected.empty();
}
void Engine::clear_selection() { impl_->selected.clear(); }
bool Engine::activate(int x,int y) {
    if(!ready()) return false;
    if(impl_->ai.observer() || impl_->battle.enabled()) { const bool ok=select(x,y); impl_->command_message=ok?"Observer selection":"Selection cleared"; return ok; }
    auto& f=impl_->player->funcs();
    auto* target=hit_unit(f,x,y,true);
    const auto* current=impl_->first_selected();
    const bool loading=current && target && ((f.unit_provides_space(target) && f.unit_can_load_target(target,current)) || (f.unit_provides_space(current) && f.unit_can_load_target(current,target)));
    const bool repairing=current && target && f.unit_is(current,UnitTypes::Terran_SCV) && (f.ut_mechanical(target) || f.ut_building(target)) && (!f.u_completed(target) || target->hp<target->unit_type->hitpoints);
    const bool healing=current && target && f.unit_is(current,UnitTypes::Terran_Medic) && f.ut_organic(target) && target->hp<target->unit_type->hitpoints;
    const bool gas=current && target && f.ut_worker(current) && f.unit_is_refinery(target);
    const bool recharge=current && target && f.unit_can_use_shield_battery(current,target);
    if(target && !loading && !repairing && !healing && !gas && !recharge) {
        const bool ok=select(x,y);impl_->command_message=ok?"Unit selected":"Selection cleared";return ok;
    }
    if(!current) { const bool ok=select(x,y); impl_->command_message=ok?"Unit selected":"Select a unit first"; return ok; }
    return command(Command::context,x,y);
}
unsigned Engine::select_army(int cx,int cy) {
    impl_->selected.clear();
    if(!ready()) return 0;
    const auto& f=impl_->player->funcs();
    for(const auto* u:ptr(impl_->player->st().visible_units)) {
        if(u->owner!=0 || f.unit_dead(u) || f.ut_turret(u) || !f.u_completed(u) || f.ut_worker(u) || f.ut_building(u) || f.us_hidden(u)) continue;
        const auto p=u->sprite->position;
        if(p.x<cx || p.x>=cx+400 || p.y<cy || p.y>=cy+240) continue;
        if(impl_->selected.size()==192) break;
        impl_->selected.push_back(f.get_unit_id_32(u));
    }
    return impl_->selected.size();
}
unsigned Engine::select_workers() {
    impl_->selected.clear();
    if(!ready()) return 0;
    const auto& f=impl_->player->funcs();
    for(const auto* u:ptr(impl_->player->st().visible_units)) {
        if(u->owner!=0 || f.unit_dead(u) || !f.u_completed(u) || !f.ut_worker(u) || f.us_hidden(u)) continue;
        if(impl_->selected.size()==192) break;
        impl_->selected.push_back(f.get_unit_id_32(u));
    }
    return impl_->selected.size();
}
bool Engine::store_group(unsigned slot) {
    if(slot>=4 || !ready()) return false;
    impl_->groups[slot].clear();
    const auto& f=impl_->player->funcs();
    for(auto id:impl_->selected) if(auto* u=f.get_unit(id))
        if(!f.unit_dead(u)) impl_->groups[slot].push_back(id);
    return !impl_->groups[slot].empty();
}
bool Engine::recall_group(unsigned slot,int* x,int* y) {
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
        if(f.us_hidden(u)) continue;
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
    const unit_t* moving=nullptr;
    for(unsigned p=0;p!=2;++p) for(const auto* u:ptr(st.player_units[p])) {
        if(f.unit_dead(u) || f.ut_building(u) || f.ut_worker(u) || f.ut_turret(u) || f.us_hidden(u))continue;
        const auto* target=u->order_target.unit;
        if(target && target->owner<2 && target->owner!=p && !f.unit_dead(target)) {
            x=(u->sprite->position.x+target->sprite->position.x)/2;
            y=(u->sprite->position.y+target->sprite->position.y)/2; return true;
        }
        if(u->order_type->id==Orders::AttackMove) moving=u;
    }
    if(moving) { x=moving->sprite->position.x; y=moving->sprite->position.y; return true; }
    const unsigned side=(st.current_frame/360)%2;
    for(const auto* u:ptr(st.player_units[side])) if(!f.unit_dead(u) && f.ut_building(u)) {
        x=u->sprite->position.x; y=u->sprite->position.y; return true;
    }
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
        for(auto* g:ptr(impl_->player->st().visible_units))if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !f.unit_dead(g)) {
            auto p=g->sprite->position;if(std::abs(x-p.x)<=64 && std::abs(y-p.y)<=48) {view.x=p.x-view.width/2;view.y=p.y-view.height/2;found=true;break;}
        }
        if(!found)return view;
    }
    view.valid=!impl_->ai.enabled(0) && !impl_->battle.enabled() && u && u->owner==0 && !f.unit_dead(u) &&
        f.unit_can_build(u,type) && f.has_available_resources_for(0,type) && x>=view.width/2 && y>=view.height/2 &&
        f.can_place_building(u,0,type,{view.x+view.width/2,view.y+view.height/2},true,true);
    return view;
}
PlacementView Engine::placement(Command kind,int x,int y) const {
    if(kind!=Command::depot && kind!=Command::barracks && kind!=Command::refinery)return {};
    return placement_unit(static_cast<unsigned>(kind==Command::depot?UnitTypes::Terran_Supply_Depot:kind==Command::refinery?UnitTypes::Terran_Refinery:UnitTypes::Terran_Barracks),x,y);
}
bool Engine::command(Command command, int x, int y) {
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
        auto* target = hit_unit(funcs, x, y, false);
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
        if (command == Command::context) {
            message = target && funcs.unit_is_mineral_field(target) ? "Gather minerals" :
                target && funcs.unit_is_refinery(target) && target->owner==0 ? "Gather gas" :
                target && target->owner == 1 ? "Attack target" : "Move / interact / rally";
        } else message=command==Command::stop ? "Stop" : command==Command::primary ? "Siege mode toggled" : "Attack-move to cursor";
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
        if (!funcs.u_completed(u) && funcs.ut_building(u)) accepted = commands.action_cancel_building_unit(0);
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
        if(n>=capacity)return;auto& o=output[n++];o.code=option_code(kind,id);o.target=target;o.enabled=enabled;
        if(minerals || gas)std::snprintf(o.label,sizeof o.label,"%s %s %d/%d",enabled?"":"x",name,minerals,gas);
        else std::snprintf(o.label,sizeof o.label,"%s%s",enabled?"":"x ",name);
    };
    if(page==OptionPage::basic || page==OptionPage::advanced) {
        unsigned index=0;catalogue::buildings([&](const terran::Product& b){
            if(b.parent!=u->unit_type->id)return;
            const auto* t=f.get_unit_type(b.type);if(f.ut_addon(t))return;
            if((page==OptionPage::basic)==(index++<8))add(b.name,OptionKind::build,unsigned(b.type),true,f.unit_can_build(u,t) && f.has_available_resources_for(0,t),t->mineral_cost,t->gas_cost);
        });return n;
    }
    if(f.ut_building(u) || f.unit_is_carrier(u) || f.unit_is_reaver(u)) {
        catalogue::units([&](const terran::Product& b){if(b.parent==u->unit_type->id && page==OptionPage::primary) {
            const auto* t=f.get_unit_type(b.type);add(b.name,OptionKind::train,unsigned(b.type),false,f.unit_can_build(u,t) && f.has_available_resources_for(0,t) && f.has_available_supply_for(0,t),t->mineral_cost,t->gas_cost);
        }
        });catalogue::buildings([&](const terran::Product& b){if(b.parent==u->unit_type->id && page==OptionPage::primary) {
            const auto* t=f.get_unit_type(b.type);add(b.name,OptionKind::addon,unsigned(b.type),false,f.unit_can_build(u,t) && f.has_available_resources_for(0,t),t->mineral_cost,t->gas_cost);
        }
        });catalogue::techs([&](const terran::Research& t){if(t.parent==u->unit_type->id) {
            const auto* type=f.get_tech_type(t.type);add(t.name,OptionKind::research,unsigned(t.type),false,f.unit_can_research(u,type,0) && f.has_available_resources_for(0,type,false),type->mineral_cost,type->gas_cost);
        }
        });catalogue::upgrades([&](const terran::Upgrade& t){if(t.parent==u->unit_type->id) {
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
        if(f.unit_is(u,UnitTypes::Protoss_Shield_Battery))add("Recharge shields",OptionKind::order,unsigned(Orders::RechargeShieldsBattery),true,f.u_completed(u) && !f.unit_is_disabled(u) && u->energy>0_fp8);
        if(u->unit_type->id==UnitTypes::Terran_Comsat_Station)add("Scanner Sweep",OptionKind::order,unsigned(Orders::CastScannerSweep),true,true);
        return n;
    }
    auto ability=[&](const char* name,Orders order,TechTypes tech) {add(name,OptionKind::order,unsigned(order),true,f.unit_can_use_tech(u,f.get_tech_type(tech)));};
    switch(u->unit_type->id) {
    case UnitTypes::Protoss_High_Templar:
        ability("Psionic Storm",Orders::CastPsionicStorm,TechTypes::Psionic_Storm);ability("Hallucination",Orders::CastHallucination,TechTypes::Hallucination);
        add("Merge nearest Templar",OptionKind::merge,0,false,true);break;
    case UnitTypes::Protoss_Dark_Templar:add("Merge nearest Dark Templar",OptionKind::merge,1,false,true);break;
    case UnitTypes::Protoss_Dark_Archon:
        ability("Feedback",Orders::CastFeedback,TechTypes::Feedback);ability("Mind Control",Orders::CastMindControl,TechTypes::Mind_Control);ability("Maelstrom",Orders::CastMaelstrom,TechTypes::Maelstrom);break;
    case UnitTypes::Protoss_Arbiter:ability("Recall",Orders::CastRecall,TechTypes::Recall);ability("Stasis Field",Orders::CastStasisField,TechTypes::Stasis_Field);break;
    case UnitTypes::Protoss_Corsair:ability("Disruption Web",Orders::CastDisruptionWeb,TechTypes::Disruption_Web);break;
    case UnitTypes::Protoss_Shuttle:add("Load unit",OptionKind::order,unsigned(Orders::PickupTransport),true,true);add("Unload all",OptionKind::unload,0,false,!f.loaded_units(u).empty());break;
    case UnitTypes::Terran_Marine:case UnitTypes::Terran_Firebat:add("Stim Packs",OptionKind::stim,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Stim_Packs)));break;
    case UnitTypes::Terran_Siege_Tank_Tank_Mode:case UnitTypes::Terran_Siege_Tank_Siege_Mode:add("Toggle Siege Mode",OptionKind::siege,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Tank_Siege_Mode)));break;
    case UnitTypes::Terran_Vulture:ability("Spider Mine",Orders::PlaceMine,TechTypes::Spider_Mines);break;
    case UnitTypes::Terran_Wraith:add("Toggle Cloak",OptionKind::cloak,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Cloaking_Field)));break;
    case UnitTypes::Terran_Ghost:
        add("Toggle Cloak",OptionKind::cloak,0,false,f.unit_can_use_tech(u,f.get_tech_type(TechTypes::Personnel_Cloaking)));
        ability("Lockdown",Orders::CastLockdown,TechTypes::Lockdown);add("Nuclear Strike",OptionKind::order,unsigned(Orders::NukePaint),true,true);break;
    case UnitTypes::Terran_Medic:ability("Heal",Orders::MedicHeal,TechTypes::Healing);ability("Restoration",Orders::CastRestoration,TechTypes::Restoration);ability("Optical Flare",Orders::CastOpticalFlare,TechTypes::Optical_Flare);break;
    case UnitTypes::Terran_Science_Vessel:ability("Defensive Matrix",Orders::CastDefensiveMatrix,TechTypes::Defensive_Matrix);ability("EMP Shockwave",Orders::CastEMPShockwave,TechTypes::EMP_Shockwave);ability("Irradiate",Orders::CastIrradiate,TechTypes::Irradiate);break;
    case UnitTypes::Terran_Battlecruiser:ability("Yamato Gun",Orders::FireYamatoGun,TechTypes::Yamato_Gun);break;
    case UnitTypes::Terran_Dropship:add("Load unit",OptionKind::order,unsigned(Orders::PickupTransport),true,true);add("Unload all",OptionKind::unload,0,false,!f.loaded_units(u).empty());break;
    default:break;
    }
    if(page==OptionPage::secondary) {
        add("Attack-move",OptionKind::order,unsigned(Orders::AttackMove),true,true);
        add("Hold position",OptionKind::order,unsigned(Orders::HoldPosition),false,true);
        add("Stop",OptionKind::order,unsigned(Orders::Stop),false,true);
    }
    return n;
}
bool Engine::perform(unsigned code,int x,int y) {
    auto fail=[&](const char* m){impl_->command_message=m;return false;};
    if(!ready())return fail("Load a scenario first");
    if(impl_->ai.enabled(0) || impl_->battle.enabled())return fail("Observer: AI controls both armies");
    auto* u=impl_->first_selected();if(!u || u->owner!=0)return fail("Select an own unit/building");
    const auto kind=static_cast<OptionKind>(code>>16);const unsigned id=code&65535;
    auto& p=*impl_->player;auto& f=p.funcs();action_state state;action_functions a(p.st(),state);a.action_select(0,u);bool ok=false;
    if(kind==OptionKind::build || kind==OptionKind::addon || kind==OptionKind::train) {
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
            ok=a.action_build(0,a.get_order_type(f.unit_race(type)==race_t::protoss?Orders::PlaceProtossBuilding:Orders::PlaceBuilding),type,{size_t(place.x/32),size_t(place.y/32)});
        }
    } else if(kind==OptionKind::research) {
        bool allowed=false;catalogue::techs([&](const terran::Research& t){if(unsigned(t.type)==id && t.parent==u->unit_type->id)allowed=true;});
        if(!allowed)return fail("This building cannot research that technology");
        ok=a.action_research(0,a.get_tech_type(static_cast<TechTypes>(id)));
    } else if(kind==OptionKind::upgrade) {
        bool allowed=false;catalogue::upgrades([&](const terran::Upgrade& t){if(unsigned(t.type)==id && t.parent==u->unit_type->id)allowed=true;});
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
            if(kind==OptionKind::stim)ok=a.action_stim_pack(0)||ok;
            else if(kind==OptionKind::siege)ok=(f.unit_is_sieged_tank(member)?a.action_unsiege(0,false):a.action_siege(0,false))||ok;
            else if(kind==OptionKind::cloak) {
                const auto tech=f.unit_is_ghost(member)?TechTypes::Personnel_Cloaking:TechTypes::Cloaking_Field;
                if(!f.unit_can_use_tech(member,f.get_tech_type(tech)))continue;
                if(f.u_requires_detector(member))a.action_decloak(0);else if(member->energy>=fp8::integer(f.get_tech_type(tech)->energy_cost))a.action_cloak(0);else continue;
                ok=true; // Native action_cloak changes state but returns false.
            } else if(kind==OptionKind::unload) {if(!f.loaded_units(member).empty()) {a.action_unload_all(0,false);ok=true;}}
            else if(kind==OptionKind::order && id<unsigned(Orders::None)) {
                const auto order=static_cast<Orders>(id);
                const bool ground=order==Orders::CastPsionicStorm || order==Orders::CastStasisField || order==Orders::CastRecall || order==Orders::CastMaelstrom || order==Orders::CastDisruptionWeb || order==Orders::PlaceMine || order==Orders::CastEMPShockwave || order==Orders::CastScannerSweep || order==Orders::NukePaint || order==Orders::AttackMove || order==Orders::Move || order==Orders::Patrol;
                auto* target=ground?nullptr:hit_unit(f,x,y,false);
                ok=a.action_order(0,a.get_order_type(static_cast<Orders>(id)),{x,y},target,nullptr,false)||ok;
            }
        }
    }
    if(!ok)return fail("Unavailable: prerequisites, resources, energy or target");
    impl_->command_message="Command accepted";return true;
}
bool Engine::move_selected(int x, int y) {
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
std::size_t Engine::visible_units(UnitView* output, std::size_t capacity) const {
    if (!ready()) return 0;
    const auto& funcs = impl_->player->funcs();
    std::size_t n = 0;
    for (const auto* u : ptr(impl_->player->st().visible_units)) {
        if (n == capacity) break;
        if (funcs.unit_dead(u) || funcs.ut_turret(u)) continue;
        const auto* t = u->unit_type;
        const bool selected=std::find(impl_->selected.begin(),impl_->selected.end(),funcs.get_unit_id_32(u))!=impl_->selected.end();
        unsigned cargo=0;for(auto* passenger:funcs.loaded_units(u)){(void)passenger;++cargo;}
        output[n++] = {u->sprite->position.x, u->sprite->position.y, static_cast<unsigned>(u->owner), selected,
                       static_cast<unsigned>(t->id), u->hp.integer_part(), t->hitpoints.integer_part(),
                       t->dimensions.from.x + t->dimensions.to.x + 1, t->dimensions.from.y + t->dimensions.to.y + 1,
                       funcs.u_completed(u), funcs.ut_worker(u), funcs.unit_is_mineral_field(u),funcs.unit_is(u,UnitTypes::Resource_Vespene_Geyser),funcs.unit_is_refinery(u),funcs.u_flying(u),cargo,u->shield_points.integer_part(),int(t->shield_points),unsigned(funcs.unit_is_carrier(u)?funcs.unit_interceptor_count(u):funcs.unit_is_reaver(u)?funcs.unit_scarab_count(u):0),unsigned(u->order_type->id),u->energy.integer_part()};
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
SpriteInfo Engine::sprite_info(unsigned image,unsigned frame) const {
    if(!ready() || image>=impl_->player->st().global->image_grp.size())return {};
    const auto& grp=impl_->player->st().global->image_grp[image];if(!grp || frame>=grp->frames.size())return {};
    const auto& f=grp->frames[frame];return {unsigned(f.size.x),unsigned(f.size.y),true};
}
bool Engine::sprite_pixels(unsigned image,unsigned frame,unsigned owner,uint16_t* out,std::size_t capacity) const {
    const auto info=sprite_info(image,frame);if(!info.valid || info.width>512 || info.height>512 || size_t(info.width)*info.height>capacity)return false;
    const auto& st=impl_->player->st();const auto& source=st.global->image_grp[image]->frames[frame];
    std::fill_n(out,size_t(info.width)*info.height,0);const unsigned team=owner<2?owner:11;
    const auto& type=st.global->image_types.vec[image];
    auto color=[&](unsigned c) {
        if(type.modifier==9 && type.color_shift>=1 && type.color_shift<=7) {
            const auto& light=impl_->lights[type.color_shift-1];
            const unsigned at=c?256*(c-1):unsigned(light.size());
            if(at>=light.size())return uint16_t(0);c=light[at];
        } else if(c>=8 && c<16)c=impl_->team[team*8+c-8];
        return impl_->palette[c];
    };
    for(unsigned y=0;y<info.height;++y) {
        const auto* p=source.data_container.data()+source.line_data_offset[y];unsigned x=0;
        while(x<info.width) {
            unsigned token=*p++,n=token&127;if(token&128){x+=n;continue;}
            if(token&64){n=token&63;const auto c=color(*p++);while(n-- && x<info.width)out[y*info.width+x++]=c;}
            else while(n-- && x<info.width)out[y*info.width+x++]=color(*p++);
        }
    }
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
std::size_t Engine::visible_images(SpriteView* output, std::size_t capacity, int cx, int cy) const {
    if (!ready()) return 0;
    const auto& st = impl_->player->st();
    const auto& funcs = impl_->player->funcs();
    std::size_t n = 0;
    for (const auto& line : st.sprites_on_tile_line) for (const auto& sprite : line) {
        if (sprite.flags & sprite_t::flag_hidden) continue;
        unsigned layer = 0;
        for (const auto& img : reverse(sprite.images)) {
            if((img.flags & image_t::flag_hidden) && !(img.modifier>=2 && img.modifier<=7) && !(sprite.flags&sprite_t::flag_burrowed))continue;
            // Glow/cloak/warp GRPs contain palette remap indices rather than
            // ordinary colors. Omit those effects until their blending tables
            // are supported; drawing them directly produces opaque artifacts.
            if(img.modifier<0 || (img.modifier>7 && img.modifier!=9 && img.modifier!=10 && img.modifier!=12))continue;
            const auto p = funcs.get_image_map_position(&img);
            const auto& frame = img.grp->frames.at(img.frame_index);
            if (p.x >= cx + 400 || p.y >= cy + 240 || p.x + static_cast<int>(frame.size.x) <= cx ||
                p.y + static_cast<int>(frame.size.y) <= cy) continue;
            if (n == capacity) continue;
            output[n++] = {p.x, p.y, static_cast<unsigned>(img.image_type->id), static_cast<unsigned>(img.frame_index),
                           static_cast<unsigned>(sprite.owner), (img.flags & image_t::flag_horizontally_flipped) != 0,
                           img.modifier == 10,
                           ((static_cast<std::uint64_t>(sprite.elevation_level) * 8192 + sprite.position.y) * 4096 + sprite.index) * 64 + layer++,
                           ((img.modifier>=2 && img.modifier<=7) || (sprite.flags&sprite_t::flag_burrowed))?112u:img.modifier==12?96u:img.modifier==9?200u:255u};
        }
    }
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
}
