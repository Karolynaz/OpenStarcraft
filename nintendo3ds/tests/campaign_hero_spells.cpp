// Isolated owned-data arena; shipped campaign maps and stats remain untouched.
#include "engine.h"
#include "campaign_map.h"
#include "actions.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <unistd.h>
using namespace bwgame;
static void init(game_player& p,const char* path){std::ifstream in(path,std::ios::binary);a_vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});game_load_functions loader(p.st());auto setup=[&]{loader.setup_info.create_no_units=true;for(auto& v:p.st().players){v.race=race_t::terran;v.controller=player_t::controller_occupied;}};loader.load_map_data(map.data(),map.size(),setup);
 // Isolated flat arena: repeat one fully walkable/buildable owned tileset tile.
 // This fixture geometry is never exported or substituted for a mission map.
 unsigned tile=0;bool found=false;for(unsigned i=0;i<p.st().tiles.size();++i){auto flags=p.st().tiles[i].flags;if((flags&tile_t::flag_walkable) && !(flags&(tile_t::flag_partially_walkable|tile_t::flag_unbuildable))){tile=p.st().game->gfx_tiles[i].raw_value;found=true;break;}}assert(found);for(unsigned at=0;at+8<=map.size();){unsigned n=unsigned(map[at+4])|unsigned(map[at+5])<<8|unsigned(map[at+6])<<16|unsigned(map[at+7])<<24;if(!memcmp(map.data()+at,"MTXM",4))for(unsigned j=0;j<n;j+=2){map[at+8+j]=tile;map[at+9+j]=tile>>8;}at+=8+n;}loader.load_map_data(map.data(),map.size(),setup);p.st().game->triggers.clear();for(auto& t:p.st().running_triggers)t.clear();}
static unit_t* create(state_functions& f,UnitTypes type,xy pos,unsigned owner=0){f.reveal_sight_at(pos,11,1u<<owner,true);auto* u=f.trigger_create_unit(f.get_unit_type(type),pos,owner);if(!u)fprintf(stderr,"create failed type%u pos%d,%d map%u,%u\n",unsigned(type),pos.x,pos.y,unsigned(f.st.game->map_width),unsigned(f.st.game->map_height));assert(u);return u;}

int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);
 for(unsigned mode=0;mode<4;++mode){game_player p(assets);init(p,argv[2]);auto& f=p.funcs();auto& st=p.st();for(auto& a:st.game->tech_available)a.fill(true);for(auto& a:st.tech_researched)a.fill(false);st.alliances[0][1]=st.alliances[1][0]=2;
  auto* hero=create(f,mode==3?UnitTypes::Hero_Tassadar:UnitTypes::Hero_Infested_Kerrigan,{600,600});f.set_unit_energy(hero,fp8::integer(200));auto* target=create(f,UnitTypes::Zerg_Ultralisk,{760,600});action_state state;action_functions commands(st,state);commands.action_select(0,hero);
  if(mode==0){assert(!f.unit_is_ghost(hero));assert(f.unit_can_use_tech(hero,f.get_tech_type(TechTypes::Personnel_Cloaking)));assert(!f.unit_can_use_tech(hero,f.get_tech_type(TechTypes::Lockdown)));commands.action_cloak(0);for(unsigned i=0;i<24;++i)f.next_frame();assert(f.u_requires_detector(hero) && hero->energy<fp8::integer(200));commands.action_select(0,hero);commands.action_decloak(0);for(unsigned i=0;i<24;++i)f.next_frame();assert(!f.u_requires_detector(hero));puts("Infested Kerrigan native cloak/energy/decloak, no Ghost/Lockdown classification PASS");}
  if(mode==1){assert(commands.action_order(0,f.get_order_type(Orders::CastEnsnare),target->sprite->position,nullptr,nullptr,false));for(unsigned i=0;i<160;++i)f.next_frame();assert(target->ensnare_timer && hero->energy<fp8::integer(150));puts("Infested Kerrigan native Ensnare timer and energy cost PASS");}
  if(mode==2){f.set_unit_energy(hero,fp8::integer(50));assert(commands.action_order(0,f.get_order_type(Orders::CastConsume),target->sprite->position,target,nullptr,false));for(unsigned i=0;i<160;++i)f.next_frame();assert(f.unit_dying(target) || target->order_type->id==Orders::Die);assert(hero->energy>=fp8::integer(90));puts("Infested Kerrigan native Consume removes own target and restores energy PASS");}
  if(mode==3){auto hp=target->hp;assert(commands.action_order(0,f.get_order_type(Orders::CastPsionicStorm),target->sprite->position,nullptr,nullptr,false));for(unsigned i=0;i<160;++i)f.next_frame();assert(target->hp<hp && hero->energy<fp8::integer(150));puts("Tassadar native Psionic Storm damage and energy cost PASS");}
 }
}
