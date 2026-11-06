// Enemy inspection exposes information, never command authority.
#include "engine.h"
#include "bwenums.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char** argv){assert(argc==3);Engine e;assert(e.load(argv[1],argv[2],Scenario::bw_zerg_campaign));e.set_fog_enabled(false);for(int i=0;i<12;i++)assert(e.tick());UnitView units[1700];auto count=e.visible_units(units,1700);bool soldier=false,building=false;
 for(unsigned i=0;i<count;i++){const auto u=units[i];if(u.burrowed || !u.owner || u.mineral || u.geyser || (u.type>=106 && building) || (u.type<106 && soldier))continue;
  e.clear_selection();assert(e.activate(u.x,u.y));auto info=e.selection_info();if(info.own || info.type!=u.type)continue;assert(info.valid && info.count==1 && info.hp==u.hp && info.max_hp==u.max_hp);auto before=e.state_digest();MenuOption options[64];assert(e.options(OptionPage::primary,options,64)==0 && e.options(OptionPage::secondary,options,64)==0);assert(!e.move_selected(u.x+32,u.y));assert(!e.command(Command::stop));assert(!e.perform(option_code(OptionKind::order,unsigned(bwgame::Orders::Stop))));assert(!e.cancel_selected_work(0));assert(e.state_digest()==before);if(u.type>=106)building=true;else soldier=true;
 }
 assert(soldier && building);puts("Visible hostile soldier/building selection and HP/armor inspection; movement/orders/production/cancellation remain read-only PASS");
}
