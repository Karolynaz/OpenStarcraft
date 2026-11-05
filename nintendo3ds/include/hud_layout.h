#pragma once
namespace port3ds {
// Figma 128:869 / 128:991, in native bottom-screen pixels. Shared by
// rendering and touch routing; none of these depend on the active race skin.
struct HudLayout {
 struct Rect {int x,y,w,h;bool hit(int px,int py)const{return px>=x && py>=y && px<x+w && py<y+h;}};
 static Rect selection(unsigned i){return {19+int(i%2)*41,16+int(i/2)*35,36,34};}
 static Rect upgrade(unsigned i){return {20+int(i%2)*41,150+int(i/2)*39,36,34};}
 static Rect menu(){return {122,130,60,18};}
 static Rect portrait(){return {122,152,60,56};}
 static Rect minimap(){return {206,130,96,96};}
 static const char* skin(unsigned race){return race==0?"hud-zerg.bin":race==2?"hud-protoss.bin":"hud-terran.bin";}
};
}
