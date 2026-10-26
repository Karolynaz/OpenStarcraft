// Original Desperate Alliance: no grants or rewritten triggers. Survive using
// paid mining/production, bunker loading and SCV repair through public controls.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <set>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_campaign_3));assert(e.start_recording("/tmp/sc3ds-terran03-session"));e.set_fog_enabled(false);unsigned voices=0,repair_orders=0;bool timer_seen=false;
for(unsigned frame=0;frame<65000 && e.snapshot().winner<0;++frame){if(!e.tick()){fprintf(stderr,"frame%u %s\n",e.frame(),e.error());return 1;}SoundEvent sounds[256];unsigned ns=e.drain_sounds(sounds,256);for(unsigned i=0;i<ns;++i)if(sounds[i].id>=1200)++voices;if(frame%48)continue;
 UnitView list[1700];unsigned n=e.visible_units(list,1700);std::vector<UnitView> workers,parents,army,bunkers;std::set<unsigned> occupied;auto s=e.snapshot();timer_seen|=s.countdown_active && s.countdown_seconds>1700;
 for(unsigned i=0;i<n;++i){auto& u=list[i];if(u.owner==0){if(u.completed){parents.push_back(u);if(u.type==7)workers.push_back(u);if(u.type==0 || u.type==2)army.push_back(u);}if(u.type==125)bunkers.push_back(u);}}
 // Enter nearby bunkers first. Remaining soldiers guard the two approaches.
 if(frame%240==0)for(const auto& u:army){const UnitView* target=nullptr;int best=0x7fffffff;for(const auto& b:bunkers)if(b.completed && b.cargo<4 && u.type==0){int dx=u.x-b.x,dy=u.y-b.y,d=dx*dx+dy*dy;if(d<best){best=d;target=&b;}}if(target && e.select(u.x,u.y))e.command(Command::context,target->x,target->y);else if(e.select(u.x,u.y))e.command(Command::attack,u.x<640?160:1056,u.x<640?2400:2800);}
 // Two repair workers per damaged defensive structure, with unique selection.
 if(frame%240==0)for(const auto& b:parents)if((b.type==125 || b.type==124 || b.type==106) && b.hp<b.max_hp*9/10){unsigned assigned=0;for(const auto& w:workers){unsigned id=e.own_unit_at(w.x,w.y);if(!id || occupied.count(id))continue;int dx=w.x-b.x,dy=w.y-b.y;if(dx*dx+dy*dy>1200*1200)continue;if(e.select(w.x,w.y) && e.command(Command::context,b.x,b.y)){occupied.insert(id);++repair_orders;if(++assigned==2)break;}}}
 for(const auto& w:workers)if(!occupied.count(e.own_unit_at(w.x,w.y)) && (w.order==unsigned(bwgame::Orders::PlayerGuard) || w.order==unsigned(bwgame::Orders::Guard) || w.order==unsigned(bwgame::Orders::Stop))){const UnitView* target=nullptr;int best=0x7fffffff;for(unsigned i=0;i<n;++i)if(list[i].mineral){int dx=list[i].x-w.x,dy=list[i].y-w.y,d=dx*dx+dy*dy;if(d<best){best=d;target=&list[i];}}if(target && e.select(w.x,w.y))e.command(Command::context,target->x,target->y);}
 unsigned want=s.supply_max-s.supply<5?109:e.count_units(0,111,false)<3?111:0;
 // Only one worker construction at a time; include foundations in counts.
 bool building=false;for(const auto& w:workers)if(w.order==unsigned(bwgame::Orders::PlaceBuilding) || w.order==unsigned(bwgame::Orders::ConstructingBuilding))building=true;
 if(want && !building && s.minerals>=150 && !workers.empty()){bool placed=false;const auto& w=workers.back();if(!occupied.count(e.own_unit_at(w.x,w.y)) && e.select(w.x,w.y))for(int r=3;r<16 && !placed;++r)for(int y=-r;y<=r && !placed;++y)for(int x=-r;x<=r && !placed;++x){if(abs(x)!=r && abs(y)!=r)continue;int px=448+x*32,py=2800+y*32;if(e.placement_unit(want,px,py).valid)placed=e.perform(option_code(OptionKind::build,want),px,py);}}
 for(const auto& p:parents)if(e.select(p.x,p.y)){if(p.type==106 && e.count_units(0,7,false)<14)e.perform(option_code(OptionKind::train,7));else if(p.type==111 && e.count_units(0,0,false)<60)e.perform(option_code(OptionKind::train,0));}
 if(frame%4800==0){printf("Desperate Alliance frame%u timer%u minerals%d marines%u SCV%u bunkers%zu repairs%u\n",e.frame(),s.countdown_seconds,s.minerals,e.count_units(0,0),e.count_units(0,7),bunkers.size(),repair_orders);fflush(stdout);}
}
printf("Desperate Alliance result%d frame%u transmissions%u repairs%u\n",e.snapshot().winner,e.frame(),voices,repair_orders);assert(e.snapshot().winner==0 && timer_seen && voices>=5);SaveView view;assert(e.save_game("/tmp/sc3ds-terran03-completed.sav",view));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-terran03-completed.sav","/tmp/sc3ds-terran03-replay","/tmp/sc3ds-terran03-map",loaded));assert(e.state_digest()==replay.state_digest());puts("Desperate Alliance original countdown/evacuation, paid defense, dialogue and completed save replay PASS");}
