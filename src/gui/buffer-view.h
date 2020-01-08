#pragma once

#include "gui/buffer-edit.h"

struct Decoration {
  void Draw(gui::DrawCtx& cr, gui::Point& st, double line_height, size_t num_lines) const {
    if (!toplevel) {
      cr.move_to({st.x + 13, st.y + 6});
      cr.line_to({st.x + 13, st.y + line_height * num_lines});
      cr.stroke();
      st.x += 20;
    }
  }
  void Adjust(gui::Point& st, double line_height, size_t num_lines) const {
    if (!toplevel) {
      st.x += 20;
    }
  }
  bool toplevel = true;
};

struct ChunkView {
  std::string title;

  BufferPos cursor {0, 0};
  size_t buffer_id_ = 0;
  size_t float_pos = string_view::npos;
  double scroll = 0.0;

  std::vector<Buffer*> buffers;
  std::vector<Decoration> decorations;
  std::vector<size_t> gaps;

  void SanitizeCursor() {
    const auto& lines = buffers[buffer_id_]->lines;
    if (cursor.row >= lines.size()) {
      cursor.row = lines.size() - 1;
    }
    if (cursor.col > lines[cursor.row].size()) {
      cursor.col = lines[cursor.row].size();
    }
  }
};

bool DoKeyPress(ChunkView& view, uint32_t keyval);

void DrawView(gui::DrawCtx& cr, const ChunkView& view, size_t window_height, size_t window_width);
