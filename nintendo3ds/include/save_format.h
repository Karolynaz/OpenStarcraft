#pragma once
#include "skirmish.h"
#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <functional>
namespace port3ds {
struct SaveView {float camera_x=0,camera_y=0,cursor_x=200,cursor_y=120;bool auto_camera=false;};
struct SaveInfo {unsigned frame=0,scenario=0;bool raw_map=false;MatchSetup setup;SaveView view;};
namespace savegame {
constexpr unsigned header_size=256,record_size=24,max_map=2*1024*1024,max_events=1000000,max_frames=24*60*60*12;
enum Op:unsigned {select=1,clear,activate,army,workers,store,recall,command,perform,move,cancel,fog,surrender,elapsed,rectangle,similar,last=similar};
inline uint32_t get(const unsigned char* p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
inline void put(unsigned char* p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=v>>(8*i);}
inline uint32_t hash(uint32_t h,const void* data,size_t n){auto* p=static_cast<const unsigned char*>(data);while(n--){h^=*p++;h*=16777619u;}return h;}
struct Header {
 std::array<unsigned char,header_size> bytes{};
 unsigned field(unsigned i)const{return get(bytes.data()+8+i*4);}void field(unsigned i,unsigned v){put(bytes.data()+8+i*4,v);}
 void view(const SaveView& v){float a[]={v.camera_x,v.camera_y,v.cursor_x,v.cursor_y};for(unsigned i=0;i<4;++i){uint32_t b;std::memcpy(&b,&a[i],4);field(9+i,b);}field(13,v.auto_camera);}
 SaveInfo info()const{SaveInfo i;i.frame=field(1);i.scenario=field(6);i.raw_map=field(4);i.setup.enabled=field(14);i.setup.difficulty=AiDifficulty(field(15));i.setup.game_type=field(16);i.setup.observer=field(17);for(unsigned p=0;p<3;++p){i.setup.races[p]=int32_t(field(18+p));i.setup.teams[p]=field(21+p);i.setup.side_difficulty[p]=int32_t(field(24+p));}float* a[]={&i.view.camera_x,&i.view.camera_y,&i.view.cursor_x,&i.view.cursor_y};for(unsigned p=0;p<4;++p){auto b=field(9+p);std::memcpy(a[p],&b,4);}i.view.auto_camera=field(13);return i;}
 void setup(const SaveInfo& i){field(6,i.scenario);field(14,i.setup.enabled);field(15,unsigned(i.setup.difficulty));field(16,i.setup.game_type);field(17,i.setup.observer);for(unsigned p=0;p<3;++p){field(18+p,i.setup.races[p]);field(21+p,i.setup.teams[p]);field(24+p,i.setup.side_difficulty[p]);}}
};
struct Record {unsigned frame=0,op=0;int a=0,b=0,c=0,d=0;};
inline void encode(const Record& r,unsigned char* b){put(b,r.frame);put(b+4,r.op);put(b+8,r.a);put(b+12,r.b);put(b+16,r.c);put(b+20,r.d);}
inline Record decode(const unsigned char* b){return {get(b),get(b+4),int32_t(get(b+8)),int32_t(get(b+12)),int32_t(get(b+16)),int32_t(get(b+20))};}
inline bool copy(FILE* from,FILE* to,size_t bytes,uint32_t* digest=nullptr){unsigned char buffer[4096];while(bytes){auto n=std::min(bytes,sizeof buffer);if(std::fread(buffer,1,n,from)!=n)return false;if(digest)*digest=hash(*digest,buffer,n);if(to && std::fwrite(buffer,1,n,to)!=n)return false;bytes-=n;}return true;}
class Writer {
 FILE* file_=nullptr;public:unsigned events=0,depth=0;Header header;std::string path;bool failed=false;
 ~Writer(){close();}void close(){if(file_)std::fclose(file_);file_=nullptr;events=depth=0;failed=false;}
 bool active()const{return file_ && !failed;}bool flush(){if(!active())return false;if(std::fflush(file_)){failed=true;return false;}return true;}
 bool open(const char* name,const Header& h,FILE* map){close();header=h;path=name;file_=std::fopen(name,"wb+");if(!file_)return false;failed=std::fwrite(h.bytes.data(),1,header_size,file_)!=header_size || !copy(map,file_,h.field(3));if(failed){close();return false;}return true;}
 bool clone(const char* save,const char* session,const Header& h){close();FILE* in=std::fopen(save,"rb");if(!in)return false;file_=std::fopen(session,"wb+");bool ok=file_ && copy(in,file_,header_size+h.field(3)+size_t(h.field(2))*record_size);std::fclose(in);if(!ok){close();return false;}header=h;events=h.field(2);path=session;return true;}
 void append(const Record& r){if(!active())return;if(events>=max_events || r.frame>max_frames){failed=true;return;}unsigned char b[record_size];encode(r,b);if(std::fwrite(b,1,sizeof b,file_)!=sizeof b)failed=true;else ++events;}
 struct Guard {Writer& w;Guard(Writer& writer,Record r):w(writer){if(w.depth++==0)w.append(r);}~Guard(){--w.depth;}};
};
}
}
