#include "frontend.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
int main(){using namespace port3ds;
 char tmp[]="/tmp/openbw-menu-XXXXXX";const char* d=mkdtemp(tmp);assert(d);std::string root=std::string(d)+"/";mkdir((root+"maps").c_str(),0700);mkdir((root+"maps/folder").c_str(),0700);std::ofstream(root+"maps/test.scx").put('x');std::ofstream(root+"maps/no.txt").put('x');
 Frontend f;f.init(root.c_str());assert(f.brood && f.race==1 && f.maps.size()==2);assert(f.hit(165,205)==3);f.activate(4);assert(f.dialog==Frontend::Dialog::version);f.activate(30);assert(!f.brood && f.dialog==Frontend::Dialog::none);Frontend saved;saved.init(root.c_str());assert(!saved.brood && !saved.save_failed);
 f.activate(0);assert(f.dialog==Frontend::Dialog::unsupported);f.activate(31);assert(f.dialog==Frontend::Dialog::unsupported);f.back();assert(f.dialog==Frontend::Dialog::none);
 f.activate(3);assert(f.screen==Frontend::Screen::custom);f.activate(100);assert(f.maps[0].name=="..");f.activate(100);assert(f.directory==f.base);f.activate(101);assert(f.selected_map==1);assert(f.hit(260,195)==10);
 for(unsigned race=0;race<3;++race){f.race=race;assert(f.scenario()==(race==0?15:race==1?4:13));f.save();Frontend again;again.init(root.c_str());assert(again.race==race);}
 f.activate(7);f.activate(8);f.activate(9);assert(f.game_type==1 && f.subtype==1 && f.speed==5);f.activate(10);assert(f.play);f.back();assert(f.screen==Frontend::Screen::campaign);f.back();assert(f.dialog==Frontend::Dialog::exit);f.activate(33);assert(f.quit);
 f.x=0;f.y=0;f.stick(-156,156,1);assert(f.x==0 && f.y==0);f.stick(156,-156,100);assert(f.x==319 && f.y==239);
 std::remove((root+"maps/test.scx").c_str());std::remove((root+"maps/no.txt").c_str());rmdir((root+"maps/folder").c_str());rmdir((root+"maps").c_str());std::remove((root+"player1.cfg").c_str());rmdir(root.c_str());std::puts("Frontend profiles/version persistence/dialog gates/maps/folders/settings/race dispatch PASS");
}
