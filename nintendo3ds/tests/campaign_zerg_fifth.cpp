// Public controls on the unchanged owned Amerigo map. Fog-off navigation only.
// Native cloak, combat, automatic doors, unlock switch, teleport and Zerg
// regeneration; no unit/resource/stat/AI/trigger overrides.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace port3ds;using namespace bwgame;
int main(int n,char**v){assert(n==3);Engine e;assert(e.load(v[1],v[2],Scenario::zerg_campaign_5));assert(e.start_recording("/tmp/sc3ds-zerg05.journal"));e.set_fog_enabled(false);bool cloaked=false;unsigned storms=0;bool healed=false,decloaked=false;bool unlocked=false;bool teleported=false;int previous_y=0;
for(unsigned f=0;f<160000 && e.snapshot().winner<0;++f){if(!e.tick()){printf("ERROR %s\n",e.error());return 1;}SoundEvent snd[256];e.drain_sounds(snd,256);if(f%24)continue;UnitView u[1700];auto n=e.visible_units(u,1700);UnitView* hero=nullptr;for(unsigned i=0;i<n;++i)if(u[i].owner==0 && u[i].type==51)hero=&u[i];if(!hero)break;
 if(previous_y>3400 && hero->y<2000)teleported=true;previous_y=hero->y;if(teleported && healed && hero->hp<250){healed=false;decloaked=false;}if(teleported && !healed){if(e.select(hero->x,hero->y)){int dx=hero->x-784,dy=hero->y-1584;if(dx*dx+dy*dy>40*40)e.move_selected(784,1584);else{if(!decloaked){decloaked=e.perform(option_code(OptionKind::cloak));if(decloaked)cloaked=false;}e.perform(option_code(OptionKind::order,unsigned(Orders::HoldPosition)));if(hero->hp>=390 && hero->energy>=240){healed=true;printf("HEALED f%u\n",e.frame());}}}if(f%2400==0){printf("REST f%u at%d,%d hp%d energy%d\n",e.frame(),hero->x,hero->y,hero->hp,hero->energy);fflush(stdout);}continue;}if(hero->x>=2144 && hero->x<=2272 && hero->y>=2336 && hero->y<=2432)unlocked=true;int gx=!unlocked?2208:teleported?304:688,gy=!unlocked?2384:teleported?2128:3648;bool casting=false;if(f==0 && e.select(hero->x,hero->y)){MenuOption o[32];auto k=e.options(OptionPage::primary,o,32);for(unsigned j=0;j<k;++j)printf("OPTION %s %x enabled%d\n",o[j].label,o[j].code,o[j].enabled);}if(e.select(hero->x,hero->y)){
 if(hero->energy<60){for(unsigned i=0;i<n;++i)if(u[i].owner==0 && u[i].type==37){auto dx=u[i].x-hero->x,dy=u[i].y-hero->y;if(dx*dx+dy*dy<180*180){casting=e.perform(option_code(OptionKind::order,unsigned(Orders::CastConsume)),u[i].x,u[i].y);if(casting)break;}}}
 if(!cloaked && e.select(hero->x,hero->y)){cloaked=e.perform(option_code(OptionKind::cloak));}
 if(!casting && hero->energy>=75){const UnitView* best=nullptr;unsigned score=0;for(unsigned i=0;i<n;++i)if(u[i].owner>=6 && u[i].type<106){auto dx=u[i].x-hero->x,dy=u[i].y-hero->y;if(dx*dx+dy*dy>240*240)continue;unsigned count=0;for(unsigned j=0;j<n;++j)if(u[j].owner>=6 && u[j].type<106){auto a=u[i].x-u[j].x,b=u[i].y-u[j].y;if(a*a+b*b<80*80)++count;}if(count>score){best=&u[i];score=count;}}if(best){casting=e.perform(option_code(OptionKind::order,unsigned(Orders::CastPsionicStorm)),best->x,best->y);if(casting)++storms;}}
 }
 if(f%240==0)for(unsigned i=0;i<n;++i)if(u[i].owner==0 && (u[i].type==37 || u[i].type==53 || (u[i].type==51 && !casting))){if(e.select(u[i].x,u[i].y)){if(u[i].type==51){if(healed){if(hero->x<650 && hero->y>1950)e.move_selected(gx,gy);else e.command(Command::attack,gx,gy);}else e.move_selected(gx,gy);}else if(u[i].type==37)e.move_selected(hero->x,hero->y);else e.command(Command::attack,gx,gy);}}
 if(f%480==0){printf("f%u hero%d,%d hp%d e%d lings%u hunters%u storms%u winner%d\n",e.frame(),hero->x,hero->y,hero->hp,hero->energy,e.count_units(0,37),e.count_units(0,53),storms,e.snapshot().winner);fflush(stdout);}}
printf("Amerigo result%d frame%u Kerrigan%u storms%u\n",e.snapshot().winner,e.frame(),e.count_units(0,51),storms);
assert(e.snapshot().winner==0 && unlocked && teleported && healed && storms==0 && e.count_units(0,51)==1);
SaveView view;assert(e.save_game("/tmp/sc3ds-zerg05-completed.ob3",view));Engine replay;SaveView restored;
assert(replay.load_save(v[1],"/tmp/sc3ds-zerg05-completed.ob3","/tmp/sc3ds-zerg05-replay","/tmp/sc3ds-zerg05-replay-map",restored));
assert(e.state_digest()==replay.state_digest());puts("The Amerigo original victory, surviving Kerrigan, original unlock/teleport, native combat/regeneration and completed save replay PASS");return 0;}
