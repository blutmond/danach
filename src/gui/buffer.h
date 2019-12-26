#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>

#include "parser/parser-support.h"

struct BufferPos {
  size_t row = 0;
  size_t col = 0;
};

struct Buffer {
  std::vector<std::string> lines{{""}};

  bool operator==(const Buffer& other) const {
    return lines == other.lines;
  }

  void Print();
};

void Insert(Buffer& buffer, BufferPos& cursor, char c);
void Insert(Buffer& buffer, BufferPos& cursor, string_view text);
void Delete(Buffer& buffer, BufferPos s_pos, BufferPos e_pos);
void Init(Buffer& buffer, string_view text);

struct IdBuffer {
  size_t id;
  Buffer* buffer;
};

void SaveFile(std::ostream& ss, const std::vector<IdBuffer>& buffers);

struct ParsedIdBuffer {
  size_t id;
  Buffer buffer;
  bool operator==(const ParsedIdBuffer& other) const {
    return other.id == id && other.buffer == buffer;
  }
};
std::vector<ParsedIdBuffer> ParseMultiBuffer(string_view data);
