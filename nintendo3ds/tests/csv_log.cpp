#include "csv_log.h"
#include <cassert>
#include <fstream>
#include <string>
#include <cstdio>
#include <unistd.h>
int main(){
 char path[]="/tmp/openbw-csv-XXXXXX";int fd=mkstemp(path);assert(fd>=0);close(fd);
 port3ds::CsvLog log;assert(!log.open("/does-not-exist/openbw.csv","frame\n"));assert(log.stats().errors);
 assert(log.open(path,"frame\n"));unsigned accepted=0;
 for(unsigned n=0;n<10000;++n){auto row=std::to_string(n)+"\n";if(log.write(row.c_str()))++accepted;}
 log.flush();log.close();log.close();assert(!log && !log.stats().pending && !log.stats().errors);
 std::ifstream f(path);std::string line;std::getline(f,line);assert(line=="frame");unsigned count=0,previous=0;
 while(std::getline(f,line)){auto n=std::stoul(line);if(count)assert(n>previous);previous=n;++count;}
 assert(count==accepted && count+log.stats().dropped==10000);std::remove(path);
 std::puts("Async CSV ordered queue / overflow accounting / drain-on-exit / failed open / idempotent close PASS");
}
