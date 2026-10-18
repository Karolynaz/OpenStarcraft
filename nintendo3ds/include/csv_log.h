#pragma once
#include <memory>
namespace port3ds {
struct CsvStats {unsigned flush_us=0,dropped=0,errors=0,pending=0;};
class CsvLog {
 struct Impl;std::unique_ptr<Impl> impl_;
public:
 CsvLog();~CsvLog();bool open(const char* path,const char* header);void close();
 explicit operator bool() const;bool write(const char* row);void flush();CsvStats stats()const;
};
}
