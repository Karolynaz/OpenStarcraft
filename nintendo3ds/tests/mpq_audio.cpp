#include "data_loading.h"
#include <cassert>
#include <cstdio>
using namespace bwgame;using namespace bwgame::data_loading;
int main(){
 for(unsigned length:{14u,16u}){
  // One mono ADPCM sector, both a partial and exact full final sector.
  const unsigned samples=length/2,compressed=8+1+4+samples-1;
  a_vector<uint8_t> bytes(compressed,0x80);set_value_at<true,uint32_t>(bytes.data(),8);set_value_at<true,uint32_t>(bytes.data()+4,compressed);
  bytes[8]=0x40;bytes[9]=0;bytes[10]=0;bytes[11]=0x34;bytes[12]=0x12;
  data_reader_le base(bytes.data(),bytes.data()+bytes.size());auto table=get_crypt_table();
  mpq_archive_file_reader<data_reader_le> reader("synthetic ADPCM",base,16,{0,compressed,length,0x200},0,table);
  uint8_t out[18];std::fill_n(out,18,0x55);reader.get_bytes(out,length);
  for(unsigned i=0;i<length;i+=2)assert((value_at<uint16_t,true>(out+i)==0x1234));
  assert(out[length]==0x55 && out[length+1]==0x55);
 }
 uint8_t encoded[]={0,0,0x34,0x12,0x80};uint8_t small[5]={0,0,0x55,0x55,0x55};bool rejected=false;
 try{decompress_adpcm(encoded,sizeof encoded,small,3,1);}catch(const std::exception&){rejected=true;}
 assert(rejected && small[2]==0x55 && small[3]==0x55 && small[4]==0x55);
 std::puts("MPQ exact/partial ADPCM sectors / compression-prefix length / odd output bounds PASS");
}
