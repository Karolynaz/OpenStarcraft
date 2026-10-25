#include "battle_ai.h"
#include <cassert>
#include <cstdio>
using namespace bwgame;using namespace port3ds;
struct Fixture {
 game_player player;game_load_functions f;unit_t* base[2]{};
 Fixture(const char* root,const char* map,unsigned race):player(data_loading::data_files_directory(root)),f(player.st()){
  a_vector<uint8_t> chk;data_loading::mpq_file<> archive(map);archive(chk,"staredit/scenario.chk");
  f.load_map_data(chk.data(),chk.size(),[&]{f.setup_info.starting_units=1;f.setup_info.resource_type=1;f.setup_info.starting_minerals=50;for(unsigned p=0;p<12;++p){auto& s=player.st().players[p];s.controller=p<2?player_t::controller_occupied:player_t::controller_inactive;s.race=race_t(race);f.setup_info.create_melee_units_for_player[p]=p<2;}});
  for(unsigned p=0;p<2;++p)for(auto* u:ptr(player.st().player_units[p]))if(f.ut_resource_depot(u)){base[p]=u;u->status_flags|=unit_t::status_flag_invincible;}assert(base[0] && base[1]);
 }
 unit_t* spawn(UnitTypes t,xy pos,unsigned p){for(unsigned n=0;n<64;++n){auto d=xy{int(n%8)*16-64,int(n/8)*16-64};if(auto* u=f.create_initial_unit(f.get_unit_type(t),pos+d,p))return u;}fprintf(stderr,"spawn failed type%u at%d,%d\n",unsigned(t),pos.x,pos.y);assert(false);return nullptr;}
 void step(detail::BattleTactics& t,unsigned n){std::array<bool,3> enemy{{true,false,false}};while(n--){player.next_frame();if(player.st().current_frame%12==0)t.step(player,1,base[1],enemy,AiDifficulty::medium);}}
};
static void transport(const char* root,const char* map,unsigned race){
 Fixture x(root,map,race);detail::BattleTactics t;auto home=x.base[1]->sprite->position;
 assert(!x.f.is_reachable(x.base[1],x.base[0]));
 auto type=race==0?UnitTypes::Zerg_Overlord:race==2?UnitTypes::Protoss_Shuttle:UnitTypes::Terran_Dropship;
 if(!race)x.player.st().upgrade_levels[1][UpgradeTypes::Ventral_Sacs]=1;
 auto* carrier=x.spawn(type,home+xy{0,128},1);carrier->status_flags|=unit_t::status_flag_invincible;
 for(unsigned n=0;n<8;++n)x.spawn(race==0?UnitTypes::Zerg_Zergling:race==2?UnitTypes::Protoss_Zealot:UnitTypes::Terran_Marine,home+xy{int(n%4)*24-48,128+int(n/4)*24},1);
 x.player.st().current_frame=24*60*3;
 x.step(t,24*180);printf("race%u island=%u loadorders=%u drops=%u\n",race,t.island(),t.loads(),t.drops());assert(t.island() && t.loads()>0 && t.drops()>0);
 bool arrived=false;for(auto* u:ptr(x.player.st().player_units[1]))if(!x.f.u_flying(u) && !x.f.ut_worker(u) && !x.f.ut_building(u) && x.f.is_reachable(u,x.base[0]))arrived=true;assert(arrived);
}
static void spell(const char* root,const char* map,UnitTypes caster,TechTypes tech,UnitTypes target,unsigned race){
 Fixture x(root,map,race);detail::BattleTactics t;auto home=x.base[1]->sprite->position;
 auto* c=x.spawn(caster,home+xy{0,192},1);c->energy=fp8::integer(250);c->status_flags|=unit_t::status_flag_invincible;x.player.st().tech_researched[1][tech]=true;
 for(unsigned n=0;n<4;++n){x.spawn(target,c->sprite->position+xy{160+int(n)*8,0},0);}
 // Real sight is updated by native frames, including the hostile target's island.
 bool energy_paid=false;for(unsigned i=0;i<24*15;++i){x.step(t,1);if(c->energy<fp8::integer(200))energy_paid=true;}
 printf("spell caster=%u accepted=%u energy=%d\n",unsigned(caster),t.casts(),c->energy.integer_part());assert(t.casts()>0 && energy_paid);
}
static void knowledge(const char* root,const char* map){Fixture x(root,map,1);detail::BattleTactics t;auto* e=x.spawn(UnitTypes::Terran_Wraith,x.base[0]->sprite->position+xy{0,128},0);e->sprite->visibility_flags&=~2;std::array<bool,3> enemy{{true,false,false}};
 t.step(x.player,1,x.base[1],enemy,AiDifficulty::medium);assert(t.seen_air()==0);e->sprite->visibility_flags|=2;t.step(x.player,1,x.base[1],enemy,AiDifficulty::medium);assert(t.seen_air()==1);e->sprite->visibility_flags&=~2;x.player.st().current_frame+=24*91;t.step(x.player,1,x.base[1],enemy,AiDifficulty::medium);assert(t.seen_air()==0);
 puts("Hidden aircraft ignored, visible aircraft remembered, stale intelligence expires PASS");}
