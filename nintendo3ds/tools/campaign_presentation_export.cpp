// Export only privately owned presentation resources; no game data is embedded.
#include "original_archive.h"
#include <fstream>
#include <sys/stat.h>
int main(int argc,char**argv){if(argc!=4)return 2;try{
 auto files=data_loading::data_files_directory(argv[1]);
 data_loading::mpq_file<DiscReader> sc(a_string(argv[1])+"/StarCraft.mpq"),bw(a_string(argv[1])+"/BroodWar.mpq");
 mkdir(argv[3],0777);std::ifstream list(argv[2]);std::string name;unsigned count=0;
 while(std::getline(list,name)){if(name.empty())continue;a_vector<uint8_t> bytes;bool found=false;
  for(auto& archive:files.mpqs)if(extract(archive,name.c_str(),bytes)){found=true;break;}
  if(!found)found=extract(bw,name.c_str(),bytes);if(!found)found=extract(sc,name.c_str(),bytes);if(!found)continue;
  auto flat=name;for(auto& c:flat)if(c=='/' || c=='\\')c='_';std::ofstream out(std::string(argv[3])+"/"+flat,std::ios::binary);out.write((char*)bytes.data(),bytes.size());if(!out)throw std::runtime_error("Asset write failed");++count;
 }
 a_vector<uint8_t> dat;files(dat,"arr/units.dat");auto types=data_loading::load_units_dat(dat);
 std::ofstream map(std::string(argv[3])+"/unit-portraits.txt");for(const auto& u:types.vec)map<<u.portrait<<'\n';
 a_vector<uint8_t> ports,tbl;files(ports,"arr/portdata.dat");files(tbl,"arr/portdata.tbl");if(ports.size()!=1320)throw std::runtime_error("Portrait DAT bounds");
 const unsigned strings=tbl[0]|tbl[1]<<8;
 for(unsigned id=0;id<110;++id)for(unsigned talking=0;talking<2;++talking){
  const unsigned entry=data_loading::value_at<uint32_t,true>(ports.data()+(talking*110+id)*4);if(!entry || entry>strings)continue;
  const unsigned off=tbl[2+(entry-1)*2]|tbl[3+(entry-1)*2]<<8;unsigned end=off;while(end<tbl.size() && tbl[end])++end;if(end>=tbl.size())throw std::runtime_error("Portrait TBL bounds");
  a_string name="portrait/"+a_string((char*)tbl.data()+off,end-off)+"0.smk";for(auto& c:name)if(c=='\\')c='/';
  a_vector<uint8_t> bytes;bool found=false;for(auto& archive:files.mpqs)if(extract(archive,name,bytes)){found=true;break;}if(!found)found=extract(bw,name,bytes);if(!found)found=extract(sc,name,bytes);if(!found)continue;
  std::ofstream out(std::string(argv[3])+"/portrait-"+std::to_string(id)+(talking?"-talk.smk":"-idle.smk"),std::ios::binary);out.write((char*)bytes.data(),bytes.size());if(!out)throw std::runtime_error("Portrait write failed");
 }
 printf("Exported %u private presentation assets, idle/talking portraits and unit portrait map\n",count);
}catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
