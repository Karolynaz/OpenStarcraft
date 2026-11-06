// Remove the human force, or the required rescuable Raynor in Terran 2,
// in a private fixture. Terran 2 has no generic empty-human loss trigger.
// This verifies original defeat triggers; shipped maps are never modified.
#include "engine.h"
#include "campaign_map.h"
#include <fstream>
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);for(const auto& mission:campaign::missions){if(std::string(mission.key)=="expprotoss04")continue; // Public Kerrigan-loss mode is in campaign_bw_protoss_fourth.
if(std::string(mission.key)=="expterran02")continue; // Full public platform/fleet loss is tested by campaign_bw_terran_second defeat.
if(std::string(mission.key)=="expterran03"){
 Engine e;const auto path=std::string(argv[2])+"/"+mission.key+".chk";assert(e.load(argv[1],path.c_str(),Scenario(mission.scenario)));e.set_fog_enabled(false);bool sent=false;
 for(unsigned frame=0;frame<30000 && e.snapshot().winner<0;++frame){assert(e.tick());SoundEvent sounds[256];e.drain_sounds(sounds,256);if(frame%48)continue;UnitView units[1700];auto n=e.visible_units(units,1700);
  for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==99 && e.select(units[i].x,units[i].y)){const UnitView* target=nullptr;int best=0x7fffffff;for(unsigned j=0;j<n;++j)if(units[j].owner>=3 && units[j].owner<=6 && (units[j].type==0 || units[j].type==32 || units[j].type==3 || units[j].type==5 || units[j].type==38 || units[j].type==30 || units[j].type==146 || units[j].type==125)){int dx=units[j].x-units[i].x,dy=units[j].y-units[i].y,d=dx*dx+dy*dy;if(d<best){best=d;target=&units[j];}}if(target){sent=true;e.move_selected(target->x,target->y);}}
 }
 printf("%s original public pilot/required hero loss winner%d frame%u\n",mission.key,e.snapshot().winner,e.frame());fflush(stdout);assert(sent && e.snapshot().winner==1);continue;}
if(std::string(mission.key)=="expzerg03"){
 Engine e;const auto path=std::string(argv[2])+"/expzerg03.chk";assert(e.load(argv[1],path.c_str(),Scenario(mission.scenario)));e.set_fog_enabled(false);bool found=false;
 for(unsigned frame=0;frame<30000 && e.snapshot().winner<0;++frame){assert(e.tick());SoundEvent sounds[256];e.drain_sounds(sounds,256);if(frame%120)continue;UnitView units[1700];auto n=e.visible_units(units,1700);for(unsigned j=0;j<n;++j)if(units[j].owner==0 && units[j].type==78 && e.select(units[j].x,units[j].y)){found=true;e.move_selected(3584,3696);}}
 printf("expzerg03 original Fenix loss winner%d frame%u (public movement into original enemy base)\n",e.snapshot().winner,e.frame());assert(found && e.count_units(0,78)==0 && e.snapshot().winner==1);continue;}
 if(std::string(mission.key)=="expzerg02"){
 Engine e;const auto path=std::string(argv[2])+"/expzerg02.chk";assert(e.load(argv[1],path.c_str(),Scenario(mission.scenario)));bool armed=false;
 for(unsigned frame=0;frame<24000 && e.snapshot().winner<0;++frame){assert(e.tick());if(e.count_units(0,41)>=2)armed=true;if(frame%48)continue;UnitView list[1700];auto n=e.visible_units(list,1700);for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type!=101 && list[i].type<106 && e.select(list[i].x,list[i].y)){if(armed)e.move_selected(2624,304);else if(list[i].type==7)e.move_selected(1184,3488);}}
 printf("expzerg02 original armed loss winner%d frame%u (public intro/rescue then force sent into original enemy defenses)\n",e.snapshot().winner,e.frame());fflush(stdout);assert(armed && e.snapshot().winner==1);continue;}
std::ifstream in(std::string(argv[2])+"/"+mission.key+".chk",std::ios::binary);std::vector<uint8_t> b((std::istreambuf_iterator<char>(in)),{});assert(!b.empty());unsigned removed=0;for(size_t at=0;at+8<=b.size();){unsigned n=campaign::get(b.data()+at+4);assert(n<=b.size()-at-8);if(!memcmp(b.data()+at,"UNIT",4)){for(unsigned j=0;j<n;){auto* u=b.data()+at+8+j;if(mission.scenario==22?(u[8]|unsigned(u[9])<<8)==19:u[16]==0){b.erase(b.begin()+at+8+j,b.begin()+at+8+j+36);n-=36;++removed;}else j+=36;}campaign::put(b.data()+at+4,n);}at+=8+n;}assert(removed);std::string path=std::string("/tmp/sc3ds-defeat-")+mission.key+".chk";{std::ofstream out(path,std::ios::binary);out.write((char*)b.data(),b.size());}Engine e;assert(e.load(argv[1],path.c_str(),Scenario(mission.scenario)));for(unsigned i=0;i<3000 && e.snapshot().winner<0;++i)assert(e.tick());printf("%s original loss trigger winner%d frame%u (original required-force/hero absence fixture)\n",mission.key,e.snapshot().winner,e.frame());fflush(stdout);assert(e.snapshot().winner==1);}puts("Catalogue defeat fixtures PASS; BW Terran 2 is covered separately by the full public fleet-loss mode");}
