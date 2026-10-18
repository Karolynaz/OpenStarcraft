#include "engine.h"
#include "actions.h"
#include "graphics_format.h"
#include "battle_ai.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace port3ds {
using namespace bwgame;
struct Engine::Impl {
    struct SoundFunctions : state_functions {
        std::array<SoundEvent,64> events{};
        unsigned count=0, dropped=0;
        explicit SoundFunctions(state& st) : state_functions(st) {}
        void play_sound(int id, xy pos, const unit_t*, bool race) override {
            if(id<0) return;
            if(count==events.size()) { ++dropped; return; }
            events[count++]={static_cast<unsigned>(id+(race?1:0)),static_cast<int>(pos.x),static_cast<int>(pos.y)};
        }
    };
    std::unique_ptr<game_player> player;
    std::unique_ptr<SoundFunctions> sound;
    char message[192] = "Not loaded";
    bool loaded = false;
    static_vector<unit_id_32,64> selected;
    std::array<static_vector<unit_id_32,64>,4> groups;
    detail::BattleAI ai;
    unit_t* first_selected() const {
        if (!loaded) return nullptr;
        for (auto id:selected) if (auto* u=player->funcs().get_unit(id))
            if (!player->funcs().unit_dead(u)) return u;
        return nullptr;
    }
    const char* command_message = "A select / B act";
};

