#include <string>
#include <experimental/string_view>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include <algorithm>
#include <functional>

using std::experimental::string_view;

#include "parser/line_number_helper.cc"

std::string LoadFile(const std::string& filename) {
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (!in) {
    fprintf(stderr, "Could not read file: %s\n", filename.c_str());
    exit(-1);
  }
  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();
  return contents;
}
