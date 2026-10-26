#include "input.h"
#include <cassert>
#include <cstdio>
using namespace port3ds;
int main(){
 UnitDoubleClick clicks;assert(!clicks.tap(7,1000000));assert(clicks.tap(7,1300000));assert(!clicks.tap(7,1350000));assert(!clicks.tap(8,1400000));assert(!clicks.tap(0,1450000));assert(!clicks.tap(8,1500000));assert(!clicks.tap(8,1900000));clicks.reset();assert(!clicks.tap(8,1950000));
 InputRouter r;InputFrame f;f.x=true;assert(r.read(f).action==InputAction::power && r.read(f).slot==0);f.l=true;assert(r.read(f).slot==2);f={};f.r=f.x=true;assert(r.read(f).action==InputAction::army);f.x=false;f.y=true;assert(r.read(f).action==InputAction::workers);
 f={};f.r=f.a=true;assert(r.read(f).action==InputAction::patrol);f.a=false;f.b=true;assert(r.read(f).action==InputAction::stop);
 f={};f.select=true;assert(r.read(f).action==InputAction::all);f={};f.start=true;assert(r.read(f).action==InputAction::system);
 for(unsigned i=0;i<9;++i){f={};if(i==0)f.x=true;if(i==1)f.y=true;if(i==2)f.l=f.a=true;if(i==3)f.l=f.b=true;if(i==4)f.l=f.x=true;if(i==5)f.l=f.y=true;if(i==6)f.r=f.x=true;if(i==7)f.r=f.y=true;if(i==8){f.l=f.r=f.r_pressed=true;}auto e=r.read(f,true,true);assert(e.action==InputAction::hotkey && e.slot==i);}
 f={};f.l=true;f.direction=2;f.time=1000000;assert(r.read(f).action==InputAction::store);f.l=false;f.r=true;assert(r.read(f).action==InputAction::recall);f.time+=200000;assert(r.read(f).center);f.r=false;assert(r.read(f).action==InputAction::none);
 f={};f.b=true;assert(r.read(f,true,true).action==InputAction::cancel);f={};f.a=true;assert(r.read(f,true,true).action==InputAction::confirm);
 puts("World/catalogue chord precedence, nine fixed shortcuts, selection groups and double recall PASS");
}
