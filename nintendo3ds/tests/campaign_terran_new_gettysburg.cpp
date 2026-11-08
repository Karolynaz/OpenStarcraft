// Original New Gettysburg victory through ordinary player commands.
// The host pilot observes the whole map to choose orders.
#include "bwenums.h"
#include "engine.h"
#include "result_screen.h"
#include <cassert>
#include <cstdio>
#include <set>
#include <string>
#include <vector>
using namespace port3ds;
using namespace bwgame;
int main(int argc, char **argv) {
    assert(argc == 4);
    const std::string journal = argv[3];
    const auto scenario = Scenario::terran_campaign_9;
    Engine e;
    assert(e.load(argv[1], argv[2], scenario));
    assert(e.start_recording(journal.c_str()));
    e.set_fog_enabled(false);
    constexpr unsigned kerrigan_type = unsigned(UnitTypes::Hero_Sarah_Kerrigan);
    bool attack = false;
    unsigned pending = 0, pending_at = 0, before = 0;
    std::set<unsigned> gas_ids;
    const int hx = 1056, hy = 720;
    unsigned protected_buildings = 0;
    bool evacuated = false;
    for (unsigned f = 0; f < 66000 && e.snapshot().winner < 0; ++f) {
        if (!e.tick()) {
            printf("ERROR f%u %s\n", e.frame(), e.error());
            return 1;
        }
        SoundEvent snd[256];
        e.drain_sounds(snd, 256);
        if (f % 48) continue;
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
            }
            if (u.owner == 1 || (u.owner == 3 && u.type < 106))
                enemies.push_back(u);
        }
        const auto s = e.snapshot();
        const auto uid = [&](const UnitView& unit) { return e.own_unit_at(unit.x, unit.y); };
        // The starting mineral line suffices if spending is held briefly for the fleet.
        const bool keep_reserve = f >= 11000 && f <= 16000;
        unsigned zerg_buildings = 0;
        for (unsigned i = 0; i < n; ++i)
            if (list[i].owner == 3 && list[i].type >= 106 && list[i].type < 202)
                ++zerg_buildings;
        if (f == 0) protected_buildings = zerg_buildings;
        assert(zerg_buildings >= protected_buildings);
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
                if ((u.type == 29 || (u.type == kerrigan_type && f<39500) || (u.type==19 && f<40800)) && e.select(u.x, u.y))
                    e.move_selected(hx, hy);
        // Evacuate Kerrigan before sending the main fleet away from the base.
        if (f >= 39500) {
            const UnitView *ship=nullptr,*kerrigan=nullptr;
            for(const auto& u:own) {
                if(u.type==11 && (!ship || u.cargo>ship->cargo))ship=&u;
                if(u.type==kerrigan_type)kerrigan=&u;
            }
            if(ship && e.select(ship->x,ship->y)) {
                if(ship->cargo){evacuated=true;e.move_selected(352,2180);}
                else if(kerrigan){
                    e.command(Command::context,kerrigan->x,kerrigan->y);
                    if(e.select(kerrigan->x,kerrigan->y))e.command(Command::context,ship->x,ship->y);
                }
            }
        }
        if (pending && (e.count_units(0, pending, false) > before || f > pending_at + 1800))
            pending = 0;
        unsigned want = s.supply_max - s.supply < 8        ? 109
                        : workers.size() >= 7 && !e.count_units(0, 110, false)    ? 110
                        : e.count_units(0, 111, false) < 1 ? 111
                        : !e.count_units(0, 113, false)    ? 113
                        : e.count_units(0, 114, false) < 3 ? 114
                        : !e.count_units(0, 116, false)    ? 116
                                                           : 0;
        if (want && !pending && !workers.empty() && (!keep_reserve || s.minerals>475)) {
            bool built = false;
            for (auto it = workers.rbegin(); it != workers.rend() && !built; ++it)
                if (it->y<1300 && !gas_ids.count(uid(*it)) && e.select(it->x,it->y)) {
                    if (want == 110) {
                        for (unsigned j = 0; j < n && !built; ++j)
                            if (list[j].geyser &&
                                abs(list[j].x - hx) + abs(list[j].y - hy) < 2000 &&
                                e.placement_unit(want, list[j].x, list[j].y).valid)
                                built = e.perform(option_code(OptionKind::build, want), list[j].x,
                                                  list[j].y);
                    } else
                        for (int r = 3; r < (f>7500?9:17) && !built; ++r)
                            for (int y = -r; y <= r && !built; ++y)
                                for (int x = -r; x <= r && !built; ++x) {
                                    int px = hx + x * 64, py = hy + y * 64;
                                    if(f>7500 && (py<425 || px<740))continue;
                                    if(f>15000 && (want==114 || want==116)) {
                                        px=1456+x*64;py=960+y*64;
                                        if(px<880 || py<735 || py>1120)continue;
                                    }
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
                    if (list[j].mineral && list[j].x>1400 && list[j].x<1650 && list[j].y<850) {
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
                        if (k < 3 && w.y<1200 && w.order!=unsigned(Orders::ConstructingBuilding) && w.order!=unsigned(Orders::PlaceBuilding) && e.select(w.x,w.y)) {
                            if (e.command(Command::context, g.x + 40, g.y + 16)) {
                                gas_ids.insert(e.own_unit_at(w.x, w.y));
                                ++k;
                            }
                        }
                }
        for (const auto &u : own)
            if (!u.flying && e.select(u.x, u.y) && !e.snapshot().queue) {
                if (u.type == 106 && workers.size() < 22u && (f<=11000 || !keep_reserve || s.minerals>=475))
                    e.perform(option_code(OptionKind::train, 7));
                if (u.type == 111 && workers.size() >= 5 && e.count_units(0, 0) < 32)
                    e.perform(option_code(OptionKind::train, 0));
                if (u.type == 118 && !e.researched(unsigned(TechTypes::Yamato_Gun)) && f%240==0 &&
                    e.perform(option_code(OptionKind::research,unsigned(TechTypes::Yamato_Gun))))
                    printf("Yamato research started at %u\n",f);
                if (u.type == 114) {
                    if (!e.count_units(0, 115, false) &&
                        !e.perform(option_code(OptionKind::addon, 115)) && f % 480 == 0)
                        e.perform(option_code(OptionKind::liftoff));
                    if ((!e.count_units(0,118) || e.researched(unsigned(TechTypes::Yamato_Gun)) || e.count_units(0,12,false)<3) && !e.perform(option_code(OptionKind::train, 12)))
                        e.perform(option_code(OptionKind::addon, 115));
                }
                if (u.type == 116 && !e.count_units(0, 118, false) &&
                    !e.perform(option_code(OptionKind::addon, 118)) && f % 480 == 0)
                    e.perform(option_code(OptionKind::liftoff));
            }
        unsigned fleet = e.count_units(0, 8) + e.count_units(0, 12),
                 army = fleet + e.count_units(0, 0);
        if (e.count_units(0, 12) >= 12u)
            attack = true;
        if (fleet < 4 && army < 16)
            attack = false;
        if(f<40800 && !attack && f%240==0) {
            const UnitView *t = nullptr;
            int best = 0x7fffffff;
            for (const auto &u : enemies) {
                int dx = u.x - hx, dy = u.y - hy, d = dx * dx + dy * dy;
                if (d < 780 * 780 && d < best && u.type < 202) {
                    best = d;
                    t = &u;
                }
            }
            if (t) {
                for (const auto &u : own)
                    if ((u.type == 12 || (best < 780*780 && (u.type == 0 || u.type == 2 || u.type == 3 || u.type == 5))) &&
                        e.select(u.x, u.y))
                        e.command(Command::context, t->x, t->y);
            }
        }
        // Approach the Protoss from the east so the fleet doesn't fire on
        // protected Zerg buildings along the northern route.
        if (f >= 40800 && f % 96 == 0) {
            const UnitView* target=nullptr;
            int best=0x7fffffff;
            for(const auto& u:enemies)if(u.owner==1 && u.type>=106 && u.type<202) {
                int nearest=0x7fffffff;
                for(const auto& ship:own)if(ship.type==12)nearest=std::min(nearest,(ship.x-u.x)*(ship.x-u.x)+(ship.y-u.y)*(ship.y-u.y));
                if(nearest<best){best=nearest;target=&u;}
            }
            for(const auto& u:own) {
                if(u.type!=0 && u.type!=2 && u.type!=3 && u.type!=5 && u.type!=12 && u.type!=19)continue;
                if(u.type==12 && attack && target && (u.hp>200 || u.x>1800)) {
                    if(u.order==unsigned(Orders::FireYamatoGun) || u.order==unsigned(Orders::MoveToFireYamatoGun))continue;
                    if(!e.select(u.x,u.y))continue;
                    if(u.x<2100 && u.y<680){e.move_selected(1750,840);continue;}
                    if(u.x<2100){e.move_selected(2336,852);continue;}
                    const UnitView* threat=nullptr;
                    int threat_priority=0;
                    for(const auto& p:enemies)if(p.owner==1 && (p.type==72 || p.type==71 || p.type==162)) {
                        const int dx=p.x-u.x,dy=p.y-u.y;
                        if(dx*dx+dy*dy> (p.type==162?350:530)*(p.type==162?350:530))continue;
                        int priority=p.type==72?4:p.type==71?3:2;
                        if(priority>threat_priority){threat_priority=priority;threat=&p;}
                    }
                    const UnitView* aim=threat?threat:target;
                    if(u.energy>=150 && (aim->hp+aim->shields>200 || aim->type==162) &&
                        e.perform(option_code(OptionKind::order,unsigned(Orders::FireYamatoGun)),aim->x,aim->y))continue;
                    e.command(Command::attack,aim->x,aim->y);
                } else {
                    const unsigned id=e.own_unit_at(u.x,u.y);
                    if(!id || !e.select(u.x,u.y))continue;
                    int gx=(u.type==12?1280:960)+(id%5)*48, gy=(u.type==12?590:540)+(id%3)*70;
                    if (u.type != 12) { gx = 1010 + (id % 6) * 75; gy = 460 + (id % 4) * 105; }
                    if((u.x-gx)*(u.x-gx)+(u.y-gy)*(u.y-gy)>120*120) {
                        if(u.order!=unsigned(Orders::Move))e.move_selected(gx,gy);
                    } else if(u.order!=unsigned(Orders::HoldPosition))
                        e.perform(option_code(OptionKind::order,unsigned(Orders::HoldPosition)),gx,gy);
                }
            }
            for(const auto& u:own)if(u.type==12 && u.hp+80<u.max_hp && u.x<1700)
                for(const auto& w:workers)if(!gas_ids.count(e.own_unit_at(w.x,w.y)) &&
                    (w.x-u.x)*(w.x-u.x)+(w.y-u.y)*(w.y-u.y)<320*320 && e.select(w.x,w.y))
                    if(e.command(Command::context,u.x,u.y))break;
        }
        if (f % 4800 == 0)
            printf("Frame %u, economy %d/%d, SCVs %zu, Battlecruisers %u\n",
                   f, s.minerals, s.gas, workers.size(), e.count_units(0, 12));
    }
    const auto result = e.snapshot();
    assert(result.winner == 0 && evacuated && e.count_units(0, kerrigan_type) == 1);
    UnitView survivors[1700];
    const auto remaining = e.visible_units(survivors, 1700);
    unsigned zerg_buildings = 0;
    for (unsigned i = 0; i < remaining; ++i)
        if (survivors[i].owner == 3 && survivors[i].type >= 106 && survivors[i].type < 202)
            ++zerg_buildings;
    assert(protected_buildings > 0 && zerg_buildings >= protected_buildings);
    assert(result.minerals_gathered > 0 && e.researched(unsigned(TechTypes::Yamato_Gun)));
    assert(ResultScreen::next_mission(unsigned(scenario), true, result.next_scenario) ==
           campaign::key_index("terran12"));
    const auto save = journal + ".sav";
    assert(e.save_game(save.c_str(), SaveView{}));
    Engine replay;
    SaveView restored;
    assert(replay.load_save(argv[1], save.c_str(), (journal + "-replay").c_str(),
                            (journal + "-map").c_str(), restored));
    assert(replay.state_digest() == e.state_digest());
    printf("New Gettysburg: original victory at frame %u and exact replay PASS\n", e.frame());
}
