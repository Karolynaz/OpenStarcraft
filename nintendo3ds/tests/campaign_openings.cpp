#include "engine.h"
#include "frontend.h"
#include "campaign_map.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
using namespace port3ds;
int main(int argc,char**argv){
 assert(argc==3 || argc==4);
 char temporary[]="/tmp/openbw-campaign-openings-XXXXXX";
 const char* directory=argc==4?argv[3]:mkdtemp(temporary);
 assert(directory);
 const std::string work=std::string(directory)+"/";
 for(unsigned i=0;i<campaign::count;++i){const auto& m=campaign::missions[i];std::string path=std::string(argv[2])+"/"+m.key+".chk";
  Frontend front;front.base=std::string(argv[2])+"/../maps";front.brood=m.brood;unsigned card=m.brood?(m.race==1?0:m.race==2?1:2):(m.race==0?0:m.race==1?1:2);
  for(unsigned j=0;j<i;++j)
   if(campaign::missions[j].brood==m.brood && campaign::missions[j].race==m.race){
    if(campaign::same_chapter(j,i))front.next_missions[(m.brood?3:0)+m.race]=m.key;
    else if(!campaign::bonus(campaign::missions[j]))front.completed[j]=true;
   }
  if(campaign::bonus(m))front.next_missions[3]=m.key;
  front.activate(card);
  if(m.scenario==campaign::tutorial_scenario)front.activate(131);else front.activate(130);assert(front.dialog==Frontend::Dialog::story);assert(front.mission==i);front.activate(115);assert(front.dialog==Frontend::Dialog::briefing);front.update_presentation(Frontend::briefing_entry_seconds,false);front.activate(116);front.update_presentation(Frontend::briefing_entry_seconds,false);assert(front.play && front.scenario()==m.scenario && front.map_path().find(m.key)!=std::string::npos);
  Engine e;if(!e.load(argv[1],path.c_str(),Scenario(m.scenario))){fprintf(stderr,"%s load: %s\n",m.key,e.error());return 1;}assert(e.snapshot().campaign && e.fog_enabled());assert(e.start_recording((work+m.key+".journal").c_str()));
  for(unsigned n=0;n<1200 && e.snapshot().winner<0;++n){if(!e.tick()){fprintf(stderr,"%s frame%u: %s\n",m.key,e.frame(),e.error());return 1;}}
  SaveView view;const std::string save=work+m.key+".ob3";assert(e.save_game(save.c_str(),view));Engine replay;SaveView loaded;if(!replay.load_save(argv[1],save.c_str(),(work+"replay").c_str(),(work+"restored-map.chk").c_str(),loaded)){fprintf(stderr,"%s replay: %s\n",m.key,replay.error());return 1;}assert(e.state_digest()==replay.state_digest());for(unsigned n=0;n<48;++n){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}
  auto s=e.snapshot();printf("%s startup frame%u units%u winner%d objectives%zu\n",m.key,e.frame(),s.units,s.winner,s.objectives.size());fflush(stdout);assert(s.winner!=1 && !s.objectives.empty());
  for(unsigned n=0;n<16000 && e.snapshot().winner<0;++n)if(!e.tick()){fprintf(stderr,"%s extended frame%u: %s\n",m.key,e.frame(),e.error());return 1;}printf("%s extended frame%u units%u winner%d\n",m.key,e.frame(),e.snapshot().units,e.snapshot().winner);fflush(stdout);
 }
 printf("%u original maps / menu routing / trigger startup PASS\n",campaign::count);
}
