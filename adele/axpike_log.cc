#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include "processor.h"
#include "axpike_log.h"

AxPIKE::LogFile::LogFile(const char* file) {
  this->fp.open(file, std::ios::out | std::ios::trunc);
  fname = file;
}

AxPIKE::LogFile::~LogFile() {
  this->fp.close();
}


void AxPIKE::LogFile::write(std::string& msg) {
  fp << msg;
}

void AxPIKE::LogFile::write(const char* format, std::va_list& args) {


  char* buff = new char[1024];
  int n;
  std::va_list targs;
  va_copy (targs, args);

  n = std::vsnprintf(buff, 1024, format, targs);
  va_end(targs);
  if (n > 1023) {
    delete[] buff;
    buff = new char[n+1];
    std::vsnprintf(buff, n+1, format, args);
    fp << buff;
  }
  if (n > 0) {
    fp << buff;
  }
  else {
    fp << "Format error on printf-like log output" << std::endl;
  }
}

void AxPIKE::LogFile::write(std::ostream& msg) {
  fp << msg.rdbuf();
}

AxPIKE::Log::Log(processor_t *p) {
  this->suffix = "_pid" + std::to_string(::getpid()) + "_hart" + std::to_string(p->get_id()) + ".log";
}

AxPIKE::Log::~Log() {
  for (auto& f : files) {
    delete f.second;
  }
}

AxPIKE::LogFile* AxPIKE::Log::get(const char* file) {
  LogFile* log;
  try {
    log = files.at(file);
  }
  catch (const std::out_of_range& e) {
    std::string fname = file;
    fname = fname + this->suffix;
    log = new LogFile(fname.c_str());
    files[file] = log;
  }
  return log;
}

void AxPIKE::Log::write(const char* file, std::string& msg) {
  LogFile* log = this->get(file);
  log->write(msg);
};

void AxPIKE::Log::write(const char* file, const char* format, ...) {
  LogFile* log = this->get(file);
  std::va_list args;
  va_start(args, format);
  log->write(format, args);
  va_end(args);
};

void AxPIKE::Log::write(const char* file, std::ostream& msg) {
  LogFile* log = this->get(file);
  log->write(msg);
};

AxPIKE::CustomCounters::Counters::~Counters() {
  if (counters.empty()) {
    std::cerr << "There are no AxPIKE custom counters to be written." << std::endl;
  }
  else {
    std::fstream fp;
    std::string file = "AxPIKE_CustomCounters_pid" + std::to_string(::getpid()) + "_hart" + std::to_string(p->get_id()) + ".csv";
    std::cerr << "Writing AxPIKE custom counters to " << file << std::endl;
    fp.open(file.c_str(), std::ios::out | std::ios::trunc);
    fp << "\"Type\", \"Name\", \"Value\"" << std::endl;

    for (auto& counter : counters) {
      for (auto& c: *(counter.second)) {
        fp << "\"" << counter.first << "\", \"" << c.first << "\", ";
        (c.second)->save(fp);
        fp << std::endl;
      }
    }

    fp.close();
  }
}

