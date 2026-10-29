// Owned-map-derived synthetic hero arena. Never replaces a campaign map.
#include "engine.h"
#include "campaign_map.h"
#include "bwenums.h"
#include <cassert>
#include <fstream>
#include <cstdio>
#include <unistd.h>
using namespace port3ds;using namespace bwgame;
struct Hero {UnitTypes type;unsigned code;};
int main(int argc,char**argv){assert(argc==3);
 const Hero heroes[]={{UnitTypes::Hero_Infested_Kerrigan,option_code(OptionKind::cloak)}, {UnitTypes::Hero_Tassadar,option_code(OptionKind::order,unsigned(Orders::CastPsionicStorm))}, {UnitTypes::Hero_Matriarch,option_code(OptionKind::order,unsigned(Orders::CastEnsnare))}, {UnitTypes::Hero_Unclean_One,option_code(OptionKind::order,unsigned(Orders::CastPlague))}, {UnitTypes::Hero_Danimoth,option_code(OptionKind::order,unsigned(Orders::CastRecall))}, {UnitTypes::Hero_Raszagal,option_code(OptionKind::order,unsigned(Orders::CastDisruptionWeb))}, {UnitTypes::Hero_Jim_Raynor_Marine,option_code(OptionKind::stim)}, {UnitTypes::Hero_Gui_Montag,option_code(OptionKind::stim)}, {UnitTypes::Hero_Jim_Raynor_Vulture,option_code(OptionKind::order,unsigned(Orders::PlaceMine))}, {UnitTypes::Hero_Edmund_Duke_Tank_Mode,option_code(OptionKind::siege)}, {UnitTypes::Hero_Edmund_Duke_Siege_Mode,option_code(OptionKind::siege)}, {UnitTypes::Hero_Tom_Kazansky,option_code(OptionKind::cloak)}, {UnitTypes::Hero_Magellan,option_code(OptionKind::order,unsigned(Orders::CastIrradiate))}, {UnitTypes::Hero_Hyperion,option_code(OptionKind::order,unsigned(Orders::FireYamatoGun))}, {UnitTypes::Hero_Norad_II,option_code(OptionKind::order,unsigned(Orders::FireYamatoGun))}, {UnitTypes::Hero_Gerard_DuGalle,option_code(OptionKind::order,unsigned(Orders::FireYamatoGun))}, {UnitTypes::Hero_Infested_Duran,option_code(OptionKind::order,unsigned(Orders::CastConsume))}};
 for(const auto& h:heroes){std::ifstream in(argv[2],std::ios::binary);std::vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});
  auto chunk=[&](const char* tag,const std::vector<uint8_t>& data){for(size_t at=0;at+8<=map.size();){auto n=campaign::get(map.data()+at+4);if(!memcmp(map.data()+at,tag,4)){map.erase(map.begin()+at+8,map.begin()+at+8+n);map.insert(map.begin()+at+8,data.begin(),data.end());campaign::put(map.data()+at+4,data.size());return;}at+=8+n;}assert(false);};
  std::vector<uint8_t> owners(12),races(12,1),forces(20);owners[0]=6;chunk("OWNR",owners);chunk("SIDE",races);chunk("FORC",forces);chunk("TRIG",{});chunk("THG2",{});
  std::vector<uint8_t> unit(36);unit[4]=0;unit[5]=4;unit[6]=0;unit[7]=4;unit[8]=unsigned(h.type);unit[9]=unsigned(h.type)>>8;unit[14]=14;unit[17]=unit[18]=unit[19]=100;chunk("UNIT",unit);
  for(size_t at=0;at+8<=map.size();){auto n=campaign::get(map.data()+at+4);unsigned count=!memcmp(map.data()+at,"PTEC",4)?24:!memcmp(map.data()+at,"PTEx",4)?44:0;if(count){assert(n==count*38);auto* p=map.data()+at+8;for(unsigned i=0;i<n;++i)p[i]=(i<12*count || (i>=24*count && i<25*count) || i>=26*count)?1:0;}at+=8+n;}
  const auto base=std::string("/tmp/sc3ds-hero-command-")+std::to_string(getpid());{std::ofstream out(base+".chk",std::ios::binary);out.write((char*)map.data(),map.size());}
  Engine e;assert(e.load(argv[1],(base+".chk").c_str(),Scenario::bw_protoss_campaign));assert(e.start_recording((base+".journal").c_str()));e.set_fog_enabled(false);for(unsigned i=0;i<8;++i)assert(e.tick());assert(e.select(1024,1024));assert(std::string(e.selection_info().name)!="Unit");MenuOption options[32];auto n=e.options(OptionPage::primary,options,32);bool found=false;for(unsigned i=0;i<n;++i)if(options[i].code==h.code && options[i].enabled)found=true;if(!found)fprintf(stderr,"Missing hero%u code%x options%u\n",unsigned(h.type),h.code,n);assert(found);
  if(h.type==UnitTypes::Hero_Tassadar)for(unsigned i=0;i<n;++i)assert(options[i].code!=option_code(OptionKind::merge));
  if(h.type==UnitTypes::Hero_Infested_Kerrigan){assert(n==4);auto energy=e.snapshot().selected_energy;assert(e.perform(h.code));for(unsigned i=0;i<48;++i)assert(e.tick());assert(e.snapshot().selected_energy<energy);SaveView view;view.camera_x=824;view.camera_y=904;assert(e.save_game((base+".sav").c_str(),view));Engine replay;SaveView restored;assert(replay.load_save(argv[1],(base+".sav").c_str(),(base+".replay").c_str(),(base+".map").c_str(),restored));for(unsigned i=0;i<48;++i){assert(e.tick() && replay.tick());assert(e.state_digest()==replay.state_digest());}}
  printf("Hero%u native-enabled command%x PASS\n",unsigned(h.type),h.code);
 }
 puts("17 hero command aliases, hero-only merge exclusion and Infested Kerrigan cloak save replay PASS");
}