namespace {
const char* unit_name(const unit_t* u) {
    switch (u->unit_type->id) {
    case UnitTypes::Terran_SCV: return "SCV";
    case UnitTypes::Terran_Marine: return "Marine";
    case UnitTypes::Terran_Command_Center: return "Command Center";
    case UnitTypes::Terran_Barracks: return "Barracks";
    case UnitTypes::Terran_Refinery: return "Refinery";
    case UnitTypes::Terran_Supply_Depot: return "Supply Depot";
    default: return "Unit";
    }
}
// Use the engine collision box with a small thumbstick tolerance. Prefer
// the nearest center when two boxes overlap; never select dead/hidden units.
unit_t* hit_unit(state_functions& funcs, int x, int y, bool own_only) {
    unit_t* best = nullptr;
    int distance = 0x7fffffff;
    for (auto* u : ptr(funcs.st.visible_units)) {
        if ((own_only && u->owner != 0) || funcs.unit_dead(u) ||
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
                p.race = race_t::terran;
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
Snapshot Engine::snapshot() const {
    Snapshot s;
    if (!impl_->loaded) return s;
    auto& st = impl_->player->st();
    s.frame = st.current_frame;
    s.width = st.game->map_tile_width; s.height = st.game->map_tile_height;
    s.minerals = st.current_minerals[0]; s.gas = st.current_gas[0];
    const auto race = static_cast<unsigned>(race_t::terran);
    s.supply = st.supply_used[0][race].integer_part();
    s.supply_max = st.supply_available[0][race].integer_part();
    const auto& funcs = impl_->player->funcs();
    if (const auto* u = impl_->first_selected()) {
        s.selected_name = unit_name(u);
        s.selected_hp = u->hp.integer_part(); s.selected_max_hp = u->unit_type->hitpoints.integer_part();
        s.queue = u->build_queue.size();
        s.selection=funcs.ut_worker(u) ? SelectionKind::workers : funcs.unit_is(u,UnitTypes::Terran_Command_Center) ?
            SelectionKind::command_center : funcs.unit_is(u,UnitTypes::Terran_Barracks) ? SelectionKind::barracks : SelectionKind::other;
    }
    for(auto id:impl_->selected) if(auto* u=funcs.get_unit(id)) if(!funcs.unit_dead(u)) ++s.selected;
    s.observer=impl_->ai.observer(); s.winner=impl_->ai.winner();
    for(unsigned p=0;p!=2;++p) {
        auto& v=s.players[p];
        v.minerals=st.current_minerals[p]; v.gas=st.current_gas[p];
        v.supply=st.supply_used[p][race].integer_part(); v.supply_max=st.supply_available[p][race].integer_part();
        v.ai=impl_->ai.enabled(p); v.trained=impl_->ai.trained(p); v.built=impl_->ai.built(p); v.waves=impl_->ai.waves(p); v.lost=impl_->ai.lost(p);
        for(const auto& u:st.player_units[p]) if(!funcs.unit_dead(&u)) {
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
bool Engine::select(int x, int y) {
    if (!ready()) return false;
    auto& funcs = impl_->player->funcs();
    auto* nearest = hit_unit(funcs, x, y, !impl_->ai.observer());
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
    if(impl_->ai.observer()) { const bool ok=select(x,y); impl_->command_message=ok?"Observer selection":"Selection cleared"; return ok; }
    auto& f=impl_->player->funcs();
    auto* target=hit_unit(f,x,y,true);
    const auto* current=impl_->first_selected();
    if(target && (!current || !f.ut_worker(current) || !f.ut_building(target) ||
        (f.u_completed(target) && target->hp>=target->unit_type->hitpoints && !f.unit_is_refinery(target)))) {
        const bool ok=select(x,y); impl_->command_message=ok?"Unit selected":"Selection cleared"; return ok;
    }
    if(!current) { const bool ok=select(x,y); impl_->command_message=ok?"Unit selected":"Select a unit first"; return ok; }
    return command(Command::context,x,y);
}
unsigned Engine::select_army(int cx,int cy) {
    impl_->selected.clear();
    if(!ready()) return 0;
    const auto& f=impl_->player->funcs();
    for(const auto* u:ptr(impl_->player->st().visible_units)) {
        if(u->owner!=0 || f.unit_dead(u) || !f.u_completed(u) || f.ut_worker(u) || f.ut_building(u) || f.us_hidden(u)) continue;
        const auto p=u->sprite->position;
        if(p.x<cx || p.x>=cx+400 || p.y<cy || p.y>=cy+240) continue;
        if(impl_->selected.size()==64) break;
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
        if(impl_->selected.size()==64) break;
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
    static_vector<unit_id_32,64> retained;
    for(auto id:impl_->groups[slot]) if(auto* u=f.get_unit(id)) {
        if(f.unit_dead(u) || (u->owner!=0 && !impl_->ai.observer())) continue;
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
        if(f.unit_dead(u) || !f.unit_is(u,UnitTypes::Terran_Marine)) continue;
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
PlacementView Engine::placement(Command building, int x, int y) const {
    PlacementView view;
    if (!ready() || (building != Command::depot && building != Command::barracks && building != Command::refinery)) return view;
    const auto& funcs = impl_->player->funcs();
    auto* type = funcs.get_unit_type(building == Command::depot ? UnitTypes::Terran_Supply_Depot : building == Command::refinery ? UnitTypes::Terran_Refinery : UnitTypes::Terran_Barracks);
    view.width = type->placement_size.x; view.height = type->placement_size.y;
    view.x = (x-view.width/2)/32*32; view.y = (y-view.height/2)/32*32;
    if(building==Command::refinery) {
        bool found=false;
        for(const auto* g:ptr(impl_->player->st().visible_units)) if(funcs.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !funcs.unit_dead(g)) {
            const auto p=g->sprite->position;
            if(std::abs(x-p.x)<=64 && std::abs(y-p.y)<=48) {
                view.x=p.x-view.width/2;view.y=p.y-view.height/2;found=true;break;
            }
        }
        if(!found) return view;
    }
    const auto* u = impl_->first_selected();
    view.valid = !impl_->ai.enabled(0) && u && u->owner == 0 && !funcs.unit_dead(u) && funcs.unit_can_build(u,type) &&
        funcs.has_available_resources_for(0,type) && x >= view.width/2 && y >= view.height/2 &&
        funcs.can_place_building(u,0,type,{view.x+view.width/2,view.y+view.height/2},true,true);
    return view;
}
bool Engine::command(Command command, int x, int y) {
    auto fail = [&](const char* why) { impl_->command_message = why; return false; };
    if (!ready()) return fail("Load game with L first");
    auto& player = *impl_->player;
    auto& funcs = player.funcs();
    auto* u = impl_->first_selected();
    if(impl_->ai.enabled(0)) return fail("Observer mode: AI controls both armies");
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
bool Engine::move_selected(int x, int y) {
    if (!ready() || impl_->ai.enabled(0)) return false;
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
        if (funcs.unit_dead(u)) continue;
        const auto* t = u->unit_type;
        const bool selected=std::find(impl_->selected.begin(),impl_->selected.end(),funcs.get_unit_id_32(u))!=impl_->selected.end();
        output[n++] = {u->sprite->position.x, u->sprite->position.y, static_cast<unsigned>(u->owner), selected,
                       static_cast<unsigned>(t->id), u->hp.integer_part(), t->hitpoints.integer_part(),
                       t->dimensions.from.x + t->dimensions.to.x + 1, t->dimensions.from.y + t->dimensions.to.y + 1,
                       funcs.u_completed(u), funcs.ut_worker(u), funcs.unit_is_mineral_field(u),funcs.unit_is(u,UnitTypes::Resource_Vespene_Geyser),funcs.unit_is_refinery(u)};
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
std::size_t Engine::image_types(unsigned* output,std::size_t capacity) const {
    if(!ready()) return 0;
    bool seen[1024]{};
    std::size_t n=0;
    for(const auto& line:impl_->player->st().sprites_on_tile_line) for(const auto& sprite:line)
        for(const auto& img:sprite.images) {
            if(img.modifier!=0 && img.modifier!=1 && img.modifier!=10) continue;
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
            if (img.flags & image_t::flag_hidden) continue;
            // Glow/cloak/warp GRPs contain palette remap indices rather than
            // ordinary colors. Omit those effects until their blending tables
            // are supported; drawing them directly produces opaque artifacts.
            if (img.modifier != 0 && img.modifier != 1 && img.modifier != 10) continue;
            const auto p = funcs.get_image_map_position(&img);
            const auto& frame = img.grp->frames.at(img.frame_index);
            if (p.x >= cx + 400 || p.y >= cy + 240 || p.x + static_cast<int>(frame.size.x) <= cx ||
                p.y + static_cast<int>(frame.size.y) <= cy) continue;
            if (n == capacity) continue;
            output[n++] = {p.x, p.y, static_cast<unsigned>(img.image_type->id), static_cast<unsigned>(img.frame_index),
                           static_cast<unsigned>(sprite.owner), (img.flags & image_t::flag_horizontally_flipped) != 0,
                           img.modifier == 10,
                           ((static_cast<std::uint64_t>(sprite.elevation_level) * 8192 + sprite.position.y) * 4096 + sprite.index) * 64 + layer++};
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
