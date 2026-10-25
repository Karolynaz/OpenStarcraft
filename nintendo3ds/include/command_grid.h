#pragma once
namespace port3ds {
struct CommandGrid {
 struct Cell{int x,y,w,h;};
 static Cell cell(unsigned i){return {118+int(i<4?i:i-4)*37,i<4?29:81,35,35};}
 static const char* key(unsigned i){static const char* keys[]={"X","Y","L+A","L+B","L+X","L+Y","R+X","R+Y","L+R"};return i<9?keys[i]:"";}
 static int hit(int x,int y,unsigned count){for(unsigned i=0;i<count && i<9;++i){auto c=cell(i);if(x>=c.x && x<c.x+c.w && y>=c.y && y<c.y+c.h+10)return int(i);}return -1;}
 static unsigned move(unsigned i,unsigned direction,unsigned count){
  if(!count)return 0;count=count>9?9:count;i%=count;
  if(direction==1)return (i+1)%count;if(direction==3)return (i+count-1)%count;
  unsigned col=i<4?i:i-4;unsigned other=i<4?4+col:(col<4?col:3);return other<count?other:i;
 }
};
}
