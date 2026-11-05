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
 for(unsigned mode=0;mode<2;++mode){game_player p(assets);init(p,argv[2]);auto& f=p.funcs();auto& st=p.st();for(auto& t:st.tech_researched)t.fill(true);st.alliances[0][1]=st.alliances[1][0]=2;
  auto* caster=create(f,mode?UnitTypes::Terran_Ghost:UnitTypes::Terran_Battlecruiser,{600,600});auto* target=create(f,UnitTypes::Terran_Battlecruiser,{760,600},1);auto* unrelated=create(f,UnitTypes::Terran_Civilian,{400,400});f.set_unit_energy(caster,fp8::integer(250));action_state a;action_functions commands(st,a);commands.action_select(0,caster);auto order=mode?Orders::CastLockdown:Orders::FireYamatoGun;assert(commands.action_order(0,f.get_order_type(order),target->sprite->position,target,nullptr,false));
  // A targeted spell belongs to its caster, not the last sprite animation.
  f.iscript_unit=unrelated;assert(f.spell_order_valid(caster));f.iscript_unit=nullptr;assert(f.spell_order_valid(caster));
  for(unsigned i=0;i<240;++i)f.next_frame();assert(caster->energy<fp8::integer(mode?170:120));if(mode)assert(target->lockdown_timer);else assert(target->hp<fp8::integer(300));
 }
 puts("Yamato damage/150 energy and Lockdown timer/100 energy with unrelated or absent iscript context PASS");
}
