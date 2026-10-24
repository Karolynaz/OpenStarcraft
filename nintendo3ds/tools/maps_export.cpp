// Extract only private owned melee maps; filenames come from a user-supplied
// dictionary, never fetch or redistribute Blizzard data in the source tree.
#include "original_archive.h"
#include "engine.h"
#include "bwgame.h"
#include <fstream>
#include <set>
#include <sys/stat.h>
using namespace port3ds;
int main(int argc,char**argv){if(argc!=4){puts("maps-export OWNED_DATA FILENAME_LIST OUTPUT_MAP_DIRECTORY");return 2;}
 try{
  std::ifstream input(argv[2]);if(!input)throw std::runtime_error("Missing filename dictionary");std::set<a_string> names;std::string name;
  while(std::getline(input,name)){if(!name.empty() && name.back()=='\r')name.pop_back();std::replace(name.begin(),name.end(),'\\','/');if(name.find("multimaps/")!=0 || name.find("..")!=std::string::npos)continue;
   const auto extension=name.substr(name.find_last_of('.')+1);if(extension!="scm" && extension!="scx")continue;
   auto relative=name.substr(10);if(relative.find("broodwar/")==0)relative=relative.substr(9);
   if(relative.find("ladder/")==0)relative=relative.substr(7);else if(relative.find("oldladder/")==0)relative=relative.substr(10);
   if(relative.find('/')==std::string::npos && relative.size()>3 && relative[0]=='(' && relative[1]>='2' && relative[1]<='8')names.insert(name);
  }
  mkdir(argv[3],0777);unsigned accepted=0,rejected=0;
  for(const char* disc:{"StarCraft","BroodWar"}){const auto folder=a_string(argv[3])+"/"+disc;mkdir(folder.c_str(),0777);data_loading::mpq_file<DiscReader> archive(a_string(argv[1])+"/"+disc+".mpq");
   for(const auto& path:names){a_vector<uint8_t> bytes;if(!extract(archive,path,bytes))continue;const auto target=folder+"/"+path.substr(path.find_last_of('/')+1);{std::ofstream out(target,std::ios::binary);out.write((char*)bytes.data(),bytes.size());if(!out)throw std::runtime_error("Cannot write private map");}
    const char* reason=nullptr;MapPreview preview;
    if(!Engine::preview_map(argv[1],target.c_str(),preview))reason="invalid preview";
    else if(preview.width>128 || preview.height>128 || preview.width<32 || preview.height<32)reason="size";
    else if(preview.players<2)reason="starts";
    if(!reason){
     try{auto files=data_loading::data_files_directory(argv[1]);game_player player(files);game_load_functions loader(player.st());a_vector<uint8_t> chk;data_loading::mpq_data map(bytes.data(),bytes.size());map(chk,"staredit/scenario.chk");
      loader.load_map_data(chk.data(),chk.size(),[&]{loader.setup_info.create_no_units=true;for(auto& p:player.st().players)p.controller=player_t::controller_inactive;});
      // Every offered start must be reachable: the first three are used now,
      // but this also makes later slot-selection safe for this bounded bot.
      std::vector<xy> starts;for(size_t at=0;at+8<=chk.size();){auto len=data_loading::value_at<uint32_t,true>(chk.data()+at+4);bool units=!memcmp(chk.data()+at,"UNIT",4);at+=8;if(len>chk.size()-at)throw std::runtime_error("Truncated units");if(units)for(size_t j=at;j+36<=at+len;j+=36)if(data_loading::value_at<uint16_t,true>(chk.data()+j+8)==214)starts.push_back({int(data_loading::value_at<uint16_t,true>(chk.data()+j+4)),int(data_loading::value_at<uint16_t,true>(chk.data()+j+6))});at+=len;}
      if(starts.size()<2)reason="starts";else for(auto start:starts)if(!loader.is_reachable(starts[0],start))reason="islands";
     }catch(const std::exception&){reason="native load";}
    }
    if(reason){std::remove(target.c_str());++rejected;printf("SKIP\t%s\t%s\n",path.c_str(),reason);}
    else {++accepted;printf("MAP\t%s\t%ux%u\t%u\t%s\n",target.c_str(),preview.width,preview.height,preview.players,preview.title);}
    fflush(stdout);
   }
  }
  printf("Accepted %u original melee maps, skipped %u incompatible maps\n",accepted,rejected);
 }catch(const std::exception&e){fprintf(stderr,"Map export: %s\n",e.what());return 1;}
}
