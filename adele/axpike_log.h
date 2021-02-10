#ifndef _AXPIKE_LOG_H_
#define _AXPIKE_LOG_H_

#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdarg>

class processor_t;

namespace AxPIKE {
  class LogFile {
    private:
      std::ofstream fp;
      std::string fname;
    public:
      LogFile(const char* file);
      ~LogFile();
      
      void write(std::string& msg);
      void write(const char* format, std::va_list& args);
      void write(std::ostream& msg);
  };

  class Log {
    private:
      std::string suffix;
      std::unordered_map<const char*, LogFile*> files;

      LogFile* get(const char* file);
    public:
      Log(processor_t* p);
      ~Log();

      void write(const char* file, std::string& msg);
      void write(const char* file, const char* format, ...);
      void write(const char* file, std::ostream& msg);
  };
}


#endif
