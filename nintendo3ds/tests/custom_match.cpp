#include "engine.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <fstream>
#include <unistd.h>
int main(int argc,char**argv){using namespace port3ds;assert(argc==3);
 MapPreview preview;assert(Engine::preview_map(argv[1],argv[2],preview));assert(preview.valid && preview.width==64 && preview.height==64 && preview.players==4 && preview.title[0]);unsigned opaque=0;for(auto p:preview.pixels)opaque+=bool(p&1);assert(opaque==128*128);printf("Preview: %s / %u slots\n",preview.title,preview.players);
 // Large maps are previewed without constructing the full simulation.
 char temp[]="/tmp/openbw-preview-XXXXXX";int fd=mkstemp(temp);assert(fd>=0);close(fd);const std::string path=std::string(temp)+".chk";std::remove(temp);
 std::vector<unsigned char> map;auto chunk=[&](const char* tag,std::vector<unsigned char> bytes){map.insert(map.end(),tag,tag+4);for(unsigned i=0;i<4;++i)map.push_back((bytes.size()>>(8*i))&255);map.insert(map.end(),bytes.begin(),bytes.end());};
 chunk("DIM ",{128,0,128,0});chunk("ERA ",{1,0});chunk("MTXM",std::vector<unsigned char>(128*128*2));std::vector<unsigned char> starts(8*36);for(unsigned i=0;i<8;++i){starts[i*36+8]=214;starts[i*36+16]=i;}chunk("UNIT",starts);
 {std::ofstream out(path,std::ios::binary);out.write((char*)map.data(),map.size());}MapPreview large;assert(Engine::preview_map(argv[1],path.c_str(),large) && large.width==128 && large.players==8);
 {std::ofstream out(path,std::ios::binary);out<<"broken";}assert(!Engine::preview_map(argv[1],path.c_str(),large) && !large.valid && large.error[0]);std::remove(path.c_str());
 Engine e;MatchSetup setup;setup.enabled=true;setup.races={{1,0,2}};
 assert(e.load(argv[1],argv[2],Scenario::versus_ai,&setup));auto s=e.snapshot();assert(s.players[0].workers==4 && s.players[1].workers==4 && s.players[2].workers==4);assert(s.players[1].ai && s.players[2].ai && !s.players[0].ai);
 assert(e.count_units(0,106)==1 && e.count_units(1,131)==1 && e.count_units(2,154)==1); // CC/Hatchery/Nexus
 for(unsigned i=0;i<4000;++i)assert(e.tick());s=e.snapshot();assert(s.players[1].workers>4 && s.players[2].workers>4);assert(s.players[1].trained>0 && s.players[2].trained>0);assert(s.winner==-1);printf("Mixed AI: trained %u/%u, workers %u/%u\n",s.players[1].trained,s.players[2].trained,s.players[1].workers,s.players[2].workers);
 setup.races={{2,1,-1}};assert(e.load(argv[1],argv[2],Scenario::protoss_versus_ai,&setup));s=e.snapshot();assert(s.players[2].live==0 && !s.players[2].ai && s.players[1].ai);for(unsigned i=0;i<36;++i)assert(e.tick());assert(e.snapshot().winner==-1);
 setup.races={{1,-1,0}};assert(e.load(argv[1],argv[2],Scenario::versus_ai,&setup));for(unsigned i=0;i<36;++i)assert(e.tick());s=e.snapshot();assert(!s.players[1].live && !s.players[1].ai && s.players[2].ai && s.players[2].workers==4 && s.winner==-1);
 setup.races={{0,-1,-1}};assert(e.load(argv[1],argv[2],Scenario::zerg_versus_ai,&setup));for(unsigned i=0;i<36;++i)assert(e.tick());s=e.snapshot();assert(!s.players[1].live && !s.players[2].live && s.winner==-1);
 puts("Map metadata/terrain preview + mixed two-AI production + closed slots/no-AI sandbox PASS");}
