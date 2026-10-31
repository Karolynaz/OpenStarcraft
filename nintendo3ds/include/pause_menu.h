#pragma once
#include "frontend.h"
namespace port3ds {
struct PauseMenu {
 enum class Page {none,system,options,scenarios,legacy,catalogue,result,notice,end_confirm,save_slots,load_slots};
 static bool overlay(Page p){return p!=Page::none && p!=Page::catalogue;}
 static unsigned count(Page p){switch(p){case Page::system:case Page::save_slots:case Page::load_slots:return 5;case Page::options:return 7;case Page::scenarios:case Page::legacy:return 8;case Page::end_confirm:return 2;case Page::result:case Page::notice:return 1;default:return 0;}}
 static Page back(Page p){switch(p){case Page::save_slots:case Page::load_slots:case Page::options:case Page::notice:case Page::end_confirm:return Page::system;case Page::scenarios:return Page::options;case Page::legacy:return Page::scenarios;case Page::result:return Page::result;default:return Page::none;}}
 static UiRect row(unsigned i,unsigned count){const int height=count>6?21:30;return {36,(count==1?180:count==2?78:48)+int(i)*height,248,height-4};}
 static int hit(int x,int y,unsigned count){for(unsigned i=0;i<count;++i)if(row(i,count).hit(x,y))return int(i);return -1;}
};
}
