// Full Circle: destroy the original Temple, then carry the crystal in an Overlord.
// Native resources and public controls only. Fog-off navigation for repeatability.
#include "bwenums.h"
#include "engine.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <queue>
#include <vector>
using namespace port3ds;
struct Point {
    int x, y;
};
static std::vector<Point> flight(const UnitView *units, unsigned count, int width, int height,
                                 Point from, Point to) {
    unsigned w = (width + 63) / 64, h = (height + 63) / 64;
    std::vector<int> costs(w * h, 1), dist(w * h, 1000000000), prev(w * h, -1);
    for (unsigned i = 0; i < count; ++i) {
        const auto &u = units[i];
        if (!u.owner || u.owner >= 8 || u.hp <= 0)
            continue;
        unsigned range = 0;
        switch (u.type) {
        case 0:
        case 1:
        case 3:
        case 8:
        case 12:
        case 38:
        case 43:
        case 47:
        case 53:
        case 66:
        case 70:
        case 124:
        case 144:
        case 162:
            range = 480;
            break;
        default:
            break;
        }
        if (!range)
            continue;
        for (unsigned y = 0; y < h; ++y)
            for (unsigned x = 0; x < w; ++x) {
                auto dx = int(x * 64 + 32) - u.x, dy = int(y * 64 + 32) - u.y;
                auto d = dx * dx + dy * dy;
                if (d < int(range * range))
                    costs[y * w + x] += int((range * range - d) / 1024) + 40;
            }
    }
    auto cell = [&](Point p) {
        return std::min(h - 1, unsigned(p.y / 64)) * w + std::min(w - 1, unsigned(p.x / 64));
    };
    unsigned a = cell(from), b = cell(to);
    using Node = std::pair<int, unsigned>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    dist[a] = 0;
    open.push({0, a});
    while (!open.empty()) {
        auto at = open.top();
        open.pop();
        auto i = at.second;
        if (at.first != dist[i])
            continue;
        if (i == b)
            break;
        int x = i % w, y = i / w;
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (!dx && !dy)
                    continue;
                int nx = x + dx, ny = y + dy;
                if (nx < 0 || ny < 0 || nx >= int(w) || ny >= int(h))
                    continue;
                unsigned j = ny * w + nx;
                int d = at.first + costs[j] * (dx && dy ? 14 : 10);
                if (d < dist[j]) {
                    dist[j] = d;
                    prev[j] = i;
                    open.push({d, j});
                }
            }
    }
    assert(prev[b] >= 0);
    std::vector<Point> route;
    for (int i = b; i != int(a); i = prev[i])
        route.push_back({int((i % w) * 64 + 32), int((i / w) * 64 + 32)});
    std::reverse(route.begin(), route.end());
    std::vector<Point> turns;
    for (unsigned k = 0; k < route.size(); ++k) {
        if (k > 0 && k + 1 < route.size()) {
            auto a = route[k - 1], b = route[k], c = route[k + 1];
            if ((b.x - a.x) * (c.y - b.y) == (b.y - a.y) * (c.x - b.x))
                continue;
        }
        turns.push_back(route[k]);
    }
    return turns;
}

