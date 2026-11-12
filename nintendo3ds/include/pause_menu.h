#pragma once
#include "frontend.h"
namespace port3ds {
struct PauseMenu {
 enum class Page {none,system,options,dev,scenarios,legacy,catalogue,result,notice,end_confirm,save_slots,load_slots};
 static bool overlay(Page p){return p!=Page::none && p!=Page::catalogue;}
 static unsigned count(Page p){switch(p){case Page::system:case Page::save_slots:case Page::load_slots:return 5;case Page::options:return 5;case Page::dev:return 6;case Page::scenarios:case Page::legacy:return 8;case Page::end_confirm:return 2;case Page::result:case Page::notice:return 1;default:return 0;}}
 static Page back(Page p){switch(p){case Page::save_slots:case Page::load_slots:case Page::options:case Page::notice:case Page::end_confirm:return Page::system;case Page::dev:return Page::options;case Page::scenarios:return Page::dev;case Page::legacy:return Page::scenarios;case Page::result:return Page::result;default:return Page::none;}}
 static UiRect row(unsigned i,unsigned count){const int step=count>6?21:count==6?28:30;return {36,(count==1?180:count==2?90:count>6?48:count==6?54:70)+int(i)*step,248,count>6?17:26};}
 static UiRect panel(unsigned count){const int top=count>5?12:28;const auto last=row(count?count-1:0,count);return {24,top,272,last.y+last.h+12-top};}
 static int hit(int x,int y,unsigned count){for(unsigned i=0;i<count;++i)if(row(i,count).hit(x,y))return int(i);return -1;}
};
}
