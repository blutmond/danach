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

struct ChunkViewState {
  std::string title;

  BufferPos cursor {0, 0};
  size_t buffer_id_ = 0;
  size_t float_pos = string_view::npos;
  double scroll = 0.0;

  virtual ~ChunkViewState() {}
  virtual Buffer* GetBuffer(size_t buffer_id) = 0;
  virtual size_t num_buffers() = 0;

  virtual void Draw(gui::DrawCtx& cr, size_t window_height, size_t window_width) = 0;
  virtual bool FindChunkViewPos(gui::Point pt, size_t& buffer_id_, BufferPos& cursor) = 0;

  void SanitizeCursor() {
    if (buffer_id_ == 0 && num_buffers() == 0) return;
    const auto& lines = GetBuffer(buffer_id_)->lines;
    if (cursor.row >= lines.size()) {
      cursor.row = lines.size() - 1;
    }
    if (cursor.col > lines[cursor.row].size()) {
      cursor.col = lines[cursor.row].size();
    }
  }
};

bool DoKeyPress(ChunkViewState& view, uint32_t keyval);
bool DoKeyPress(Buffer& buffer, BufferPos& pos, size_t& float_pos, uint32_t keyval);
bool DoEscapeKeyPress(ChunkViewState& view, uint32_t keyval);

struct ChunkView;
void DrawView(gui::DrawCtx& cr, const ChunkView& view, size_t window_height, size_t window_width);

struct ChunkView : public ChunkViewState {
  std::vector<Buffer*> buffers;
  std::vector<Decoration> decorations;
  std::vector<size_t> gaps;

  Buffer* GetBuffer(size_t buffer_id) override { return buffers[buffer_id]; }
  size_t num_buffers() override { return buffers.size(); }

  void Draw(gui::DrawCtx& cr, size_t window_height, size_t window_width) override {
    DrawView(cr, *this, window_height, window_width);
  }

  bool FindChunkViewPos(gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override;
};

bool FindPos(gui::Point& st, gui::Point pt, gui::FontLayoutFace* font, BufferPos& cursor, const Buffer& buffer);
