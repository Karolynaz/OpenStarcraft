// Private compatibility audit, not a campaign completion test. Uses normalized
// owned CHKs. No data/strings are compiled in and no original assets are altered.
#include "engine.h"
#include <dirent.h>
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
int main(int argc,char**argv){if(argc!=3)return 2;std::vector<std::string> keys;if(auto* dir=opendir(argv[2])){while(auto* e=readdir(dir)){std::string n=e->d_name;if(n.size()>4 && n.substr(n.size()-4)==".chk" && n.find('.')==n.size()-4)keys.push_back(n.substr(0,n.size()-4));}closedir(dir);}std::sort(keys.begin(),keys.end());puts("key,status,frame,units,error");for(const auto& key:keys){port3ds::Engine e;auto scenario=key.compare(0,3,"exp")==0?port3ds::Scenario::bw_protoss_campaign:port3ds::Scenario::terran_campaign;const auto path=std::string(argv[2])+"/"+key+".chk";bool ok=e.load(argv[1],path.c_str(),scenario);const char* stage=ok?"startup":"load-rejected";unsigned frame=0,units=0;if(ok){for(unsigned n=0;n<1200 && e.snapshot().winner<0;++n){if(!e.tick()){stage="tick-rejected";ok=false;break;}frame=e.frame();}if(ok)units=e.snapshot().units;}printf("%s,%s,%u,%u,%s\n",key.c_str(),stage,frame,units,ok?"":e.error());fflush(stdout);}return 0;}
