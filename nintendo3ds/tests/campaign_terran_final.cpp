// Public-control original late Terran economy/fleet victory and completed-save replay. Fog-off
// navigation aid.
#include "bwenums.h"
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <set>
#include <vector>
using namespace port3ds;
using namespace bwgame;
int main(int argc, char **argv) {
    assert(argc == 4);
    std::string key = argv[3];
    assert(key == "terran12");
    Engine e;
    assert(e.load(argv[1], argv[2], Scenario::terran_campaign_10));
    assert(e.start_recording(("/tmp/sc3ds-original-" + key).c_str()));
    e.set_fog_enabled(false);
    bool attack = false;
    unsigned pending = 0, pending_at = 0, before = 0;
    std::set<unsigned> gas_ids;
    int hx = 0, hy = 0;
    for (unsigned f = 0; f < 240000 && e.snapshot().winner < 0; ++f) {
        if (!e.tick()) {
            printf("ERROR f%u %s\n", e.frame(), e.error());
            return 1;
        }
        SoundEvent snd[256];
        e.drain_sounds(snd, 256);
        if (f % 48)
            continue;
        UnitView list[1700];
        auto n = e.visible_units(list, 1700);
        std::vector<UnitView> own, workers, enemies;
        for (unsigned i = 0; i < n; ++i) {
            auto &u = list[i];
            if (!u.owner) {
                if (u.completed)
                    own.push_back(u);
                if (u.type == 7 && u.completed)
                    workers.push_back(u);
                if (u.type == 106) {
                    hx = u.x;
                    hy = u.y;
                }
            }
            if (u.owner == 1 || u.owner == 6)
                enemies.push_back(u);
        }
        auto s = e.snapshot();
        if (!hx)
            continue;
        for (const auto &u : own)
            if (u.flying && u.type >= 106 && u.type < 120 && e.select(u.x, u.y) &&
                u.order != unsigned(Orders::BuildingLand)) {
                bool landed = false;
                for (int r = 0; r < 14 && !landed; ++r)
                    for (int y = -r; y <= r && !landed; ++y)
                        for (int x = -r; x <= r && !landed; ++x)
                            landed = e.perform(option_code(OptionKind::land), hx - 160 + x * 96,
                                               hy - 320 + y * 96);
            }
        if (f % 240 == 0)
            for (const auto &u : own)
                if ((u.type == 29 || u.type == 16 || u.type == 19) && e.select(u.x, u.y))
                    e.move_selected(hx, hy);
        if (pending && (e.count_units(0, pending, false) > before || f > pending_at + 1800))
            pending = 0;
        unsigned want = s.supply_max - s.supply < 8        ? 109
                        : !e.count_units(0, 110, false)    ? 110
                        : e.count_units(0, 111, false) < 1 ? 111
                        : !e.count_units(0, 113, false)    ? 113
                        : e.count_units(0, 114, false) < 3 ? 114
                        : !e.count_units(0, 116, false)    ? 116
                                                           : 0;
        if (want && !pending && !workers.empty()) {
            bool built = false;
            for (auto it = workers.rbegin(); it != workers.rend() && !built; ++it)
                if (!gas_ids.count(e.own_unit_at(it->x, it->y)) && e.select(it->x, it->y)) {
                    if (want == 110) {
                        for (unsigned j = 0; j < n && !built; ++j)
                            if (list[j].geyser &&
                                abs(list[j].x - hx) + abs(list[j].y - hy) < 2000 &&
                                e.placement_unit(want, list[j].x, list[j].y).valid)
                                built = e.perform(option_code(OptionKind::build, want), list[j].x,
                                                  list[j].y);
                    } else
                        for (int r = 3; r < 17 && !built; ++r)
                            for (int y = -r; y <= r && !built; ++y)
                                for (int x = -r; x <= r && !built; ++x) {
                                    int px = hx + x * 64, py = hy + y * 64;
                                    if (e.placement_unit(want, px, py).valid)
                                        built =
                                            e.perform(option_code(OptionKind::build, want), px, py);
                                }
                }
            if (built) {
                pending = want;
                pending_at = f;
                before = e.count_units(0, want, false);
            }
        }
        for (const auto &w : workers)
            if (w.order == 1 || w.order == 2 || w.order == 3) {
                const UnitView *t = nullptr;
                auto id = e.own_unit_at(w.x, w.y);
                if (gas_ids.count(id))
                    for (const auto &u : own)
                        if (u.type == 110)
                            t = &u;
                int best = t ? 0 : 0x7fffffff;
                for (unsigned j = 0; j < n; ++j)
                    if (list[j].mineral) {
                        int dx = list[j].x - w.x, dy = list[j].y - w.y, d = dx * dx + dy * dy;
                        if (d < best) {
                            best = d;
                            t = &list[j];
                        }
                    }
                if (t && e.select(w.x, w.y))
                    e.command(Command::context, t->x + (t->refinery ? 40 : 0),
                              t->y + (t->refinery ? 16 : 0));
            }
        if (f % 480 == 0)
            for (const auto &g : own)
                if (g.type == 110 && workers.size() > 5) {
                    unsigned k = 0;
                    for (const auto &w : workers)
                        if (k < 3 && e.select(w.x, w.y)) {
                            if (e.command(Command::context, g.x + 40, g.y + 16)) {
                                gas_ids.insert(e.own_unit_at(w.x, w.y));
                                ++k;
                            }
                        }
                }
        for (const auto &u : own)
            if (!u.flying && e.select(u.x, u.y) && !e.snapshot().queue) {
                if (u.type == 106 && workers.size() < 22)
                    e.perform(option_code(OptionKind::train, 7));
                if (u.type == 111 && e.count_units(0, 0) < 48)
                    e.perform(option_code(OptionKind::train, 0));
                if (u.type == 114) {
                    if (!e.count_units(0, 115, false) &&
                        !e.perform(option_code(OptionKind::addon, 115)) && f % 480 == 0)
                        e.perform(option_code(OptionKind::liftoff));
                    if (!e.perform(option_code(OptionKind::train, 12)))
                        e.perform(option_code(OptionKind::addon, 115));
                }
                if (u.type == 116 && !e.count_units(0, 118, false) &&
                    !e.perform(option_code(OptionKind::addon, 118)) && f % 480 == 0)
                    e.perform(option_code(OptionKind::liftoff));
            }
        unsigned fleet = e.count_units(0, 8) + e.count_units(0, 12),
                 army = fleet + e.count_units(0, 0);
        if (fleet >= 16 || e.count_units(0, 12) >= 8 || army >= 60)
            attack = true;
        if (fleet < 4 && army < 16)
            attack = false;
        if (f % 240 == 0) {
            const UnitView *t = nullptr;
            int best = 0x7fffffff;
            for (const auto &u : enemies) {
                int dx = u.x - hx, dy = u.y - hy, d = dx * dx + dy * dy;
                if ((attack || d < 900 * 900) && d < best && u.type < 202) {
                    best = d;
                    t = &u;
                }
            }
            if (t) {
                for (const auto &u : own)
                    if ((u.type == 0 || u.type == 1 || u.type == 2 || u.type == 3 || u.type == 5 ||
                         u.type == 8 || u.type == 12) &&
                        e.select(u.x, u.y))
                        e.command(Command::attack, t->x, t->y);
            }
        }
        if (f % 4800 == 0) {
            printf(
                "%s f%u money%d/%d scv%zu marines%u wraith%u bc%u pending%u enemies%zu attack%d\n",
                key.c_str(), e.frame(), s.minerals, s.gas, workers.size(), e.count_units(0, 0),
                e.count_units(0, 8), e.count_units(0, 12), pending, enemies.size(), attack);
            fflush(stdout);
        }
    }
    printf("%s result%d f%u\n", key.c_str(), e.snapshot().winner, e.frame());
    fflush(stdout);
    if (e.snapshot().winner != 0)
        return 1;
    SaveView view;
    auto save = "/tmp/sc3ds-" + key + "-completed.sav";
    assert(e.save_game(save.c_str(), view));
    Engine r;
    SaveView restored;
    assert(r.load_save(argv[1], save.c_str(), ("/tmp/sc3ds-" + key + "-replay").c_str(),
                       ("/tmp/sc3ds-" + key + "-map").c_str(), restored));
    assert(r.state_digest() == e.state_digest());
    puts("Original late Terran paid economy / fleet / victory / exact replay PASS");
}
