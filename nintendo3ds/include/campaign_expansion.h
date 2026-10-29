// Included inside CampaignAI. Bounded independent resource-site enumeration:
// start locations first, then connected resource clusters in y/x order. These
// adapter IDs are deterministic; retail's unpublished area numbering is not
// claimed byte-identical. No map-specific coordinate overrides or free depots.
struct ResourceSite {xy center{};};
struct Expansion {bool active=false,brood=false;unsigned owner=0,site=0,pc=0,search=0;xy position{};bool located=false;unit_id_32 worker{},depot{};};
std::array<ResourceSite,250> sites_{};unsigned site_count_=0;bool sites_ready_=false;
std::array<Expansion,32> expansions_{};
struct TownPoint {bool active=false;unsigned owner=0;xy center{};};std::array<TownPoint,32> towns_{};
void register_town(unsigned owner,xy center){for(const auto& t:towns_)if(t.active && t.owner==owner && distance2(t.center,center)<256*256)return;for(auto& t:towns_)if(!t.active){t={true,owner,center};return;}throw std::runtime_error("Campaign town capacity");}
unsigned town_count(state_functions& f,unsigned owner){unsigned count=0;for(auto& t:towns_)if(t.active && t.owner==owner){bool alive=false;for(auto* u:ptr(f.st.player_units[owner]))if(!f.unit_dying(u) && (f.ut_worker(u) || f.ut_building(u)) && distance2(u->sprite->position,t.center)<600*600){alive=true;break;}if(alive)++count;else t.active=false;}return count;}

