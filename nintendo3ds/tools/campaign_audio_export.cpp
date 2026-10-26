#include "original_archive.h"
#include "bwgame.h"

#include "sound_ids.h"
#include "campaign_catalogue.h"
#include <fstream>
#include <set>
#include <sys/stat.h>
int main(int argc,char**argv){if(argc!=3 && argc!=4)return 2;try{
 unsigned mission=0;if(argc==4){while(mission<port3ds::campaign::count && std::string(argv[3])!=port3ds::campaign::missions[mission].key)++mission;}if(mission>=port3ds::campaign::count)throw std::runtime_error("Unknown campaign opening");const auto& m=port3ds::campaign::missions[mission];
 const char* race=m.race==0?"zerg":m.race==2?"protoss":"terran";const a_string prefix=a_string("campaign/")+(m.brood?"exp":"")+race+"/"+a_string(m.key+(m.brood?3:0))+"/";
 data_loading::mpq_file<DiscReader> disc(a_string(argv[1])+(m.brood?"/BroodWar.mpq":"/StarCraft.mpq"));a_vector<uint8_t> chk;if(!extract(disc,prefix+"staredit/scenario.chk",chk))throw std::runtime_error("Missing owned mission");
 std::set<unsigned> roster;for(size_t at=0;at+8<=chk.size();){unsigned n=data_loading::value_at<uint32_t,true>(chk.data()+at+4);const auto* tag=chk.data()+at;at+=8;if(n>chk.size()-at)throw std::runtime_error("CHK bounds");const auto* p=chk.data()+at;if(!memcmp(tag,"UNIT",4))for(unsigned j=0;j+36<=n;j+=36)roster.insert(p[j+8]|p[j+9]<<8);if(!memcmp(tag,"TRIG",4))for(unsigned j=0;j+2400<=n;j+=2400)for(unsigned k=0;k<64;++k){auto* q=p+j+320+k*32;if(q[26]==11 || q[26]==44)roster.insert(q[24]|q[25]<<8);}at+=n;}
 mkdir(argv[2],0777);std::string root=std::string(argv[2])+"/";std::ofstream manifest(root+"manifest.tsv");
 auto assets=data_loading::data_files_directory(argv[1]);a_vector<uint8_t> b;assets(b,"arr/units.dat");auto units=data_loading::load_units_dat(b);assets(b,"arr/sfxdata.dat");auto sounds=data_loading::load_sfxdata_dat(b);string_table_data names;assets(names.data,"arr/sfxdata.tbl");
 auto write=[&](unsigned id,unsigned priority,unsigned flags,const a_string& name,const a_vector<uint8_t>& bytes){std::ofstream f(root+std::to_string(id)+".wav",std::ios::binary);f.write((char*)bytes.data(),bytes.size());if(!f)throw std::runtime_error("Campaign WAV write failed");manifest<<id<<'\t'<<priority<<'\t'<<flags<<'\t'<<name.c_str()<<'\n';};
 std::set<unsigned> ids;
 for(unsigned id:roster){const auto& u=units.vec.at(id);
  for(auto range:{std::make_pair(u.first_what_sound,u.last_what_sound),std::make_pair(u.first_yes_sound,u.last_yes_sound),std::make_pair(u.first_pissed_sound,u.last_pissed_sound)})for(unsigned n=range.first;n && n<=range.second;++n)ids.insert(n);if(u.ready_sound)ids.insert(u.ready_sound);
 }
 for(auto id:ids){const auto& s=sounds.vec.at(id);const auto name="sound/"+names[s.filename_index];assets(b,name);write(id,s.priority,s.flags,name,b);}

 std::vector<std::string> strings;std::set<unsigned> speech;
 auto get=[](const uint8_t* p){return data_loading::value_at<uint32_t,true>(p);};
 for(size_t at=0;at+8<=chk.size();){const auto* tag=chk.data()+at;unsigned n=get(tag+4);at+=8;if(n>chk.size()-at)throw std::runtime_error("CHK bounds");const auto* p=chk.data()+at;
  if(!memcmp(tag,"STR ",4)){unsigned count=p[0]|p[1]<<8;if(2+2*count>n)throw std::runtime_error("STR bounds");strings.resize(count+1);for(unsigned i=0;i<count;++i){unsigned off=p[2+i*2]|p[3+i*2]<<8;if(off<n){unsigned end=off;while(end<n && p[end])++end;strings[i+1]=std::string((char*)p+off,end-off);}}}
  if(!memcmp(tag,"TRIG",4) || !memcmp(tag,"MBRF",4)){const bool briefing=!memcmp(tag,"MBRF",4);if(n%2400)throw std::runtime_error("TRIG bounds");for(unsigned j=0;j<n;j+=2400)for(unsigned a=0;a<64;++a){const auto* r=p+j+320+a*32;if((r[26]==8 || (!briefing && r[26]==7)) && !(r[28]&2) && get(r+8))speech.insert(get(r+8));}}
  at+=n;
 }
 for(auto id:speech){if(id>=strings.size() || port3ds::campaign_sound_base+id>=port3ds::audio_sound_limit)throw std::runtime_error("Speech ID bounds");a_string name=strings[id];for(auto& c:name)if(c=='\\')c='/';const auto nested=prefix+name;if(!extract(disc,nested,b))throw std::runtime_error("Missing owned mission speech: "+std::string(nested.c_str()));write(port3ds::campaign_sound_base+id,200,16,nested,b);}
 if(!manifest)throw std::runtime_error("Manifest write failed");printf("Exported %zu hero responses and %zu mission/briefing WAVs\n",ids.size(),speech.size());
}catch(const std::exception&e){fprintf(stderr,"%s\n",e.what());return 1;}}
