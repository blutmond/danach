#pragma once

#include <experimental/string_view>
#include <string>

using std::experimental::string_view;

std::string Unescaped(string_view data);
