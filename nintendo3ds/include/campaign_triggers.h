// Numeric score/death storage slots remain valid without an active participant.
auto campaign_counter_players(int owner,int group)const {
 bwgame::static_vector<int,12> players;
 if(group>=0 && group<12)players.push_back(group);
 else if(group==13)players.push_back(owner);
 else for(int p:trigger_players(owner,group))players.push_back(p);
 return players;
}
// Included inside SoundFunctions. Extra original CHK semantics used by the six
// audited opening missions; display-only operations stay in the presentation.
bool test_trigger_condition(const trigger::condition& c,int owner)const override{
 if(!campaign.active)return state_functions::test_trigger_condition(c,owner);
 if(c.type==1)return trigger_count_comparison(c,campaign.countdown());
 if(c.type==23)return false;
 if(c.type==6 || c.type==7 || c.type==16 || c.type==17){if(c.unit_id<0 || c.unit_id>232)throw std::runtime_error("Campaign extrema unit bounds");if(c.unit_id==228)return true;auto extrema=[&](const auto& counts){const int own=trigger_command_count(counts,owner,owner,c.unit_id,true);for(unsigned p=0;p<8;++p)if(p!=unsigned(owner) && player_slot_active(p)){const int other=trigger_command_count(counts,owner,p,c.unit_id,true);if((c.type<16 && other>own) || (c.type>=16 && other<own))return false;}return true;};if(c.type==7 || c.type==17){if(c.location<=0 || unsigned(c.location)>st.locations.size())throw std::runtime_error("Campaign extrema location bounds");return extrema(trigger_bring_count(st.locations[c.location-1]));}return extrema(st);}
 if(c.type==11){if(c.extra_n<0 || unsigned(c.extra_n)>=campaign.switches.size())return false;return c.num_n==2?campaign.switches[c.extra_n]:c.num_n==3?!campaign.switches[c.extra_n]:false;}
 if(c.type==5 || c.type==15 || c.type==21){unsigned total=0;for(int p:campaign_counter_players(owner,c.group)){
  if(c.type==5 || c.type==15){const auto& counts=c.type==5?campaign.kills:campaign.deaths;if(c.unit_id>=0 && c.unit_id<228)total+=counts[p][c.unit_id];else for(unsigned id=0;id<228;++id){auto* t=get_unit_type(UnitTypes(id));if(c.unit_id==229 || (c.unit_id==230 && (t->group_flags&GroupFlags::Men)) || (c.unit_id==231 && (t->group_flags&GroupFlags::Building)) || (c.unit_id==232 && (t->group_flags&GroupFlags::Factory)))total+=counts[p][id];}}
  else switch(c.extra_n){case 1:total+=st.unit_score[p];break;case 2:total+=st.building_score[p];break;case 3:total+=st.unit_score[p]+st.building_score[p];break;case 7:total+=campaign.scores[p];break;default:break;}
 }return trigger_count_comparison(c,total);}
 return state_functions::test_trigger_condition(c,owner);
}
void on_combat_kill(unit_t* victim,int killer_owner)override{
 if(campaign.active && killer_owner>=0 && killer_owner<12 && unsigned(victim->unit_type->id)<228)++campaign.kills[killer_owner][unsigned(victim->unit_type->id)];
}
bool on_neutral_aggression(unit_t* unit,unit_t* target)override{
 if(!campaign.active || unit->owner>=8)return false;
 // The bounded campaign adapter owns AI dispatch, including provoked neutral
 // participants. Native targeting, damage and acquisition still do the fighting.
 st.players[unit->owner].controller=player_t::controller_occupied;
 set_unit_order(unit,unit->unit_type->human_ai_idle);
 action_state state;action_functions commands(st,state);commands.action_select(unit->owner,unit);
 commands.action_order(unit->owner,get_order_type(Orders::AttackDefault),target->sprite->position,target,nullptr,false);
 return true;
}
void on_kill_unit(unit_t* u)override{
 if(!campaign.active || campaign.suppress_deaths || ut_turret(u) || u->owner>=12 || unsigned(u->unit_type->id)>=228 || u->index>=1700)return;
 unsigned id=get_unit_id_32(u).raw_value;if(campaign.death_ids[u->index]==id)return;campaign.death_ids[u->index]=id;++campaign.deaths[u->owner][unsigned(u->unit_type->id)];
}
bool campaign_matches(const unit_t* u,int type)const{
 if(ut_turret(u) || unit_dying(u))return false;
 if(type==229)return true;if(type==230)return (u->unit_type->group_flags&GroupFlags::Men)!=0;if(type==231)return ut_building(u);if(type==232)return (u->unit_type->group_flags&GroupFlags::Factory)!=0;return int(u->unit_type->id)==type;
}
bool campaign_action(execute_trigger_struct& ets,int owner,running_trigger& rt,running_trigger::action& ra,const trigger::action& a,bool& handled){
 handled=true;
 auto arithmetic=[](int& value,unsigned amount,unsigned op){if(op==7)value=int(amount);else if(op==8)value+=int(amount);else if(op==9)value=std::max(0,value-int(amount));};
 auto location=[&](unsigned id)->const bwgame::location&{if(!id || id>st.locations.size())throw std::runtime_error("Campaign action location bounds");return st.locations[id-1];};
 switch(a.type){
 case 46:{
  // Use native group destinations instead of collapsing the entire scripted
  // squad onto one center. Narrow UMS rendezvous/Bring chains depend on this.
  if(a.num_n<0 || a.num_n>2){handled=false;return true;}
  const auto& src=location(a.location);const auto& dst=location(a.group2_n);
  const xy target=(dst.area.from+dst.area.to)/2;
  const auto* order=get_order_type(a.num_n==0?Orders::Move:a.num_n==1?Orders::Patrol:Orders::AttackMove);
  action_state state;action_functions commands(st,state);
  for(int p:trigger_players(owner,a.group_n)){
   static_vector<unit_t*,12> batch;
   auto send=[&](){if(batch.empty())return;commands.action_select(p,batch);commands.action_order(p,order,target,nullptr,nullptr,false);batch.clear();};
   for(auto* u:find_units(src.area)){
    if(u->owner!=p || !campaign_matches(u,a.extra_n) || !unit_is_at_elevation_flags(u,src.elevation_flags) || !u_can_move(u) || unit_is_rescuable(u) || unit_is_fighter(u) || unit_is(u,UnitTypes::Terran_Nuclear_Missile))continue;
    if(ut_building(u) && (u->order_type->id==Orders::BuildingLand || u->order_type->id==Orders::BuildingLiftoff))continue;
    if(u_burrowed(u)){unburrow_unit(u);set_queued_order(u,false,order,target);continue;}
    if(!unit_can_receive_order(u,order,p))continue;
    batch.push_back(u);if(batch.size()==12)send();
   }
   send();
  }
  return true;
 }
 case 15:case 16:{xy center=st.game->start_locations[owner];const rect* area=nullptr;if(a.type==16){const auto& l=location(a.location);center=(l.area.from+l.area.to)/2;area=&l.area;}campaign_ai.start(*this,unsigned(a.group2_n),unsigned(owner),center,area);return true;}
 case 41:if(a.string_index<=0 || unsigned(a.string_index)>st.game->map_strings.size())throw std::runtime_error("Campaign next scenario string bounds");campaign.next_scenario=unsigned(a.string_index);return true;
 case 14:campaign.timer(unsigned(a.time_n),a.num_n);return true;
 case 54:campaign.countdown_paused=true;return true;
 case 55:campaign.countdown_paused=false;return true;
 case 45:if(a.extra_n<0 || a.extra_n>=228)throw std::runtime_error("Campaign deaths type bounds");for(int p:campaign_counter_players(owner,a.group_n)){auto& value=campaign.deaths[p][a.extra_n];const unsigned amount=unsigned(a.group2_n);if(a.num_n==7)value=amount;else if(a.num_n==8)value+=amount;else if(a.num_n==9)value=value>amount?value-amount:0;}return true;
 case 13:if(a.group2_n>=campaign.switches.size())throw std::runtime_error("Campaign switch bounds");if(a.num_n==4)campaign.switches[a.group2_n]=true;else if(a.num_n==5)campaign.switches[a.group2_n]=false;else if(a.num_n==6)campaign.switches[a.group2_n]=!campaign.switches[a.group2_n];else if(a.num_n==11)campaign.switches[a.group2_n]=(lcg_rand(16)&1)!=0;return true;
 case 19:case 21:case 36:case 37:case 47:return true; // leaderboard presentation / author comment
 case 27:if(a.extra_n!=7)throw std::runtime_error("Unsupported campaign score modifier");for(int p:campaign_counter_players(owner,a.group_n))arithmetic(campaign.scores[p],a.group2_n,a.num_n);return true;
 case 28:if(owner==0){auto& l=location(a.location);campaign.ping=(l.area.from+l.area.to)/2;campaign.ping_frame=st.current_frame;}return true;
 
 case 11:{const auto& loc=location(a.location);if(a.extra_n<0 || a.extra_n>=228)throw std::runtime_error("Campaign create type bounds");
  for(int p:trigger_players(owner,a.group_n))for(unsigned i=0;i<std::max(1,a.num_n);++i){auto* u=trigger_create_unit(get_unit_type(UnitTypes(a.extra_n)),(loc.area.from+loc.area.to)/2,p);if(!u)continue;
   if((a.flags&8) && a.group2_n>0 && a.group2_n<=64){const auto& prop=campaign.properties[a.group2_n-1];auto word=[&](unsigned n){return unsigned(prop[n])|(unsigned(prop[n+1])<<8);};unsigned valid=word(2),special=word(0),bits=word(14);
    if(valid&2)set_unit_hp(u,std::max(fp8::from_raw(1),u->unit_type->hitpoints*std::min<int>(100,prop[5])/100));if(valid&4)set_unit_shield_points(u,fp8::integer(u->unit_type->shield_points)*std::min<int>(100,prop[6])/100);if(valid&8)set_unit_energy(u,unit_max_energy(u)*std::min<int>(100,prop[7])/100);
    if(valid&16)set_unit_resources(u,data_loading::value_at<uint32_t,true>(prop.data()+8));if((special&8) && (bits&8))make_unit_hallucination(u);
    // A trigger can create a Terran building already in flight, as in UNIT.
    if((special&4) && (bits&4))set_initial_building_in_transit(u);
    if(special&16)u_set_status_flag(u,unit_t::status_flag_invincible,(bits&16)!=0);if((special&2) && (bits&2)){set_unit_burrowed(u);set_unit_order(u,get_order_type(Orders::Burrowed));}if((special&1) && (bits&1))cloak_unit(u);
   }
  }return true;}
 case 22:case 23:case 24:case 25:case 39:case 43:case 48:case 49:case 50:case 51:case 52:{a_vector<unit_t*> targets;auto keep=[&](unit_t* u){return campaign_matches(u,a.extra_n) && trigger_players_pred(owner,a.group_n,u->owner);};if(a.type==22 || a.type==24){for(auto* u:ptr(st.visible_units))if(keep(u))targets.push_back(u);}else{const auto& loc=location(a.location);for(auto* u:find_units(loc.area))if(keep(u) && unit_is_at_elevation_flags(u,loc.elevation_flags))targets.push_back(u);}
  unsigned remaining=a.num_n?a.num_n:1700;
  for(auto* u:targets){if((a.type==23 || a.type==25 || a.type==39 || a.type==48 || a.type==49 || a.type==50 || a.type==51) && !remaining--)break;
   if(a.type==22 || a.type==23)kill_unit(u);
   else if(a.type==24 || a.type==25){bool old=campaign.suppress_deaths;campaign.suppress_deaths=true;hide_unit(u);kill_unit(u);campaign.suppress_deaths=old;}
   else if(a.type==43)u_set_status_flag(u,unit_t::status_flag_invincible,a.num_n==4?true:a.num_n==5?false:!u_invincible(u));
   else if(a.type==48){unsigned destination=a.group2_n==13?unsigned(owner):a.group2_n;if(destination>=12)throw std::runtime_error("Campaign give destination bounds");trigger_give_unit_to(u,destination);}
   else if(a.type==39){const auto& dst=location(a.group2_n);xy pos=(dst.area.from+dst.area.to)/2;auto place=find_unit_placement(u,pos,{pos-xy(256,256),pos+xy(256,256)},true);if(place.first)move_unit(u,place.second);}
   else if(a.type==49)set_unit_hp(u,std::max(fp8::from_raw(1),u->unit_type->hitpoints*std::max(0,std::min(100,a.group2_n))/100));
   else if(a.type==50)set_unit_energy(u,unit_max_energy(u)*std::max(0,std::min(100,a.group2_n))/100);
   else if(a.type==51)set_unit_shield_points(u,fp8::integer(u->unit_type->shield_points)*std::max(0,std::min(100,a.group2_n))/100);
   else if(a.type==52)set_unit_resources(u,std::max(0,a.group2_n));
  }return true;}
 case 42:{const auto& loc=location(a.location);a_vector<unit_t*> targets;for(auto* u:find_units(loc.area))if(campaign_matches(u,a.extra_n) && ((a.group_n==17 && (unit_is_door(u) || unit_is_trap(u))) || trigger_players_pred(owner,a.group_n,u->owner)) && unit_is_at_elevation_flags(u,loc.elevation_flags))targets.push_back(u);for(auto* u:targets){if(a.num_n!=0 && a.num_n!=4 && a.num_n!=5 && a.num_n!=6)throw std::runtime_error("Campaign doodad modifier");const bool disabled=u_disabled(u) || u->order_type->id==Orders::OpenDoor;set_doodad_state(u,a.num_n==4 || ((a.num_n==0 || a.num_n==6) && disabled));}return true;}
 case 57:for(int p:trigger_players(owner,a.group_n))st.alliances[owner][p]=a.extra_n==0?0:2;return true;
 default:handled=false;return true;
 }
}
