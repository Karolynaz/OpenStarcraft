// Private host-only test reference: decompress originals with OpenBW, retain
// palette indices/coverage so a separate Python PCX reader checks GPU colors.
#include "bwgame.h"
#include <fstream>
#include <cstdio>
using namespace bwgame;
int main(int argc,char** argv){
 if(argc!=3)return 2;
 try{
  auto files=data_loading::data_files_directory(argv[1]);a_vector<uint8_t> data;files(data,"unit/cmdbtns/cmdicons.grp");
  auto grp=read_grp(data_loading::data_reader_le(data.data(),data.data()+data.size()));
  std::ofstream out(a_string(argv[2])+"/icons.raw",std::ios::binary);
  auto u32=[&](unsigned value){for(unsigned b=0;b<4;++b)out.put((value>>(b*8))&255);};u32(grp.frames.size());
  for(const auto& frame:grp.frames){
   u32(frame.offset.x);u32(frame.offset.y);u32(frame.size.x);u32(frame.size.y);
   a_vector<uint16_t> indices(frame.size.x*frame.size.y,256);
   for(unsigned y=0;y<unsigned(frame.size.y);++y){
    const auto* p=frame.data_container.data()+frame.line_data_offset[y];unsigned x=0;
    while(x<unsigned(frame.size.x)){unsigned token=*p++,count=token&127;if(token&128){x+=count;continue;}
     if(token&64){count=token&63;const auto c=*p++;while(count--)indices[y*frame.size.x+x++]=c;}
     else while(count--)indices[y*frame.size.x+x++]=*p++;
    }
   }
   for(auto value:indices){out.put(value&255);out.put(value>>8);}
  }
  if(!out)throw std::runtime_error("UI reference write failed");
  const char* names[]={"unit/cmdbtns/ticon.pcx","game/tconsole.pcx","game/pconsole.pcx","game/zconsole.pcx"};
  for(auto name:names){data.clear();files(data,name);a_string base=name;base=base.substr(base.find_last_of('/')+1);std::ofstream pcx(a_string(argv[2])+"/"+base,std::ios::binary);pcx.write((const char*)data.data(),data.size());if(!pcx)throw std::runtime_error("UI PCX write failed");}
  std::printf("Original command GRP index/coverage reference: %zu frames\n",grp.frames.size());
 }catch(const std::exception& e){std::fprintf(stderr,"UI export: %s\n",e.what());return 1;}
}
