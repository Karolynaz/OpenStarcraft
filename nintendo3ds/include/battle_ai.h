#pragma once
// Small deterministic test opponent, deliberately separate from Blizzard AI
// bytecode compatibility. All actions pay normal costs and use native orders.
#include "actions.h"
namespace port3ds { namespace detail {
using namespace bwgame;
class BattleAI {
    struct Side {
        bool enabled = false, building = false;
        unit_id_32 builder, gas_worker;
        UnitTypes plan = UnitTypes::Terran_Supply_Depot;
        unsigned search = 24, trained = 0, built = 0, waves = 0;
        unsigned build_frame = 0, attack_frame = 0;
        unsigned lost = 0;
        static_vector<unit_id_32,64> previous;
    };
    std::array<Side,2> sides_{};
    int winner_ = -1;
    bool gas_economy_ = false,protoss_=false,zerg_=false;
    static bool idle(const unit_t* u) {
        const auto o=u->order_type->id;
        return o==Orders::PlayerGuard || o==Orders::Guard || o==Orders::Stop || o==Orders::Nothing || o==u->unit_type->return_to_idle->id;
    }
    static xy spiral(unsigned n) {
        unsigned r=1;
        while(n>=8*r) { n-=8*r; ++r; }
        const int k=n%(2*r), d=r;
        switch(n/(2*r)) {
        case 0: return {d,-d+k};
        case 1: return {d-k,d};
        case 2: return {-d,d-k};
        default:return {-d+k,-d};
        }
    }
    void update_zerg(game_player& player,unsigned owner) {
        auto& side=sides_[owner];if(!side.enabled)return;
        auto& f=player.funcs();auto& st=player.st();action_state state;action_functions a(st,state);
        static_vector<unit_t*,64> workers,army,larvae;
        unit_t* base=nullptr;unit_t* pool=nullptr;unit_t* den=nullptr;unit_t* refinery=nullptr;
        unsigned live=0,pending_workers=0,pending_supply=0,pending_army=0,hydras=0;
        bool unfinished=false;
        for(auto* u:ptr(st.player_units[owner])) {
            if(f.unit_dead(u))continue;++live;
            if(f.unit_is(u,UnitTypes::Zerg_Hatchery))base=u;
            if(f.unit_is(u,UnitTypes::Zerg_Spawning_Pool))pool=u;
            if(f.unit_is(u,UnitTypes::Zerg_Hydralisk_Den))den=u;
            if(f.unit_is(u,UnitTypes::Zerg_Extractor))refinery=u;
            if(f.ut_building(u) && !f.u_completed(u))unfinished=true;
            if(f.unit_is(u,UnitTypes::Zerg_Drone)){if(f.u_completed(u))workers.push_back(u);else ++pending_workers;}
            if(f.unit_is(u,UnitTypes::Zerg_Zergling) || f.unit_is(u,UnitTypes::Zerg_Hydralisk)) {
                if(f.u_completed(u)){army.push_back(u);if(f.unit_is(u,UnitTypes::Zerg_Hydralisk))++hydras;}
            }
            if(f.unit_is(u,UnitTypes::Zerg_Larva) && u->build_queue.empty())larvae.push_back(u);
            if(!u->build_queue.empty()) {
                const auto* type=u->build_queue.front();
                if(type->id==UnitTypes::Zerg_Drone)++pending_workers;
                else if(type->id==UnitTypes::Zerg_Overlord)++pending_supply;
                else if(type->id==UnitTypes::Zerg_Zergling){pending_army+=2;++live;}
                else if(type->id==UnitTypes::Zerg_Hydralisk)++pending_army;
            }
        }
        auto* builder=f.get_unit(side.builder);
        if(side.building && (!builder || f.unit_dead(builder) ||
           f.ut_building(builder) || (st.current_frame>side.build_frame+48 && builder->order_type->id!=Orders::DroneStartBuild && builder->order_type->id!=Orders::DroneLand))) {
            side.building=false;side.builder={};side.search=0;
        }
        auto* gas_worker=f.get_unit(side.gas_worker);
        if(!gas_worker || f.unit_dead(gas_worker) || !f.unit_is(gas_worker,UnitTypes::Zerg_Drone) || !refinery || !f.u_completed(refinery)) {
            side.gas_worker={};gas_worker=nullptr;
        }
        if(refinery && f.u_completed(refinery) && !gas_worker)for(auto* w:workers)if(!(side.building && f.get_unit_id_32(w)==side.builder)) {
            a.action_select(owner,w);
            if(a.action_default_order(owner,refinery->sprite->position,refinery,nullptr,false)){side.gas_worker=f.get_unit_id_32(w);gas_worker=w;}
            break;
        }
        for(auto* w:workers) {
            if(!idle(w) || (side.building && f.get_unit_id_32(w)==side.builder))continue;
            if(w==gas_worker){a.action_select(owner,w);a.action_default_order(owner,refinery->sprite->position,refinery,nullptr,false);continue;}
            unit_t* mineral=nullptr;int best=0x7fffffff;
            for(auto* m:ptr(st.visible_units))if(f.unit_is_mineral_field(m) && !f.unit_dead(m) && m->building.resource.resource_count>0) {
                const auto d=m->sprite->position-w->sprite->position;const int score=d.x*d.x+d.y*d.y;
                if(score<best){best=score;mineral=m;}
            }
            if(mineral){a.action_select(owner,w);a.action_default_order(owner,mineral->sprite->position,mineral,nullptr,false);}
        }
        if(base && f.u_completed(base)) {
            // Creep limits the search to a compact ring around the Hatchery.
            // A Drone is consumed only by the native construction order.
            const bool need_pool=!pool && workers.size()>=6;
            const bool need_gas=pool && f.u_completed(pool) && !refinery;
            const bool need_den=refinery && f.u_completed(refinery) && !den;
            const auto plan=need_pool?UnitTypes::Zerg_Spawning_Pool:need_gas?UnitTypes::Zerg_Extractor:UnitTypes::Zerg_Hydralisk_Den;
            const auto* type=f.get_unit_type(plan);
            if(!side.building && !unfinished && (need_pool || need_gas || need_den) && f.has_available_resources_for(owner,type)) {
                unit_t* worker=nullptr;for(auto* w:workers)if(w!=gas_worker && f.unit_can_build(w,type)){worker=w;break;}
                if(worker) {
                    if(side.plan!=plan){side.plan=plan;side.search=0;}
                    auto place=[&](xy center){
                        if(center.x<type->placement_size.x/2 || center.y<type->placement_size.y/2)return false;
                        const auto tile=(center-type->placement_size/2)/32;const auto pos=tile*32+type->placement_size/2;
                        if(!f.can_place_building(worker,owner,type,pos,true,true))return false;
                        a.action_select(owner,worker);
                        if(!a.action_build(owner,a.get_order_type(Orders::DroneStartBuild),type,{size_t(tile.x),size_t(tile.y)}))return false;
                        side.builder=f.get_unit_id_32(worker);side.building=true;side.build_frame=st.current_frame;++side.built;return true;
                    };
                    if(need_gas) {
                        unit_t* geyser=nullptr;int best=512*512;
                        for(auto* g:ptr(st.visible_units))if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !f.unit_dead(g)) {
                            const auto d=g->sprite->position-base->sprite->position;const int score=d.x*d.x+d.y*d.y;
                            if(score<best){best=score;geyser=g;}
                        }
                        if(geyser)place(geyser->sprite->position);
                    } else for(unsigned n=0;n<8;++n){
                        if(side.search>=224)side.search=0;
                        if(place(base->sprite->position+spiral(side.search++)*32))break;
                    }
                }
            }
            const auto race=unsigned(race_t::zerg);
            const int free=(st.supply_available[owner][race]-st.supply_used[owner][race]).integer_part();
            for(auto* larva:larvae) {
                const bool supply=free<=3 && !pending_supply && st.supply_available[owner][race]<fp1::integer(40);
                const bool drone=workers.size()+pending_workers<8;
                const bool hydra=den && f.u_completed(den) && hydras<(army.size()+pending_army)/2+1 && st.current_gas[owner]>=25;
                const auto next=supply?UnitTypes::Zerg_Overlord:drone?UnitTypes::Zerg_Drone:hydra?UnitTypes::Zerg_Hydralisk:UnitTypes::Zerg_Zergling;
                // Reserve room for natural Larva replenishment and the second
                // Zergling from each egg; morphing does not immediately add one.
                if(live+(next==UnitTypes::Zerg_Zergling?1:0)>unit_budget-3)break;
                if(!supply && !drone && (army.size()+pending_army+(next==UnitTypes::Zerg_Zergling?2:1)>20 || !pool || !f.u_completed(pool)))continue;
                // Save for the first Pool and gas technology before excess Lings.
                if(!supply && !drone && (!pool || (need_gas && st.current_minerals[owner]<100) || (need_den && st.current_minerals[owner]<150)))continue;
                a.action_select(owner,larva);
                if(a.action_morph(owner,a.get_unit_type(next))){
                    ++side.trained;
                    if(supply)++pending_supply;
                    else if(drone)++pending_workers;
                    else {pending_army+=next==UnitTypes::Zerg_Zergling?2:1;if(next==UnitTypes::Zerg_Zergling)++live;}
                }
            }
        }
        if(st.current_frame>=side.attack_frame && army.size()>=8) {
            unit_t* target=nullptr;
            for(auto* e:ptr(st.player_units[1-owner]))if(!f.unit_dead(e)){
                if(!target || f.ut_building(e))target=e;
                if(f.unit_is(e,UnitTypes::Zerg_Hatchery))break;
            }
            if(target){unsigned sent=0;for(auto* u:army)if(idle(u)){
                a.action_select(owner,u);if(a.action_order(owner,a.get_order_type(Orders::AttackMove),target->sprite->position,nullptr,nullptr,false))++sent;
            }if(sent>=4)++side.waves;}
            side.attack_frame=st.current_frame+240;
        }
    }
    void update(game_player& player, unsigned owner) {
        if(zerg_){update_zerg(player,owner);return;}
        const auto base_type=protoss_?UnitTypes::Protoss_Nexus:UnitTypes::Terran_Command_Center;
        const auto worker_type=protoss_?UnitTypes::Protoss_Probe:UnitTypes::Terran_SCV;
        const auto supply_type=protoss_?UnitTypes::Protoss_Pylon:UnitTypes::Terran_Supply_Depot;
        const auto factory_type=protoss_?UnitTypes::Protoss_Gateway:UnitTypes::Terran_Barracks;
        const auto soldier_type=protoss_?UnitTypes::Protoss_Zealot:UnitTypes::Terran_Marine;
        const auto gas_type=protoss_?UnitTypes::Protoss_Assimilator:UnitTypes::Terran_Refinery;
        const auto place_order=protoss_?Orders::PlaceProtossBuilding:Orders::PlaceBuilding;
        auto& side=sides_[owner];
        if(!side.enabled) return;
        auto& f=player.funcs(); auto& st=player.st();
        action_state state;
        action_functions a(st,state);
        static_vector<unit_t*,64> workers, marines, factories;
        unit_t* base=nullptr; unit_t* unfinished=nullptr; unit_t* refinery=nullptr; unit_t* cyber=nullptr;
        unsigned live=0, depots=0, barracks=0,dragoons=0;
        for(auto* u:ptr(st.player_units[owner])) {
            if(f.unit_dead(u)) continue;
            ++live;
            if(f.unit_is(u,base_type)) base=u;
            if(protoss_ && f.unit_is(u,UnitTypes::Protoss_Cybernetics_Core))cyber=u;
            if(f.unit_is(u,gas_type)) refinery=u;
            if(protoss_ && f.u_completed(u) && f.unit_is(u,UnitTypes::Protoss_Dragoon))++dragoons;
            if(f.unit_is(u,supply_type)) ++depots;
            if(f.unit_is(u,factory_type)) { ++barracks; if(f.u_completed(u)) factories.push_back(u); }
            if(f.ut_building(u) && !f.u_completed(u)) unfinished=u;
            if(f.ut_building(u) && !u->current_build_unit && !u->build_queue.empty() &&
               (u->build_queue.front()->id==worker_type || u->build_queue.front()->id==soldier_type || (protoss_ && u->build_queue.front()->id==UnitTypes::Protoss_Dragoon))) ++live;
            if(f.u_completed(u) && f.unit_is(u,worker_type)) workers.push_back(u);
            if(f.u_completed(u) && (f.unit_is(u,soldier_type) || (protoss_ && f.unit_is(u,UnitTypes::Protoss_Dragoon)))) marines.push_back(u);
        }
        auto* builder=f.get_unit(side.builder);
        if(side.building && builder && !f.unit_dead(builder) && builder->order_type->id==place_order) ++live;
        if(side.building && (!builder || f.unit_dead(builder) ||
            (st.current_frame>side.build_frame+48 && builder->order_type->id!=place_order && builder->order_type->id!=Orders::ConstructingBuilding))) {
            side.building=false; side.builder={}; side.search=24;
        }
        // Resume an abandoned foundation after a builder dies.
        if(!protoss_ && unfinished && !side.building && (!unfinished->connected_unit || f.unit_dead(unfinished->connected_unit))) {
            for(auto* w:workers) if(f.unit_can_build(w,f.get_unit_type(supply_type))) {
                a.action_select(owner,w);
                if(a.action_default_order(owner,unfinished->sprite->position,unfinished,nullptr,false)) {
                    side.builder=f.get_unit_id_32(w); side.building=true; side.build_frame=st.current_frame;
                }
                break;
            }
        }
        // Reserve one worker for the completed refinery. Native harvest orders
        // handle entering, gas cargo, return trips and depleted-geyser behavior.
        unit_t* gas_worker=f.get_unit(side.gas_worker);
        if(!refinery || !f.u_completed(refinery) || !gas_worker || f.unit_dead(gas_worker)) {
            side.gas_worker={};gas_worker=nullptr;
        }
        if(gas_economy_ && refinery && f.u_completed(refinery)) {
            if(!gas_worker) { for(auto* w:workers) if(!(side.building && f.get_unit_id_32(w)==side.builder)) {
                a.action_select(owner,w);
                if(a.action_default_order(owner,refinery->sprite->position,refinery,nullptr,false)) {
                    side.gas_worker=f.get_unit_id_32(w);gas_worker=w;
                }
                break;
            }
            } else if(idle(gas_worker)) {
                a.action_select(owner,gas_worker);
                a.action_default_order(owner,refinery->sprite->position,refinery,nullptr,false);
            }
        }
        for(auto* w:workers) {
            if(w==gas_worker || !idle(w) || (side.building && f.get_unit_id_32(w)==side.builder)) continue;
            unit_t* mineral=nullptr; int distance=0x7fffffff;
            for(auto* m:ptr(st.visible_units)) if(f.unit_is_mineral_field(m) && !f.unit_dead(m) && m->building.resource.resource_count>0) {
                const auto d=m->sprite->position-w->sprite->position;
                const int score=d.x*d.x+d.y*d.y;
                if(score<distance) { distance=score; mineral=m; }
            }
            if(mineral) { a.action_select(owner,w); a.action_default_order(owner,mineral->sprite->position,mineral,nullptr,false); }
        }
        if(base && f.u_completed(base)) {
            const auto race=static_cast<unsigned>(protoss_?race_t::protoss:race_t::terran);
            const int free=(st.supply_available[owner][race]-st.supply_used[owner][race]).integer_part();
            const bool need_depot=depots==0 || (free<=4 && depots<(protoss_?8u:4u));
            const bool need_barracks=barracks<2;
            const bool need_refinery=gas_economy_ && !refinery && barracks>=2;
            const bool need_core=protoss_ && !cyber && refinery && f.u_completed(refinery);
            const auto plan=need_depot ? supply_type : need_barracks ? factory_type : need_core?UnitTypes::Protoss_Cybernetics_Core:gas_type;
            const auto* type=f.get_unit_type(plan);
            if(!side.building && !unfinished && live<unit_budget && (need_depot || need_barracks || need_refinery || need_core) && f.has_available_resources_for(owner,type)) {
                unit_t* worker=nullptr;
                for(auto* w:workers) if(w!=gas_worker && f.unit_can_build(w,type)) { worker=w; break; }
                if(worker) {
                    if(side.plan!=plan) { side.plan=plan; side.search=24; }
                    // Bound the expensive placement search to eight candidates
                    // per 12 simulation frames, rather than scanning a whole base.
                    if(plan==gas_type) {
                        unit_t* geyser=nullptr;int best=0x7fffffff;
                        for(auto* g:ptr(st.visible_units)) if(f.unit_is(g,UnitTypes::Resource_Vespene_Geyser) && !f.unit_dead(g)) {
                            const auto d=g->sprite->position-base->sprite->position;const int distance=d.x*d.x+d.y*d.y;
                            if(distance<best) {best=distance;geyser=g;}
                        }
                        if(geyser && best<512*512 && f.can_place_building(worker,owner,type,geyser->sprite->position,true,true)) {
                            const auto tile=(geyser->sprite->position-type->placement_size/2)/32;
                            a.action_select(owner,worker);
                            if(a.action_build(owner,a.get_order_type(place_order),type,{static_cast<size_t>(tile.x),static_cast<size_t>(tile.y)})) {
                                side.builder=f.get_unit_id_32(worker);side.building=true;side.build_frame=st.current_frame;++side.built;++live;
                            }
                        }
                    } else for(unsigned n=0;n!=8;++n) {
                        if(side.search>=624) side.search=24;
                        const auto offset=spiral(side.search++);
                        if(std::max(std::abs(offset.x),std::abs(offset.y))<6) continue;
                        const auto center=base->sprite->position+offset*32;
                        if(center.x<type->placement_size.x/2 || center.y<type->placement_size.y/2) continue;
                        const xy tile={(center.x-type->placement_size.x/2)/32,(center.y-type->placement_size.y/2)/32};
                        const xy pos=tile*32+type->placement_size/2;
                        bool crowded=false;
                        for(const auto* b:ptr(st.player_units[owner])) if(f.ut_building(b) && !f.unit_dead(b)) {
                            const auto delta=pos-b->sprite->position;
                            if(std::abs(delta.x)<(type->placement_size.x+b->unit_type->placement_size.x)/2+32 &&
                               std::abs(delta.y)<(type->placement_size.y+b->unit_type->placement_size.y)/2+32) { crowded=true; break; }
                        }
                        if(crowded) continue;
                        if(!f.can_place_building(worker,owner,type,pos,true,true)) continue;
                        a.action_select(owner,worker);
                        if(a.action_build(owner,a.get_order_type(place_order),type,{static_cast<size_t>(tile.x),static_cast<size_t>(tile.y)})) {
                            side.builder=f.get_unit_id_32(worker); side.building=true; side.build_frame=st.current_frame; ++side.built; ++live;
                        }
                        break;
                    }
                }
            }
            // One queue entry at a time and a controller production budget;
            // never delete existing units or change simulation pool limits.
            if(workers.size()<(protoss_?8u:gas_economy_?7u:6u) && base->build_queue.empty() && live<unit_budget) {
                a.action_select(owner,base);
                if(a.action_train(owner,a.get_unit_type(worker_type))) { ++side.trained; ++live; }
            }
        }
        unsigned queued=0;
        for(auto* b:factories) queued+=b->build_queue.size();
        for(auto* b:factories) if(b->build_queue.empty() && marines.size()+queued<24 && live<unit_budget) {
            a.action_select(owner,b);
            const auto train=protoss_ && cyber && f.u_completed(cyber) && dragoons<(marines.size()+queued)/3+1 && st.current_gas[owner]>=50?UnitTypes::Protoss_Dragoon:soldier_type;
            if(a.action_train(owner,a.get_unit_type(train))) { ++side.trained; ++queued; ++live; }
        }
        if(st.current_frame>=side.attack_frame && marines.size()>=4) {
            unit_t* target=nullptr;
            for(auto* e:ptr(st.player_units[1-owner])) if(!f.unit_dead(e)) {
                if(!target || f.ut_building(e)) target=e;
                if(f.unit_is(e,base_type)) break;
            }
            if(target) {
                unsigned sent=0;
                for(auto* m:marines) if(idle(m)) {
                    a.action_select(owner,m);
                    if(a.action_order(owner,a.get_order_type(Orders::AttackMove),target->sprite->position,nullptr,nullptr,false)) ++sent;
                }
                if(sent>=4) ++side.waves;
            }
            side.attack_frame=st.current_frame+240;
        }
    }
public:
    static constexpr unsigned unit_budget=40;
    void reset(bool red,bool blue,bool gas=false,bool protoss=false,bool zerg=false) { gas_economy_=gas;protoss_=protoss;zerg_=zerg; sides_={}; sides_[0].enabled=red; sides_[1].enabled=blue; winner_=-1; }
    bool enabled(unsigned p) const { return p<2 && sides_[p].enabled; }
    bool observer() const { return enabled(0) && enabled(1); }
    int winner() const { return winner_; }
    unsigned trained(unsigned p) const { return sides_[p].trained; }
    unsigned built(unsigned p) const { return sides_[p].built; }
    unsigned waves(unsigned p) const { return sides_[p].waves; }
    unsigned lost(unsigned p) const { return sides_[p].lost; }
    void step(game_player& player) {
        if(winner_>=0 || (!enabled(0) && !enabled(1))) return;
        if(player.st().current_frame%12) return;
        bool buildings[2]={false,false};
        for(unsigned p=0;p!=2;++p) {
            auto& side=sides_[p];
            for(auto id:side.previous) {
                auto* u=player.funcs().get_unit(id);
                if(!u || player.funcs().unit_dead(u)) ++side.lost;
            }
            side.previous.clear();
            for(auto* u:ptr(player.st().player_units[p])) if(!player.funcs().unit_dead(u) && side.previous.size()<64)
                side.previous.push_back(player.funcs().get_unit_id_32(u));
        }
        for(unsigned p=0;p!=2;++p) for(auto* u:ptr(player.st().player_units[p]))
            if(!player.funcs().unit_dead(u) && player.funcs().ut_building(u)) buildings[p]=true;
        if(!buildings[0] || !buildings[1]) { winner_=buildings[0]?0:buildings[1]?1:2; return; }
        update(player,0); update(player,1);
    }
};
} }
