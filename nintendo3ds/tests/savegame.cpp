#include "engine.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
using namespace port3ds;
static void ticks(Engine& e,unsigned n){while(n--){assert(e.tick());SoundEvent sounds[256];e.drain_sounds(sounds,256);}}
static void compare(Engine& a,Engine& b,unsigned n){while(n--){assert(a.state_digest()==b.state_digest());ticks(a,1);ticks(b,1);}assert(a.state_digest()==b.state_digest());}
int main(int argc,char** argv){assert(argc==4);const std::string tmp="/tmp/sc3ds-save-"+std::to_string(getpid());assert(!mkdir(tmp.c_str(),0700));const auto session=tmp+"/session",slot=tmp+"/slot",session2=tmp+"/session2",map=tmp+"/map";char error[192];SaveInfo info;SaveView view;view.camera_x=150.5f;view.camera_y=180.25f;view.cursor_x=120;view.auto_camera=true;
 {
 Engine original;assert(original.load(argv[1],argv[3],Scenario::terran_lab));assert(original.start_recording(session.c_str()));UnitView units[512];auto n=original.visible_units(units,512);for(unsigned i=0;i<n;++i)if(units[i].owner==0 && units[i].type==106){assert(original.select(units[i].x,units[i].y));break;}
 assert(original.perform(option_code(OptionKind::train,7)));assert(original.perform(option_code(OptionKind::train,7)));assert(original.cancel_selected_work(1));original.store_group(0);original.select_workers();original.move_selected(900,900);original.recall_group(0);original.set_fog_enabled(true);ticks(original,123);original.set_fog_enabled(false);ticks(original,77);
 assert(original.save_game(slot.c_str(),view));assert(Engine::inspect_save(slot.c_str(),info,error,sizeof error));assert(info.frame==original.frame() && info.view.camera_x==view.camera_x);
 Engine restored;SaveView loaded;assert(restored.load_save(argv[1],slot.c_str(),session2.c_str(),map.c_str(),loaded));assert(loaded.camera_y==view.camera_y && loaded.auto_camera);compare(original,restored,300);
 restored.select_workers();restored.move_selected(1000,1000);ticks(restored,24);
 assert(restored.save_game(slot.c_str(),view));assert(access((slot+".bak").c_str(),F_OK)==0);Engine twice;assert(twice.load_save(argv[1],slot.c_str(),session.c_str(),map.c_str(),loaded));compare(restored,twice,100);
 std::ifstream in(slot,std::ios::binary);std::string bytes((std::istreambuf_iterator<char>(in)),{});bytes.back()^=1;{std::ofstream bad(tmp+"/bad",std::ios::binary);bad.write(bytes.data(),bytes.size());}const auto digest=twice.state_digest();assert(!twice.load_save(argv[1],(tmp+"/bad").c_str(),session.c_str(),map.c_str(),loaded) && twice.ready() && twice.state_digest()==digest);
 {auto foreign=bytes;foreign[136]^=1;std::ofstream bad(tmp+"/foreign",std::ios::binary);bad.write(foreign.data(),foreign.size());bad.close();assert(!Engine::inspect_save((tmp+"/foreign").c_str(),info,error,sizeof error));assert(std::string(error).find("different build")!=std::string::npos);}
 {std::ofstream bad(tmp+"/short",std::ios::binary);bad.write(bytes.data(),100);bad.close();assert(!Engine::inspect_save((tmp+"/short").c_str(),info,error,sizeof error));}
 bool cancelled=false;assert(!twice.load_save(argv[1],slot.c_str(),session.c_str(),map.c_str(),loaded,[&](unsigned,unsigned){cancelled=true;return false;}));assert(cancelled);
 Engine interrupted;unsigned callbacks=0;assert(!interrupted.load_save(argv[1],slot.c_str(),session2.c_str(),map.c_str(),loaded,[&](unsigned done,unsigned){++callbacks;return done<48;}));assert(callbacks>=3 && !interrupted.ready());
 puts("Save queue/cancel/selection/groups/fog/camera + exact continuation + re-save/backup + corrupt preflight PASS");
 }
 {
 Engine a;MatchSetup setup;setup.enabled=setup.observer=true;setup.races={{1,2,0}};setup.teams={{0,1,1}};setup.side_difficulty={{1,0,0}};
 assert(a.load(argv[1],argv[2],Scenario::gas_battle,&setup));assert(a.start_recording(session.c_str()));ticks(a,20000);assert(a.save_game(slot.c_str(),view));Engine b;SaveView loaded;assert(b.load_save(argv[1],slot.c_str(),session2.c_str(),map.c_str(),loaded));compare(a,b,720);puts("Mixed AI teams/tech/scouts/transport plans/RNG replay + 720-frame exact continuation PASS");
 }
 {
 Engine a;assert(a.load(argv[1],argv[3],Scenario::terran_battle));assert(a.start_recording(session.c_str()));for(unsigned i=0;i<120;++i){a.advance_wall_time(17321);ticks(a,1);}assert(a.save_game(slot.c_str(),view));Engine b;SaveView loaded;assert(b.load_save(argv[1],slot.c_str(),session2.c_str(),map.c_str(),loaded));compare(a,b,60);puts("Stress fixture wall-clock journal round-trip PASS");
 }
}
