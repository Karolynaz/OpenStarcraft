#include "frontend.h"
#include "engine.h"
#include "pause_menu.h"
#include "result_screen.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
int main(){using namespace port3ds;
 using P=PauseMenu::Page;
 assert(!PauseMenu::overlay(P::none) && !PauseMenu::overlay(P::catalogue));
 for(P page:{P::system,P::options,P::dev,P::ai_tests,P::scenarios,P::legacy,P::notice,P::end_confirm,P::result,P::save_slots,P::load_slots}){
  assert(PauseMenu::overlay(page));for(unsigned i=0;i<PauseMenu::count(page);++i){auto r=PauseMenu::row(i,PauseMenu::count(page));assert(r.y>=48 && r.y+r.h<230);assert(PauseMenu::hit(r.x+1,r.y+1,PauseMenu::count(page))==int(i));}
 }
 P page=P::legacy;page=PauseMenu::back(page);assert(page==P::scenarios);page=PauseMenu::back(page);assert(page==P::dev);page=PauseMenu::back(page);assert(page==P::options);page=PauseMenu::back(page);assert(page==P::system && PauseMenu::overlay(page));page=PauseMenu::back(page);assert(!PauseMenu::overlay(page));
 assert(PauseMenu::back(P::ai_tests)==P::dev);
 assert(PauseMenu::back(P::notice)==P::system && PauseMenu::back(P::end_confirm)==P::system && PauseMenu::back(P::result)==P::result);
 Frontend picker;picker.dialog=Frontend::Dialog::load_slots;picker.load_enabled[2]=true;picker.activate(120);assert(picker.load_requested_slot==~0u);picker.activate(122);assert(picker.load_requested_slot==2);picker.back();assert(picker.dialog==Frontend::Dialog::none);
 Frontend pagination;pagination.briefing={"one two three four five six seven eight nine ten eleven twelve"};pagination.briefing_voices={1400};pagination.paginate_briefing([](const char* text){return float(std::strlen(text));},12,2);assert(pagination.briefing.size()==4 && pagination.briefing_voices==std::vector<unsigned>({1400,1400,1400,1400}));std::string joined;for(const auto& page:pagination.briefing){assert(std::count(page.begin(),page.end(),'\n')<2);if(!joined.empty())joined+=' ';joined+=page;}std::replace(joined.begin(),joined.end(),'\n',' ');assert(joined=="one two three four five six seven eight nine ten eleven twelve");
 char tmp[]="/tmp/openbw-menu-XXXXXX";const char* d=mkdtemp(tmp);assert(d);std::string root=std::string(d)+"/";mkdir((root+"maps").c_str(),0700);mkdir((root+"maps/folder").c_str(),0700);std::ofstream(root+"maps/test.scx").put('x');std::ofstream(root+"maps/folder/nested.scm").put('x');std::ofstream(root+"maps/no.txt").put('x');for(unsigned i=0;i<6;++i)std::ofstream(root+"maps/zz"+std::to_string(i)+".scx").put('x');
 Frontend f;f.init(root.c_str());assert(f.hit(int(f.x),int(f.y))==3);assert(f.brood && f.race==1 && f.maps.size()==8 && f.maps[0].name=="folder/nested.scm");
 assert(f.hit(160,16)==-1);
 for(unsigned i=0;i<ai_observer_tests.size();++i){
  const auto watch=ai_observer_tests[i].match();
  assert(watch.enabled && watch.observer && watch.races[0]>=0);
  if(i==3){
   assert(watch.teams[1]==watch.teams[2] && watch.teams[0]!=watch.teams[1]);
   assert((watch.side_difficulty==std::array<int,3>{{1,0,0}}));
   assert((watch.races==std::array<int,3>{{1,2,0}}));
  }else assert(watch.races[0]==watch.races[1] && watch.races[2]==-1);
 }
 f.activate(5);assert(f.load && !f.play && f.dialog==Frontend::Dialog::none);f.load=false;
 f.activate(4);f.activate(30);assert(!f.brood);Frontend saved;saved.init(root.c_str());assert(!saved.brood && !saved.save_failed);
 assert(saved.music_volume==100 && saved.effects_volume==100);
 f.music_volume=30;f.effects_volume=60;f.save();
 saved.init(root.c_str());assert(saved.music_volume==30 && saved.effects_volume==60);
 f.activate(1);assert(f.dialog==Frontend::Dialog::campaign_missions && f.mission_choices.size()==1);f.activate(130);assert(f.dialog==Frontend::Dialog::error);f.back();mkdir((root+"campaign").c_str(),0700);{std::ofstream brief(root+"campaign/terran01.brief");brief<<"Objectives\fBriefing";}{std::ofstream voices(root+"campaign/terran01.brief-audio");voices<<"0\n1400\n";}f.activate(1);f.activate(130);assert(f.dialog==Frontend::Dialog::briefing && f.briefing.size()==2 && f.briefing_sound()==0);f.activate(110);assert(f.briefing_page==1 && f.briefing_sound()==1400);f.back();assert(f.briefing_page==0);f.activate(110);f.activate(110);assert(f.play && f.campaign_start && f.scenario()==16 && !f.match(1).enabled);f.play=f.campaign_start=false;{std::ofstream progress(root+"campaign/progress.cfg");progress<<"OPENBW_CAMPAIGN1\nterran=1\n";}Frontend completed;completed.init(root.c_str());assert(completed.completed[0]);
 {std::ofstream brief(root+"campaign/terran02.brief");brief<<"Second mission";}completed.brood=false;completed.activate(1);assert(completed.mission_choices.size()==2 && completed.episode_resume==6);
 completed.direction(2);assert(completed.hover==131);completed.confirm();assert(completed.mission==0);
 completed.back();completed.activate(1);completed.activate(130);assert(completed.mission==6 && completed.dialog==Frontend::Dialog::briefing);assert(completed.hit(26,38)==112 && completed.hit(294,38)==113);completed.direction(3);assert(completed.mission==0 && completed.briefing.size()==2);completed.direction(1);assert(completed.mission==6 && completed.briefing.size()==1 && completed.briefing_sound()==0);completed.activate(110);assert(completed.scenario()==22 && completed.play);{std::ofstream brief(root+"campaign/terran03.brief");brief<<"Evacuation";}{std::ofstream brief(root+"campaign/terran04.brief");brief<<"Installation";}completed.play=false;completed.completed[6]=true;completed.activate(1);completed.activate(130);assert(completed.mission==9 && completed.dialog==Frontend::Dialog::briefing);completed.direction(1);assert(completed.mission==12);completed.direction(3);assert(completed.mission==9);std::remove((root+"campaign/terran04.brief").c_str());std::remove((root+"campaign/terran03.brief").c_str());std::remove((root+"campaign/terran02.brief").c_str());
 Frontend episodes;episodes.init(root.c_str());
 episodes.open_episode_complete(campaign::index(55));
 assert(episodes.dialog==Frontend::Dialog::episode_complete && !episodes.campaign_finished);
 episodes.confirm();assert(episodes.dialog==Frontend::Dialog::campaign_missions && episodes.episode_opening==campaign::opening(false,0));
 episodes.open_episode_complete(campaign::index(56));episodes.confirm();
 assert(episodes.episode_opening==campaign::opening(false,2));
 episodes.open_episode_complete(campaign::index(52));
 assert(episodes.campaign_finished);episodes.confirm();assert(episodes.dialog==Frontend::Dialog::none);
 episodes.open_episode_complete(campaign::index(unsigned(Scenario::bw_protoss_campaign_8)));
 assert(episodes.brood && !episodes.campaign_finished);
 episodes.confirm();assert(episodes.episode_opening==campaign::opening(true,1));
 episodes.open_episode_complete(campaign::index(unsigned(Scenario::bw_terran_campaign_8)));
 episodes.confirm();assert(episodes.episode_opening==campaign::opening(true,0));
 episodes.open_episode_complete(campaign::index(campaign::omega_scenario));
 assert(episodes.brood && episodes.campaign_finished);
 episodes.confirm();assert(episodes.dialog==Frontend::Dialog::none);
 assert(episodes.complete_mission(22,"Terran03.scx"));
 assert(episodes.complete_mission(16,"Terran02.scx"));
 assert(episodes.next_missions[1]=="terran03");
 const auto patriots_blood=campaign::index(unsigned(Scenario::bw_terran_campaign_7));
 const auto beast=campaign::index(unsigned(Scenario::bw_terran_campaign_8));
 const auto brood_terran=campaign::opening(true,1);
 const auto ground_zero=campaign::key_index("expterran05a");
 const auto birds=campaign::key_index("expterran05b");
 const auto emperors_flight=campaign::key_index("expterran06");
 assert(ground_zero<campaign::count && birds<campaign::count && emperors_flight<campaign::count);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_terran_campaign_4),true,"Terran05a.scx")==ground_zero);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_terran_campaign_4),true,"Terran05b.scx")==birds);
 assert(ResultScreen::next_mission(campaign::missions[ground_zero].scenario,true,"Terran06.scx")==emperors_flight);
 assert(ResultScreen::next_mission(campaign::missions[ground_zero].scenario,true,"")==emperors_flight);
 assert(ResultScreen::next_mission(campaign::missions[birds].scenario,true,"Terran06.scx")==emperors_flight);
 const auto brood_zerg=campaign::opening(true,0);
 const auto korhal_a=campaign::index(unsigned(Scenario::bw_zerg_campaign_4a));
 const auto korhal_f=campaign::index(unsigned(Scenario::bw_zerg_campaign_4f));
 const auto true_colors=campaign::index(campaign::true_colors_scenario);
 const auto reckoning=campaign::index(campaign::reckoning_scenario);
 const auto dark_origin=campaign::index(campaign::dark_origin_scenario);
 const auto omega=campaign::index(campaign::omega_scenario);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_zerg_campaign_3),true,"Zerg04f.scx")==korhal_f);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_zerg_campaign_3),true,"Zerg04.scx")==korhal_a);
 for(unsigned version=korhal_a;version<=korhal_f;++version){
  assert(campaign::same_chapter(version,korhal_a));
  const auto filename=std::string("Zerg04")+char('a'+version-korhal_a)+".scx";
  assert(ResultScreen::next_mission(unsigned(Scenario::bw_zerg_campaign_3),true,filename)==version);
  assert(ResultScreen::next_mission(campaign::missions[version].scenario,true,"")==true_colors);
 }
 const Scenario late_zerg[]={Scenario::bw_zerg_campaign_5,Scenario::bw_zerg_campaign_6,
     Scenario::bw_zerg_campaign_7,Scenario::bw_zerg_campaign_8,Scenario::bw_zerg_campaign_9};
 for(unsigned i=1;i<sizeof late_zerg/sizeof *late_zerg;++i)
  assert(ResultScreen::next_mission(unsigned(late_zerg[i-1]),true,"")==campaign::index(unsigned(late_zerg[i])));
 assert(campaign::adjacent(reckoning,1)==omega);
 assert(campaign::race(campaign::dark_origin_scenario)==2);
 assert(ResultScreen::next_mission(campaign::reckoning_scenario,true,"Zerg10.scx")==omega);
 assert(ResultScreen::next_mission(campaign::reckoning_scenario,true,"Bonus.scx")==dark_origin);
 assert(ResultScreen::next_mission(campaign::dark_origin_scenario,true,"Zerg10.scx")==omega);
 assert(ResultScreen::finale(campaign::omega_scenario));
 Frontend zerg_route;
 zerg_route.init(root.c_str());
 for(unsigned j=brood_zerg;j!=reckoning;j=campaign::adjacent(j,1))zerg_route.completed[j]=true;
 zerg_route.open_episode(brood_zerg);
 assert(zerg_route.episode_resume==reckoning);
 assert(std::find(zerg_route.mission_choices.begin(),zerg_route.mission_choices.end(),dark_origin)==zerg_route.mission_choices.end());
 assert(zerg_route.complete_mission(campaign::reckoning_scenario,"Bonus.scx"));
 zerg_route.open_episode(brood_zerg);
 assert(zerg_route.episode_resume==dark_origin);
 assert(zerg_route.complete_mission(campaign::dark_origin_scenario,"Zerg10.scx"));
 zerg_route.open_episode(brood_zerg);
 assert(zerg_route.episode_resume==omega);
 zerg_route.completed[reckoning]=zerg_route.completed[dark_origin]=false;
 zerg_route.bonus_unlocked=false;
 assert(zerg_route.complete_mission(campaign::reckoning_scenario,"Zerg10.scx"));
 assert(!zerg_route.bonus_unlocked);
 assert(zerg_route.complete_mission(campaign::reckoning_scenario,"Bonus.scx"));
 Frontend secret_replay;
 secret_replay.init(root.c_str());
 secret_replay.open_episode(brood_zerg);
 assert(secret_replay.episode_resume==omega && secret_replay.bonus_unlocked);
 assert(std::find(secret_replay.mission_choices.begin(),secret_replay.mission_choices.end(),dark_origin)!=secret_replay.mission_choices.end());
 assert(secret_replay.complete_mission(campaign::dark_origin_scenario,"Zerg10.scx"));
 assert(secret_replay.next_missions[3]=="expzerg10");
 Frontend branch;
 for(unsigned i=brood_terran;i!=ground_zero;i=campaign::adjacent(i,1))branch.completed[i]=true;
 branch.next_missions[4]="expterran05b";
 branch.open_episode(brood_terran);
 assert(branch.episode_resume==birds);
 branch.completed[birds]=true;
 branch.next_missions[4].clear();
 branch.open_episode(brood_terran);
 assert(branch.episode_resume==emperors_flight && !branch.completed[ground_zero]);
 branch.completed[birds]=false;
 branch.completed[ground_zero]=true;
 branch.open_episode(brood_terran);
 assert(branch.episode_resume==emperors_flight && !branch.completed[birds]);
 assert(patriots_blood<campaign::count && beast<campaign::count);
 assert(campaign::adjacent(campaign::index(unsigned(Scenario::bw_terran_campaign_6)),1)==patriots_blood);
 assert(campaign::adjacent(patriots_blood,1)==beast);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_terran_campaign_6),true,"")==patriots_blood);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_terran_campaign_7),true,"")==beast);
 assert(ResultScreen::next_mission(unsigned(Scenario::bw_terran_campaign_7),false,"")==~0u);
 {std::ofstream brief(root+"campaign/expterran07.brief");brief<<"Reactor override";}
 {std::ofstream brief(root+"campaign/expterran08.brief");brief<<"Bring medics to the beacons.";}
 for(unsigned j=brood_terran, n=0;j!=patriots_blood && n<campaign::count;j=campaign::adjacent(j,1),++n)
  episodes.completed[j]=true;
 episodes.brood=true;
 episodes.open_episode(brood_terran);
 assert(episodes.episode_resume==patriots_blood && !episodes.episode_finished);
 episodes.activate(130);
 assert(episodes.dialog==Frontend::Dialog::briefing && episodes.mission==patriots_blood);
 episodes.activate(110);
 assert(episodes.play && episodes.scenario()==unsigned(Scenario::bw_terran_campaign_7));
 assert(episodes.complete_mission(episodes.scenario(),""));
 episodes.play=false;
 episodes.open_episode(brood_terran);
 assert(episodes.episode_resume==beast && !episodes.episode_finished);
 episodes.activate(130);
 assert(episodes.dialog==Frontend::Dialog::briefing && episodes.mission==beast);
 episodes.activate(110);
 assert(episodes.play && episodes.scenario()==unsigned(Scenario::bw_terran_campaign_8));
 assert(episodes.complete_mission(episodes.scenario(),""));
 Frontend terran_progress;
 terran_progress.init(root.c_str());
 terran_progress.open_episode(brood_terran);
 assert(terran_progress.completed[patriots_blood] && terran_progress.completed[beast] && terran_progress.episode_finished);
 std::remove((root+"campaign/expterran07.brief").c_str());
 std::remove((root+"campaign/expterran08.brief").c_str());
 const auto brood_protoss=campaign::opening(true,2);
 const auto return_to_char_id=unsigned(Scenario::bw_protoss_campaign_6);
 const auto return_to_char=campaign::index(return_to_char_id);
 assert(return_to_char<campaign::count);
 const auto uraj=unsigned(Scenario::bw_protoss_campaign_4);
 const auto braxis=campaign::index(unsigned(Scenario::bw_protoss_campaign_5));
 assert(braxis<campaign::count && campaign::adjacent(campaign::index(uraj),1)==braxis);
 assert(ResultScreen::next_mission(uraj,true,"")==braxis);
 assert(ResultScreen::next_mission(campaign::missions[braxis].scenario,true,"")==return_to_char);
 {std::ofstream brief(root+"campaign/expprotoss06.brief");brief<<"Recover the Khalis crystal.";}
 Frontend protoss_progress;
 protoss_progress.init(root.c_str());
 for(unsigned j=brood_protoss,n=0;j!=return_to_char && n<campaign::count;j=campaign::adjacent(j,1),++n)
  protoss_progress.completed[j]=true;
 protoss_progress.brood=true;
 protoss_progress.open_episode(brood_protoss);
 assert(protoss_progress.episode_resume==return_to_char);
 protoss_progress.activate(130);
 assert(protoss_progress.dialog==Frontend::Dialog::briefing && protoss_progress.mission==return_to_char);
 protoss_progress.activate(110);
 assert(protoss_progress.play && protoss_progress.scenario()==return_to_char_id);
 const auto insurgent=campaign::index(unsigned(Scenario::bw_protoss_campaign_7));
 assert(insurgent<campaign::count && campaign::adjacent(return_to_char,1)==insurgent);
 assert(ResultScreen::next_mission(return_to_char_id,true,"Protoss07.scx")==insurgent);
 assert(ResultScreen::next_mission(return_to_char_id,true,"")==insurgent);
 const auto countdown=campaign::key_index("expprotoss08");
 assert(countdown<campaign::count && campaign::adjacent(insurgent,1)==countdown);
 assert(ResultScreen::next_mission(campaign::missions[insurgent].scenario,true,"")==countdown);
 std::remove((root+"campaign/expprotoss06.brief").c_str());
 f.activate(0);f.activate(130);assert(f.dialog==Frontend::Dialog::error);f.activate(31);assert(f.dialog==Frontend::Dialog::error);f.back();
 for(unsigned i=0;i<3;++i){const auto r=Frontend::campaign_button(i);assert(r.w>=130 && r.h>=24 && r.y+r.h<=240 && f.hit(r.x+2,r.y+2)==(i==0?5:i==1?3:4));}
 f.activate(3);assert(f.leaving && f.screen==Frontend::Screen::campaign && f.hit(80,220)==-1);f.advance(.19f);assert(f.screen==Frontend::Screen::campaign && f.slide()>0);f.activate(4);assert(f.dialog==Frontend::Dialog::none);f.advance(.19f);assert(f.screen==Frontend::Screen::custom && f.slide()==1 && f.hit(290,97)==-1);f.stick(0,0,.19f);assert(f.slide()>.12f && f.slide()<.13f);f.stick(0,0,.19f);assert(f.slide()==0 && f.hit(290,97)==45);
 assert(f.hit(20,198)==13 && f.hit(180,198)==10 && f.hit(20,230)==-1);
 assert(f.hit(200,74)==60);f.activate(60);assert(f.dialog==Frontend::Dialog::map_list);f.activate(91);assert(f.scroll==5);f.activate(71);assert(f.selected_map==6 && f.preview_dirty && f.dialog==Frontend::Dialog::none);f.activate(60);f.direction(2);assert(f.hover==72);f.activate(f.hover);assert(f.selected_map==7);f.selected_map=0;
 assert(f.preview_dirty);f.preview.valid=true;f.preview.players=8;assert(f.capacity()==3 && f.ai_available(1));f.activate(43);assert(f.selected_map==1);f.activate(42);assert(f.selected_map==0);f.activate(40);assert(f.game_type==2 && !f.custom_row_enabled(2) && !f.custom_row_enabled(5));f.activate(41);assert(f.game_type==0);
 f.ai={{3,3}};f.activate(48);assert(f.ai[1]==2);f.activate(49);assert(f.ai[1]==3);f.activate(49);assert(f.ai[1]==4 && std::string(f.ai_name(1))=="Off");auto m=f.match(123);assert(m.enabled && m.races[2]==-1 && m.races[1]>=0 && m.races[1]<3);
 f.preview.players=2;f.ai[1]=3;f.activate(49);assert(f.ai[1]==3 && std::string(f.ai_name(1))=="Off" && f.match(123).races[2]==-1 && f.hit(290,143)==-1);
 f.preview.players=3;f.ai={{4,3}};assert(f.match(123).races[1]==-1 && f.match(123).races[2]>=0);f.ai={{4,4}};m=f.match(123);assert(m.races[1]==-1 && m.races[2]==-1);
 for(unsigned race=0;race<3;++race){f.race=race;assert(f.match(1).races[0]==int(race));f.show_portraits=race!=1;f.save();Frontend again;again.init(root.c_str());assert(again.race==race && again.show_portraits==(race!=1));}
 f.x=200;f.y=97;f.direction(1);assert(f.race==0);f.direction(3);assert(f.race==2);f.direction(2);assert(f.y==120);
 f.activate(51);assert(f.difficulty==1 && f.match(1).difficulty==AiDifficulty::medium);Frontend stored;stored.init(root.c_str());assert(stored.difficulty==1);f.x=200;f.y=166;f.direction(1);assert(f.difficulty==0);f.direction(0);assert(f.y==143);
 assert(f.match(1).teams[1]==f.match(1).teams[2]);f.game_type=1;assert(f.match(1).game_type==1 && f.match(1).teams[1]!=f.match(1).teams[2]);f.dialog=Frontend::Dialog::error;assert(f.hit(160,190)==32);f.activate(32);assert(f.dialog==Frontend::Dialog::none);f.activate(10);assert(f.play);f.back();assert(f.leaving && f.screen==Frontend::Screen::custom);f.advance(.38f);assert(f.screen==Frontend::Screen::campaign && f.transition==0);f.advance(.38f);f.back();f.activate(33);assert(f.quit);
 for(unsigned i=0;i<6;++i)std::remove((root+"maps/zz"+std::to_string(i)+".scx").c_str());std::remove((root+"maps/test.scx").c_str());std::remove((root+"maps/no.txt").c_str());std::remove((root+"maps/folder/nested.scm").c_str());rmdir((root+"maps/folder").c_str());rmdir((root+"maps").c_str());std::remove((root+"player1.cfg").c_str());std::remove((root+"campaign/terran01.brief").c_str());std::remove((root+"campaign/terran01.brief-audio").c_str());std::remove((root+"campaign/progress.cfg").c_str());rmdir((root+"campaign").c_str());rmdir(root.c_str());
 puts("Version/profile + recursive maps + panel animation/input gate + bidirectional settings + random/off/2-/3-/8-slot AI cap PASS");}
