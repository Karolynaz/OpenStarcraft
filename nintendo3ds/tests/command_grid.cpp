#include "command_grid.h"
#include <cassert>
#include <cstdio>
int main(){using port3ds::CommandGrid;
 for(unsigned n=1;n<=9;++n)for(unsigned i=0;i<n;++i){auto c=CommandGrid::cell(i);assert(CommandGrid::hit(c.x,c.y,n)==int(i));assert(CommandGrid::hit(c.x+35,c.y+41,n)==int(i));for(unsigned d=0;d<4;++d)assert(CommandGrid::move(i,d,n)<n);}
 assert(CommandGrid::cell(3).x==233 && CommandGrid::cell(4).x==122 && CommandGrid::cell(8).x==270);
 assert(CommandGrid::hit(121,26,9)==-1 && CommandGrid::hit(158,26,9)==-1);assert(CommandGrid::move(3,2,9)==7 && CommandGrid::move(8,0,9)==3);
 puts("Figma 4+5 cells, badges/touch bounds and uneven-row D-pad navigation PASS");
}
