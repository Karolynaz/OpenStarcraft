#include "engine.h"
#include "actions.h"
#include "graphics_format.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace port3ds {
using namespace bwgame;
struct Engine::Impl {
    std::unique_ptr<game_player> player;
    char message[192] = "Not loaded";
    bool loaded = false;
    unit_id selected;
};

namespace {
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
    impl_->selected = unit_id{};
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
            loader.setup_info.create_no_units = scenario != Scenario::melee;
            loader.setup_info.victory_condition = 0;
            loader.setup_info.resource_type = 1;
            loader.setup_info.starting_minerals = 50;
            for (unsigned i = 0; i != 12; ++i) {
                auto& p = player.st().players[i];
                p.controller = i < 2 ? player_t::controller_occupied : player_t::controller_inactive;
                p.race = race_t::terran;
                loader.setup_info.create_melee_units_for_player[i] = i < 2 && scenario == Scenario::melee;
            }
        });
        // Both slots are human/idle: this probe does not claim a functioning AI.
        if (scenario == Scenario::two_units) {
            auto* type = loader.get_unit_type(UnitTypes::Terran_Marine);
            if (!loader.create_initial_unit(type, {128, 128}, 0) ||
                !loader.create_initial_unit(type, {static_cast<int>(width * 32 - 128),
                                                     static_cast<int>(height * 32 - 128)}, 1))
                throw std::runtime_error("Could not place the two probe units");
        }
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
        impl_->player->next_frame();
        return true;
    } catch (const std::exception& e) {
        std::snprintf(impl_->message, sizeof impl_->message, "%s", e.what());
        impl_->loaded = false;
        return false;
    }
}
Snapshot Engine::snapshot() const {
    Snapshot s;
    if (!impl_->loaded) return s;
    auto& st = impl_->player->st();
    s.frame = st.current_frame;
    s.width = st.game->map_tile_width; s.height = st.game->map_tile_height;
    for (unsigned p = 0; p != 12; ++p)
        for (const auto& u : st.player_units[p]) { (void)u; ++s.units; }
    return s;
}
const char* Engine::error() const { return impl_->message; }
bool Engine::ready() const { return impl_->loaded; }
bool Engine::select(int x, int y) {
    if (!ready()) return false;
    auto& funcs = impl_->player->funcs();
    unit_t* nearest = nullptr;
    int distance = 24 * 24 + 1;
    for (auto* u : ptr(impl_->player->st().visible_units)) {
        if (u->owner != 0 || funcs.unit_dead(u)) continue;
        const int dx = u->sprite->position.x - x, dy = u->sprite->position.y - y;
        if (dx * dx + dy * dy < distance) { distance = dx * dx + dy * dy; nearest = u; }
    }
    impl_->selected = nearest ? funcs.get_unit_id(nearest) : unit_id{};
    return nearest != nullptr;
}
bool Engine::move_selected(int x, int y) {
    if (!ready()) return false;
    auto& player = *impl_->player;
    auto& funcs = player.funcs();
    auto* u = funcs.get_unit(impl_->selected);
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
    const auto* selected = funcs.get_unit(impl_->selected);
    std::size_t n = 0;
    for (const auto* u : ptr(impl_->player->st().visible_units)) {
        if (n == capacity) break;
        if (funcs.unit_dead(u)) continue;
        output[n++] = {u->sprite->position.x, u->sprite->position.y, static_cast<unsigned>(u->owner), u == selected};
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