static int distance2(xy a,xy b){auto d=a-b;return d.x*d.x+d.y*d.y;}
void init_sites(state_functions& f){
 if(sites_ready_)return;sites_ready_=true;
 a_vector<xy> resources;for(auto* u:ptr(f.st.visible_units))if(!f.unit_dying(u) && f.ut_resource(u) && u->building.resource.resource_count>0)resources.push_back(u->sprite->position);
 std::sort(resources.begin(),resources.end(),[](xy a,xy b){return a.y!=b.y?a.y<b.y:a.x<b.x;});
 a_vector<unsigned> group(resources.size());for(unsigned i=0;i<group.size();++i)group[i]=i;
 auto root=[&](unsigned i){while(group[i]!=i){group[i]=group[group[i]];i=group[i];}return i;};
 for(unsigned i=0;i<resources.size();++i)for(unsigned j=i+1;j<resources.size();++j){if(resources[j].y-resources[i].y>384)break;if(distance2(resources[i],resources[j])<=384*384){auto a=root(i),b=root(j);if(a!=b)group[b]=a;}}
 a_vector<xy> centers;for(unsigned i=0;i<resources.size();++i)if(root(i)==i){xy sum{};unsigned n=0;for(unsigned j=0;j<resources.size();++j)if(root(j)==i){sum+=resources[j];++n;}centers.push_back(sum/int(n));}
 std::sort(centers.begin(),centers.end(),[](xy a,xy b){return a.y!=b.y?a.y<b.y:a.x<b.x;});
 a_vector<bool> used(centers.size());
 // Index zero is the sentinel. CHK start-location order survives human-owner normalization.
 static_vector<xy,12> starts;if(f.st.game->start_location_count){for(unsigned n=0;n<f.st.game->start_location_count;++n)starts.push_back(f.st.game->start_locations[f.st.game->start_location_order[n]]);}else for(auto pos:f.st.game->start_locations)if(pos!=xy{})starts.push_back(pos);
 for(auto pos:starts)if(pos!=xy{}){
  bool duplicate=false;for(unsigned n=1;n<=site_count_;++n)if(distance2(pos,sites_[n].center)<64*64)duplicate=true;if(duplicate)continue;
  int nearest=768*768;unsigned at=~0u;for(unsigned n=0;n<centers.size();++n)if(!used[n] && distance2(pos,centers[n])<nearest){nearest=distance2(pos,centers[n]);at=n;}
  if(at!=~0u)used[at]=true;if(site_count_+1>=sites_.size())throw std::runtime_error("Campaign resource site capacity");sites_[++site_count_].center=pos;
 }
 for(unsigned n=0;n<centers.size();++n)if(!used[n]){if(site_count_+1>=sites_.size())throw std::runtime_error("Campaign resource site capacity");sites_[++site_count_].center=centers[n];}
}
unsigned nearest_site(xy pos)const{unsigned result=0;int best=768*768;for(unsigned i=1;i<=site_count_;++i){int d=distance2(pos,sites_[i].center);if(d<best){best=d;result=i;}}return result;}
bool site_resources(state_functions& f,unsigned site)const{for(auto* u:ptr(f.st.visible_units))if(!f.unit_dying(u) && f.ut_resource(u) && u->building.resource.resource_count>0 && nearest_site(u->sprite->position)==site)return true;return false;}
bool expansion_worker(unit_id_32 id)const{for(const auto& e:expansions_)if(e.active && e.worker==id)return true;return false;}
void request_expansion(state_functions& f,Thread& t,unsigned site,unsigned pc){
 init_sites(f);if(!site || site>site_count_ || !site_resources(f,site))return;
 for(const auto& e:expansions_)if(e.active && e.site==site)return;
 for(auto* u:ptr(f.st.visible_units))if(!f.unit_dying(u) && f.ut_resource_depot(u) && distance2(u->sprite->position,sites_[site].center)<512*512)return;
 for(auto& e:expansions_)if(!e.active){e={};e.active=true;e.owner=t.owner;e.site=site;e.pc=pc;e.brood=t.brood;return;}
 throw std::runtime_error("Campaign expansion capacity");
}
void request_nearest_expansion(state_functions& f,Thread& t,unsigned quantity,unsigned pc){
 init_sites(f);unsigned pending=0;for(const auto& e:expansions_)if(e.active && e.owner==t.owner)++pending;if(town_count(f,t.owner)+pending>=quantity)return;
 unsigned best=0;int distance=0x7fffffff;for(unsigned site=1;site<=site_count_;++site){if(!site_resources(f,site))continue;bool taken=false;for(const auto& e:expansions_)if(e.active && e.site==site)taken=true;for(auto* u:ptr(f.st.visible_units))if(!f.unit_dying(u) && f.ut_resource_depot(u) && distance2(u->sprite->position,sites_[site].center)<512*512)taken=true;if(taken)continue;
  bool reachable=false;for(auto* w:ptr(f.st.player_units[t.owner]))if(f.ut_worker(w) && f.u_completed(w) && !f.unit_dying(w) && !f.us_hidden(w) && f.is_reachable(w,sites_[site].center)){reachable=true;break;}if(!reachable)continue;int d=distance2(t.center,sites_[site].center);if(d<distance){best=site;distance=d;}
 }if(best)request_expansion(f,t,best,pc);
}
void update_expansions(state_functions& f){
 if(f.st.current_frame%24)return;action_state state;action_functions a(f.st,state);
 for(auto& e:expansions_)if(e.active){
  auto center=sites_[e.site].center;auto race=f.st.players[e.owner].race;
  auto type=f.get_unit_type(race==race_t::zerg?UnitTypes::Zerg_Hatchery:race==race_t::protoss?UnitTypes::Protoss_Nexus:UnitTypes::Terran_Command_Center);
  unit_t* depot=f.get_unit(e.depot);
  if(!depot || f.unit_dying(depot) || depot->owner!=e.owner){e.depot={};depot=nullptr;for(auto* u:ptr(f.st.player_units[e.owner]))if(!f.unit_dying(u) && f.ut_resource_depot(u) && distance2(u->sprite->position,center)<512*512){depot=u;e.depot=f.get_unit_id_32(u);break;}}
  if(depot && f.u_completed(depot)){
   center=depot->sprite->position;rect area{{std::max(0,center.x-600),std::max(0,center.y-600)},{std::min(int(f.st.game->map_width),center.x+600),std::min(int(f.st.game->map_height),center.y+600)}};
   launch(e.pc,e.owner,e.brood,center,f.st.current_frame,area);e.active=false;continue;
  }
  auto* worker=f.get_unit(e.worker);if(!worker || f.unit_dying(worker) || worker->owner!=e.owner || !f.ut_worker(worker) || f.us_hidden(worker)){e.worker={};worker=nullptr;}
  if(depot && race!=race_t::terran)continue;
  if(!depot && !site_resources(f,e.site)){e.active=false;continue;}
  if(!worker && !depot && !f.has_available_resources_for(e.owner,type))continue;
  if(!worker){int nearest=0x7fffffff;for(auto* u:ptr(f.st.player_units[e.owner]))if(f.ut_worker(u) && f.u_completed(u) && !f.unit_dying(u) && !f.us_hidden(u) && !f.unit_is_disabled(u) && !expansion_worker(f.get_unit_id_32(u)) && f.get_unit_id_32(u).raw_value!=sides_[e.owner].builder.raw_value && f.is_reachable(u,center)){
    int d=distance2(u->sprite->position,center);if(d<nearest){nearest=d;worker=u;}
   }if(worker)e.worker=f.get_unit_id_32(worker);else continue;
  }
  if(depot){if(worker->order_type->id!=Orders::ConstructingBuilding){a.action_select(e.owner,worker);a.action_default_order(e.owner,depot->sprite->position,depot,nullptr,false);}continue;}
  if(!e.located){for(unsigned n=0;n<24;++n){if(e.search>=640)e.search=0;xy pos=center+spiral(e.search++)*32;auto tile=(pos-type->placement_size/2)/32;pos=tile*32+type->placement_size/2;
    if(pos.x<type->placement_size.x/2 || pos.y<type->placement_size.y/2 || pos.x>=int(f.st.game->map_width) || pos.y>=int(f.st.game->map_height))continue;
    if(f.can_place_building(worker,e.owner,type,pos,true,false)){e.position=pos;e.located=true;break;}
   }if(!e.located)continue;
  }
  if(distance2(worker->sprite->position,e.position)>160*160){if(f.st.current_frame%96==0 || idle(worker)){a.action_select(e.owner,worker);a.action_order(e.owner,a.get_order_type(Orders::Move),e.position,nullptr,nullptr,false);}continue;}
  if(worker->order_type->id==Orders::PlaceBuilding || worker->order_type->id==Orders::PlaceProtossBuilding || worker->order_type->id==Orders::DroneStartBuild || worker->order_type->id==Orders::DroneLand)continue;
  if(!f.has_available_resources_for(e.owner,type))continue;
  if(!f.can_place_building(worker,e.owner,type,e.position,true,true)){e.located=false;continue;}
  a.action_select(e.owner,worker);auto order=race==race_t::zerg?Orders::DroneStartBuild:race==race_t::protoss?Orders::PlaceProtossBuilding:Orders::PlaceBuilding;
  a.action_build(e.owner,a.get_order_type(order),type,{size_t((e.position.x-type->placement_size.x/2)/32),size_t((e.position.y-type->placement_size.y/2)/32)});
 }
}
