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
static void ai_test(const char* root,const char* map,unsigned race,AiDifficulty difficulty,bool require_expansion=false){
 auto assets=data_loading::data_files_directory(root);game_player player(assets);game_load_functions loader(player.st());a_vector<uint8_t> chk;data_loading::mpq_file<> archive(map);archive(chk,"staredit/scenario.chk");
 loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.starting_units=1;loader.setup_info.resource_type=1;loader.setup_info.starting_minerals=50;for(unsigned p=0;p<12;++p){player.st().players[p].controller=p<2?player_t::controller_occupied:player_t::controller_inactive;player.st().players[p].race=race_t(race);loader.setup_info.create_melee_units_for_player[p]=p<2;}});
 detail::BattleAI ai;MatchSetup setup;setup.enabled=true;setup.difficulty=difficulty;setup.races={{int(race),int(race),-1}};ai.configure(setup);
 // A non-fighting, invulnerable human base keeps this long economy fixture
 // alive, so the AI must fund its technology through actual native harvesting.
 for(auto* u:ptr(player.st().player_units[0]))u->status_flags|=unit_t::status_flag_invincible;
 std::set<unsigned> seen;unsigned max_army=0,first_wave=0,max_workers=0,max_bases=0;const auto profile=ai_profile(difficulty);
 for(unsigned frame=0;frame<36000;++frame){player.next_frame();ai.step(player);assert(ai.winner()<0);if(ai.waves(1) && !first_wave)first_wave=player.st().current_frame;assert(!first_wave || first_wave>=profile.first_attack);if(frame%24)continue;
  unsigned live=0,workers=0,bases=0;for(auto* u:ptr(player.st().player_units[1]))if(!loader.unit_dying(u)){++live;if(loader.ut_resource_depot(u))++bases;if(loader.ut_worker(u))++workers;seen.insert(unsigned(u->unit_type->id));}max_army=std::max(max_army,live);max_workers=std::max(max_workers,workers);max_bases=std::max(max_bases,bases);
 }
 unsigned upgrades=0;for(const auto level:player.st().upgrade_levels[1])upgrades+=level;
 std::printf("difficulty=%u firstwave=%u workers=%u bases=%u race %u trained=%u built=%u waves=%u upgrades=%u maxlive=%u types:",unsigned(difficulty),first_wave,max_workers,max_bases,race,ai.trained(1),ai.built(1),ai.waves(1),upgrades,max_army);for(auto t:seen)std::printf(" %u",t);puts("");fflush(stdout);
 assert(ai.trained(1)>6 && ai.built(1)>=3 && max_army<=profile.budget+5 && max_workers<=profile.workers && first_wave>=profile.first_attack && first_wave<profile.first_attack+24*60*3);
 if(require_expansion)assert(max_bases>=2);
 if(difficulty==AiDifficulty::medium){assert(upgrades>0);
 if(race==1){assert(seen.count(unsigned(UnitTypes::Terran_Factory)));assert(seen.count(unsigned(UnitTypes::Terran_Siege_Tank_Tank_Mode)));}
 if(race==2){assert(seen.count(unsigned(UnitTypes::Protoss_Dragoon)));assert(seen.count(unsigned(UnitTypes::Protoss_Stargate)));}
 if(race==0){assert(seen.count(unsigned(UnitTypes::Zerg_Lair)));assert(seen.count(unsigned(UnitTypes::Zerg_Mutalisk)));}
 }
 // All active enemy buildings destroyed => victory. A killed human base =>
 // defeat; remaining workers or army cannot keep a building-less side alive.
 for(auto* u:ptr(player.st().player_units[1]))if(loader.ut_building(u))loader.kill_unit(u);
 for(unsigned i=0;i<24;++i){player.next_frame();ai.step(player);}assert(ai.winner()==0);
 ai.configure(setup);for(auto* u:ptr(player.st().player_units[0]))if(loader.ut_building(u)){u->status_flags&=~unit_t::status_flag_invincible;loader.kill_unit(u);}for(unsigned i=0;i<24;++i){player.next_frame();ai.step(player);}assert(ai.winner()==1 || ai.winner()==3);
}
static void knowledge_test(const char* root,const char* map){
 auto assets=data_loading::data_files_directory(root);game_player player(assets);game_load_functions f(player.st());a_vector<uint8_t> chk;data_loading::mpq_file<> archive(map);archive(chk,"staredit/scenario.chk");
 f.load_map_data(chk.data(),chk.size(),[&]{f.setup_info.starting_units=1;for(unsigned p=0;p<12;++p){player.st().players[p].controller=p<2?player_t::controller_occupied:player_t::controller_inactive;player.st().players[p].race=race_t::terran;f.setup_info.create_melee_units_for_player[p]=p<2;}});
 MatchSetup setup;setup.enabled=true;setup.difficulty=AiDifficulty::medium;detail::BattleAI ai;ai.configure(setup);
 unit_t* enemy=nullptr;for(auto* u:ptr(player.st().player_units[0])){u->sprite->visibility_flags&=~2;if(f.ut_resource_depot(u))enemy=u;}assert(enemy);auto* spare=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Supply_Depot),enemy->sprite->position+xy{0,-256},0);assert(spare);spare->sprite->visibility_flags&=~2;
 const auto pos=enemy->sprite->position;auto& tile=player.st().tiles[f.tile_index(pos)];tile.visible|=2;
 ai.step(player);assert(ai.remembered(1)==0);
 enemy->sprite->visibility_flags|=2;tile.visible&=~2;ai.step(player);assert(ai.remembered(1)==1);
 enemy->sprite->visibility_flags&=~2;tile.visible|=2;f.kill_unit(enemy);ai.step(player);assert(ai.remembered(1)==1); // hidden death cannot erase memory
 tile.visible&=~2;ai.step(player);assert(ai.remembered(1)==0); // scout confirms empty location
 // Grace protects against early offensive waves, but does not disable defence.
 unit_t* base=nullptr;for(auto* u:ptr(player.st().player_units[1]))if(f.ut_resource_depot(u))base=u;assert(base);
 for(unsigned n=0;n<6;++n)assert(f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Marine),base->sprite->position+xy{int(n)*16,96},1));
 auto* invader=f.create_initial_unit(f.get_unit_type(UnitTypes::Terran_Marine),base->sprite->position+xy{192,-96},0);assert(invader);invader->sprite->visibility_flags|=2;
 setup.difficulty=AiDifficulty::easy;ai.configure(setup);ai.step(player);assert(ai.waves(1)>0 && ai.target(1)==invader->sprite->position);
 puts("AI sight / retained building memory / hidden death / scouted empty location / Easy pre-grace defence PASS");
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
static void three_side_test(const char* root,const char* map){
 Engine engine;MatchSetup setup;setup.enabled=true;setup.difficulty=AiDifficulty::medium;setup.races={{1,0,2}};
 assert(engine.load(root,map,Scenario::versus_ai,&setup));unsigned peak=0;Snapshot last;
 for(unsigned frame=0;frame<36000;++frame){assert(engine.tick());SoundEvent sounds[256];engine.drain_sounds(sounds,256);assert(engine.dropped_sounds()==0);if(frame%24)continue;last=engine.snapshot();peak=std::max(peak,last.units);assert(last.players[1].live<=ai_unit_budget+6 && last.players[2].live<=ai_unit_budget+6);if(last.winner>=0)break;}
 assert(last.players[1].trained>6 && last.players[2].trained>6 && last.players[1].built>=3 && last.players[2].built>=3);
 printf("Three-race/two-AI native match frame=%u peak=%u winner=%d Zerg trained=%u built=%u Protoss trained=%u built=%u zero sound-event drops PASS\n",last.frame,peak,last.winner,last.players[1].trained,last.players[1].built,last.players[2].trained,last.players[2].built);
}
int main(int argc,char** argv){assert(argc>=3 && argc<=5);const std::string folder="/tmp/sc3ds-melee-"+std::to_string(getpid());assert(!mkdir(folder.c_str(),0700));
 const auto path=folder+"/sparse.chk",cache=folder+"/terrain.bin";save(path,fixture(argv[2],128,true));Engine e;MatchSetup setup;setup.enabled=true;setup.races={{1,2,-1}};
 assert(e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));auto s=e.snapshot();assert(s.width==128 && s.height==128 && s.players[0].workers==4 && s.players[1].workers==4 && !s.players[2].live);assert(e.write_terrain(argv[1],cache.c_str()));assert(e.surrender() && e.snapshot().winner==1 && !e.surrender());
 std::ifstream bank(cache,std::ios::binary);std::vector<unsigned char> encoded((std::istreambuf_iterator<char>(bank)),{});
 auto dword=[&](size_t at){return data_loading::value_at<uint32_t,true>(encoded.data()+at);};const unsigned data_offset=24+128*128*4+128*128*2;
 {Graphics g;assert(g.load(cache.c_str(),e.terrain_signature()));for(unsigned y=0;y<4096-240;y+=192)for(unsigned x=0;x<4096-400;x+=256){fake::draws.clear();g.terrain(x,y);g.minimap(0,0,115,108);assert(g.ready() && !fake::draws.empty());for(const auto& draw:fake::draws){assert(draw.tex && draw.tex->data);const unsigned rx=unsigned(std::lround(draw.sub.left*512)),ry=unsigned(std::lround((1-draw.sub.top)*512));
  for(unsigned row: {0u,7u,16u,31u})for(unsigned col: {0u,7u,16u,31u}){unsigned expected=0;if(draw.sub.width==128)expected=data_loading::value_at<uint16_t,true>(encoded.data()+24+128*128*4+(row*128+col)*2);else {const unsigned wx=unsigned(int(draw.x)+int(x)),wy=unsigned(int(draw.y)+int(y));const auto id=dword(24+(wy/32*128+wx/32)*4);expected=data_loading::value_at<uint16_t,true>(encoded.data()+data_offset+id*2048+(row*32+col)*2);}const unsigned px=rx+col,py=ry+row,m=(px&1)|((py&1)<<1)|((px&2)<<1)|((py&2)<<2)|((px&4)<<2)|((py&4)<<3);assert(static_cast<const uint16_t*>(draw.tex->data)[((py/8)*64+px/8)*64+m]==expected);}
 }}g.fog(e,3500,3500);g.fog_minimap(e,0,0,115,108);}
 assert(fake::allocations==0);eviction_test(cache,encoded,e.terrain_signature());assert(fake::allocations==0);setup.game_type=2;assert(!e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));assert(std::strstr(e.error(),"Map Settings"));setup.game_type=0;
 save(path,fixture(argv[2],256,false));assert(!e.load(argv[1],path.c_str(),Scenario::versus_ai,&setup));assert(std::strstr(e.error(),"128"));
 knowledge_test(argv[1],argv[2]);
 for(auto difficulty: {AiDifficulty::easy,AiDifficulty::medium})for(unsigned race: {1u,2u,0u})ai_test(argv[1],argv[2],race,difficulty);
 if(argc>=4)for(unsigned race: {1u,2u,0u})ai_test(argv[1],argv[3],race,AiDifficulty::medium,true);
 if(argc==5)three_side_test(argv[1],argv[4]);
 std::remove(path.c_str());std::remove(cache.c_str());rmdir(folder.c_str());puts("Actual 128x128/sparse-start loading + streamed terrain/fog + native economy/technology + victory/defeat PASS");
}
