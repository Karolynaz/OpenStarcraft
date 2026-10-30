#include "engine.h"
#include "save_slots.h"
#include <cassert>
#include <unistd.h>
#include <utime.h>
#include <fstream>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);const std::string dir="/tmp/sc3ds-five-slots-"+std::to_string(getpid())+"/";assert(!mkdir(dir.c_str(),0700));assert(!mkdir((dir+"saves").c_str(),0700));SaveInfo trump;trump.scenario=34;assert(SaveSlots::mission(trump)=="Terran mission 7");SaveInfo amerigo;amerigo.scenario=35;assert(SaveSlots::mission(amerigo)=="Zerg mission 5");SaveInfo templar;templar.scenario=36;assert(SaveSlots::mission(templar)=="Zerg mission 6");SaveInfo culling;culling.scenario=37;assert(SaveSlots::mission(culling)=="Zerg mission 7");SaveInfo eye;eye.scenario=38;assert(SaveSlots::mission(eye)=="Zerg mission 8");SaveInfo fire;fire.scenario=39;assert(SaveSlots::mission(fire)=="BW Zerg mission 2");SaveSlots slots;auto scan=[&](){slots.scan(dir,Engine::inspect_save);};scan();for(auto& s:slots.slots)assert(!s.available && !s.occupied && s.label.find("Empty")!=std::string::npos);
 Engine engine;assert(engine.load(argv[1],argv[2],Scenario(17)));assert(engine.start_recording((dir+"saves/session").c_str()));SaveView view;std::array<uint32_t,5> hashes;std::array<unsigned,5> frames;
 std::tm time{};time.tm_year=126;time.tm_mon=9;time.tm_mday=28;time.tm_hour=22;time.tm_min=24;time.tm_isdst=-1;const auto stamp=std::mktime(&time);utimbuf times{stamp,stamp};
 for(unsigned i=0;i<5;++i){for(unsigned n=0;n<24*(i+1);++n){assert(engine.tick());SoundEvent out[256];engine.drain_sounds(out,256);}frames[i]=engine.frame();hashes[i]=engine.state_digest();view.camera_x=i*50;assert(engine.save_game(SaveSlots::path(dir,i).c_str(),view,stamp));assert(!utime(SaveSlots::path(dir,i).c_str(),&times));}
 scan();for(unsigned i=0;i<5;++i){const auto& s=slots.slots[i];assert(s.available && s.occupied && s.info.frame==frames[i] && s.info.saved_at==std::uint64_t(stamp));assert(s.label.find("Slot "+std::to_string(i+1)+" • Zerg mission 1 • 22:24")!=std::string::npos);assert(s.label.find("Oct 28")!=std::string::npos);
  Engine restored;SaveView loaded;assert(restored.load_save(argv[1],s.path.c_str(),(dir+"replay"+std::to_string(i)).c_str(),(dir+"map"+std::to_string(i)).c_str(),loaded));assert(restored.state_digest()==hashes[i] && loaded.camera_x==i*50);
 }
 // Overwrite one slot; damage only its primary and prove recovery from its own backup.
 assert(engine.save_game(SaveSlots::path(dir,2).c_str(),view));{std::fstream f(SaveSlots::path(dir,2),std::ios::in|std::ios::out|std::ios::binary);f.seekp(-1,std::ios::end);f.put('\xff');}scan();assert(slots.slots[2].available && slots.slots[2].path.find(".bak")!=std::string::npos && slots.slots[2].info.frame==frames[2]);
 // A foreign-build save remains described instead of pretending the slot is empty.
 {const auto path=SaveSlots::path(dir,3);std::fstream f(path,std::ios::in|std::ios::out|std::ios::binary);savegame::Header h;f.read((char*)h.bytes.data(),h.bytes.size());h.bytes[136]^=1;h.field(28,0);h.field(28,savegame::hash(2166136261u,h.bytes.data(),h.bytes.size()));f.seekp(0);f.write((char*)h.bytes.data(),h.bytes.size());}scan();assert(slots.slots[3].occupied && !slots.slots[3].available && slots.slots[3].label.find("Zerg mission 1")!=std::string::npos);
 puts("5 real campaign save slots: independent states, timestamp labels, exact replay, per-slot backup and foreign-build metadata PASS");
}
