// Original SC Terran8 starts with six buildings already in flight.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <vector>
using namespace port3ds;using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_campaign));assert(e.start_recording("/tmp/sc3ds-initial-transit.journal"));e.set_fog_enabled(false);UnitView list[1700];auto count=e.visible_units(list,1700);unsigned airborne=0;UnitView starport;
 for(unsigned i=0;i<count;++i)if(list[i].owner==0 && list[i].type>=106 && list[i].type<=116){assert(list[i].flying && list[i].completed);++airborne;if(list[i].type==114)starport=list[i];}
 // Check every original placement, including both Command Centers. A count and
 // one Starport coordinate would miss a takeoff offset on another building.
 const unsigned placements[][3]={{106,3616,3952},{106,3904,3952},
  {111,3904,3760},{113,3968,3856},{114,3808,3856},{116,3776,3952}};
 assert(airborne==6);for(const auto& p:placements){unsigned matches=0;for(unsigned i=0;i<count;++i)
  if(list[i].owner==0 && list[i].type==p[0] && list[i].x==int(p[1]) && list[i].y==int(p[2]))++matches;
  assert(matches==1);}
 assert(e.select(starport.x,starport.y));assert(!e.perform(option_code(OptionKind::train,8)));assert(e.perform(option_code(OptionKind::order,unsigned(Orders::Move)),3280,3504));
 for(unsigned i=0;i<600;++i)assert(e.tick());bool moved=false;count=e.visible_units(list,1700);for(unsigned i=0;i<count;++i)if(list[i].owner==0 && list[i].type==114){starport=list[i];moved=starport.flying && (abs(starport.x-3808)+abs(starport.y-3856)>128);}assert(moved);
 SaveView view;view.camera_x=3100;view.camera_y=3380;assert(e.save_game("/tmp/sc3ds-initial-transit.ob3",view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],"/tmp/sc3ds-initial-transit.ob3","/tmp/sc3ds-initial-transit-replay","/tmp/sc3ds-initial-transit-map",restored));assert(e.state_digest()==replay.state_digest());for(unsigned i=0;i<48;++i){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}
 assert(e.select(starport.x,starport.y));bool landing=false;for(int y=3136;y<3904 && !landing;y+=64)for(int x=2560;x<3904 && !landing;x+=64)landing=e.perform(option_code(OptionKind::land),x,y);assert(landing);bool landed=false;for(unsigned frame=0;frame<3000 && !landed;++frame){assert(e.tick());if(frame%24)continue;count=e.visible_units(list,1700);for(unsigned i=0;i<count;++i)if(list[i].owner==0 && list[i].type==114)landed=!list[i].flying;}assert(landed);
 // A set state bit is ignored unless its corresponding valid-state bit is set.
 std::ifstream input(argv[2],std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),{});bool changed=false;for(size_t at=0;at+8<=bytes.size();){unsigned size=unsigned(bytes[at+4])|unsigned(bytes[at+5])<<8|unsigned(bytes[at+6])<<16|unsigned(bytes[at+7])<<24;assert(size<=bytes.size()-at-8);if(!memcmp(bytes.data()+at,"UNIT",4))for(unsigned off=0;off<size;off+=36){auto* u=bytes.data()+at+8+off;if(u[16]==0 && (u[8]|unsigned(u[9])<<8)==111){u[12]&=~4;changed=true;}}at+=8+size;}assert(changed);const char* fixture="/tmp/sc3ds-transit-valid-mask.chk";{std::ofstream output(fixture,std::ios::binary);output.write((char*)bytes.data(),bytes.size());}Engine mask;assert(mask.load(argv[1],fixture,Scenario::terran_campaign));mask.set_fog_enabled(false);count=mask.visible_units(list,1700);unsigned grounded_barracks=0;for(unsigned i=0;i<count;++i)if(list[i].owner==0 && list[i].type==111){assert(!list[i].flying);++grounded_barracks;}assert(grounded_barracks==1);
 puts("Original six preplaced airborne buildings, unchanged initial coordinates, flight/production restriction, exact save+48 and native landing PASS");
}
