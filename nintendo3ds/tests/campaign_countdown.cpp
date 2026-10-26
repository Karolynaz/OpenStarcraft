// A private CHK fixture checks countdown scheduling through actual trigger ticks
// and save replay. Purchased map files and shipped mission triggers are intact.
#include "engine.h"
#include "campaign_map.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);std::ifstream in(argv[2],std::ios::binary);std::vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});std::vector<uint8_t> trig(2400*3);trig[15]=22;auto* a=trig.data()+320;campaign::put(a+12,4);a[26]=14;a[27]=7;trig[2372]=1;
a=trig.data()+2400;campaign::put(a+8,0);a[14]=1;a[15]=1;a[320+26]=1;a[2372]=1;
a=trig.data()+4800;a[15]=23;a[320+26]=2;a[2372]=1;
bool replaced=false;for(size_t at=0;at+8<=map.size();){unsigned n=campaign::get(map.data()+at+4);if(!memcmp(map.data()+at,"TRIG",4)){map.erase(map.begin()+at+8,map.begin()+at+8+n);map.insert(map.begin()+at+8,trig.begin(),trig.end());campaign::put(map.data()+at+4,trig.size());replaced=true;break;}at+=8+n;}assert(replaced);const char* path="/tmp/sc3ds-countdown.chk";{std::ofstream out(path,std::ios::binary);out.write((char*)map.data(),map.size());}
Engine e;assert(e.load(argv[1],path,Scenario::terran_campaign));assert(e.start_recording("/tmp/sc3ds-countdown-session"));for(unsigned i=0;i<60;++i)assert(e.tick());assert(e.snapshot().winner<0);SaveView view;assert(e.save_game("/tmp/sc3ds-countdown-save",view));Engine replay;SaveView loaded;assert(replay.load_save(argv[1],"/tmp/sc3ds-countdown-save","/tmp/sc3ds-countdown-replay","/tmp/sc3ds-countdown-map",loaded));
for(unsigned i=0;i<150 && e.snapshot().winner<0;++i){assert(e.state_digest()==replay.state_digest());assert(e.tick() && replay.tick());}assert(e.state_digest()==replay.state_digest());assert(e.snapshot().winner==0 && e.frame()>=96 && e.frame()<=150);printf("Countdown reached original victory at frame%u; Never defeat stayed false; active timer save/replay identical PASS\n",e.frame());}
