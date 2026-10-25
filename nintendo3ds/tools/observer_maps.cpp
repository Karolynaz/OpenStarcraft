// Extract two private owned island maps for repeatable observer transport tests.
#include "original_archive.h"
#include <fstream>
#include <sys/stat.h>
int main(int argc,char** argv){if(argc!=3)return 2;try{mkdir(argv[2],0777);data_loading::mpq_file<DiscReader> archive(a_string(argv[1])+"/StarCraft.mpq");
 for(auto item:{std::make_pair("multimaps/(2)The Small Divide.scm","islands.scm"),std::make_pair("multimaps/(3)Triumvirate.scm","three-islands.scm")}){
 a_vector<uint8_t> bytes;if(!extract(archive,item.first,bytes))throw std::runtime_error("Missing owned observer map");std::ofstream out(a_string(argv[2])+"/"+item.second,std::ios::binary);out.write((char*)bytes.data(),bytes.size());if(!out)throw std::runtime_error("Cannot write observer map");printf("%s %zu bytes\n",item.second,bytes.size());}
 }catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
