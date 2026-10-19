#pragma once
// Host-only conservative traversal of every animation branch reachable from
// the Terran roster. This includes unobserved directions, death and spell FX.
#include "race_catalogue.h"
#include <set>
#include <vector>
inline void terran_asset_closure(bwgame::state& st,std::set<unsigned>& images,std::set<unsigned>& sounds,bool protoss=false) {
 using namespace bwgame;using namespace iscript_opcodes;
 if(!protoss)for(unsigned id:{84u,127u,239u})sounds.insert(id); // Native nuclear launch / warning / spotter sounds.
 auto& g=*st.global;std::vector<unsigned> queue(images.begin(),images.end());
 auto image=[&](unsigned id){if(id<g.image_types.vec.size() && images.insert(id).second)queue.push_back(id);};
 auto sprite=[&](unsigned id){if(id<g.sprite_types.vec.size())image(unsigned(static_cast<ImageTypes>(g.sprite_types.vec[id].image)));};
 auto weapon=[&](const weapon_type_t* w){if(w && w->flingy)sprite(unsigned(static_cast<SpriteTypes>(w->flingy->sprite)));};
 auto unit=[&](UnitTypes id){const auto& u=st.game->unit_types.vec.at(unsigned(id));if(u.flingy)sprite(unsigned(static_cast<SpriteTypes>(u.flingy->sprite)));weapon(u.ground_weapon);weapon(u.air_weapon);};
 if(protoss){
  for(const auto& p:port3ds::protoss::buildings)unit(p.type);
  for(const auto& p:port3ds::protoss::units)unit(p.type);
  unit(UnitTypes::Protoss_Archon);unit(UnitTypes::Protoss_Dark_Archon);unit(UnitTypes::Spell_Disruption_Web);
  for(auto id:{ImageTypes::IMAGEID_Psionic_Storm,ImageTypes::IMAGEID_Recall_Field,ImageTypes::IMAGEID_Hallucination_Hit,ImageTypes::IMAGEID_Stasis_Field_Hit,ImageTypes::IMAGEID_Disruption_Web})image(unsigned(id));
  for(unsigned id=unsigned(ImageTypes::IMAGEID_Mind_Control_Hit_Small);id<=unsigned(ImageTypes::IMAGEID_Feedback_Hit_Large);++id)image(id);
  for(unsigned id:{550u,551u,552u,553u,616u,618u,1061u,1062u,1064u})sounds.insert(id);
 }else{
 for(const auto& p:port3ds::terran::buildings)unit(p.type);
 for(const auto& p:port3ds::terran::units)unit(p.type);
 for(auto id:{UnitTypes::Terran_Goliath_Turret,UnitTypes::Terran_Siege_Tank_Tank_Mode_Turret,UnitTypes::Terran_Siege_Tank_Siege_Mode,UnitTypes::Terran_Siege_Tank_Siege_Mode_Turret,UnitTypes::Terran_Vulture_Spider_Mine,UnitTypes::Spell_Scanner_Sweep})unit(id);
 for(unsigned id=0;id<=unsigned(WeaponTypes::Irradiate);++id)weapon(&st.game->weapon_types.vec.at(id));
 for(auto id:{WeaponTypes::Halo_Rockets,WeaponTypes::Optical_Flare,WeaponTypes::Restoration})weapon(&st.game->weapon_types.vec.at(unsigned(id)));
 auto range=[&](ImageTypes from,ImageTypes to){for(unsigned i=unsigned(from);i<=unsigned(to);++i)image(i);};
 range(ImageTypes::IMAGEID_Lockdown_Field_Small,ImageTypes::IMAGEID_Lockdown_Field_Large);
 range(ImageTypes::IMAGEID_Defensive_Matrix_Front_Small,ImageTypes::IMAGEID_Irradiate_Large);
 range(ImageTypes::IMAGEID_Restoration_Hit_Small,ImageTypes::IMAGEID_Restoration_Hit_Large);
 range(ImageTypes::IMAGEID_Optical_Flare_Hit_Small,ImageTypes::IMAGEID_Optical_Flare_Hit_Large);
 image(unsigned(ImageTypes::IMAGEID_Nuke_Hit));image(unsigned(ImageTypes::IMAGEID_Nuclear_Missile_Death));
 }
 const auto& data=g.iscript.program_data;
 for(size_t q=0;q<queue.size();++q){
  const unsigned im=queue[q];const auto script=g.iscript.scripts.find(g.image_types.vec.at(im).iscript_id);if(script==g.iscript.scripts.end())continue;
  std::vector<size_t> pcs(script->second.animation_pc.begin(),script->second.animation_pc.end());std::set<size_t> visited;
  for(size_t n=0;n<pcs.size();++n){
   const size_t pc=pcs[n];if(!pc || pc>=data.size() || !visited.insert(pc).second)continue;
   const int op=data[pc]-0x808091;if(op<0 || op>68)throw std::runtime_error("Invalid decoded iscript PC");
   size_t next=pc+1;while(next<data.size() && data[next]<0x808091)++next;
   auto arg=[&](unsigned i){if(pc+1+i>=next)throw std::runtime_error("Invalid decoded iscript argument");return data[pc+1+i];};
   if(op==opc_imgol || op==opc_imgul || op==opc_imgolorig || op==opc_switchul || op==opc_imgoluselo || op==opc_imguluselo)image(arg(0));
   if(op==opc_sprol || op==opc_highsprol || op==opc_lowsprul || op==opc_uflunstable || op==opc_spruluselo || op==opc_sprul || op==opc_sproluselo || op==opc_grdsprol)sprite(arg(0));
   if(op==opc_imgulnextid)image(im+1);
   if(op==opc_useweapon)weapon(&st.game->weapon_types.vec.at(arg(0)));
   if(op==opc_playsnd)sounds.insert(arg(0));
   if(op==opc_playsndrand || op==opc_attackmelee)for(int k=0;k<arg(0);++k)sounds.insert(arg(k+1));
   if(op==opc_playsndbtwn)for(int k=arg(0);k<=arg(1);++k)sounds.insert(k);
   if(op==opc_goto || op==opc_call)pcs.push_back(arg(0));
   if(op==opc_randcondjmp || op==opc_pwrupcondjmp || op==opc_trgtrangecondjmp || op==opc_trgtarccondjmp || op==opc_curdirectcondjmp || op==opc_liftoffcondjmp)pcs.push_back(data[next-1]);
   if(op!=opc_end && op!=opc_return && op!=opc_goto && next<data.size())pcs.push_back(next);
  }
 }
}
