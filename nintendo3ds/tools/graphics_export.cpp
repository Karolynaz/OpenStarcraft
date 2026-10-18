// Host-only reader: use OpenBW's proven MPQ/GRP code, not a second decoder.
#include "actions.h"
#include "asset_closure.h"
#include "../tests/gameplay.h"
#include "../tests/gas.h"
#include "../tests/battle.h"
#include "../tests/terran.h"
#include "../tests/terran_build.h"
#include <cstdio>
#include <set>
#include <stdexcept>
using namespace bwgame;
static FILE* out;
static void u16(unsigned v) { std::fputc(v & 255, out); std::fputc(v >> 8, out); }
static void u32(unsigned v) { u16(v & 65535); u16(v >> 16); }
static void bytes(const void* p, size_t n) { if (std::fwrite(p, 1, n, out) != n) throw std::runtime_error("write failed"); }
static void collect(state& st, std::set<unsigned>& ids) {
    for (auto& line : st.sprites_on_tile_line) for (auto& sprite : line)
        for (auto& image : sprite.images)
            if (image.modifier == 0 || image.modifier == 1 || image.modifier == 10)
                ids.insert((unsigned)image.image_type->id);
}
int main(int argc, char** argv) {
    if (argc != 4) { std::fprintf(stderr, "graphics-export DATA MAP OUTPUT\n"); return 2; }
    try {
        auto assets = data_loading::data_files_directory(argv[1]);
        std::unique_ptr<game_player> current;
        std::set<unsigned> ids;
        // Drive the same public commands as the device, covering cargo,
        // construction, training, weapons and death effects as well as idle.
        auto profile=[&](port3ds::Engine& engine) {
            unsigned images[256];const auto count=engine.image_types(images,256);
            for(unsigned i=0;i<count;++i) ids.insert(images[i]);
        };
        const std::string map_name(argv[2]);const auto slash=map_name.find_last_of('/');
        const auto arena=map_name.substr(0,slash==std::string::npos?0:slash+1)+"arena.chk";
        terran_construction_tests(argv[1],arena.c_str(),profile);
        terran_tests(argv[1],arena.c_str(),profile);terran_battle_test(argv[1],arena.c_str(),profile);
        gameplay_tests(argv[1],argv[2],profile);
        battle_test(argv[1],argv[2],profile);
        gas_tests(argv[1],argv[2],profile);
        battle_test(argv[1],argv[2],profile,port3ds::Scenario::gas_battle);
        for (unsigned mode = 0; mode != 2; ++mode) {
            // Like the device adapter, use a fresh state for each map load.
            // Reusing upstream's live state leaves stale unit-finder pointers.
            current.reset(new game_player(assets));
            auto& player = *current;
            game_load_functions loader(player.st());
            a_vector<uint8_t> chk;
            if(map_name.size()>=4 && map_name.substr(map_name.size()-4)==".chk") {data_loading::file_reader<> file(argv[2]);chk.resize(file.size());file.get_bytes(chk.data(),chk.size());}
            else {data_loading::mpq_file<> map(argv[2]);map(chk,"staredit/scenario.chk");}
            loader.load_map_data(chk.data(),chk.size(), [&] {
                loader.setup_info.create_no_units = mode == 1;
                loader.setup_info.resource_type = 1;
                for (unsigned i = 0; i != 12; ++i) {
                    player.st().players[i].controller = i < 2 ? player_t::controller_occupied : player_t::controller_inactive;
                    player.st().players[i].race = race_t::terran;
                    loader.setup_info.create_melee_units_for_player[i] = i < 2 && mode == 0;
                }
            });
            if (mode == 1) {
                auto* marine = loader.get_unit_type(UnitTypes::Terran_Marine);
                loader.create_initial_unit(marine, {128,128}, 0);
                loader.create_initial_unit(marine, {512,128}, 1);
            }
            for (int n = 0; n != 480; ++n) { collect(player.st(), ids); player.next_frame(); }
        }
        auto& st = current->st(); auto& global = *st.global; auto& game = *st.game;
        std::set<unsigned> sound_ids;terran_asset_closure(st,ids,sound_ids);
        const char* names[] = {"badlands","platform","install","ashworld","jungle","desert","ice","twilight"};
        a_vector<uint8_t> palette, vr4, vx4, pcx;
        const auto tileset = game.tileset_index;
        assets(palette, format("Tileset/%s.wpe", names[tileset]));
        assets(vr4, format("Tileset/%s.vr4", names[tileset]));
        assets(vx4, format("Tileset/%s.vx4", names[tileset]));
        assets(pcx, "game/tunit.pcx");
        if (palette.size()!=1024 || pcx.size()<128) throw std::runtime_error("invalid palette");
        data_loading::data_reader_le pr(pcx.data()+128, pcx.data()+pcx.size());
        uint8_t team[128]; unsigned at=0;
        while (at<128) {
            unsigned c=pr.get<uint8_t>(), n=1;
            if ((c&192)==192) { n=c&63; c=pr.get<uint8_t>(); }
            if (!n || n>128-at) throw std::runtime_error("invalid team PCX");
            while (n--) team[at++]=c;
        }
        out = std::fopen(argv[3], "wb"); if (!out) throw std::runtime_error("cannot create raw graphics");
        bytes("OBWRAW1\0",8); u32(game.map_tile_width); u32(game.map_tile_height); u32(tileset);
        bytes(palette.data(),palette.size()); bytes(team,128);
        std::set<unsigned> tiles;
        u32(st.tiles_mega_tile_index.size());
        for (auto id : st.tiles_mega_tile_index) { u16(id); tiles.insert(id); }
        u32(tiles.size());
        for (auto id : tiles) {
            u16(id);
            for (unsigned y=0;y<32;++y) for (unsigned x=0;x<32;++x) {
                const unsigned off=id*32+((y/8)*4+x/8)*2;
                if (off+2>vx4.size()) throw std::runtime_error("VX4 bounds");
                unsigned mini=vx4[off] | (vx4[off+1]<<8);
                unsigned src=(mini/2)*64+(y%8)*8+((mini&1)?7-x%8:x%8);
                if (src>=vr4.size()) throw std::runtime_error("VR4 bounds");
                std::fputc(vr4[src],out);
            }
        }
        u32(ids.size()); unsigned total=0;
        for (auto id:ids) {
            const auto& grp=*global.image_grp.at(id);
            u16(id); u16(grp.frames.size());
            for (const auto& f:grp.frames) {
                u16(f.size.x); u16(f.size.y);
                for (unsigned y=0;y<f.size.y;++y) {
                    const auto* p=f.data_container.data()+f.line_data_offset[y];
                    for (unsigned x=0;x<f.size.x;) {
                        unsigned token=*p++, n=token&127;
                        if (token&128) { while(n--) { u16(65535); ++x; } }
                        else if(token&64) { n=token&63; unsigned c=*p++; while(n--) { u16(c); ++x; } }
                        else { while(n--) { u16(*p++); ++x; } }
                    }
                }
                ++total;
            }
        }
        std::fclose(out); out=nullptr;
        std::printf("Exported %zu tiles, %zu image types, %u frames (%zux%zu map)\n",tiles.size(),ids.size(),total,game.map_tile_width,game.map_tile_height);
    } catch(const std::exception& e) { if(out) std::fclose(out); std::fprintf(stderr,"%s\n",e.what());return 1; }
}
