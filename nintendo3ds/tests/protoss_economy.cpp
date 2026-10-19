#include "graphics.h"
#include "battle.h"
#include <set>
#include <fstream>
#include <atomic>
#include <stdexcept>
namespace fake {std::atomic<unsigned> queued{0},allocations{0},pauses{0};std::vector<Draw> draws;}
static unsigned u32(std::ifstream& f){unsigned char b[4];f.read(reinterpret_cast<char*>(b),4);if(!f)throw std::runtime_error("Audio bank truncated");return b[0]|b[1]<<8|b[2]<<16|unsigned(b[3])<<24;}
int main(int argc,char** argv) {
 if(argc!=5)return 2;
 std::ifstream f(argv[4],std::ios::binary);char magic[8];f.read(magic,8);if(!f || std::string(magic,7)!="OBWAUD2")throw std::runtime_error("Audio bank version");
 const unsigned count=u32(f);u32(f);u32(f);std::set<unsigned> ids;
 for(unsigned i=0;i<count;++i){ids.insert(u32(f));for(unsigned j=0;j<4;++j)u32(f);}
 unsigned events=0,drawn=0;bool loaded=false;
 {
  port3ds::Graphics graphics;
  auto observe=[&](port3ds::Engine& engine){
   port3ds::SoundEvent sounds[256];auto n=engine.drain_sounds(sounds,256);events+=n;
   for(unsigned i=0;i<n;++i)if(!ids.count(sounds[i].id)){std::fprintf(stderr,"Missing Protoss economy effect %u\n",sounds[i].id);throw std::runtime_error("Economy sound absent from existing Protoss bank");}
   if(engine.dropped_sounds())throw std::runtime_error("Native sound event overflow");
   if(!loaded){if(!graphics.load(argv[3],engine.terrain_signature()))throw std::runtime_error(graphics.error());loaded=true;}
   if(engine.snapshot().frame%96)return;
   int x=0,y=0;engine.observer_focus(x,y);const auto s=engine.snapshot();
   const int cx=std::max(0,std::min(int(s.width*32)-400,x-200)),cy=std::max(0,std::min(int(s.height*32)-240,y-120));
   port3ds::SpriteView views[2048];auto images=engine.visible_images(views,2048,cx,cy);fake::draws.clear();
   if(graphics.sprites(views,images,cx,cy,engine))throw std::runtime_error("Missing economy sprites / cache overflow");
   ++drawn;
  };
  battle_test(argv[1],argv[2],observe,port3ds::Scenario::protoss_ai_battle);
 }
 if(!events || !drawn || fake::allocations!=0)throw std::runtime_error("Incomplete economy coverage / graphics leak");
 std::printf("Protoss complete economy battle / original map graphics / %u viewport draws / %u banked native audio events PASS\n",drawn,events);
}
