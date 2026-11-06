// Synthetic states use owned DAT/iscript files; no game data is compiled in.
// Real native attack orders exercise value_area registration and consumption.
#include "campaign_ai.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <set>
using namespace bwgame;using port3ds::detail::CampaignAI;
static void put(a_vector<uint8_t>& b,unsigned at,unsigned v,unsigned n){for(unsigned i=0;i<n;++i)b.at(at+i)=v>>(i*8);}
static void code(CampaignAI& ai,const a_vector<uint8_t>& instructions){a_vector<uint8_t> bank(36);put(bank,0,4,4);put(bank,4,0x54534554,4);put(bank,8,36,4);bank.insert(bank.end(),instructions.begin(),instructions.end());ai.load([&](a_vector<uint8_t>& out,const a_string& name){if(name=="scripts/aiscript.bin")out=bank;else{out.assign(12,0);put(out,0,4,4);}});}
static void init(game_player& p,const char* path){std::ifstream in(path,std::ios::binary);a_vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});game_load_functions loader(p.st());auto setup=[&]{loader.setup_info.create_no_units=true;for(auto& v:p.st().players){v.race=race_t::terran;v.controller=player_t::controller_occupied;}};loader.load_map_data(map.data(),map.size(),setup);
 // Isolated flat arena: repeat one fully walkable/buildable owned tileset tile.
 // This fixture geometry is never exported or substituted for a mission map.
 unsigned tile=0;bool found=false;for(unsigned i=0;i<p.st().tiles.size();++i){auto flags=p.st().tiles[i].flags;if((flags&tile_t::flag_walkable) && !(flags&(tile_t::flag_partially_walkable|tile_t::flag_unbuildable))){tile=p.st().game->gfx_tiles[i].raw_value;found=true;break;}}assert(found);for(unsigned at=0;at+8<=map.size();){unsigned n=unsigned(map[at+4])|unsigned(map[at+5])<<8|unsigned(map[at+6])<<16|unsigned(map[at+7])<<24;if(!memcmp(map.data()+at,"MTXM",4))for(unsigned j=0;j<n;j+=2){map[at+8+j]=tile;map[at+9+j]=tile>>8;}at+=8+n;}loader.load_map_data(map.data(),map.size(),setup);p.st().game->triggers.clear();for(auto& t:p.st().running_triggers)t.clear();}
static unit_t* create(state_functions& f,UnitTypes type,xy pos,unsigned owner=0){f.reveal_sight_at(pos,11,1u<<owner,true);auto* u=f.trigger_create_unit(f.get_unit_type(type),pos,owner);if(!u)fprintf(stderr,"create failed type%u pos%d,%d map%u,%u\n",unsigned(type),pos.x,pos.y,unsigned(f.st.game->map_width),unsigned(f.st.game->map_height));assert(u);return u;}

// Original opcode42 has no parameters. Existing enemies suppress registration;
// a buildingless region receives one defensive response, owned buildings retain it.
int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);
 for(unsigned mode=0;mode<4;++mode){
  game_player p(assets);init(p,argv[2]);auto&f=p.funcs();auto&st=p.st();
  auto*r=f.get_region_at({1000,1000});xy center{r->center.x.integer_part(),r->center.y.integer_part()};auto* marine=create(f,UnitTypes::Terran_Marine,{500,1000});
  if(mode==1)create(f,UnitTypes::Terran_Supply_Depot,{960,960});
  unit_t* prior=nullptr;
  if(mode==2)prior=create(f,UnitTypes::Zerg_Drone,center,1);
  if(mode==3){st.alliances[0][2]=st.alliances[2][0]=2;prior=create(f,UnitTypes::Zerg_Drone,center,2);}
  f.reveal_sight_at(center,11,1,true);CampaignAI ai;code(ai,{42,36});ai.start(f,0x54534554,0,center);
  auto* enemy=mode==2?prior:create(f,UnitTypes::Zerg_Drone,center,1);
  assert(f.get_region_at(enemy->sprite->position)->index==f.get_region_at(center)->index);
  st.current_frame=96;ai.step(f);
  if(mode==2){assert(marine->order_type->id==Orders::PlayerGuard || marine->order_type->id==Orders::Guard);puts("value_area: already-hostile region ignored PASS");continue;}
  assert(marine->order_type->id==Orders::AttackMove && marine->order_target.pos==enemy->sprite->position);
  f.kill_unit(enemy);f.set_unit_order(marine,f.get_order_type(Orders::PlayerGuard));
  auto* next=create(f,UnitTypes::Zerg_Drone,center,1);assert(f.get_region_at(next->sprite->position)->index==f.get_region_at(center)->index);
  st.current_frame=192;ai.step(f);
  if(mode==1){assert(marine->order_type->id==Orders::AttackMove && marine->order_target.pos==next->sprite->position);puts("value_area: own building retains defensive region for next wave PASS");}
  else {assert(marine->order_type->id==Orders::PlayerGuard);puts(mode==3?"value_area: ally does not suppress marking; buildingless response consumed once PASS":"value_area: distant own guard responds once, no repeated permanent defense PASS");}
 }
}
