// Display counters must come from the native economy/completion state and
// survive actual public-order save reconstruction, not inferred AI counters.
#include "engine.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){
 assert(argc==3);MatchSetup setup;setup.enabled=true;
 for(unsigned race=0;race<3;++race){setup.races[0]=race;Engine check;assert(check.load(argv[1],argv[2],Scenario::versus_ai,&setup));assert(check.snapshot().human_race==race);}
 setup.races[0]=1;Engine e;assert(e.load(argv[1],argv[2],Scenario::versus_ai,&setup));
 assert(e.start_recording("/tmp/sc3ds-result-statistics.journal"));e.set_fog_enabled(false);
 for(unsigned i=0;i<120;++i)assert(e.tick());
 auto before=e.snapshot();assert(before.human_race==1);
 UnitView list[1700];unsigned n=e.visible_units(list,1700);const UnitView* worker=nullptr;const UnitView* mineral=nullptr;
 for(unsigned i=0;i<n;++i){if(list[i].owner==0 && list[i].type==7)worker=&list[i];}
 assert(worker);int best=0x7fffffff;for(unsigned i=0;i<n;++i)if(list[i].mineral){int dx=list[i].x-worker->x,dy=list[i].y-worker->y,d=dx*dx+dy*dy;if(d<best){best=d;mineral=&list[i];}}
 assert(mineral && e.select(worker->x,worker->y) && e.command(Command::context,mineral->x,mineral->y));
 for(unsigned i=0;i<2400;++i)assert(e.tick());
 assert(e.snapshot().minerals_gathered>before.minerals_gathered);
 n=e.visible_units(list,1700);bool queued=false;
 for(unsigned i=0;i<n && !queued;++i)if(list[i].owner==0 && list[i].type==106 && e.select(list[i].x+40,list[i].y+24))queued=e.perform(option_code(OptionKind::train,7));
 assert(queued);for(unsigned i=0;i<1200;++i)assert(e.tick());
 auto result=e.snapshot();assert(result.units_completed>before.units_completed);
 SaveView view;assert(e.save_game("/tmp/sc3ds-result-statistics.sav",view));
 Engine restored;SaveView loaded;assert(restored.load_save(argv[1],"/tmp/sc3ds-result-statistics.sav","/tmp/sc3ds-result-statistics.replay","/tmp/sc3ds-result-statistics.map",loaded));
 auto replay=restored.snapshot();assert(e.state_digest()==restored.state_digest());
 assert(result.units_completed==replay.units_completed && result.buildings_completed==replay.buildings_completed && result.minerals_gathered==replay.minerals_gathered && result.gas_gathered==replay.gas_gathered);
 puts("Native gathered-resource/completed-unit result statistics and exact active-save reconstruction PASS");
}
