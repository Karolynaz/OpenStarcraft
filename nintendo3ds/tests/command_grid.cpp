#include "command_grid.h"
#include "input.h"
#include <cassert>
#include <cstdio>
int main(){using namespace port3ds;
 for(unsigned n=1;n<=8;++n){
  for(unsigned i=0;i<n;++i){
   const int x=CommandGrid::x+i%CommandGrid::columns*CommandGrid::cell_width,y=CommandGrid::y+i/CommandGrid::columns*CommandGrid::cell_height;
   assert(CommandGrid::hit(x,y,n)==int(i));assert(CommandGrid::hit(x+31,y+31,n)==int(i));
   for(unsigned direction=0;direction<4;++direction)assert(CommandGrid::move(i,direction,n)<n);
   const auto down=CommandGrid::move(i,2,n);assert(down%CommandGrid::columns==i%CommandGrid::columns && CommandGrid::move(down,0,n)==i);
   assert(CommandGrid::move(CommandGrid::move(i,1,n),3,n)==i);
  }
  assert(CommandGrid::hit(165,49,n)==-1 && CommandGrid::hit(286,49,n)==-1 && CommandGrid::hit(166,154,n)==-1);
  assert(CommandGrid::hit(198,49,n)==-1); // decorative column gap

 }
 InputRouter input;InputFrame frame;for(int direction=0;direction<4;++direction){frame.direction=direction;const auto e=input.read(frame,true);assert(e.slot==unsigned(direction) && e.action==(direction==0 || direction==3?InputAction::previous:InputAction::next));}
 std::puts("Catalogue touch bounds / incomplete row / all four D-pad directions / inverse navigation PASS");
}
