// Engine definitions, included inside namespace port3ds after Impl is complete.
namespace {
bool checked_save(const char* path,savegame::Header& h,char* error,size_t capacity){
 auto fail=[&](const char* s){std::snprintf(error,capacity,"%s",s);return false;};FILE* file=std::fopen(path,"rb");if(!file)return fail("No saved game in this slot");
 bool ok=false;do{
  if(std::fread(h.bytes.data(),1,h.bytes.size(),file)!=h.bytes.size() || std::memcmp(h.bytes.data(),"OB3SAVE1",8) || h.field(0)!=1){fail("Invalid save header");break;}
  // The two preceding public test builds changed presentation only. Their
  // journal opcodes, simulation and digest are identical; arbitrary builds
  // remain rejected, and load_save still checks the exact reconstructed state.
  const auto* build=reinterpret_cast<char*>(h.bytes.data()+136);
  if(std::strncmp(build,source_id,32) && std::strncmp(build,"src-2dc0b1b024abc416",32) && std::strncmp(build,"src-d8212ce708a062eb",32)){fail("Save belongs to a different build");break;}
  const auto crc=h.field(28);h.field(28,0);const auto actual=savegame::hash(2166136261u,h.bytes.data(),h.bytes.size());h.field(28,crc);
  if(crc!=actual || h.field(1)>savegame::max_frames || h.field(2)>savegame::max_events || !h.field(3) || h.field(3)>savegame::max_map || h.field(4)>1 || (h.field(6)>15 && !campaign::scenario(h.field(6)))){fail("Invalid or damaged save metadata");break;}
  const auto info=h.info();bool valid=unsigned(info.setup.difficulty)<=1 && info.setup.game_type<=2;
  for(unsigned p=0;p<3;++p)valid=valid && info.setup.races[p]>=-1 && info.setup.races[p]<=2 && info.setup.teams[p]<=2 && info.setup.side_difficulty[p]>=-1 && info.setup.side_difficulty[p]<=1;
  for(float v:{info.view.camera_x,info.view.camera_y,info.view.cursor_x,info.view.cursor_y})valid=valid && std::isfinite(v) && std::abs(v)<1000000;
  if(!valid){fail("Invalid save settings");break;}
  uint32_t digest=2166136261u;if(!savegame::copy(file,nullptr,h.field(3),&digest)){fail("Truncated saved map");break;}
  unsigned previous=0;valid=true;unsigned char bytes[savegame::record_size];
  for(unsigned n=0;n<h.field(2);++n){if(std::fread(bytes,1,sizeof bytes,file)!=sizeof bytes){valid=false;break;}digest=savegame::hash(digest,bytes,sizeof bytes);auto r=savegame::decode(bytes);if(r.frame<previous || r.frame>h.field(1) || r.op<1 || r.op>savegame::last){valid=false;break;}previous=r.frame;
   if((r.op==savegame::store || r.op==savegame::recall) && unsigned(r.a)>=4)valid=false;
   if(r.op==savegame::command && unsigned(r.a)>unsigned(Command::refinery))valid=false;
  }
  if(!valid || digest!=h.field(7) || std::fgetc(file)!=EOF || std::ferror(file)){fail("Save checksum failed or file truncated");break;}ok=true;
 }while(false);std::fclose(file);return ok;
}
}
bool Engine::inspect_save(const char* path,SaveInfo& info,char* error,size_t capacity){savegame::Header h;if(!checked_save(path,h,error,capacity))return false;info=h.info();return true;}
uint32_t Engine::state_digest()const{
 if(!ready())return 0;const auto& st=impl_->player->st();const auto& f=impl_->player->funcs();uint32_t hash=2166136261u;auto add=[&](uint32_t v){unsigned char bytes[4];savegame::put(bytes,v);hash=savegame::hash(hash,bytes,4);};
 add(st.players[0].victory_state);const auto& campaign=impl_->sound->campaign;add(campaign.active);add(campaign.cinematic);add(campaign.objective);add(campaign.message);add(campaign.message_until);add(campaign.camera_serial);add(campaign.camera.x);add(campaign.camera.y);if(campaign.active){for(auto v:st.shared_vision)add(v);for(const auto& row:st.alliances)for(auto v:row)add(v);add(campaign.mission);add(campaign.next_scenario);add(uint32_t(campaign.countdown_frames));add(uint32_t(campaign.countdown_frames>>32));add(campaign.countdown_paused);add(campaign.countdown_active);for(auto v:campaign.switches)add(v);for(auto v:campaign.scores)add(v);for(const auto& row:campaign.deaths)for(auto v:row)add(v);for(const auto& row:campaign.kills)for(auto v:row)add(v);for(auto v:campaign.death_ids)add(v);add(campaign.ping_frame);add(campaign.ping.x);add(campaign.ping.y);impl_->sound->campaign_ai.digest(add);}
 add(st.current_frame);add(st.lcg_rand_state);add(st.active_orders_size);add(st.active_bullets_size);add(st.active_thingies_size);add(impl_->fog);
 for(unsigned p=0;p<12;++p){add(st.current_minerals[p]);add(st.current_gas[p]);for(auto v:st.tech_researched[p])add(v);for(auto v:st.upgrade_levels[p])add(v);
  for(auto* u:ptr(st.player_units[p])){add(f.get_unit_id_32(u).raw_value);add(unsigned(u->unit_type->id));add(u->status_flags);add(u->kill_count);add(u->hp.raw_value);add(u->shield_points.raw_value);add(u->energy.raw_value);add(u->exact_position.x.raw_value);add(u->exact_position.y.raw_value);add(unsigned(u->order_type->id));add(u->order_state);add(u->order_signal);add(u->main_order_timer);add(u->sprite?u->sprite->elevation_level:0);for(const auto& order:u->order_queue){add(unsigned(order.order_type->id));add(order.target.position.x);add(order.target.position.y);add(order.target.unit?f.get_unit_id_32(order.target.unit).raw_value:0);}add(0xfffffffeu);add(u->order_target.pos.x);add(u->order_target.pos.y);add(u->order_target.unit?f.get_unit_id_32(u->order_target.unit).raw_value:0);add(u->remaining_build_time);add(u->carrying_flags);if(f.ut_worker(u))add(u->worker.powerup?f.get_unit_id_32(u->worker.powerup).raw_value:0);for(auto* t:u->build_queue)add(unsigned(t->id));add(0xffffffffu);}
 }
 for(const auto& tile:st.tiles){add(tile.visible);add(tile.explored);add(tile.flags);}
 for(auto id:impl_->selected)add(id.raw_value);add(0xffffffffu);for(const auto& group:impl_->groups){for(auto id:group)add(id.raw_value);add(0xffffffffu);}
 for(unsigned p=0;p<3;++p){add(impl_->ai.trained(p));add(impl_->ai.built(p));add(impl_->ai.waves(p));add(impl_->ai.target(p).x);add(impl_->ai.target(p).y);add(impl_->ai.tactics(p).casts());add(impl_->ai.tactics(p).drops());}add(impl_->ai.winner());add(impl_->battle.winner());add(impl_->battle.seconds());return hash;
}
bool Engine::start_recording(const char* path){
 if(!ready() || frame()!=0){std::snprintf(impl_->message,sizeof impl_->message,"Recording must start at frame zero");return false;}
 FILE* map=std::fopen(impl_->initial_map.c_str(),"rb");if(!map)return false;std::fseek(map,0,SEEK_END);const auto size=std::ftell(map);std::rewind(map);
 if(size<=0 || size>savegame::max_map){std::fclose(map);return false;}savegame::Header h;std::memcpy(h.bytes.data(),"OB3SAVE1",8);h.field(0,1);h.field(3,unsigned(size));h.field(4,chk_path(impl_->initial_map));std::snprintf(reinterpret_cast<char*>(h.bytes.data()+136),32,"%s",source_id);SaveInfo info;info.scenario=unsigned(impl_->initial_scenario);info.setup=impl_->initial_setup;h.setup(info);
 const bool ok=impl_->journal.open(path,h,map);std::fclose(map);return ok;
}
bool Engine::save_game(const char* path,const SaveView& view,std::uint64_t saved_at){
 auto fail=[&](const char* s){std::snprintf(impl_->message,sizeof impl_->message,"%s",s);return false;};auto& writer=impl_->journal;
 if(!ready() || !writer.flush())return fail("Save unavailable: session journal failed");
 if(writer.path==path)return fail("Save and session paths must differ");
 auto h=writer.header;h.field(1,frame());h.field(2,writer.events);h.field(5,state_digest());h.view(view);if(!saved_at){const auto now=std::time(nullptr);if(now>0)saved_at=std::uint64_t(now);}h.field(29,uint32_t(saved_at));h.field(30,uint32_t(saved_at>>32));h.field(28,0);
 const std::string temp=std::string(path)+".tmp",backup=std::string(path)+".bak";FILE* in=std::fopen(writer.path.c_str(),"rb");if(!in)return fail("Cannot read session journal");FILE* out=std::fopen(temp.c_str(),"wb+");if(!out){std::fclose(in);return fail("Cannot create save on SD");}
 bool ok=std::fwrite(h.bytes.data(),1,h.bytes.size(),out)==h.bytes.size() && std::fseek(in,savegame::header_size,SEEK_SET)==0;uint32_t checksum=2166136261u;
 if(ok)ok=savegame::copy(in,out,h.field(3)+size_t(h.field(2))*savegame::record_size,&checksum);std::fclose(in);h.field(7,checksum);h.field(28,savegame::hash(2166136261u,h.bytes.data(),h.bytes.size()));
 if(ok)ok=std::fseek(out,0,SEEK_SET)==0 && std::fwrite(h.bytes.data(),1,h.bytes.size(),out)==h.bytes.size();if(std::fclose(out))ok=false;
 if(!ok){std::remove(temp.c_str());return fail("Save write failed; previous save retained");}
 SaveInfo check;char reason[192];if(!inspect_save(temp.c_str(),check,reason,sizeof reason)){std::remove(temp.c_str());return fail(reason);}
 const bool had=exists(path);if(had){std::remove(backup.c_str());if(std::rename(path,backup.c_str())){std::remove(temp.c_str());return fail("Cannot preserve previous save");}}
 if(std::rename(temp.c_str(),path)){if(had)std::rename(backup.c_str(),path);return fail("Cannot finish save; previous save retained");}
 impl_->command_message="Game saved";return true;
}
bool Engine::load_save(const char* root,const char* path,const char* session,const char* extracted,SaveView& view,const std::function<bool(unsigned,unsigned)>& progress){
 savegame::Header h;if(!checked_save(path,h,impl_->message,sizeof impl_->message))return false;
 FILE* input=std::fopen(path,"rb");if(!input)return false;const auto map_path=std::string(extracted)+(h.field(4)?".chk":".scx");FILE* map=std::fopen(map_path.c_str(),"wb");bool ok=map && std::fseek(input,savegame::header_size,SEEK_SET)==0 && savegame::copy(input,map,h.field(3));if(map && std::fclose(map))ok=false;
 if(!ok){std::fclose(input);std::snprintf(impl_->message,sizeof impl_->message,"Cannot extract saved map");return false;}
 const auto info=h.info();if(progress && !progress(0,h.field(1))){std::fclose(input);std::snprintf(impl_->message,sizeof impl_->message,"Load cancelled");return false;}
 if(!load(root,map_path.c_str(),Scenario(info.scenario),&info.setup)){std::fclose(input);return false;}
 bool cancelled=false;unsigned char bytes[savegame::record_size];unsigned n=0;savegame::Record next{};bool have=false;
 auto read=[&](){if(n>=h.field(2)){have=false;return true;}if(std::fread(bytes,1,sizeof bytes,input)!=sizeof bytes)return false;next=savegame::decode(bytes);++n;have=true;return true;};ok=read();
 while(ok){
  while(have && next.frame==frame()){
   auto r=next;switch(r.op){case savegame::similar:select_similar(r.a,r.b,r.c,r.d);break;case savegame::rectangle:select_area(r.a,r.b,r.c,r.d);break;case savegame::select:select(r.a,r.b);break;case savegame::clear:clear_selection();break;case savegame::activate:activate(r.a,r.b);break;case savegame::army:select_army(r.a,r.b);break;case savegame::workers:select_workers(r.a,r.b);break;case savegame::store:store_group(r.a);break;case savegame::recall:recall_group(r.a);break;case savegame::command:command(Command(r.a),r.b,r.c);break;case savegame::perform:perform(unsigned(r.a),r.b,r.c);break;case savegame::move:move_selected(r.a,r.b);break;case savegame::cancel:cancel_selected_work(r.a);break;case savegame::fog:set_fog_enabled(r.a);break;case savegame::surrender:surrender();break;case savegame::elapsed:advance_wall_time(uint64_t(uint32_t(r.a))|(uint64_t(uint32_t(r.b))<<32));break;default:ok=false;break;}if(!ok || !(ok=read()))break;
  }
  if(!ok || frame()==h.field(1))break;
  if(!(ok=tick()))break;impl_->sound->count=0;impl_->sound->dropped=0;
  if(frame()%24==0 && progress && !progress(frame(),h.field(1))){cancelled=true;ok=false;break;}
 }
 std::fclose(input);if(ok)ok=!have && frame()==h.field(1) && state_digest()==h.field(5);
 if(ok)ok=impl_->journal.clone(path,session,h);
 if(!ok){impl_->loaded=false;std::snprintf(impl_->message,sizeof impl_->message,"%s",cancelled?"Load cancelled":"Save reconstruction failed or assets differ");return false;}
 impl_->sound->count=0;impl_->sound->dropped=0;view=info.view;impl_->command_message="Saved game loaded";return true;
}
