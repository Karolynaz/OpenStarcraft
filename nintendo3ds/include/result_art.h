#pragma once
#include "result_screen.h"
#include "ui_art.h"
namespace port3ds {
// Loaded only after world graphics/audio have been released at match end.
class ResultArt {
 UiArt images_;
public:
 void clear(){images_.clear();}
 bool load(const char* root,unsigned race,bool victory){
  return images_.load((std::string(root)+"campaign/result-"+ResultScreen::race(race)+(victory?"-victory.bin":"-defeat.bin")).c_str());
 }
 bool top(){return images_.draw("top",0,0);}
 bool bottom(){return images_.draw("bottom",0,0);}
};
}
