#include "rules/string-utils.h"

std::string Unescaped(string_view data) {
  std::string out;
  // TODO: This is bad (unsafe)
  data.remove_prefix(1);
  data.remove_suffix(1);
  size_t pos = data.find('\\');
  while (pos != string_view::npos) {
    out.append(data.data(), pos);
    data.remove_prefix(pos);
    data.remove_prefix(1);
    if (data[0] == 'n') {
      out.append(1, '\n');
      data.remove_prefix(1);
    } else if (data[0] == '\\') {
      out.append(1, '\\');
      data.remove_prefix(1);
    }
    pos = data.find('\\');
  }
  out.append(data.data(), data.size());
  return out;
}
