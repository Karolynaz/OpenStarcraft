// Isolated trigger-owner fixture. Original campaign maps are never replaced.
#include "bwgame.h"
#include "campaign_map.h"
#include <cassert>
#include <fstream>
#include <cstdio>
using namespace bwgame;
int main(int argc,char**argv){assert(argc==3);std::ifstream in(argv[2],std::ios::binary);a_vector<uint8_t> map((std::istreambuf_iterator<char>(in)),{});
 auto chunk=[&](const char* tag,const a_vector<uint8_t>& data){for(size_t at=0;at+8<=map.size();){unsigned n=port3ds::campaign::get(map.data()+at+4);if(!memcmp(map.data()+at,tag,4)){map.erase(map.begin()+at+8,map.begin()+at+8+n);map.insert(map.begin()+at+8,data.begin(),data.end());port3ds::campaign::put(map.data()+at+4,data.size());return;}at+=8+n;}assert(false);};
 a_vector<uint8_t> owners(12),forces(20),triggers(3*2400);owners[0]=6;owners[1]=3;owners[3]=7;chunk("OWNR",owners);chunk("FORC",forces);chunk("UNIT",{});chunk("THG2",{});
 for(unsigned i=0;i<3;++i){auto* t=triggers.data()+i*2400;t[15]=22;t[2372+(i==0?1:i==1?17:18)]=1;}chunk("TRIG",triggers);
 game_player p(data_loading::data_files_directory(argv[1]));game_load_functions loader(p.st());loader.load_map_data(map.data(),map.size(),[&]{loader.setup_info.create_no_units=true;p.st().players[0].controller=player_t::controller_occupied;},false);auto& st=p.st();
 assert(st.players[1].controller==player_t::controller_rescue_passive);assert(st.running_triggers[1].size()==1);assert(st.running_triggers[1][0].t==&st.game->triggers[0]);assert(st.running_triggers[2].empty());printf("queues%zu/%zu/%zu/%zu forces%u/%u/%u/%u\n",st.running_triggers[0].size(),st.running_triggers[1].size(),st.running_triggers[2].size(),st.running_triggers[3].size(),st.players[0].force,st.players[1].force,st.players[2].force,st.players[3].force);assert(st.running_triggers[0].size()==2 && st.running_triggers[3].size()==2);
 puts("Explicit rescue-owner script included; Force/All Players not inherited; inactive participant excluded PASS");
}
