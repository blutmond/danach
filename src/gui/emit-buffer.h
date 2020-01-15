#pragma once


#include "gui/buffer.h"

struct CollapsedBuffer {
  size_t id;
  std::string text;
};

std::vector<CollapsedBuffer> Collapse(const std::vector<ParsedIdBuffer>& src);
void EmitFromMultiBuffer(const std::vector<CollapsedBuffer>& buffers);
