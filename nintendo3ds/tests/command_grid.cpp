#include "command_grid.h"
#include "input.h"
#include <cassert>
#include <cstdio>
int main(){using namespace port3ds;
 for(unsigned n=1;n<=8;++n){
  for(unsigned i=0;i<n;++i){
   const int x=CommandGrid::x+i%2*CommandGrid::cell_width,y=CommandGrid::y+i/2*CommandGrid::cell_height;
   assert(CommandGrid::hit(x,y,n)==int(i));assert(CommandGrid::hit(x+91,y+23,n)==int(i));
   for(unsigned direction=0;direction<4;++direction)assert(CommandGrid::move(i,direction,n)<n);
   const auto down=CommandGrid::move(i,2,n);assert(down%2==i%2 && CommandGrid::move(down,0,n)==i);
   assert(CommandGrid::move(CommandGrid::move(i,1,n),3,n)==i);
  }
  assert(CommandGrid::hit(135,144,n)==-1 && CommandGrid::hit(320,144,n)==-1 && CommandGrid::hit(136,240,n)==-1);
  if(n%2)assert(CommandGrid::hit(228,144+int(n/2)*24,n)==-1);
 }
 InputRouter input;InputFrame frame;for(int direction=0;direction<4;++direction){frame.direction=direction;const auto e=input.read(frame,true);assert(e.slot==unsigned(direction) && e.action==(direction==0 || direction==3?InputAction::previous:InputAction::next));}
 std::puts("Catalogue touch bounds / incomplete row / all four D-pad directions / inverse navigation PASS");
}
