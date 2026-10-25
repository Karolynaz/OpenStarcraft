// Engine HUD helpers, included with Impl complete.
unsigned Engine::select_area(int x,int y,int width,int height){
 savegame::Writer::Guard record(impl_->journal,{frame(),savegame::rectangle,x,y,width,height});impl_->selected.clear();if(!ready())return 0;
 auto& f=impl_->player->funcs();unit_t* building=nullptr;
 for(auto* u:ptr(impl_->player->st().visible_units)){
  if(u->owner || f.unit_dead(u) || f.us_hidden(u) || f.ut_turret(u) || !player_sees(f,u))continue;
  const auto p=u->sprite->position;if(p.x<x || p.y<y || p.x>x+width || p.y>y+height)continue;
  if(f.ut_building(u)){if(!building)building=u;continue;}
  if(impl_->selected.size()<192)impl_->selected.push_back(f.get_unit_id_32(u));
 }
 if(impl_->selected.empty() && building)impl_->selected.push_back(f.get_unit_id_32(building));return impl_->selected.size();
}
void Engine::movement_options(MenuOption* out)const{
 if(!ready())return;const auto& f=impl_->player->funcs();const Orders orders[]={Orders::Move,Orders::Patrol,Orders::Stop,Orders::AttackMove};
 const char* names[]={"Move / action","Patrol","Stop","Attack move"};
 for(unsigned i=0;i<4;++i){out[i]={};std::snprintf(out[i].label,sizeof out[i].label,"%s",names[i]);out[i].code=option_code(OptionKind::order,unsigned(orders[i]));out[i].target=i!=2;out[i].icon=unsigned(f.get_order_type(orders[i])->highlight);out[i].enabled=false;
  for(auto id:impl_->selected)if(auto* u=f.get_unit(id))if(!u->owner && !f.unit_dead(u) && !f.ut_building(u))out[i].enabled=true;
 }
}
unsigned Engine::abilities(MenuOption* out,unsigned cap)const{
 if(!ready())return 0;auto saved=impl_->selected;struct Restore{Impl* p;static_vector<unit_id_32,192> saved;~Restore(){p->selected=saved;}} restore{impl_.get(),saved};bool visited[228]{};unsigned n=0;
 for(auto uid:saved){auto* u=impl_->player->funcs().get_unit(uid);if(!u || u->owner || impl_->player->funcs().unit_dead(u))continue;unsigned type=unsigned(u->unit_type->id);if(type>=228 || visited[type])continue;visited[type]=true;
  impl_->selected.clear();for(auto candidate:saved)if(auto* v=impl_->player->funcs().get_unit(candidate))if(v->unit_type==u->unit_type)impl_->selected.push_back(candidate);
  MenuOption rows[32];unsigned count=options(OptionPage::primary,rows,32);
  for(unsigned i=0;i<count;++i){auto kind=OptionKind(rows[i].code>>16);if(kind==OptionKind::build || kind==OptionKind::research || kind==OptionKind::upgrade || kind==OptionKind::addon || kind==OptionKind::larvae)continue;
   unsigned j=0;for(;j<n;++j)if(out[j].code==rows[i].code){out[j].enabled|=rows[i].enabled;break;}if(j==n && n<cap)out[n++]=rows[i];
  }
 }
 impl_->selected=saved;
 auto priority=[](const MenuOption& o){auto k=OptionKind(o.code>>16);if(k==OptionKind::stim)return 0;if(k==OptionKind::siege)return 1;if(o.code==option_code(OptionKind::order,unsigned(Orders::NukePaint)))return 2;if(k==OptionKind::cloak)return 4;return 3;};
 std::stable_sort(out,out+n,[&](const MenuOption&a,const MenuOption&b){return priority(a)<priority(b);});return n;
}
SpriteView Engine::portrait()const{
 if(!ready())return {};const auto* u=impl_->first_selected();if(!u || u->unit_type->portrait<0 || u->unit_type->portrait>=110)return {};
 return {0,0,65384,unsigned(u->unit_type->portrait),11,false,false,0,255};
}
