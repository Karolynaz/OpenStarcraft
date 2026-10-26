// Deliberately remove the human starting force in a private test fixture.
// This verifies original defeat triggers; shipped maps are never modified.
#include "engine.h"
#include "campaign_map.h"
#include <fstream>
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);for(const auto& mission:campaign::missions){std::ifstream in(std::string(argv[2])+"/"+mission.key+".chk",std::ios::binary);std::vector<uint8_t> b((std::istreambuf_iterator<char>(in)),{});assert(!b.empty());unsigned removed=0;for(size_t at=0;at+8<=b.size();){unsigned n=campaign::get(b.data()+at+4);assert(n<=b.size()-at-8);if(!memcmp(b.data()+at,"UNIT",4)){for(unsigned j=0;j<n;){auto* u=b.data()+at+8+j;if(u[16]==0){b.erase(b.begin()+at+8+j,b.begin()+at+8+j+36);n-=36;++removed;}else j+=36;}campaign::put(b.data()+at+4,n);}at+=8+n;}assert(removed);std::string path=std::string("/tmp/sc3ds-defeat-")+mission.key+".chk";{std::ofstream out(path,std::ios::binary);out.write((char*)b.data(),b.size());}Engine e;assert(e.load(argv[1],path.c_str(),Scenario(mission.scenario)));for(unsigned i=0;i<3000 && e.snapshot().winner<0;++i)assert(e.tick());printf("%s original loss trigger winner%d frame%u (empty-human fixture)\n",mission.key,e.snapshot().winner,e.frame());fflush(stdout);assert(e.snapshot().winner==1);}puts("Six original defeat branches PASS");}
