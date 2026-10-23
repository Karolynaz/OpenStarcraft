#include "frontend.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
int main(){using namespace port3ds;
 char tmp[]="/tmp/openbw-menu-XXXXXX";const char* d=mkdtemp(tmp);assert(d);std::string root=std::string(d)+"/";mkdir((root+"maps").c_str(),0700);mkdir((root+"maps/folder").c_str(),0700);std::ofstream(root+"maps/test.scx").put('x');std::ofstream(root+"maps/folder/nested.scm").put('x');std::ofstream(root+"maps/no.txt").put('x');
 Frontend f;f.init(root.c_str());assert(f.brood && f.race==1 && f.maps.size()==2 && f.maps[0].name=="folder/nested.scm");
 f.activate(4);f.activate(30);assert(!f.brood);Frontend saved;saved.init(root.c_str());assert(!saved.brood && !saved.save_failed);
 f.activate(0);assert(f.dialog==Frontend::Dialog::unsupported);f.activate(31);assert(f.dialog==Frontend::Dialog::unsupported);f.back();
 f.activate(3);assert(f.screen==Frontend::Screen::custom && f.slide()==1 && f.hit(290,104)==-1);f.stick(0,0,.19f);assert(f.slide()>.12f && f.slide()<.13f);f.stick(0,0,.19f);assert(f.slide()==0 && f.hit(290,104)==45);
 assert(f.preview_dirty);f.preview.valid=true;f.preview.players=8;assert(f.capacity()==3 && f.ai_available(1));f.activate(43);assert(f.selected_map==1);f.activate(42);assert(f.selected_map==0);f.activate(40);assert(f.game_type==2);f.activate(41);assert(f.game_type==0);
 f.ai={{3,3}};f.activate(48);assert(f.ai[1]==2);f.activate(49);assert(f.ai[1]==3);f.activate(49);assert(f.ai[1]==4 && std::string(f.ai_name(1))=="Off");auto m=f.match(123);assert(m.enabled && m.races[2]==-1 && m.races[1]>=0 && m.races[1]<3);
 f.preview.players=2;f.ai[1]=3;f.activate(49);assert(f.ai[1]==3 && std::string(f.ai_name(1))=="Off" && f.match(123).races[2]==-1 && f.hit(290,162)==-1);
 f.preview.players=3;f.ai={{4,3}};assert(f.match(123).races[1]==-1 && f.match(123).races[2]>=0);f.ai={{4,4}};m=f.match(123);assert(m.races[1]==-1 && m.races[2]==-1);
 for(unsigned race=0;race<3;++race){f.race=race;assert(f.match(1).races[0]==int(race));f.save();Frontend again;again.init(root.c_str());assert(again.race==race);}
 f.x=200;f.y=104;f.direction(1);assert(f.race==0);f.direction(3);assert(f.race==2);f.direction(2);assert(f.y==137);
 f.game_type=1;assert(f.match(1).game_type==1);f.dialog=Frontend::Dialog::error;assert(f.hit(160,190)==32);f.activate(32);assert(f.dialog==Frontend::Dialog::none);f.activate(10);assert(f.play);f.back();assert(f.screen==Frontend::Screen::campaign);f.back();f.activate(33);assert(f.quit);
 std::remove((root+"maps/test.scx").c_str());std::remove((root+"maps/no.txt").c_str());std::remove((root+"maps/folder/nested.scm").c_str());rmdir((root+"maps/folder").c_str());rmdir((root+"maps").c_str());std::remove((root+"player1.cfg").c_str());rmdir(root.c_str());
 puts("Version/profile + recursive maps + panel animation/input gate + bidirectional settings + random/off/2-/3-/8-slot AI cap PASS");}
