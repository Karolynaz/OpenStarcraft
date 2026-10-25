#include "engine.h"
#include "frontend.h"
#include <cassert>
#include <cstdio>
#include <set>
using namespace port3ds;
int main(int argc,char** argv){assert(argc==3 || argc==4);
 for(unsigned scenario=0;scenario<4;++scenario){Frontend front;front.base=argv[2];front.observer_test=scenario;auto setup=front.match(1);Engine e;
  if(!e.load(argv[1],front.map_path().c_str(),Scenario(front.scenario()),&setup)){fprintf(stderr,"Observer%u: %s\n",scenario,e.error());return 1;}
  auto s=e.snapshot();assert(s.observer && !e.fog_enabled());unsigned sides=scenario==3?3:2;
  for(unsigned p=0;p<sides;++p){assert(s.players[p].ai && s.players[p].workers==4);assert(s.players[p].difficulty==(scenario==3 && p>0?AiDifficulty::easy:AiDifficulty::medium));}
  std::set<std::pair<int,int>> focus;for(unsigned i=0;i<24*8*sides;++i){assert(e.tick());if(i%(24*8)==0){int x,y;assert(e.observer_focus(x,y));focus.emplace(x,y);}SoundEvent sounds[256];e.drain_sounds(sounds,256);}
  if(scenario==3 && argc==4){unsigned peak=0;for(unsigned frame=0;frame<90000 && e.snapshot().winner<0;++frame){assert(e.tick());SoundEvent sounds[256];e.drain_sounds(sounds,256);peak=std::max(peak,e.snapshot().units);}auto end=e.snapshot();printf("Mixed full observer frame=%u winner=%d peak=%u trained=%u/%u/%u drops=%u/%u/%u casts=%u/%u/%u\n",end.frame,end.winner,peak,end.players[0].trained,end.players[1].trained,end.players[2].trained,end.players[0].drop_units,end.players[1].drop_units,end.players[2].drop_units,end.players[0].casts,end.players[1].casts,end.players[2].casts);assert(end.players[0].trained>6 && end.players[1].trained>6 && end.players[2].trained>6);}
  assert(focus.size()==sides);printf("Observer%u native map load, %u AI sides, mixed profiles, no fog, %zu distinct base focuses PASS\n",scenario,sides,focus.size());
 }
}
