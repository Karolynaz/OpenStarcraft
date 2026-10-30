// Isolated synthetic CHK fixture, never substituted for a shipped mission.
// A scripted squad destination must match the public native group order.
#include "engine.h"
#include "campaign_map.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <tuple>
#include <algorithm>
#include <unistd.h>
using namespace port3ds;
using Bytes=std::vector<uint8_t>;
static void chunk(Bytes& map,const char* tag,const Bytes& data){
 for(size_t at=0;at+8<=map.size();){auto n=campaign::get(map.data()+at+4);if(!memcmp(map.data()+at,tag,4)){map.erase(map.begin()+at+8,map.begin()+at+8+n);map.insert(map.begin()+at+8,data.begin(),data.end());campaign::put(map.data()+at+4,data.size());return;}at+=8+n;}
 map.insert(map.end(),tag,tag+4);auto at=map.size();map.resize(at+4);campaign::put(map.data()+at,data.size());map.insert(map.end(),data.begin(),data.end());
}
static auto positions(const Engine& e){std::vector<std::pair<int,int>> result;UnitView units[32];auto n=e.visible_units(units,32);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==0)result.emplace_back(units[i].x,units[i].y);std::sort(result.begin(),result.end());return result;}
int main(int argc,char**argv){
 assert(argc==3);std::ifstream input(argv[2],std::ios::binary);Bytes original((std::istreambuf_iterator<char>(input)),{});
 for(unsigned attack=0;attack<2;++attack){
  Bytes map=original,owners(12),races(12,1),units(108),locations(20*255),trig(2400);owners[0]=6;
  chunk(map,"OWNR",owners);chunk(map,"SIDE",races);chunk(map,"THG2",{});
  for(unsigned i=0;i<3;++i){auto* u=units.data()+i*36;unsigned x=384+(i==1?40:0),y=384+(i==2?40:0);u[4]=x;u[5]=x>>8;u[6]=y;u[7]=y>>8;u[17]=100;}
  chunk(map,"UNIT",units);for(unsigned i=0;i<4;++i){campaign::put(locations.data()+i*4,i<2?300:500);campaign::put(locations.data()+20+i*4,i<2?1088:1216);}chunk(map,"MRGN",locations);
  trig[15]=22;trig[2372]=1;auto* a=trig.data()+320;campaign::put(a,1);campaign::put(a+16,0);campaign::put(a+20,2);a[24]=0;a[26]=46;a[27]=attack?2:0;chunk(map,"TRIG",trig);
  const auto base=std::string("/tmp/sc3ds-group-")+std::to_string(getpid())+"-"+std::to_string(attack);{std::ofstream f(base+"-trigger.chk",std::ios::binary);f.write((char*)map.data(),map.size());}chunk(map,"TRIG",{});{std::ofstream f(base+"-public.chk",std::ios::binary);f.write((char*)map.data(),map.size());}
  Engine scripted,manual;assert(scripted.load(argv[1],(base+"-trigger.chk").c_str(),Scenario::terran_campaign));assert(manual.load(argv[1],(base+"-public.chk").c_str(),Scenario::terran_campaign));scripted.set_fog_enabled(false);manual.set_fog_enabled(false);bool issued=false;
  for(unsigned tick=0;tick<480;++tick){assert(scripted.tick() && manual.tick());if(!issued){UnitView u[32];auto n=scripted.visible_units(u,32);for(unsigned i=0;i<n;++i)if(u[i].type==0 && u[i].order==(attack?14u:6u)){assert(manual.select_area(300,300,200,200)==3);assert(manual.command(attack?Command::attack:Command::context,1152,1152));issued=true;break;}}}
  assert(issued);auto expected=positions(manual),actual=positions(scripted);assert(expected.size()==3 && actual==expected && actual.front()!=actual.back());for(const auto& p:actual)assert(abs(p.first-1152)<160 && abs(p.second-1152)<160);
 }
 puts("Scripted Move/Attack squads match public native group destinations without collapsing their formation PASS");
}