int main(int argc, char **argv) {
    assert(argc == 3);
    struct {
        const char *key;
    } m{"zerg10"};
    Engine e;
    assert(e.load(argv[1], argv[2], Scenario::zerg_campaign_10));
    assert(e.start_recording((std::string("/tmp/sc3ds-z10-guardian-") + m.key).c_str()));
    e.set_fog_enabled(false);
    const unsigned worker = 41, gas = 149;
    unsigned build_pending = 0, build_at = 0, build_count = 0;
    unsigned carrier = 0, flight_stage = 0;
    bool loaded = false, unloaded = false;
    std::vector<Point> path;
    bool delivered = false;
    bool attack = false;
    int home_x = 0, home_y = 0;
    for (unsigned frame = 0; frame < 220000 && e.snapshot().winner < 0; ++frame) {
        if (!e.tick()) {
            fprintf(stderr, "%s frame%u: %s\n", m.key, e.frame(), e.error());
            return 1;
        }
        SoundEvent events[256];
        e.drain_sounds(events, 256);
        if (frame % 48)
            continue;
        UnitView list[1700];
        unsigned n = e.visible_units(list, 1700);
        auto state = e.snapshot();
        std::vector<UnitView> workers, parents;
        std::vector<UnitView> minerals;
        std::vector<UnitView> enemies;
        for (unsigned i = 0; i < n; ++i) {
            auto &u = list[i];
            if (u.owner == 0) {
                if (u.type == worker && u.completed)
                    workers.push_back(u);
                if (!home_x && (u.type == 131 || u.type == 132 || u.type == 133)) {
                    home_x = u.x;
                    home_y = u.y;
                }
                if (u.completed)
                    parents.push_back(u);
            }
            if (u.mineral)
                minerals.push_back(u);
            if (u.owner == 2 || u.owner == 3 || u.owner == 7)
                enemies.push_back(u);
        }
        if (!home_x)
            continue;
        bool crystal_phase = state.objectives.find("remains") != std::string::npos;
        if (crystal_phase) {
            UnitView *crystal = nullptr;
            UnitView *drone = nullptr;
            UnitView *ferry = nullptr;
            for (unsigned i = 0; i < n; ++i) {
                if (list[i].type == 219)
                    crystal = &list[i];
                if (!list[i].owner && list[i].type == 41 && list[i].carried_powerup == 219)
                    drone = &list[i];
                if (!list[i].owner && list[i].type == 42 &&
                    (!ferry || list[i].cargo > ferry->cargo))
                    ferry = &list[i];
            }
            if (drone) {
                carrier = e.own_unit_at(drone->x, drone->y);
                if (!loaded && ferry && e.select(ferry->x, ferry->y))
                    e.perform(
                        option_code(OptionKind::order, unsigned(bwgame::Orders::PickupTransport)),
                        drone->x, drone->y);
                if (unloaded && e.select(drone->x, drone->y)) {
                    e.move_selected(1984, 2064);
                    delivered = true;
                }
            } else if (!carrier && crystal && !workers.empty() &&
                       e.select(workers.back().x, workers.back().y)) {
                carrier = e.own_unit_at(workers.back().x, workers.back().y);
                e.command(Command::context, crystal->x, crystal->y);
            } else if (carrier && !loaded && crystal)
                for (const auto &w : workers)
                    if (e.own_unit_at(w.x, w.y) == carrier && e.select(w.x, w.y))
                        e.command(Command::context, crystal->x, crystal->y);
            if (ferry && ferry->cargo && !loaded) {
                loaded = true;
                path = flight(list, n, state.width * 32, state.height * 32, {ferry->x, ferry->y},
                              {1984, 2064});
                printf("Crystal flight f%u path%zu\n", e.frame(), path.size());
            }
            if (ferry && loaded && !unloaded && !path.empty()) {
                auto goal = path[flight_stage];
                int dx = ferry->x - goal.x, dy = ferry->y - goal.y;
                if (dx * dx + dy * dy < 48 * 48) {
                    if (flight_stage + 1 < path.size())
                        ++flight_stage;
                    else if (e.select(ferry->x, ferry->y))
                        unloaded = e.perform(option_code(OptionKind::unload));
                }
                if (!unloaded && e.select(ferry->x, ferry->y))
                    e.move_selected(path[flight_stage].x, path[flight_stage].y);
            }
        }
        // Native guards defend the colony until a paid army has assembled.
        unsigned army = e.count_units(0, 38) + e.count_units(0, 37) + e.count_units(0, 43);
        if (e.count_units(0, 44) >= 10 || e.count_units(0, 43) >= 20 || army >= 65)
            attack = true;
        if (e.count_units(0, 43) + e.count_units(0, 44) < 6 && army < 20)
            attack = false;
        if (frame % 240 == 0 && !enemies.empty()) {
            const UnitView *target = &enemies.front();
            int distance = std::numeric_limits<int>::max();
            for (const auto &u : enemies) {
                int dx = u.x - home_x, dy = u.y - home_y, d = dx * dx + dy * dy;
                if (d < distance) {
                    distance = d;
                    target = &u;
                }
            }
            if (attack && !crystal_phase)
                for (const auto &u : enemies)
                    if (u.type == 174) {
                        target = &u;
                        break;
                    }
            for (unsigned i = 0; i < n; ++i)
                if (list[i].owner == 0 && (list[i].type == 37 || list[i].type == 38 ||
                                           list[i].type == 43 || list[i].type == 44))
                    if ((attack || distance < 540 * 540) && e.select(list[i].x, list[i].y)) {
                        if (target->type == 174 && list[i].flying)
                            e.command(Command::context, target->x, target->y);
                        else
                            e.command(Command::attack, target->x, target->y);
                    }
        }
        for (const auto &w : workers)
            if (!w.carried_powerup && e.own_unit_at(w.x, w.y) != carrier &&
                (w.order == unsigned(bwgame::Orders::PlayerGuard) ||
                 w.order == unsigned(bwgame::Orders::Guard) ||
                 w.order == unsigned(bwgame::Orders::Stop))) {
                const UnitView *target = nullptr;
                int distance = std::numeric_limits<int>::max();
                for (const auto &u : minerals) {
                    int dx = u.x - w.x, dy = u.y - w.y, d = dx * dx + dy * dy;
                    if (d < distance) {
                        distance = d;
                        target = &u;
                    }
                }
                if (target && e.select(w.x, w.y))
                    e.command(Command::context, target->x, target->y);
            }
        // Assign three workers to the paid refinery once it has completed.
        if (frame % 480 == 0 && e.count_units(0, gas) && workers.size() >= 7 && !carrier) {
            UnitView *refinery = nullptr;
            for (unsigned i = 0; i < n; ++i)
                if (list[i].owner == 0 && list[i].type == gas && list[i].completed)
                    refinery = &list[i];
            if (refinery)
                for (unsigned i = 0; i < 3; ++i)
                    if (e.select(workers[i].x, workers[i].y))
                        e.command(Command::context, refinery->x + 40, refinery->y + 16);
        }
        if (build_pending &&
            (e.count_units(0, build_pending) > build_count || frame > build_at + 1500))
            build_pending = 0;
        unsigned want = 0;
        int anchor_x = home_x, anchor_y = home_y;
        if (!want && !e.count_units(0, 142))
            want = 142;
        if (!want && !e.count_units(0, gas))
            want = gas;
        if (!want && e.count_units(0, 146, false) + e.count_units(0, 144, false) +
                             e.count_units(0, 143, false) <
                         6)
            want = 143;
        if (!want && !e.count_units(0, 135))
            want = 135;
        if (!want && e.count_units(0, 132) && !e.count_units(0, 138, false))
            want = 138;
        if (!want && e.count_units(0, 131, false) < 2)
            want = 131;
        if (!want && (e.count_units(0, 132) || e.count_units(0, 133)) &&
            !e.count_units(0, 141, false) && !e.count_units(0, 137, false))
            want = 141;
        if (!build_pending && want && !workers.empty()) {
            bool placed = false;
            auto w = workers.front();
            if (e.select(w.x, w.y)) {
                if (want == gas) {
                    for (unsigned i = 0; i < n && !placed; ++i)
                        if (list[i].geyser &&
                            abs(list[i].x - home_x) + abs(list[i].y - home_y) < 1200 &&
                            e.placement_unit(want, list[i].x, list[i].y).valid)
                            placed = e.perform(option_code(OptionKind::build, want), list[i].x,
                                               list[i].y);
                } else
                    for (int ring = 2; ring < 11 && !placed; ++ring)
                        for (int y = -ring; y <= ring && !placed; ++y)
                            for (int x = -ring; x <= ring && !placed; ++x) {
                                if (std::abs(x) != ring && std::abs(y) != ring)
                                    continue;
                                int px = anchor_x + x * 32, py = anchor_y + y * 32;
                                if (e.placement_unit(want, px, py).valid)
                                    placed =
                                        e.perform(option_code(OptionKind::build, want), px, py);
                            }
            }
            if (placed) {
                build_pending = want;
                build_at = frame;
                build_count = e.count_units(0, want);
            }
        }
        for (const auto &p : parents) {
            if (!e.select(p.x, p.y))
                continue;
            if (p.type == 35) {
                unsigned next =
                    state.supply_max - state.supply < 5                                     ? 42
                    : e.count_units(0, worker) < 18                                         ? worker
                    : e.count_units(0, 137) && state.gas > 130                              ? 43
                    : e.count_units(0, 135) && state.gas > 300 && e.count_units(0, 38) < 18 ? 38
                                                                                            : 37;
                e.perform(option_code(OptionKind::morph, next));
            } else if (p.type == 132 && e.count_units(0, 138) && !e.count_units(0, 133, false))
                e.perform(option_code(OptionKind::morph, 133));
            else if (p.type == 133 && frame % 480 == 0 && state.gas > 150) {
                for (auto up : {bwgame::UpgradeTypes::Ventral_Sacs,
                                bwgame::UpgradeTypes::Pneumatized_Carapace})
                    if (!e.upgrade_level(unsigned(up)))
                        e.perform(option_code(OptionKind::upgrade, unsigned(up)));
            } else if (p.type == 141 && e.count_units(0, 133) && !e.count_units(0, 137, false))
                e.perform(option_code(OptionKind::morph, 137));
            else if (p.type == 43 && e.count_units(0, 137) && state.gas > 50)
                e.perform(option_code(OptionKind::morph, 44));
            else if (p.type == 143)
                e.perform(
                    option_code(OptionKind::morph, e.count_units(0, 144, false) < 4 ? 144 : 146));
            else if (p.type == 131 && !e.count_units(0, 132, false) &&
                     !e.count_units(0, 133, false))
                e.perform(option_code(OptionKind::morph, 132));
        }
        if (frame % 4800 == 0) {
            printf("Lair%u Hive%u Spire%u Muta%u Guard%u Greater%u ", e.count_units(0, 132),
                   e.count_units(0, 133), e.count_units(0, 141), e.count_units(0, 43),
                   e.count_units(0, 44), e.count_units(0, 137));
            printf(
                "%s frame%u minerals%d gas%d workers%u army%u buildings%u/%u pending%u attack%d\n",
                m.key, e.frame(), state.minerals, state.gas, e.count_units(0, worker), army,
                e.count_units(0, 142), e.count_units(0, 135), build_pending, attack);
            fflush(stdout);
        }
    }
    auto s = e.snapshot();
    printf("%s result%d frame%u\n", m.key, s.winner, e.frame());
    fflush(stdout);
    assert(s.winner == 0 && delivered && carrier);
    SaveView view;
    assert(e.save_game((std::string("/tmp/sc3ds-") + m.key + "-completed.sav").c_str(), view));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1],
                            (std::string("/tmp/sc3ds-") + m.key + "-completed.sav").c_str(),
                            (std::string("/tmp/sc3ds-third-replay-") + m.key).c_str(),
                            (std::string("/tmp/sc3ds-third-map-") + m.key).c_str(), restored));
    assert(e.state_digest() == replay.state_digest());
    puts("Full Circle original temple destruction, physical crystal delivery, original victory and "
         "exact replay PASS");
}
