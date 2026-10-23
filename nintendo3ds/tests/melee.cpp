#include "graphics.h"
#include "actions.h"
#include "battle_ai.h"
#include <cassert>
#include <fstream>
#include <set>
#include <unistd.h>
#include <sys/stat.h>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
using namespace bwgame;using namespace port3ds;
static void save(const std::string& name,const a_vector<uint8_t>& bytes){std::ofstream f(name,std::ios::binary);f.write((const char*)bytes.data(),bytes.size());assert(f.good());}
static a_vector<uint8_t> fixture(const char* path,unsigned size,bool sparse){
 a_vector<uint8_t> source;data_loading::mpq_file<> archive(path);archive(source,"staredit/scenario.chk");a_vector<uint8_t> result;
 auto append=[&](const uint8_t* tag,const a_vector<uint8_t>& b){result.insert(result.end(),tag,tag+4);for(unsigned j=0;j<4;++j)result.push_back(uint8_t(b.size()>>(j*8)));result.insert(result.end(),b.begin(),b.end());};
 for(size_t at=0;at+8<=source.size();){auto* tag=source.data()+at;unsigned len=data_loading::value_at<uint32_t,true>(tag+4);at+=8;assert(len<=source.size()-at);a_vector<uint8_t> bytes(source.begin()+at,source.begin()+at+len);at+=len;
  if(!memcmp(tag,"DIM ",4))bytes={uint8_t(size),uint8_t(size>>8),uint8_t(size),uint8_t(size>>8)};
  if(!memcmp(tag,"MTXM",4)){bytes.resize(size*size*2);for(unsigned y=0;y<size;++y)for(unsigned x=0;x<size;++x){bytes[(y*size+x)*2]=source[at-len+((y%64)*64+x%64)*2];bytes[(y*size+x)*2+1]=source[at-len+((y%64)*64+x%64)*2+1];}}
  if(!memcmp(tag,"MASK",4))bytes.resize(size*size);
  if(sparse && !memcmp(tag,"UNIT",4)){unsigned n=0;for(size_t i=0;i+36<=bytes.size();i+=36)if(data_loading::value_at<uint16_t,true>(bytes.data()+i+8)==214){if(n<2)bytes[i+16]=n++?6:4;else {bytes.erase(bytes.begin()+i,bytes.begin()+i+36);i-=36;}}}
  append(tag,bytes);
 }return result;
}
static void ai_test(const char* root,const char* map,unsigned race){
 auto assets=data_loading::data_files_directory(root);game_player player(assets);game_load_functions loader(player.st());a_vector<uint8_t> chk;data_loading::mpq_file<> archive(map);archive(chk,"staredit/scenario.chk");
 loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.starting_units=1;loader.setup_info.resource_type=1;loader.setup_info.starting_minerals=50;for(unsigned p=0;p<12;++p){player.st().players[p].controller=p<2?player_t::controller_occupied:player_t::controller_inactive;player.st().players[p].race=race_t(race);loader.setup_info.create_melee_units_for_player[p]=p<2;}});
 detail::BattleAI ai;MatchSetup setup;setup.enabled=true;setup.races={{int(race),int(race),-1}};ai.configure(setup);
 // A non-fighting, invulnerable human base keeps this long economy fixture
 // alive, so the AI must fund its technology through actual native harvesting.
 for(auto* u:ptr(player.st().player_units[0]))u->status_flags|=unit_t::status_flag_invincible;
 std::set<unsigned> seen;unsigned max_army=0;
 for(unsigned frame=0;frame<36000;++frame){player.next_frame();ai.step(player);assert(ai.winner()<0);if(frame%24)continue;
  unsigned live=0;for(auto* u:ptr(player.st().player_units[1]))if(!loader.unit_dying(u)){++live;seen.insert(unsigned(u->unit_type->id));}max_army=std::max(max_army,live);
 }
 unsigned upgrades=0;for(const auto level:player.st().upgrade_levels[1])upgrades+=level;
 std::printf("race %u trained=%u built=%u waves=%u upgrades=%u maxlive=%u types:",race,ai.trained(1),ai.built(1),ai.waves(1),upgrades,max_army);for(auto t:seen)std::printf(" %u",t);puts("");fflush(stdout);
 assert(ai.trained(1)>8 && ai.built(1)>=5 && upgrades>0 && max_army<=detail::BattleAI::unit_budget+5);
 if(race==1){assert(seen.count(unsigned(UnitTypes::Terran_Factory)));assert(seen.count(unsigned(UnitTypes::Terran_Siege_Tank_Tank_Mode)));}
 if(race==2){assert(seen.count(unsigned(UnitTypes::Protoss_Dragoon)));assert(seen.count(unsigned(UnitTypes::Protoss_Stargate)));}
 if(race==0){assert(seen.count(unsigned(UnitTypes::Zerg_Lair)));assert(seen.count(unsigned(UnitTypes::Zerg_Mutalisk)));}
 // All active enemy buildings destroyed => victory. A killed human base =>
 // defeat; remaining workers or army cannot keep a building-less side alive.
 for(auto* u:ptr(player.st().player_units[1]))if(loader.ut_building(u))loader.kill_unit(u);
 for(unsigned i=0;i<24;++i){player.next_frame();ai.step(player);}assert(ai.winner()==0);
 ai.configure(setup);for(auto* u:ptr(player.st().player_units[0]))if(loader.ut_building(u)){u->status_flags&=~unit_t::status_flag_invincible;loader.kill_unit(u);}for(unsigned i=0;i<24;++i){player.next_frame();ai.step(player);}assert(ai.winner()==3);
}
static void eviction_test(const std::string& path,std::vector<unsigned char> bytes,unsigned signature){
 const unsigned offset=24+128*128*4+128*128*2;bytes.resize(offset+600*2048);
 auto put=[&](unsigned at,unsigned v,unsigned n){for(unsigned i=0;i<n;++i)bytes[at+i]=uint8_t(v>>(i*8));};
 put(20,600,4);for(unsigned i=0;i<128*128;++i)put(24+i*4,i%600,4);
 for(unsigned id=0;id<600;++id)for(unsigned px=0;px<1024;++px)put(offset+id*2048+px*2,id*2+1,2);
 {std::ofstream file(path,std::ios::binary);file.write((const char*)bytes.data(),bytes.size());assert(file.good());}
 Graphics g;assert(g.load(path.c_str(),signature));unsigned checked=0;
 for(unsigned trip=0;trip<2;++trip)for(unsigned y=0;y<3800;y+=224)for(unsigned x=0;x<3600;x+=320){fake::draws.clear();g.terrain(x,y);g.minimap(0,0,115,108);assert(g.ready());
  for(const auto& draw:fake::draws){const unsigned rx=unsigned(std::lround(draw.sub.left*512)),ry=unsigned(std::lround((1-draw.sub.top)*512));
   for(unsigned row: {0u,15u,31u})for(unsigned col: {0u,15u,31u}){unsigned expected;
    if(draw.sub.width==128)expected=data_loading::value_at<uint16_t,true>(bytes.data()+24+128*128*4+(row*128+col)*2);
    else {const unsigned wx=unsigned(int(draw.x)+int(x)),wy=unsigned(int(draw.y)+int(y));expected=((wy/32*128+wx/32)%600)*2+1;}
    const unsigned px=rx+col,py=ry+row,m=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);
    assert(static_cast<const uint16_t*>(draw.tex->data)[((py/8)*64+px/8)*64+m]==expected);++checked;
   }
  }
 }
 printf("600-tile bank / 496 resident slots / repeated eviction and minimap protection: %u pixel checks PASS\n",checked);
}
int main(int argc,char** argv){assert(argc==3);const std::string folder="/tmp/sc3ds-melee-"+std::to_string(getpid());assert(!mkdir(folder.c_str(),0700));
 const auto path=folder+"/sparse.chk",cache=folder+"/terrain.bin";save(path,fixture(argv[2],128,true));Engine e;MatchSetup setup;setup.enabled=true;setup.races={{1,2,-1}};
 assert(e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));auto s=e.snapshot();assert(s.width==128 && s.height==128 && s.players[0].workers==4 && s.players[1].workers==4 && !s.players[2].live);assert(e.write_terrain(argv[1],cache.c_str()));assert(e.surrender() && e.snapshot().winner==1 && !e.surrender());
 std::ifstream bank(cache,std::ios::binary);std::vector<unsigned char> encoded((std::istreambuf_iterator<char>(bank)),{});
 auto dword=[&](size_t at){return data_loading::value_at<uint32_t,true>(encoded.data()+at);};const unsigned data_offset=24+128*128*4+128*128*2;
 {Graphics g;assert(g.load(cache.c_str(),e.terrain_signature()));for(unsigned y=0;y<4096-240;y+=192)for(unsigned x=0;x<4096-400;x+=256){fake::draws.clear();g.terrain(x,y);g.minimap(0,0,115,108);assert(g.ready() && !fake::draws.empty());for(const auto& draw:fake::draws){assert(draw.tex && draw.tex->data);const unsigned rx=unsigned(std::lround(draw.sub.left*512)),ry=unsigned(std::lround((1-draw.sub.top)*512));
  for(unsigned row: {0u,7u,16u,31u})for(unsigned col: {0u,7u,16u,31u}){unsigned expected=0;if(draw.sub.width==128)expected=data_loading::value_at<uint16_t,true>(encoded.data()+24+128*128*4+(row*128+col)*2);else {const unsigned wx=unsigned(int(draw.x)+int(x)),wy=unsigned(int(draw.y)+int(y));const auto id=dword(24+(wy/32*128+wx/32)*4);expected=data_loading::value_at<uint16_t,true>(encoded.data()+data_offset+id*2048+(row*32+col)*2);}const unsigned px=rx+col,py=ry+row,m=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);assert(static_cast<const uint16_t*>(draw.tex->data)[((py/8)*64+px/8)*64+m]==expected);}
 }}g.fog(e,3500,3500);g.fog_minimap(e,0,0,115,108);}
 assert(fake::allocations==0);eviction_test(cache,encoded,e.terrain_signature());assert(fake::allocations==0);setup.game_type=2;assert(!e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));assert(std::strstr(e.error(),"Map Settings"));setup.game_type=0;
 save(path,fixture(argv[2],256,false));assert(!e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));assert(std::strstr(e.error(),"128"));
 for(unsigned race: {1u,2u,0u})ai_test(argv[1],argv[2],race);
 std::remove(path.c_str());std::remove(cache.c_str());rmdir(folder.c_str());puts("Actual 128x128/sparse-start loading + streamed terrain/fog + native economy/technology + victory/defeat PASS");
}
