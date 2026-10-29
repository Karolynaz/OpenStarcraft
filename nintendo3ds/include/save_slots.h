#pragma once
#include "save_format.h"
#include "campaign_catalogue.h"
#include <array>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sys/stat.h>
namespace port3ds {
struct SaveSlots {
 static constexpr unsigned count=5;
 struct Slot {bool occupied=false,available=false;SaveInfo info;std::string path,label,short_label,error;};
 std::array<Slot,count> slots;
 static std::string path(const std::string& root,unsigned i){return root+"saves/slot"+std::to_string(i+1)+".ob3";}
 static std::string mission(const SaveInfo& info){const auto i=campaign::index(info.scenario);if(i>=campaign::count)return "Custom game";const auto& m=campaign::missions[i];unsigned number=0;for(unsigned j=0;j<=i;++j)if(campaign::missions[j].brood==m.brood && campaign::missions[j].race==m.race)++number;return std::string(m.brood?"BW ":"")+(m.race==0?"Zerg":m.race==2?"Protoss":"Terran")+" mission "+std::to_string(number);}
 static bool metadata(const std::string& path,SaveInfo& info){FILE* f=std::fopen(path.c_str(),"rb");if(!f)return false;savegame::Header h;bool ok=std::fread(h.bytes.data(),1,h.bytes.size(),f)==h.bytes.size();std::fclose(f);if(!ok || std::memcmp(h.bytes.data(),"OB3SAVE1",8) || h.field(0)!=1)return false;const auto crc=h.field(28);h.field(28,0);if(savegame::hash(2166136261u,h.bytes.data(),h.bytes.size())!=crc)return false;if(h.field(6)>15 && !campaign::scenario(h.field(6)))return false;info=h.info();return true;}
 template<class Inspect> void scan(const std::string& root,Inspect inspect){
  for(unsigned i=0;i<count;++i){auto& s=slots[i];s=Slot{};s.path=path(root,i);char error[192]{};struct stat st{};s.occupied=stat(s.path.c_str(),&st)==0;const auto primary=s.path;
   s.available=inspect(s.path.c_str(),s.info,error,sizeof error);s.error=error;
   if(!s.available){SaveInfo backup;const auto alternate=primary+".bak";if(inspect(alternate.c_str(),backup,error,sizeof error)){s.path=alternate;s.info=backup;s.available=s.occupied=true;stat(s.path.c_str(),&st);}else if(!s.occupied && stat(alternate.c_str(),&st)==0){s.path=alternate;s.occupied=true;}}
   bool known=s.available || (s.occupied && metadata(s.path,s.info));
   s.short_label="Slot "+std::to_string(i+1)+" • "+(!s.occupied?"Empty":known?mission(s.info):"Damaged save");
   char clock[16]{},date[16]{};const std::time_t stamp=s.info.saved_at && s.info.saved_at<4102444800ULL?std::time_t(s.info.saved_at):st.st_mtime;if(s.occupied && stamp>0){if(const auto* time=std::localtime(&stamp)){std::strftime(clock,sizeof clock,"%H:%M",time);std::strftime(date,sizeof date,"%b %d",time);}if(clock[0])s.short_label+=" • "+std::string(clock);}
   s.label=s.short_label;if(date[0])s.label+=" "+std::string(date);
   if(s.occupied && !s.available){s.short_label+=" [unavailable]";s.label=s.short_label;}
  }
 }
};
}
