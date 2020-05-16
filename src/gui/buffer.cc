#include "gui/buffer.h"

#include <fstream>

void Buffer::Print() {
  for (const auto& ln : lines) {
    printf("ln: \"%s\"\n", ln.c_str());
  }
}

void Insert(Buffer& buffer, BufferPos& cursor, char c) {
  if (c == '\n') {
    buffer.lines.insert(buffer.lines.begin() + (cursor.row + 1), 
                         buffer.lines[cursor.row].substr(cursor.col));
    buffer.lines[cursor.row] = buffer.lines[cursor.row].substr(0, cursor.col);
    cursor.row += 1;
    cursor.col = 0;
  } else {
    auto& str = buffer.lines[cursor.row];
    str.insert(str.begin() + cursor.col, c);
    cursor.col += 1;
  }
}

void Insert(Buffer& buffer, BufferPos& cursor, string_view text) {
  for (char c : text) {
    Insert(buffer, cursor, c);
  }
}

void Delete(Buffer& buffer, BufferPos s_pos, BufferPos e_pos) {
  auto& lines_ = buffer.lines;
  if (s_pos.row == e_pos.row) {
    std::string& target = lines_[e_pos.row];
    if (!(s_pos.col < e_pos.col)) {
      fprintf(stderr, "error");
      exit(EXIT_FAILURE);
    }
    if (!(e_pos.col <= target.size())) {
      fprintf(stderr, "error");
      exit(EXIT_FAILURE);
    }
    target.erase(target.begin() + s_pos.col,
                 target.begin() + e_pos.col);
  } else {
    auto tmp = std::move(lines_[e_pos.row]);
    std::string& target = lines_[s_pos.row];
    target.resize(s_pos.col);
    // This also has a copy.
    target += tmp.substr(e_pos.col);

    lines_.erase(lines_.begin() + s_pos.row + 1,
                 lines_.begin() + e_pos.row + 1);
  }
  // buffer.lines.
}

void Init(Buffer& buffer, string_view text) {
  auto cursor = text;
  Buffer tmp;
  tmp.lines.clear();
  while (!cursor.empty()) {
    auto s = cursor.find('\n');
    auto a = cursor.substr(0, s);
    tmp.lines.push_back(std::string(a));
    cursor.remove_prefix(a.size());
    if (s != string_view::npos) cursor.remove_prefix(1);
  }
  if (tmp.lines.empty()) tmp.lines.push_back("");
  buffer = tmp;
}

void SaveFile(std::ostream& ss, const std::vector<IdBuffer>& buffers) {
  for (auto& buff : buffers) {
    ss << "#" << buff.id << "\n";
    for (auto& line : buff.buffer->lines) {
      if (!line.empty() && line[0] == '#') {
        ss << "#";
      }
      ss << line << "\n";
    }
  }
}

std::vector<ParsedIdBuffer> ParseMultiBuffer(string_view data) {
  std::vector<ParsedIdBuffer> out;

  auto cursor = data;
  while (!cursor.empty()) {
    if (cursor[0] == '#') {
      cursor.remove_prefix(1);
      auto s = cursor.find('\n');
      if (s == string_view::npos) {
        fprintf(stderr, "%d: Could not properly parse data.\n", __LINE__);
        return out;
      } else {
        int id = std::stoi(std::string(cursor.substr(0, s))); 
        cursor.remove_prefix(s);
        cursor.remove_prefix(1);
        Buffer tmp;
        tmp.lines.clear();
        while (!cursor.empty()) {
          if (cursor[0] == '#') {
            if (cursor.size() > 1 && cursor[1] != '#') {
              break;
            }
            cursor.remove_prefix(1);
          }
          auto s = cursor.find('\n');
          auto a = cursor.substr(0, s);
          tmp.lines.push_back(std::string(a));
          cursor.remove_prefix(a.size());
          if (s != string_view::npos) cursor.remove_prefix(1);
        }
        if (tmp.lines.empty()) tmp.lines.push_back("");
        out.push_back({static_cast<size_t>(id), std::move(tmp)});
      }
    } else {
      fprintf(stderr, "%d: Could not properly parse data.\n", __LINE__);
      std::cerr << "Rest:\n";
      std::cerr << cursor << "######||||||####\n";
      return out;
    }
  }
  // for line in data:
  return out;
}

std::string Collapse(const Buffer& buffer) {
  std::string text;
  for (const auto& line : buffer.lines) {
    text += line;
    text += "\n";
  }
  return text;
}
