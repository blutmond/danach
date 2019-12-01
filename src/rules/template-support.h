#pragma once

#include <fstream>
#include <sstream>
#include <iostream>

class EmitStream {
 public:
  std::ostream& stream() { return ss; }

  void write(const std::string& fname) {
    ss.flush();
    std::ofstream ofs(fname, std::ofstream::out | std::ofstream::trunc);
    ofs << ss.str();
    ofs.close();
  }
 private:
  std::string data;
  std::stringstream ss{data};
};
