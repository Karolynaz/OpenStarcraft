#include "frontend.h"
#include "pause_menu.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
int main(){using namespace port3ds;
 using P=PauseMenu::Page;
 assert(!PauseMenu::overlay(P::none) && !PauseMenu::overlay(P::catalogue));
 for(P page:{P::system,P::options,P::scenarios,P::legacy,P::notice,P::end_confirm,P::result}){
  assert(PauseMenu::overlay(page));for(unsigned i=0;i<PauseMenu::count(page);++i){auto r=PauseMenu::row(i,PauseMenu::count(page));assert(r.y>=48 && r.y+r.h<230);assert(PauseMenu::hit(r.x+1,r.y+1,PauseMenu::count(page))==int(i));}
 }
 P page=P::legacy;page=PauseMenu::back(page);assert(page==P::scenarios);page=PauseMenu::back(page);assert(page==P::options);page=PauseMenu::back(page);assert(page==P::system && PauseMenu::overlay(page));page=PauseMenu::back(page);assert(!PauseMenu::overlay(page));
 assert(PauseMenu::back(P::notice)==P::system && PauseMenu::back(P::end_confirm)==P::system && PauseMenu::back(P::result)==P::result);
 char tmp[]="/tmp/openbw-menu-XXXXXX";const char* d=mkdtemp(tmp);assert(d);std::string root=std::string(d)+"/";mkdir((root+"maps").c_str(),0700);mkdir((root+"maps/folder").c_str(),0700);std::ofstream(root+"maps/test.scx").put('x');std::ofstream(root+"maps/folder/nested.scm").put('x');std::ofstream(root+"maps/no.txt").put('x');for(unsigned i=0;i<6;++i)std::ofstream(root+"maps/zz"+std::to_string(i)+".scx").put('x');
 Frontend f;f.init(root.c_str());assert(f.hit(int(f.x),int(f.y))==3);assert(f.brood && f.race==1 && f.maps.size()==8 && f.maps[0].name=="folder/nested.scm");
 f.activate(4);f.activate(30);assert(!f.brood);Frontend saved;saved.init(root.c_str());assert(!saved.brood && !saved.save_failed);
 f.activate(0);assert(f.dialog==Frontend::Dialog::unsupported);f.activate(31);assert(f.dialog==Frontend::Dialog::unsupported);f.back();
 for(unsigned i=0;i<3;++i){const auto r=Frontend::campaign_button(i);assert(r.w>=140 && r.h>=26 && r.y+r.h<=240 && f.hit(r.x+2,r.y+2)==(i==0?4:i==1?3:5));}
 f.activate(3);assert(f.leaving && f.screen==Frontend::Screen::campaign && f.hit(80,220)==-1);f.advance(.19f);assert(f.screen==Frontend::Screen::campaign && f.slide()>0);f.activate(4);assert(f.dialog==Frontend::Dialog::none);f.advance(.19f);assert(f.screen==Frontend::Screen::custom && f.slide()==1 && f.hit(290,97)==-1);f.stick(0,0,.19f);assert(f.slide()>.12f && f.slide()<.13f);f.stick(0,0,.19f);assert(f.slide()==0 && f.hit(290,97)==45);
 assert(f.hit(20,198)==13 && f.hit(180,198)==10 && f.hit(20,230)==-1);
 assert(f.hit(200,74)==60);f.activate(60);assert(f.dialog==Frontend::Dialog::map_list);f.activate(91);assert(f.scroll==5);f.activate(71);assert(f.selected_map==6 && f.preview_dirty && f.dialog==Frontend::Dialog::none);f.activate(60);f.direction(2);assert(f.hover==72);f.activate(f.hover);assert(f.selected_map==7);f.selected_map=0;
 assert(f.preview_dirty);f.preview.valid=true;f.preview.players=8;assert(f.capacity()==3 && f.ai_available(1));f.activate(43);assert(f.selected_map==1);f.activate(42);assert(f.selected_map==0);f.activate(40);assert(f.game_type==2);f.activate(41);assert(f.game_type==0);
 f.ai={{3,3}};f.activate(48);assert(f.ai[1]==2);f.activate(49);assert(f.ai[1]==3);f.activate(49);assert(f.ai[1]==4 && std::string(f.ai_name(1))=="Off");auto m=f.match(123);assert(m.enabled && m.races[2]==-1 && m.races[1]>=0 && m.races[1]<3);
 f.preview.players=2;f.ai[1]=3;f.activate(49);assert(f.ai[1]==3 && std::string(f.ai_name(1))=="Off" && f.match(123).races[2]==-1 && f.hit(290,143)==-1);
 f.preview.players=3;f.ai={{4,3}};assert(f.match(123).races[1]==-1 && f.match(123).races[2]>=0);f.ai={{4,4}};m=f.match(123);assert(m.races[1]==-1 && m.races[2]==-1);
 for(unsigned race=0;race<3;++race){f.race=race;assert(f.match(1).races[0]==int(race));f.save();Frontend again;again.init(root.c_str());assert(again.race==race);}
 f.x=200;f.y=97;f.direction(1);assert(f.race==0);f.direction(3);assert(f.race==2);f.direction(2);assert(f.y==120);
 f.activate(51);assert(f.difficulty==1 && f.match(1).difficulty==AiDifficulty::medium);Frontend stored;stored.init(root.c_str());assert(stored.difficulty==1);f.x=200;f.y=166;f.direction(1);assert(f.difficulty==0);f.direction(0);assert(f.y==143);
 f.game_type=1;assert(f.match(1).game_type==1);f.dialog=Frontend::Dialog::error;assert(f.hit(160,190)==32);f.activate(32);assert(f.dialog==Frontend::Dialog::none);f.activate(10);assert(f.play);f.back();assert(f.leaving && f.screen==Frontend::Screen::custom);f.advance(.38f);assert(f.screen==Frontend::Screen::campaign && f.transition==0);f.advance(.38f);f.back();f.activate(33);assert(f.quit);
 for(unsigned i=0;i<6;++i)std::remove((root+"maps/zz"+std::to_string(i)+".scx").c_str());std::remove((root+"maps/test.scx").c_str());std::remove((root+"maps/no.txt").c_str());std::remove((root+"maps/folder/nested.scm").c_str());rmdir((root+"maps/folder").c_str());rmdir((root+"maps").c_str());std::remove((root+"player1.cfg").c_str());rmdir(root.c_str());
 puts("Version/profile + recursive maps + panel animation/input gate + bidirectional settings + random/off/2-/3-/8-slot AI cap PASS");}
