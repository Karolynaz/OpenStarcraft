#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;using namespace bwgame;
static UnitView pick(Engine& e,unsigned type){UnitView units[512];auto n=e.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==type){assert(e.select(units[i].x,units[i].y));return units[i];}std::printf("missing type %u among %zu units\n",type,n);assert(false);return {};}
int main(int argc,char**argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_lab));
 pick(e,unsigned(UnitTypes::Terran_Command_Center));auto info=e.selection_info();assert(info.valid && info.count==1 && info.building && info.completed && info.supply_provided==10 && info.work==WorkKind::none && info.hp==1500 && info.max_hp==1500);
 const int minerals=e.snapshot().minerals;
 for(unsigned i=0;i<5;++i)assert(e.perform(option_code(OptionKind::train,unsigned(UnitTypes::Terran_SCV))));
 info=e.selection_info();assert(info.queue_count==5 && info.queue[0]==unsigned(UnitTypes::Terran_SCV) && info.work==WorkKind::training && e.snapshot().minerals==minerals-250);
 for(unsigned i=0;i<24;++i)assert(e.tick());info=e.selection_info();assert(info.remaining<info.total && info.remaining>0);
 auto cell=ProductionPanel::cell(2);assert(ProductionPanel::hit(cell.x+12,cell.y+12,info)==2);assert(e.cancel_selected_work(2));assert(e.snapshot().minerals==minerals-200 && e.selection_info().queue_count==4);assert(!e.cancel_selected_work(4));
 assert(e.cancel_selected_work(0));assert(e.selection_info().queue_count==3 && e.snapshot().minerals==minerals-150);
 for(unsigned i=0;e.selection_info().queue_count && i<100;++i){const auto before=e.snapshot().minerals;if(!e.cancel_selected_work(e.selection_info().queue_count-1)){assert(e.snapshot().minerals==before);assert(e.tick());}}std::printf("drained queue=%u work=%u money=%d expected=%d\n",e.selection_info().queue_count,unsigned(e.selection_info().work),e.snapshot().minerals,minerals);assert(e.selection_info().work==WorkKind::none && e.snapshot().minerals==minerals);
 auto wire=e.selection_wireframe();auto bounds=e.sprite_info(wire.image,wire.frame);assert(bounds.valid && bounds.width==64 && bounds.height==64);uint16_t pixels[4096];assert(e.sprite_pixels(wire.image,wire.frame,wire.owner,pixels,4096));unsigned green=0;for(auto p:pixels)if(p&1){assert(((p>>6)&31)>((p>>11)&31));++green;}assert(green>100);
 pick(e,unsigned(UnitTypes::Terran_Barracks));const unsigned queue[]={0,32,34,0,32};for(auto type:queue)assert(e.perform(option_code(OptionKind::train,type)));assert(e.cancel_selected_work(2));info=e.selection_info();assert(info.queue_count==4 && info.queue[0]==0 && info.queue[1]==32 && info.queue[2]==0 && info.queue[3]==32);
 pick(e,unsigned(UnitTypes::Terran_Academy));const int m=e.snapshot().minerals,g=e.snapshot().gas;assert(e.perform(option_code(OptionKind::research,unsigned(TechTypes::Stim_Packs))));info=e.selection_info();assert(info.work==WorkKind::research && info.total>0);assert(e.cancel_selected_work(0));assert(e.snapshot().minerals==m && e.snapshot().gas==g && !e.researched(unsigned(TechTypes::Stim_Packs)));
 pick(e,unsigned(UnitTypes::Terran_Engineering_Bay));assert(e.perform(option_code(OptionKind::upgrade,unsigned(UpgradeTypes::Terran_Infantry_Weapons))));assert(e.selection_info().work==WorkKind::upgrade);assert(e.cancel_selected_work(0));assert(e.upgrade_level(unsigned(UpgradeTypes::Terran_Infantry_Weapons))==0);
 pick(e,unsigned(UnitTypes::Terran_SCV));info=e.selection_info();assert(!info.building && info.work==WorkKind::none && info.hp==60 && info.max_shields==0 && info.ground_damage>0);
 // Cancel an actual worker-built structure through the same touched active cell.
 const int before_build=e.snapshot().minerals;bool ordered=false;
 for(int y=1800;y>=1700 && !ordered;y-=32)for(int x=1000;x<1600 && !ordered;x+=32){auto site=e.placement_unit(unsigned(UnitTypes::Terran_Supply_Depot),x,y);if(site.valid)ordered=e.perform(option_code(OptionKind::build,unsigned(UnitTypes::Terran_Supply_Depot)),x,y);}
 assert(ordered);bool cancelled=false;
 for(unsigned frame=0;frame<1800 && !cancelled;++frame){assert(e.tick());UnitView units[512];const auto n=e.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==unsigned(UnitTypes::Terran_Supply_Depot) && !units[i].completed){assert(e.select(units[i].x,units[i].y));info=e.selection_info();assert(info.work==WorkKind::construction && info.total>0 && ProductionPanel::hit(82,178,info)==0);assert(e.cancel_selected_work(0));cancelled=true;break;}}
 assert(cancelled && e.snapshot().minerals==before_build-25);for(unsigned i=0;i<24;++i)assert(e.tick());
 assert(e.select_workers()>1);assert(!e.cancel_selected_work(0));info=e.selection_info();assert(ProductionPanel::hit(82,178,info)==-1);
 assert(e.load(argv[1],argv[2],Scenario::protoss_lab));pick(e,unsigned(UnitTypes::Protoss_Nexus));info=e.selection_info();assert(info.max_shields>0 && info.supply_provided>0);assert(e.perform(option_code(OptionKind::train,unsigned(UnitTypes::Protoss_Probe))));assert(e.selection_info().work==WorkKind::training);assert(e.cancel_selected_work(0));
 assert(e.load(argv[1],argv[2],Scenario::zerg_lab));for(unsigned i=0;i<48;++i)assert(e.tick());pick(e,unsigned(UnitTypes::Zerg_Larva));assert(e.perform(option_code(OptionKind::morph,unsigned(UnitTypes::Zerg_Drone))));for(unsigned i=0;i<24;++i)assert(e.tick());assert(e.selection_info().work==WorkKind::morph);assert(e.cancel_selected_work(0));
 puts("Single info/native stats + 5-slot queue/progress + exact middle/head refunds + research/upgrade/morph cancellation + wireframe + group guard PASS");}
