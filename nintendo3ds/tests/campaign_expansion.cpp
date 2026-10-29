// Bounded expansion adapter fixture; no purchased mission map is modified.
#include "campaign_ai.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;using port3ds::detail::CampaignAI;
static void put(a_vector<uint8_t>& b,unsigned at,unsigned v,unsigned n){for(unsigned i=0;i<n;++i)b.at(at+i)=v>>(i*8);}
static void code(CampaignAI& ai,const a_vector<uint8_t>& instructions){a_vector<uint8_t> bank(36);put(bank,0,4,4);put(bank,4,0x54534554,4);put(bank,8,36,4);bank.insert(bank.end(),instructions.begin(),instructions.end());ai.load([&](a_vector<uint8_t>& out,const a_string& name){if(name=="scripts/aiscript.bin")out=bank;else{out.assign(12,0);put(out,0,4,4);}});}
static void init(game_player& p,const char* path){std::ifstream in(path,std::ios::binary);a_vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});game_load_functions loader(p.st());auto setup=[&]{loader.setup_info.create_no_units=true;for(auto& v:p.st().players){v.race=race_t::terran;v.controller=player_t::controller_occupied;}};loader.load_map_data(map.data(),map.size(),setup);
 // Isolated flat arena: repeat one fully walkable/buildable owned tileset tile.
 // This fixture geometry is never exported or substituted for a mission map.
 unsigned tile=0;bool found=false;for(unsigned i=0;i<p.st().tiles.size();++i){auto flags=p.st().tiles[i].flags;if((flags&tile_t::flag_walkable) && !(flags&(tile_t::flag_partially_walkable|tile_t::flag_unbuildable))){tile=p.st().game->gfx_tiles[i].raw_value;found=true;break;}}assert(found);for(unsigned at=0;at+8<=map.size();){unsigned n=unsigned(map[at+4])|unsigned(map[at+5])<<8|unsigned(map[at+6])<<16|unsigned(map[at+7])<<24;if(!memcmp(map.data()+at,"MTXM",4))for(unsigned j=0;j<n;j+=2){map[at+8+j]=tile;map[at+9+j]=tile>>8;}at+=8+n;}loader.load_map_data(map.data(),map.size(),setup);p.st().game->triggers.clear();for(auto& t:p.st().running_triggers)t.clear();}
static unit_t* create(state_functions& f,UnitTypes type,xy pos,unsigned owner=0){f.reveal_sight_at(pos,11,1u<<owner,true);auto* u=f.trigger_create_unit(f.get_unit_type(type),pos,owner);if(!u)fprintf(stderr,"create failed type%u pos%d,%d map%u,%u\n",unsigned(type),pos.x,pos.y,unsigned(f.st.game->map_width),unsigned(f.st.game->map_height));assert(u);return u;}

int main(int argc,char**argv){assert(argc==3);auto assets=data_loading::data_files_directory(argv[1]);
 for(unsigned race=0;race<3;++race)for(unsigned mode=0;mode<6;++mode){game_player p(assets);init(p,argv[2]);auto& f=p.funcs();auto& st=p.st();st.game->start_locations.fill({});st.game->start_location_count=0;st.players[0].race=static_cast<race_t>(race);st.alliances[0][1]=st.alliances[1][0]=2;
  auto* worker=create(f,race==0?UnitTypes::Zerg_Drone:race==2?UnitTypes::Protoss_Probe:UnitTypes::Terran_SCV,{600,600});auto* marker=create(f,UnitTypes::Hero_Dark_Templar,{600,700});auto depot_type=race==0?UnitTypes::Zerg_Hatchery:race==2?UnitTypes::Protoss_Nexus:UnitTypes::Terran_Command_Center;
  for(auto pos:{xy{1400,1300},xy{1464,1300},xy{1400,1400}}){auto* resource=create(f,UnitTypes::Resource_Mineral_Field,pos,11);f.set_unit_resources(resource,mode==2?0:1500);}
  if(mode==1)create(f,depot_type,{1664,1536},1);
  unsigned price=race==0?300:400;st.current_minerals[0]=mode==3?0:price;st.current_gas[0]=0;CampaignAI ai;if(mode==5)code(ai,{3,17,2,46,0,17,2,46,0,36,3,38,36});else code(ai,{111,1,45,0,111,1,45,0,36,38,36});ai.start(f,0x54534554,0,{400,400});unsigned paid_at=0,completed_at=0;unit_t* depot=nullptr;
  for(unsigned frame=0;frame<(mode && mode!=5?480u:7000u);++frame){f.next_frame();ai.step(f);if(mode==4 && frame==96)f.trigger_give_unit_to(worker,1);if(!paid_at && mode!=3 && st.current_minerals[0]<int(price))paid_at=st.current_frame;for(auto* u:ptr(st.player_units[0]))if(f.unit_is(u,depot_type)){depot=u;if(f.u_completed(u) && !completed_at)completed_at=st.current_frame;}if(!completed_at)assert(marker->order_type->id!=Orders::Move);}
  if(mode && mode!=5){assert(!depot && !paid_at && st.current_minerals[0]==int(mode==3?0:price));if(mode!=4)assert(worker->sprite->position==xy(600,600));printf("race%u %s area skipped with no worker diversion/payment PASS\n",race,mode==1?"occupied":mode==2?"depleted":mode==3?"insufficient funds":"captured worker");}
  else{assert(depot && completed_at && paid_at && paid_at<completed_at);if(mode!=5)assert(st.current_minerals[0]==0);assert(marker->order_type->id==Orders::Move || (marker->sprite->position-depot->sprite->position).x*(marker->sprite->position-depot->sprite->position).x+(marker->sprite->position-depot->sprite->position).y*(marker->sprite->position-depot->sprite->position).y<256*256);unsigned bases=0;for(auto* u:ptr(st.player_units[0]))if(f.unit_is(u,depot_type))++bases;assert(bases==1);printf("race%u native paid depot at%d,%d completed%u, child block/duplicate suppression PASS\n",race,depot->sprite->position.x,depot->sprite->position.y,completed_at);}
 }
}