static void team_victory(const char* root,const char* map){
 Fixture x(root,map,1);x.player.st().players[2].controller=player_t::controller_occupied;
 auto* ally=x.spawn(UnitTypes::Terran_Supply_Depot,x.base[0]->sprite->position+xy{256,0},2);assert(ally);
 MatchSetup setup;setup.enabled=setup.observer=true;setup.races={{1,2,0}};setup.teams={{0,1,1}};setup.side_difficulty={{1,0,0}};detail::BattleAI ai;ai.configure(setup);ai.step(x.player);assert(ai.winner()<0 && ai.enabled(0) && ai.enabled(1) && ai.enabled(2));assert(ai.difficulty(0)==AiDifficulty::medium && ai.difficulty(1)==AiDifficulty::easy && ai.difficulty(2)==AiDifficulty::easy);
 x.base[1]->status_flags&=~unit_t::status_flag_invincible;x.f.kill_unit(x.base[1]);ai.step(x.player);assert(ai.winner()<0); // the surviving Easy ally keeps the team alive
 x.base[0]->status_flags&=~unit_t::status_flag_invincible;x.f.kill_unit(x.base[0]);ai.step(x.player);assert(ai.winner()==2);
 puts("Mixed difficulty: dead Easy partner does not end team game; surviving Easy team wins PASS");
}
static void economy(const char* root,const char* map,unsigned race){
 Fixture x(root,map,race);detail::BattleAI ai;MatchSetup setup;setup.enabled=true;setup.observer=false;setup.difficulty=AiDifficulty::medium;setup.races={{int(race),int(race),-1}};ai.configure(setup);
 bool carrier=false,caster=false;unsigned drops=0,casts=0;
 for(unsigned n=0;n<60000;++n){x.player.next_frame();ai.step(x.player);if(n%120)continue;
  for(auto* u:ptr(x.player.st().player_units[1])){if(x.f.unit_provides_space(u) && (race!=0 || x.player.st().upgrade_levels[1][UpgradeTypes::Ventral_Sacs]))carrier=true;
   if(x.f.unit_is(u,race==0?UnitTypes::Zerg_Queen:race==2?UnitTypes::Protoss_High_Templar:UnitTypes::Terran_Science_Vessel))caster=true;}
  drops=std::max(drops,ai.tactics(1).drops());casts=std::max(casts,ai.tactics(1).casts());
 }
 unsigned live=0,workers=0;for(auto* u:ptr(x.player.st().player_units[1])){++live;if(x.f.ut_worker(u))++workers;}
 printf("economy workers=%u live=%u minerals=%d gas=%d built=%u trained=%u\n",workers,live,x.player.st().current_minerals[1],x.player.st().current_gas[1],ai.built(1),ai.trained(1));
 printf("Island native economy race%u carrier=%u caster=%u drops=%u castorders=%u scouts=%u casterdrops=%u\n",race,carrier,caster,drops,casts,ai.tactics(1).scouts(),ai.tactics(1).caster_drops());assert(carrier && caster && drops && ai.tactics(1).scouts());
}
int main(int argc,char** argv){assert(argc==4 || argc==5);if(argc==5){for(unsigned race:{1u,2u,0u})economy(argv[1],argv[3],race);return 0;}team_victory(argv[1],argv[2]);knowledge(argv[1],argv[2]);for(unsigned race:{1u,2u,0u})transport(argv[1],argv[3],race);
 spell(argv[1],argv[2],UnitTypes::Protoss_High_Templar,TechTypes::Psionic_Storm,UnitTypes::Terran_Marine,2);
 spell(argv[1],argv[2],UnitTypes::Terran_Science_Vessel,TechTypes::Irradiate,UnitTypes::Zerg_Ultralisk,1);
 spell(argv[1],argv[2],UnitTypes::Terran_Science_Vessel,TechTypes::EMP_Shockwave,UnitTypes::Protoss_Dragoon,1);
 spell(argv[1],argv[2],UnitTypes::Zerg_Queen,TechTypes::Spawn_Broodlings,UnitTypes::Zerg_Ultralisk,0);
 spell(argv[1],argv[2],UnitTypes::Zerg_Queen,TechTypes::Ensnare,UnitTypes::Terran_Marine,0);
 spell(argv[1],argv[2],UnitTypes::Zerg_Defiler,TechTypes::Plague,UnitTypes::Terran_Marine,0);
 puts("Native sight/energy/casting and all-race island cargo arrival PASS");}
