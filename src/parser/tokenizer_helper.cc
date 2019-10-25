#include "parser/parser-support.h"

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

LineInfo GetLineInfo(const char* st, const char* cur) {
  if (cur < st) {
    std::cerr << "Problem!" << __FILE__ << " " << __LINE__ << "\n";
    exit(255);
  }
  LineInfo res {1, 0};
  const char* last_start = st;
  for (; st < cur; ++st) {
    if (*st == '\n') {
      last_start = st + 1;
      res.line += 1;
    }
  }
  res.col = 1 + (cur - last_start);
  return res;
}

