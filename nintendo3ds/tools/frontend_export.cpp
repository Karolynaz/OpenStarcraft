// Host-only extractor for privately owned frontend art. The original disc
// containers contain an embedded MPQ; uncompressed SMKs need no video decoder
// here (offline ffmpeg generates the bounded menu animation sheets).
#include "data_loading.h"
#include <fstream>
#include <sys/stat.h>
using namespace bwgame;
#include "original_archive.h"
int main(int argc,char** argv){if(argc!=3)return 2;try{
 auto files=data_loading::data_files_directory(argv[1]);data_loading::mpq_file<DiscReader> disc(a_string(argv[1])+"/StarCraft.mpq");
 mkdir(argv[2],0777);
 a_vector<a_string> names={"glue/PalCs/backgnd.pcx","glue/PalCs/tFont.pcx","glue/PalCs/Dlg.grp","glue/campaign/pBotL.pcx","glue/campaign/pBotR.pcx","glue/create/pListMap2.pcx","glue/create/pInfo.pcx","glue/create/pSlots.pcx","glue/PalNl/pCancel.pcx","glue/PalNl/pOk.pcx","glue/PalMm/Retail_Ex.pcx","cursor/arrow.grp"};
 for(const char* folder:{"campaign","Expcampaign"}){for(const char* race:{"Terr","Prot","Zerg"})for(const char* state:{"","On"})names.push_back(a_string("glue/")+folder+"/"+(folder[0]=='E'?"X":"")+race+state+".smk");names.push_back(a_string("glue/")+folder+"/disk.smk");}
 for(const auto& name:names){a_vector<uint8_t> bytes;bool found=false;for(auto& archive:files.mpqs)if(extract(archive,name,bytes)){found=true;break;}if(!found)found=extract(disc,name,bytes);if(!found)throw std::runtime_error("Missing original frontend asset");
  a_string flat=name;std::replace(flat.begin(),flat.end(),'/','_');if(name=="glue/PalMm/Retail_Ex.pcx")flat="logo-bw.pcx";if(name=="cursor/arrow.grp")flat="cursor.grp";
  std::ofstream out(a_string(argv[2])+"/"+flat,std::ios::binary);out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());if(!out)throw std::runtime_error("Cannot write private frontend art");
 }
 std::puts("Original campaign/custom art, idle/hover animations, logos and cursor extracted privately");
 }catch(const std::exception& e){std::fprintf(stderr,"Frontend export: %s\n",e.what());return 1;}}
