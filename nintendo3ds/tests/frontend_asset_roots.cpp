// Read-only bundle assets must never receive profiles, progress or custom maps.
#include "frontend.h"
#include <cassert>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
using namespace port3ds;
int main(){const auto base=std::string("/tmp/sc3ds-roots-")+std::to_string(getpid());const auto assets=base+"/assets/",sd=base+"/sd/";mkdir(base.c_str(),0777);mkdir(assets.c_str(),0777);mkdir(sd.c_str(),0777);mkdir((assets+"maps").c_str(),0777);mkdir((assets+"campaign").c_str(),0777);mkdir((sd+"maps").c_str(),0777);mkdir((sd+"campaign").c_str(),0777);
 {std::ofstream f(assets+"player1.cfg");f<<"unchanged asset fixture";}{std::ofstream f(assets+"campaign/progress.cfg");f<<"unchanged progress fixture";}
 {std::ofstream f(sd+"campaign/progress.cfg",std::ios::binary);f<<"OPENBW_CAMPAIGN1\r\nterran01=1\r\nexpzerg99=1\r\n";}
 Frontend f;f.init(assets.c_str(),sd.c_str());assert(f.settings==sd+"player1.cfg" && f.directory==sd+"maps" && f.campaign_folder()==assets+"campaign/");f.brood=true;f.race=0;f.save();assert(!f.save_failed && f.completed[campaign::index(16)]);assert(f.complete_mission(40,"Zerg04a.scx"));
 Frontend reloaded;reloaded.init(assets.c_str(),sd.c_str());assert(reloaded.brood && reloaded.race==0 && reloaded.completed[campaign::index(40)] && reloaded.next_missions[3]=="expzerg04a");
 assert(Frontend::read_text(sd+"campaign/progress.cfg").find("expzerg99=1")!=std::string::npos);
 assert(Frontend::read_text(assets+"player1.cfg")=="unchanged asset fixture" && Frontend::read_text(assets+"campaign/progress.cfg")=="unchanged progress fixture");puts("Read-only bundle asset paths, SD custom maps/profile/progress and reload PASS");}
