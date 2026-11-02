#include "engine.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(int argc,char**argv){assert(argc==3);Engine a,b;unsigned calls=0;
 assert(a.load(argv[1],argv[2],Scenario::zerg_campaign,nullptr,[&](){++calls;}));
 assert(calls>0 && b.load(argv[1],argv[2],Scenario::zerg_campaign));
 assert(a.state_digest()==b.state_digest());for(unsigned i=0;i<96;++i){assert(a.tick() && b.tick());}assert(a.state_digest()==b.state_digest());
 printf("%u asset progress callbacks; identical default/progress simulation at load and frame96 PASS\n",calls);
}
