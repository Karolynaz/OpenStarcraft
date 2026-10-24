#include "engine.h"
#include "frontend.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <unistd.h>
int main(int argc,char**argv){assert(argc==3);using namespace port3ds;Frontend files;files.base=argv[2];files.directory=files.base;files.scan();assert(files.maps.size()>=20);Engine engine;unsigned count=0,three=0;
 const std::string cache="/tmp/openbw-catalogue-"+std::to_string(getpid())+".bin";
 for(const auto& map:files.maps){const auto path=files.base+"/"+map.name;MapPreview preview;assert(Engine::preview_map(argv[1],path.c_str(),preview));assert(preview.width<=128 && preview.height<=128 && preview.players>=2);
  MatchSetup setup;setup.enabled=true;setup.races={{int(count%3),int((count+1)%3),preview.players>=3?int((count+2)%3):-1}};if(setup.races[2]>=0)++three;
  if(!engine.load(argv[1],path.c_str(),Scenario(setup.races[0]==0?15:setup.races[0]==2?13:4),&setup)){printf("FAIL %s: %s\n",map.name.c_str(),engine.error());fflush(stdout);assert(false);}
  auto initial=engine.snapshot();assert(initial.players[0].workers==4 && initial.players[1].workers==4 && initial.players[2].workers==(setup.races[2]>=0?4u:0u));
  assert(engine.write_terrain(argv[1],cache.c_str()));for(unsigned i=0;i<240;++i)engine.tick();assert(engine.ready());
  printf("PASS %s %ux%u %u starts mixed-race load/terrain/240 ticks\n",map.name.c_str(),preview.width,preview.height,preview.players);fflush(stdout);++count;
 }
 std::remove(cache.c_str());printf("%u owned maps validated, %u with two AI slots\n",count,three);
}
