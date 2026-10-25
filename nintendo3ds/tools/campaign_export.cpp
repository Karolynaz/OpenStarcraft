#include "original_archive.h"
#include "campaign_map.h"
#include <fstream>
#include <sys/stat.h>
int main(int argc,char**argv){if(argc!=3){puts("campaign-export OWNED_DATA OUTPUT_DIRECTORY");return 2;}try{
 data_loading::mpq_file<DiscReader> disc(a_string(argv[1])+"/StarCraft.mpq");a_vector<uint8_t> chk;
 if(!extract(disc,"campaign/terran/terran01/staredit/scenario.chk",chk))throw std::runtime_error("Owned Terran mission 1 not found");
 port3ds::campaign::normalize(chk);mkdir(argv[2],0777);const std::string base=std::string(argv[2])+"/terran01";
 std::ofstream map(base+".chk",std::ios::binary);map.write((char*)chk.data(),chk.size());if(!map)throw std::runtime_error("Campaign map write failed");
 // Briefing paragraphs remain private assets, separated into readable pages.
 std::vector<std::string> strings;std::string brief;const uint8_t* briefing=nullptr;unsigned briefing_size=0;
 for(size_t at=0;at+8<=chk.size();){const auto* p=chk.data()+at;unsigned n=port3ds::campaign::get(p+4);at+=8;if(n>chk.size()-at)throw std::runtime_error("Chunk bounds");
  if(!memcmp(p,"STR ",4)){auto* s=chk.data()+at;unsigned count=s[0]|s[1]<<8;strings.resize(count+1);for(unsigned i=0;i<count;++i){unsigned off=s[2+i*2]|s[3+i*2]<<8;if(off<n){unsigned end=off;while(end<n && s[end])++end;strings[i+1]=std::string((char*)s+off,end-off);}}}
  if(!memcmp(p,"MBRF",4)){briefing=chk.data()+at;briefing_size=n;}at+=n;
 }
 if(briefing)for(unsigned j=0;j<briefing_size;j+=2400)for(unsigned a=0;a<64;++a){const auto* p=briefing+j+320+a*32;unsigned type=p[26],id=port3ds::campaign::get(p+4);if((type==8 || type==4) && id<strings.size() && !strings[id].empty()){if(!brief.empty())brief+='\f';brief+=strings[id];}}
 std::ofstream text(base+".brief",std::ios::binary);text<<brief;if(!text)throw std::runtime_error("Briefing write failed");printf("Private Wasteland map %zu bytes, briefing %zu bytes\n",chk.size(),brief.size());
}catch(const std::exception&e){fprintf(stderr,"%s\n",e.what());return 1;}}
