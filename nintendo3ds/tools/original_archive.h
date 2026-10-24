#pragma once
#include "data_loading.h"
using namespace bwgame;
struct DiscReader:data_loading::file_reader<> {
 size_t base=0,length=0;
 explicit DiscReader(a_string path):data_loading::file_reader<>(path){const auto total=data_loading::file_reader<>::size();
  for(size_t off=0;off+32<=std::min<size_t>(total,1024*1024);off+=512){data_loading::file_reader<>::seek(off);const auto sig=get<uint32_t>(),header=get<uint32_t>(),size=get<uint32_t>();const auto version=get<uint16_t>();if(sig==0x1a51504d && header==32 && version==0 && size>=32 && size<=total-off){base=off;length=size;seek(0);return;}}
  throw std::runtime_error("No embedded classic MPQ");
 }
 void seek(size_t n){if(n>length)throw std::runtime_error("Disc bounds");data_loading::file_reader<>::seek(base+n);}size_t size()const{return length;}
};
template<class Archive> bool extract(Archive& archive,const a_string& name,a_vector<uint8_t>& bytes){
 const auto* hash=archive.mpq.find_hash_table_entry(name);if(!hash)return false;const auto block=archive.mpq.block_table.at(hash->block_index);
 if(!(block.flags&0x300) && !(block.flags&0x10000)){if(block.size!=block.compressed_size)throw std::runtime_error("Raw MPQ size mismatch");bytes.resize(block.size);archive.paged.seek(block.data_offset);archive.paged.get_bytes(bytes.data(),bytes.size());}
 else archive(bytes,name);return true;
}
