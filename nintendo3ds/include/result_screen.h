#pragma once
#include "engine.h"
#include "frontend.h"
namespace port3ds {
struct ResultScreen {
 static UiRect ok(){return {96,204,128,28};}
 static bool finale(unsigned scenario){return scenario==55 || scenario==56 || scenario==52;}
 static unsigned count(bool defeat,bool save_failed){return defeat?3:save_failed?2:1;}
 static UiRect button(unsigned choice,unsigned total){return total==1?ok():UiRect{66,232-int(total-choice)*31,188,27};}
 static const char* label(unsigned choice,bool defeat,bool save_failed){
  if(defeat){const char* labels[]={"Retry Mission","Load Game","Campaign Menu"};return labels[choice];}
  if(save_failed)return choice?"Continue without saving":"Retry Save";
  return "OK";
 }
 static int hit(int x,int y,unsigned total=1){
  for(unsigned i=0;i<total;++i)if(button(i,total).hit(x,y))return int(i);
  return -1;
 }
 static const char* race(unsigned value){return value==0?"zerg":value==2?"protoss":"terran";}
 static unsigned next_mission(unsigned scenario,bool won,const std::string& filename){
  const unsigned current=campaign::index(scenario);if(!won || current>=campaign::count)return ~0u;
  if(scenario==campaign::tutorial_scenario)return campaign::opening(false,1);
  if(!filename.empty())return campaign::key_index(campaign::next_key(scenario,filename));
  const unsigned next=campaign::adjacent(current,1);
  // Never wrap the last installed mission back to the episode opening.
  return next!=current && std::string(campaign::missions[next].key)>campaign::missions[current].key?next:~0u;
 }
};
}
