#include "gui/buffer-view.h"

#include <gtk/gtk.h>

bool DoKeyPress(ChunkView& view, uint32_t keyval) {
    auto& buffer_id_ = view.buffer_id_;
    auto& buffers = view.buffers;
    auto& buffer = *buffers[buffer_id_];
    auto& float_pos = view.float_pos;
    auto& cursor = view.cursor;
    bool float_pos_dirty = true;
    const auto& lines = buffer.lines;
    auto& pos = cursor;
    auto clip_col = [&]() {
      // TODO: Do decoration based clipping.
      if (float_pos == string_view::npos) {
        float_pos = pos.col;
      }
      pos.col = std::min(float_pos, buffers[buffer_id_]->lines[pos.row].size());
      float_pos_dirty = false;
    };
    if (keyval >= ' ' && keyval <= '~') {
      Insert(buffer, cursor, keyval);
    } else if (keyval == GDK_KEY_Left && pos.col > 0) {
      pos.col -= 1;
    } else if (keyval == GDK_KEY_Right && 
               pos.col < lines[pos.row].size()) {
      pos.col += 1;
    } else if (keyval == GDK_KEY_Down && pos.row + 1 < lines.size()) {
      pos.row += 1;
      clip_col();
    } else if (keyval == GDK_KEY_Down && buffer_id_ + 1 < buffers.size()) {
      pos.row = 0;
      buffer_id_ += 1;
      clip_col();
    } else if (keyval == GDK_KEY_Up && pos.row > 0) {
      pos.row -= 1;
      clip_col();
    } else if (keyval == GDK_KEY_Up && buffer_id_ > 0) {
      buffer_id_ -= 1;
      pos.row = buffers[buffer_id_]->lines.size() - 1;
      clip_col();
    } else if (keyval == GDK_KEY_Return) {
      Insert(buffer, cursor, '\n');
    } else if (keyval == GDK_KEY_BackSpace &&
               (pos.row > 0 || pos.col > 0)) {
      auto new_pos = pos;
      if (new_pos.col == 0) {
        new_pos.row -= 1;
        new_pos.col = lines[new_pos.row].size();
      } else {
        new_pos.col -= 1;
      }
      Delete(buffer, new_pos, pos);
      pos = new_pos;
    } else if (keyval == GDK_KEY_Delete &&
               (pos.col < lines[pos.row].size() || pos.row + 1 < lines.size())) {
      auto new_pos = pos;
      if (new_pos.col == lines[pos.row].size()) {
        new_pos.row += 1;
        new_pos.col = 0;
      } else {
        new_pos.col += 1;
      }
      Delete(buffer, pos, new_pos);
    } else if (keyval == GDK_KEY_Home) {
      pos.col = 0;
    } else if (keyval == GDK_KEY_End) {
      pos.col = lines[pos.row].size();
    } else {
      return false;
    }
    if (float_pos_dirty) float_pos = string_view::npos;
    return true;
}

void DrawView(gui::DrawCtx& cr, const ChunkView& view, size_t window_height, size_t window_width) {
  auto* font = gui::DefaultFont();
  const auto& buffers = view.buffers;
  const auto& gaps = view.gaps;
  const auto& decorations = view.decorations;
  const auto& scroll = view.scroll;
  auto buffer_id = view.buffer_id_;
  auto cursor = view.cursor;
  size_t num_lines = 0;
  for (size_t i = 0; i < buffers.size(); ++i) {
    num_lines += buffers[i]->lines.size();
  }
  double width = GetLineNumberRenderingWidth(std::max<size_t>(100, num_lines), font);

  gui::Point st {2 + width, 2 + scroll};

  size_t line_no = 1;
  for (size_t i = 0; i < buffers.size(); ++i) {
    const auto& buffer = *buffers[i];
    if (i > 0) st.y += gaps[i - 1];
    cr.set_source_rgb(1.0, 0.0, 0.0);
    DoDrawLines(cr, {2, st.y}, width, line_no, buffer.lines.size(), window_height, font);
    cr.set_source_rgb(1.0, 1.0, 0.0);
    st.x = 2 + width;
    decorations[i].Draw(cr, st, font->height(), buffer.lines.size());
    
    DrawBuffer(cr, st, buffer, buffer_id == i ? &cursor : nullptr);
    st.y += font->height() * buffer.lines.size();
    line_no += buffer.lines.size();
  }
  
  cr.set_source_rgb(1.0, 0.0, 0.0);
  DoDrawTwidle(cr, {2, st.y}, window_height, font);
}
