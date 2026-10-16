#pragma once
#include <cstdint>
namespace port3ds {
enum class InputAction { none, confirm, cancel, army, workers, primary, secondary, system, pause, previous, next, store, recall };
struct InputFrame {
    bool a=false,b=false,x=false,y=false,start=false,select=false,l=false,r=false;
    int direction=-1; // up, right, down, left
    std::uint64_t time=0;
};
struct InputEvent { InputAction action=InputAction::none; unsigned slot=0; bool center=false; };
class InputRouter {
    int last_group_=-1;
    std::uint64_t last_time_=0;
public:
    InputEvent read(const InputFrame& in,bool menu=false) {
        if(in.start) return {InputAction::system};
        if(in.select) return {InputAction::pause};
        if(in.b) return {InputAction::cancel};
        if(in.x) return {in.l ? InputAction::workers : InputAction::army};
        if(in.y) return {in.l ? InputAction::secondary : InputAction::primary};
        if(in.a) return {InputAction::confirm};
        if(in.direction>=0 && in.direction<4) {
            const auto slot=static_cast<unsigned>(in.direction);
            if(in.r) { last_group_=-1; return {InputAction::store,slot}; }
            if(menu) return {in.direction==0 || in.direction==3 ? InputAction::previous : InputAction::next};
            const bool center=last_group_==in.direction && in.time>=last_time_ && in.time-last_time_<=350000;
            last_group_=in.direction; last_time_=in.time;
            return {InputAction::recall,slot,center};
        }
        // L and R are modifiers only, including when pressed before X/Y/D-pad.
        return {};
    }
};
}
