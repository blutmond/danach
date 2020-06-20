#pragma once

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

bool FileExists(const std::string& filename);

std::string LoadFile(const std::string& filename);

struct LineInfo {
  int line;
  int col;
};

LineInfo GetLineInfo(const char* st, const char* cur);
