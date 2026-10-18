// Extract only privately owned audio used by this Terran test roster.
#include "actions.h"
#include "../tests/battle.h"
#include "../tests/gameplay.h"
#include "../tests/gas.h"
#include <set>
#include <fstream>
#include <sys/stat.h>
using namespace bwgame;
// GOG's original CD container starts with an MZ stub. Read its embedded MPQ
// without copying the 574 MiB disc or changing the runtime MPQ reader.
struct DiscReader : data_loading::file_reader<> {
    size_t base=0,length=0;
    explicit DiscReader(a_string name):data_loading::file_reader<>(std::move(name)) {
        const auto total=data_loading::file_reader<>::size();
        for(size_t off=0;off+32<=std::min<size_t>(total,1024*1024);off+=512) {
            data_loading::file_reader<>::seek(off);
            auto sig=get<uint32_t>(),header=get<uint32_t>(),size=get<uint32_t>();
            auto version=get<uint16_t>();
            if(sig==0x1a51504d && header==32 && version==0 && size>=32 && size<=total-off) {
                base=off;length=size;seek(0);return;
            }
        }
        throw std::runtime_error("No classic embedded MPQ in disc container");
    }
    void seek(size_t off) {if(off>length) throw std::runtime_error("Disc bounds");data_loading::file_reader<>::seek(base+off);}
    size_t size() const {return length;}
    size_t tell() const {return data_loading::file_reader<>::tell()-base;}
};

int main(int argc,char** argv) {
    if(argc!=4) return 2;
    try {
        std::set<unsigned> ids;
        unsigned events=0;
        auto observe=[&](port3ds::Engine& e) {
            port3ds::SoundEvent out[64];auto n=e.drain_sounds(out,64);events+=n;
            for(unsigned i=0;i<n;++i) ids.insert(out[i].id);
            if(e.dropped_sounds()) throw std::runtime_error("Sound event queue overflow");
        };
        battle_test(argv[1],argv[2],observe);
        gameplay_tests(argv[1],argv[2],observe);
        gas_tests(argv[1],argv[2],observe);
        battle_test(argv[1],argv[2],observe,port3ds::Scenario::gas_battle);
        auto assets=data_loading::data_files_directory(argv[1]);
        a_vector<uint8_t> data;assets(data,"arr/sfxdata.dat");
        auto sounds=data_loading::load_sfxdata_dat(data);
        string_table_data tbl;assets(tbl.data,"arr/sfxdata.tbl");
        game_player player(assets);
        auto units=data_loading::load_units_dat(player.st().global->units_dat);
        for(auto type:{UnitTypes::Terran_SCV,UnitTypes::Terran_Marine,UnitTypes::Terran_Command_Center,UnitTypes::Terran_Barracks,UnitTypes::Terran_Supply_Depot,UnitTypes::Terran_Refinery}) {
            const auto* u=&units.vec.at(static_cast<unsigned>(type));
            if(u->ready_sound) ids.insert(u->ready_sound);
            for(int i=u->first_what_sound;i && i<=u->last_what_sound;++i) ids.insert(i);
            for(int i=u->first_yes_sound;i && i<=u->last_yes_sound;++i) ids.insert(i);
        }
        mkdir(argv[3],0777);
        const std::string root=std::string(argv[3])+"/";
        std::ofstream manifest(root+"manifest.tsv");
        auto write=[&](const std::string& file,const a_string& name) {
            assets(data,name);std::ofstream f(root+file,std::ios::binary);f.write((const char*)data.data(),data.size());
            if(!f) throw std::runtime_error("Audio export write failed");
        };
        for(auto id:ids) {
            const auto& s=sounds.vec.at(id);
            auto name="sound/"+tbl[s.filename_index];
            write(std::to_string(id)+".wav",name);
            manifest<<id<<'\t'<<s.priority<<'\t'<<s.flags<<'\t'<<name.c_str()<<'\n';
        }
        data_loading::mpq_file<DiscReader> disc(a_string(argv[1])+"/StarCraft.mpq");
        for(unsigned n=1;n<=3;++n) {
            disc(data,format("music/terran%d.wav",n));
            std::ofstream f(root+"terran"+std::to_string(n)+".wav",std::ios::binary);
            f.write((const char*)data.data(),data.size());if(!f) throw std::runtime_error("Music export write failed");
        }
        std::printf("Exported %zu effect IDs, %u native events, 3 Terran tracks\n",ids.size(),events);
    } catch(const std::exception& e) {std::fprintf(stderr,"%s\n",e.what());return 1;}
}
