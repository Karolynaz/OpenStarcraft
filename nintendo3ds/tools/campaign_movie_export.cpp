#include "original_archive.h"

#include <array>
#include <fstream>

int main(int argc,char** argv){
    if(argc!=4){
        std::fprintf(stderr,"Usage: campaign_movie_export STARCRAFT.MPQ BLOCK OUTPUT.SMK\n");
        return 2;
    }
    try{
        data_loading::mpq_file<DiscReader> disc(argv[1]);
        const auto& block=disc.mpq.block_table.at(std::stoul(argv[2]));
        if(block.flags!=0x80000000 || block.size!=block.compressed_size || block.size<4)
            throw std::runtime_error("Expected an uncompressed movie in the original CD archive");
        disc.file.seek(block.data_offset);
        std::array<uint8_t,65536> buffer;
        disc.file.get_bytes(buffer.data(),4);
        if(std::memcmp(buffer.data(),"SMK2",4))
            throw std::runtime_error("Unexpected movie signature");
        disc.file.seek(block.data_offset);
        std::ofstream output(argv[3],std::ios::binary);
        for(size_t left=block.size;left;){
            const auto size=std::min(left,buffer.size());
            disc.file.get_bytes(buffer.data(),size);
            output.write(reinterpret_cast<const char*>(buffer.data()),size);
            left-=size;
        }
        if(!output)throw std::runtime_error("Movie extraction failed");
    }catch(const std::exception& error){
        std::fprintf(stderr,"%s\n",error.what());
        return 1;
    }
}
