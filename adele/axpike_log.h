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

  namespace CustomCounters {

    class GenericCounter {
      public:
        virtual void save(std::fstream& stream) {};
    };

    template<typename T>
    class WrappedCounter : public GenericCounter {
      public:
        T value;
        WrappedCounter() {};
        WrappedCounter(const T& init) : value(init) {};
        void save(std::fstream& stream) {stream << value;};
    };

    class Counters {
      private:
        std::unordered_map<std::string, std::unordered_map<std::string, GenericCounter*>*> counters;
        processor_t* p;

      public:
        Counters(processor_t* _p) : p(_p) {};
        ~Counters();

        template<typename T>
        T& get(const std::string& type, const std::string& key) {
          std::unordered_map<std::string, GenericCounter*>* countertype;
          WrappedCounter<T>* counter;

          try {
            countertype = counters.at(type);
          }
          catch (const std::out_of_range& e) {
            countertype = new std::unordered_map<std::string, GenericCounter*>;
            counters[type] = countertype;
          }

          try {
            counter = static_cast<WrappedCounter<T>*>(countertype->at(key));
          }
          catch (const std::out_of_range& e) {
            
            try {
              counter = new WrappedCounter<T>(0);
            }
            catch(...) {
              counter = new WrappedCounter<T>();
            }

            (*countertype)[key] = counter;
          }

          return counter->value;
        }
    };

  }

}


#endif
