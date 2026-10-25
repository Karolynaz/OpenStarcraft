#include "engine.h"
#include "campaign_map.h"
#include <fstream>
#include <cassert>
#include <cstdio>
using namespace port3ds;
static void ticks(Engine& e,unsigned n){while(n--){if(!e.tick()){fprintf(stderr,"%s\n",e.error());assert(false);}SoundEvent events[256];e.drain_sounds(events,256);}}
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_campaign));printf("initial workers%u marines%u rescued%u\n",e.count_units(0,7),e.count_units(0,0),e.count_units(0,19));fflush(stdout);
 assert(e.count_units(0,7)==5 && e.count_units(0,0)==5 && !e.count_units(0,19));assert(e.fog_enabled());assert(e.snapshot().supply==10 && e.snapshot().supply_max==18);assert(e.sprite_info(e.resource_icon(0).image,e.resource_icon(0).frame).valid);
 assert(e.start_recording("/tmp/sc3ds-campaign-session"));ticks(e,24);assert(e.snapshot().minerals==40);assert(e.snapshot().objectives.find("Find Raynor")!=std::string::npos);
 // The campaign starts with a travelling colony. Follow the original road to
 // the marshal before moving the SCVs to the established southern base.
 e.select_army(0,0);assert(e.command(Command::attack,1014,1031));e.select_workers();assert(e.move_selected(1014,1031));ticks(e,1600);
 printf("after travel rescued%u workers%u winner%d frame%u\n",e.count_units(0,19),e.count_units(0,7),e.snapshot().winner,e.frame());fflush(stdout);
 assert(e.count_units(0,19)==1);ticks(e,240);assert(e.snapshot().objectives.find("Raynor must survive")!=std::string::npos);
 SaveView view;assert(e.save_game("/tmp/sc3ds-campaign-slot",view));Engine loaded;SaveView returned;
 assert(loaded.load_save(argv[1],"/tmp/sc3ds-campaign-slot","/tmp/sc3ds-campaign-loaded","/tmp/sc3ds-campaign-map",returned));assert(e.state_digest()==loaded.state_digest());
 for(unsigned i=0;i<120;++i){ticks(e,1);ticks(loaded,1);assert(e.state_digest()==loaded.state_digest());}
 puts("Original map units/resources/objectives/rescue + campaign save/load continuation PASS");
 e.set_fog_enabled(false);
 {UnitView army[512];auto n=e.visible_units(army,512);for(unsigned i=0;i<n;++i)if(army[i].owner==0 && (army[i].type==0 || army[i].type==19)){assert(e.select(army[i].x,army[i].y));assert(e.command(Command::attack,1100,1730));}}
 ticks(e,500);
 e.select_workers();assert(e.move_selected(1160,1730));ticks(e,1000);assert(e.command(Command::context,1248,1808));
 for(unsigned i=0;i<8000 && e.snapshot().minerals<160;++i)ticks(e,1);
 printf("gather minerals%d workers%u\n",e.snapshot().minerals,e.count_units(0,7));fflush(stdout);assert(e.snapshot().minerals>=150);
 UnitView units[512];auto count=e.visible_units(units,512);bool selected=false;
 for(unsigned i=0;i<count;++i)if(units[i].owner==0 && units[i].type==7){selected=e.select(units[i].x,units[i].y);if(selected)break;}assert(selected);
 assert(!e.perform(option_code(OptionKind::build,113),880,1456)); // mission forbids Factory
 bool placed=false;for(int y=1456;y<1744 && !placed;y+=32)for(int x=816;x<1232 && !placed;x+=32)if(e.placement_unit(111,x,y).valid)placed=e.perform(option_code(OptionKind::build,111),x,y);
 assert(placed);for(unsigned i=0;i<5000 && !e.count_units(0,111);++i)ticks(e,1);assert(e.count_units(0,111)==1);
 count=e.visible_units(units,512);for(unsigned i=0;i<count;++i)if(units[i].owner==0 && units[i].type==111){assert(e.select(units[i].x,units[i].y));break;}
 for(unsigned i=0;i<15000 && e.snapshot().winner<0;++i){if(i%48==0)e.perform(option_code(OptionKind::train,0));ticks(e,1);}
 printf("win result%d marines%u frame%u\n",e.snapshot().winner,e.count_units(0,0),e.frame());fflush(stdout);assert(e.snapshot().winner==0 && e.count_units(0,0)>=10);
 view.camera_x=900;view.camera_y=1550;assert(e.save_game("/tmp/sc3ds-campaign-won-slot",view));
 puts("Wasteland completed via real rescue, paid mining/construction/training and original victory trigger PASS");
 std::ifstream input(argv[2],std::ios::binary);std::vector<uint8_t> original((std::istreambuf_iterator<char>(input)),{});
 auto twice=original;campaign::normalize(twice);campaign::normalize(twice);assert(twice==original);
 for(unsigned mode=0;mode<2;++mode){auto fixture=original;
  for(size_t at=0;at+8<=fixture.size();){unsigned size=campaign::get(fixture.data()+at+4);if(!memcmp(fixture.data()+at,"UNIT",4)){
   unsigned removed=0;for(unsigned j=0;j<size;){auto* r=fixture.data()+at+8+j;bool remove=mode==0?((r[8]|r[9]<<8)==19):r[16]==0;
    if(remove){fixture.erase(fixture.begin()+at+8+j,fixture.begin()+at+8+j+36);size-=36;removed+=36;}else j+=36;}
   assert(removed);campaign::put(fixture.data()+at+4,size);break;}at+=8+size;}
  const char* path="/tmp/sc3ds-campaign-defeat.chk";{std::ofstream out(path,std::ios::binary);out.write((char*)fixture.data(),fixture.size());}
  Engine loss;assert(loss.load(argv[1],path,Scenario::terran_campaign));for(unsigned i=0;i<600 && loss.snapshot().winner<0;++i)ticks(loss,1);assert(loss.snapshot().winner==1);
 }
 puts("Original hero-death / total-loss defeat branches + owner normalization involution PASS");
}
