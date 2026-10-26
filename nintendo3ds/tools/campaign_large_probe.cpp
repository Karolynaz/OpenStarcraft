// Private geometry/memory probe. Strips mission triggers only in TEMP fixtures:
// this must never be presented as a mission-playability or victory test.
#include "engine.h"
#include "campaign_map.h"
#include <fstream>
#include <cassert>
#include <cstdio>
int main(int argc,char**argv){assert(argc==4);std::ifstream in(argv[2],std::ios::binary);std::vector<uint8_t> chk((std::istreambuf_iterator<char>(in)),{});for(size_t at=0;at+8<=chk.size();){unsigned n=port3ds::campaign::get(chk.data()+at+4);assert(n<=chk.size()-at-8);if(!memcmp(chk.data()+at,"TRIG",4)){chk.erase(chk.begin()+at+8,chk.begin()+at+8+n);port3ds::campaign::put(chk.data()+at+4,0);n=0;}at+=8+n;}const std::string path=std::string(argv[3])+".chk";{std::ofstream out(path,std::ios::binary);out.write((char*)chk.data(),chk.size());}
port3ds::Engine e;if(!e.load(argv[1],path.c_str(),port3ds::Scenario::terran_campaign)){fprintf(stderr,"%s\n",e.error());return 1;}assert(e.start_recording((std::string(argv[3])+".session").c_str()));for(unsigned i=0;i<48;++i)assert(e.tick());port3ds::SaveView v;v.camera_x=std::max(0,e.snapshot().mission_camera_x-200);v.camera_y=std::max(0,e.snapshot().mission_camera_y-120);assert(e.save_game((std::string(argv[3])+".sav").c_str(),v));assert(e.write_terrain(argv[1],(std::string(argv[3])+".terrain").c_str()));printf("Private no-trigger geometry fixture %ux%u, units%u, save + streamed terrain written PASS\n",e.snapshot().width,e.snapshot().height,e.snapshot().units);}
