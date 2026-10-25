#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <string>
using namespace port3ds;using namespace bwgame;
static UnitView pick(Engine& e,unsigned type){UnitView u[512];unsigned n=e.visible_units(u,512);for(unsigned i=0;i<n;++i)if(!u[i].owner && u[i].type==type){assert(e.select(u[i].x,u[i].y));return u[i];}assert(false);return {};}
static void ticks(Engine&e,unsigned n){while(n--)assert(e.tick());}
int main(int argc,char**argv){assert(argc==4);Engine e;assert(e.load(argv[1],argv[2],Scenario::terran_lab));assert(e.start_recording((std::string(argv[3])+".journal").c_str()));
 pick(e,111);assert(e.perform(option_code(OptionKind::train,0)));assert(e.perform(option_code(OptionKind::train,1)));pick(e,113);assert(e.perform(option_code(OptionKind::train,5)));
 pick(e,112);assert(e.perform(option_code(OptionKind::research,unsigned(TechTypes::Stim_Packs))));pick(e,120);assert(e.perform(option_code(OptionKind::research,unsigned(TechTypes::Tank_Siege_Mode))));ticks(e,1800);
 assert(e.researched(unsigned(TechTypes::Stim_Packs)) && e.researched(unsigned(TechTypes::Tank_Siege_Mode)));
 assert(e.select_area(0,0,2048,2048)>=11);auto selected=e.snapshot().selected;MenuOption powers[64];const auto digest=e.state_digest();unsigned n=e.abilities(powers,64);assert(e.state_digest()==digest && e.snapshot().selected==selected);bool stim=false,siege=false,nuke=false;for(unsigned i=0;i<n;++i){stim|=powers[i].code==option_code(OptionKind::stim);siege|=powers[i].code==option_code(OptionKind::siege);nuke|=powers[i].code==option_code(OptionKind::order,unsigned(Orders::NukePaint));}assert(stim && siege && nuke);assert(powers[0].code==option_code(OptionKind::stim) && powers[1].code==option_code(OptionKind::siege));
 assert(e.perform(option_code(OptionKind::stim)));assert(e.snapshot().selected==selected);assert(e.perform(option_code(OptionKind::siege)));assert(e.snapshot().selected==selected);ticks(e,48);UnitView units[512];unsigned count=e.visible_units(units,512);bool marine=false,tank=false;for(unsigned i=0;i<count;++i)if(!units[i].owner){if(units[i].type==0){assert(units[i].hp==30);marine=true;}if(units[i].type==30)tank=true;if(units[i].worker)assert(units[i].hp==60);}assert(marine && tank);
 MenuOption moves[4];e.movement_options(moves);assert(e.perform(moves[1].code,1000,1800));assert(e.perform(moves[2].code));assert(e.snapshot().selected==selected);
 assert(e.select_workers(0,0)==0);assert(e.select_workers()>1);assert(e.snapshot().workers_only);pick(e,7);MenuOption build[64];assert(e.options(OptionPage::basic,build,64)==9);auto portrait=e.portrait();auto bounds=e.sprite_info(portrait.image,portrait.frame);assert(bounds.valid && bounds.width==48 && bounds.height==48);uint16_t pixels[2304];assert(e.sprite_pixels(portrait.image,portrait.frame,portrait.owner,pixels,2304));unsigned colored=0;for(auto p:pixels)if(p>1)++colored;assert(colored>100);
 SaveView view;view.camera_x=100;view.camera_y=1500;assert(e.save_game(argv[3],view));Engine other;SaveView loaded;assert(other.load_save(argv[1],argv[3],(std::string(argv[3])+".replay").c_str(),(std::string(argv[3])+".map").c_str(),loaded));assert(e.state_digest()==other.state_digest());ticks(e,48);ticks(other,48);assert(e.state_digest()==other.state_digest());
 puts("Mixed Stim/Siege/Nuke catalogue, eligible-only dispatch, selection preservation, patrol/stop, viewport workers, 9 builds, original portrait and replay PASS");
 // A Protoss worker fixture for visual/controller testing, same source as ROM.
 assert(e.load(argv[1],argv[2],Scenario::protoss_lab));assert(e.start_recording((std::string(argv[3])+".protoss-journal").c_str()));auto probe=pick(e,64);view.camera_x=probe.x-200;view.camera_y=probe.y-120;view.cursor_x=200;view.cursor_y=120;assert(e.save_game((std::string(argv[3])+".protoss").c_str(),view));
}
