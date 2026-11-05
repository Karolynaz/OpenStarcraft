// Isolated ownership/trigger fixture; never replaces a purchased mission map.
#include "engine.h"
#include "campaign_map.h"
#include "bwenums.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <unistd.h>
using namespace port3ds;
int main(int argc,char**argv){
 assert(argc==3);
 for(unsigned mode=0;mode<3;++mode){
  std::ifstream in(argv[2],std::ios::binary);std::vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});
  auto chunk=[&](const char* tag,const std::vector<uint8_t>& data){for(size_t at=0;at+8<=map.size();){auto n=campaign::get(map.data()+at+4);if(!memcmp(map.data()+at,tag,4)){map.erase(map.begin()+at+8,map.begin()+at+8+n);map.insert(map.begin()+at+8,data.begin(),data.end());campaign::put(map.data()+at+4,data.size());return;}at+=8+n;}assert(false);};
  std::vector<uint8_t> owners(12),races(12,1),forces(20);owners[0]=6;owners[1]=7;owners[2]=3;owners[4]=8;forces[1]=2;const bool force=mode==1;
  chunk("OWNR",owners);chunk("SIDE",races);chunk("FORC",forces);chunk("THG2",{});
  std::vector<uint8_t> units;
  auto spawn=[&](unsigned type,unsigned owner,unsigned x,unsigned y){auto at=units.size();units.resize(at+36);auto* p=units.data()+at;auto word=[&](unsigned n,unsigned v){p[n]=v;p[n+1]=v>>8;};word(4,x);word(6,y);word(8,type);p[16]=owner;p[17]=100;};
  if(mode==2){spawn(0,1,1024,1024);spawn(0,0,1096,1024);}else{spawn(106,1,1184,1024);spawn(7,1,1600,1600);spawn(0,0,1184,1080);}chunk("UNIT",units);
  std::vector<uint8_t> triggers(4*2400);
  auto always=[&](unsigned index,unsigned owner){auto* t=triggers.data()+2400*index;t[15]=22;t[2372+owner]=1;return t+320;};
  auto* a=always(0,force?20:1);a[26]=4;campaign::put(a+12,2000);a+=32;a[26]=15;campaign::put(a+20,1969451858u); // owned retail switch-rescue script
  for(unsigned i=1;i<4;++i){a=always(i,i==1?2:i==2?3:4);a[26]=26;campaign::put(a+16,0);campaign::put(a+20,999);a[27]=8;}
  if(mode==2){triggers.assign(4800,0);a=always(0,0);a[26]=57;campaign::put(a+16,1);a[24]=0;a=always(1,1);a[26]=57;campaign::put(a+16,0);a[24]=0;}
  chunk("TRIG",triggers);const auto base=std::string("/tmp/sc3ds-neutral-")+std::to_string(getpid())+"-"+std::to_string(mode);
  {std::ofstream out(base+".chk",std::ios::binary);out.write((char*)map.data(),map.size());}
  Engine e;assert(e.load(argv[1],(base+".chk").c_str(),Scenario::terran_campaign));assert(e.start_recording((base+".journal").c_str()));
  if(mode==2){e.set_fog_enabled(false);assert(e.select(1096,1024));assert(e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::AttackDefault)),1024,1024));}
  for(unsigned i=0;i<24;++i)assert(e.tick());if(mode!=2)assert(!e.count_units(0,106) && e.count_units(1,106)==1);assert(e.snapshot().minerals==(mode==2?0:999));
  SaveView view;assert(e.save_game((base+".sav").c_str(),view));Engine replay;SaveView returned;assert(replay.load_save(argv[1],(base+".sav").c_str(),(base+".replay").c_str(),(base+".map").c_str(),returned));
  for(unsigned i=0;i<120;++i){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}
  if(mode==2){e.set_fog_enabled(false);UnitView view[16];auto n=e.visible_units(view,16);int hp0=0,hp1=0;for(unsigned i=0;i<n;++i)if(view[i].type==0){if(view[i].owner==0)hp0=view[i].hp;else if(view[i].owner==1)hp1=view[i].hp;}fprintf(stderr,"neutral retaliation hp%d/%d\n",hp0,hp1);assert(hp0<40 && hp1<40 && e.snapshot().minerals==0);}
  else assert(e.count_units(0,106)==1 && e.count_units(0,7)==1 && !e.count_units(1,106) && e.snapshot().minerals==999);
 }
 puts("Neutral direct/Force triggers, explicit rescue triggers, inactive/closed exclusion, delayed whole-colony rescue, native provoked retaliation and exact save replay PASS");
}
