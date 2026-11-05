// Retail maps, ordinary A/context actions, no edited funds/terrain/triggers.
#include "engine.h"
#include <cassert>
#include <cstdio>
#include <string>
using namespace port3ds;
static UnitView unit(Engine&e,unsigned type){UnitView list[1700];auto n=e.visible_units(list,1700);for(unsigned i=0;i<n;++i)if(list[i].owner==0 && list[i].type==type)return list[i];assert(false);return {};}
static void attached(Engine&e,unsigned type,unsigned item){auto w=unit(e,type);UnitView candidates[1700];auto count=e.visible_units(candidates,1700);for(unsigned i=0;i<count;++i)if(!candidates[i].owner && candidates[i].type==type && candidates[i].carried_powerup==item)w=candidates[i];assert(w.carried_powerup==item);assert(e.select(w.x,w.y));assert(e.selection_info().carried_powerup==item);SpriteView views[2048];const auto before=e.state_digest();auto n=e.visible_images(views,2048,w.x-200,w.y-120);bool shown=false;for(unsigned i=0;i<n;++i)if(views[i].order%64>=32 && views[i].image<65300 && views[i].owner==0)shown=true;assert(shown && e.state_digest()==before);}
int main(int argc,char**argv){assert(argc==2);Engine e;std::string root=argv[1];
 assert(e.load(argv[1],(root+"/campaign/expzerg02.chk").c_str(),Scenario::bw_zerg_campaign_2));assert(e.start_recording("/tmp/sc3ds-powerup-controls-bw"));bool carried=false;
 for(unsigned t=0;t<6000 && !carried;++t){assert(e.tick());UnitView list[1700];auto n=e.visible_units(list,1700);for(unsigned i=0;i<n;++i)if(!list[i].owner && list[i].type==7 && list[i].carried_powerup==217)carried=true;}
 assert(carried);attached(e,7,217);
 auto scv=unit(e,7);assert(e.select(scv.x,scv.y));assert(e.activate(1184,3488));
 for(unsigned t=0;t<2400 && (e.count_units(0,41)<2 || e.snapshot().minerals<850 || e.snapshot().gas<200);++t)assert(e.tick());
 assert(e.count_units(0,41)==2 && e.snapshot().minerals==850 && e.snapshot().gas==200);attached(e,7,217);
 SaveView view;scv=unit(e,7);view.camera_x=scv.x-200;view.camera_y=scv.y-120;view.cursor_x=200;view.cursor_y=120;assert(e.save_game("/tmp/sc3ds-bwz2-carry.sav",view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],"/tmp/sc3ds-bwz2-carry.sav","/tmp/sc3ds-bwz2-carry-replay","/tmp/sc3ds-bwz2-carry-map",restored));assert(e.state_digest()==replay.state_digest());attached(replay,7,217);
 for(unsigned i=0;i<96;++i){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}
 assert(e.load(argv[1],(root+"/campaign/zerg02.chk").c_str(),Scenario::zerg_campaign_2));assert(e.start_recording("/tmp/sc3ds-powerup-controls-zerg"));for(unsigned i=0;i<24;++i)assert(e.tick());auto scout=unit(e,43);assert(e.select(scout.x,scout.y));assert(e.move_selected(335,123));for(unsigned i=0;i<240;++i)assert(e.tick());UnitView ds[1700];auto dn=e.visible_units(ds,1700);auto drone=unit(e,41);for(unsigned i=0;i<dn;++i)if(!ds[i].owner && ds[i].type==41 && ds[i].x<drone.x)drone=ds[i];assert(e.select(drone.x,drone.y));assert(e.activate(335,123));assert(e.selection_info().type==41);carried=false;
 for(unsigned t=0;t<3600 && !carried;++t){assert(e.tick());UnitView list[1700];auto n=e.visible_units(list,1700);for(unsigned i=0;i<n;++i)if(!list[i].owner && list[i].type==41 && list[i].carried_powerup==216)carried=true;}
 assert(carried);attached(e,41,216);auto v=unit(e,41);UnitView end[1700];auto en=e.visible_units(end,1700);for(unsigned i=0;i<en;++i)if(!end[i].owner && end[i].carried_powerup==216)v=end[i];view.camera_x=std::max(0,v.x-200);view.camera_y=0;view.cursor_x=v.x-view.camera_x;view.cursor_y=v.y;assert(e.save_game("/tmp/sc3ds-z2-click-held.sav",view));
 puts("Retail BWZ2 SCV intro + A-route/Drone rescue; SCZ2 A-Chrysalis pickup; visible held item/HUD; draw is read-only; exact carried-save replay PASS");
}
