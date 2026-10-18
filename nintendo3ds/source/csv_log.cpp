#include "csv_log.h"
#include <3ds.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
namespace port3ds {
struct CsvLog::Impl {
 static constexpr unsigned capacity=64,line_size=1400;
 char rows[capacity][line_size]{};std::string path,header;Thread thread=nullptr;
 std::atomic<unsigned> read{0},written{0},dropped{0},errors{0},flush_us{0};
 std::atomic<bool> running{false},ready{false},success{false},force{false};
 static void run(void* arg) {
  auto& s=*(Impl*)arg;FILE* f=std::fopen(s.path.c_str(),"a");
  if(!f){++s.errors;s.ready=true;return;}
  auto flush=[&](){const auto t=osGetTime();if(std::fflush(f))++s.errors;s.flush_us=unsigned((osGetTime()-t)*1000);};
  if(std::fputs(s.header.c_str(),f)==EOF)++s.errors;flush();s.success=s.errors==0;s.ready=true;
  unsigned since=0;
  while(s.running.load() || s.read.load()!=s.written.load()) {
   const auto at=s.read.load();
   if(at!=s.written.load()) {
    if(std::fputs(s.rows[at%capacity],f)==EOF)++s.errors;
    s.read=at+1;++since;
   } else svcSleepThread(5000000);
   if(since>=5 || s.force.exchange(false)){flush();since=0;}
  }
  flush();if(std::fclose(f))++s.errors;s.success=false;
 }
 void close(){running=false;if(thread){threadJoin(thread,U64_MAX);threadFree(thread);thread=nullptr;}success=false;}
 ~Impl(){close();}
};
CsvLog::CsvLog():impl_(new Impl){}CsvLog::~CsvLog()=default;
bool CsvLog::open(const char* path,const char* header) {
 auto& s=*impl_;s.close();s.read=0;s.written=0;s.dropped=0;s.errors=0;s.flush_us=0;s.ready=false;s.success=false;s.force=false;
 s.path=path;s.header=header;s.running=true;s.thread=threadCreate(Impl::run,&s,16*1024,0x32,0,false);
 if(!s.thread){s.running=false;++s.errors;return false;}
 while(!s.ready.load())svcSleepThread(1000000);
 if(!s.success.load()){s.close();return false;}return true;
}
void CsvLog::close(){impl_->close();}
CsvLog::operator bool()const{return impl_->success.load();}
bool CsvLog::write(const char* row){auto& s=*impl_;if(!s.success.load())return false;const auto at=s.written.load();
 if(at-s.read.load()>=Impl::capacity || std::strlen(row)>=Impl::line_size){++s.dropped;return false;}
 std::strcpy(s.rows[at%Impl::capacity],row);s.written=at+1;return true;
}
void CsvLog::flush(){impl_->force=true;}
CsvStats CsvLog::stats()const{const auto& s=*impl_;return {s.flush_us.load(),s.dropped.load(),s.errors.load(),s.written.load()-s.read.load()};}
}
