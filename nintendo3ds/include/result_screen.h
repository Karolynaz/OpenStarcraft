#pragma once
#include "engine.h"
#include "frontend.h"
namespace port3ds {
struct ResultScreen {
 static UiRect ok(){return {96,204,128,28};}
 static int hit(int x,int y){return ok().hit(x,y)?0:-1;}
 static const char* race(unsigned value){return value==0?"zerg":value==2?"protoss":"terran";}
 static unsigned next_mission(unsigned scenario,bool won,const std::string& filename){
  const unsigned current=campaign::index(scenario);if(!won || current>=campaign::count)return ~0u;
  if(!filename.empty())return campaign::key_index(campaign::next_key(scenario,filename));
  const unsigned next=campaign::adjacent(current,1);
  // Never wrap the last installed mission back to the episode opening.
  return next!=current && std::string(campaign::missions[next].key)>campaign::missions[current].key?next:~0u;
 }
};
}
