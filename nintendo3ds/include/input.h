#pragma once
#include <cstdint>
namespace port3ds {
enum class InputAction {none,confirm,cancel,army,workers,power,system,all,previous,next,store,recall,patrol,stop,attack,hotkey};
struct InputFrame {
 bool a=false,b=false,x=false,y=false,start=false,select=false,l=false,r=false,l_pressed=false,r_pressed=false;
 int direction=-1;std::uint64_t time=0;
};
struct InputEvent {InputAction action=InputAction::none;unsigned slot=0;bool center=false;};
class InputRouter {
 int last_group_=-1;std::uint64_t last_time_=0;
public:
 InputEvent read(const InputFrame& f,bool menu=false,bool catalogue=false){
  if(f.start)return {InputAction::system};
  if(catalogue){
   if(f.l && f.r && (f.l_pressed || f.r_pressed))return {InputAction::hotkey,8};
   if(f.x)return {InputAction::hotkey,f.l?4u:f.r?6u:0u};
   if(f.y)return {InputAction::hotkey,f.l?5u:f.r?7u:1u};
   if(f.l && f.a)return {InputAction::hotkey,2};
   if(f.l && f.b)return {InputAction::hotkey,3};
  }
  if(menu){if(f.b)return {InputAction::cancel};if(f.a)return {InputAction::confirm};if(f.direction>=0)return {f.direction==0 || f.direction==3?InputAction::previous:InputAction::next,unsigned(f.direction)};return {};}
  if(f.select)return {InputAction::all};
  if(f.direction>=0 && f.direction<4){
   unsigned slot=f.direction;if(f.l){last_group_=-1;return {InputAction::store,slot};}
   if(f.r){bool center=last_group_==f.direction && f.time>=last_time_ && f.time-last_time_<=350000;last_group_=f.direction;last_time_=f.time;return {InputAction::recall,slot,center};}return {};
  }
  if(f.r && f.a)return {InputAction::patrol};
  if(f.r && f.b)return {InputAction::stop};
  if(f.r && f.x)return {InputAction::army};
  if(f.r && f.y)return {InputAction::workers};
  if(f.l && f.a)return {InputAction::attack};
  if(f.l && f.r && (f.l_pressed || f.r_pressed))return {InputAction::power,4};
  if(f.x)return {InputAction::power,f.l?2u:0u};
  if(f.y)return {InputAction::power,f.l?3u:1u};
  if(f.b)return {InputAction::cancel};
  if(f.a)return {InputAction::confirm};
  return {};
 }
};
}
