#include "original_archive.h"
#include <fstream>
#include <sys/stat.h>
int main(int argc,char**argv){if(argc!=3)return 2;try{
 auto files=data_loading::data_files_directory(argv[1]);a_vector<uint8_t> dat,tbl,bytes;files(dat,"arr/portdata.dat");files(tbl,"arr/portdata.tbl");if(dat.size()!=1320)throw std::runtime_error("Portrait DAT bounds");
 mkdir(argv[2],0777);unsigned count=tbl[0]|tbl[1]<<8;
 for(unsigned id=0;id<110;++id){unsigned entry=data_loading::value_at<uint32_t,true>(dat.data()+id*4);if(!entry || entry>count)continue;unsigned off=tbl[2+(entry-1)*2]|tbl[3+(entry-1)*2]<<8;if(off>=tbl.size())throw std::runtime_error("Portrait TBL offset");unsigned end=off;while(end<tbl.size() && tbl[end])++end;a_string name="portrait/"+a_string((char*)tbl.data()+off,end-off)+"0.smk";for(auto& c:name)if(c=='\\')c='/';
  bool found=false;for(auto& archive:files.mpqs)if(extract(archive,name,bytes)){found=true;break;}if(!found)continue;
  std::ofstream out(std::string(argv[2])+"/"+std::to_string(id)+".smk",std::ios::binary);out.write((char*)bytes.data(),bytes.size());if(!out)throw std::runtime_error("Portrait write failed");printf("%u %s\n",id,name.c_str());
 }
}catch(const std::exception&e){fprintf(stderr,"%s\n",e.what());return 1;}}
