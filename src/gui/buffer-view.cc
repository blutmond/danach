#include "gui/buffer-view.h"

#include <gtk/gtk.h>

bool DoEscapeKeyPress(ChunkViewState& view, uint32_t keyval) {
  auto& buffer_id_ = view.buffer_id_;
  auto& buffer = *view.GetBuffer(view.buffer_id_);
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
    pos.col = std::min(float_pos, view.GetBuffer(buffer_id_)->lines[pos.row].size());
    float_pos_dirty = false;
  };
  if (keyval == GDK_KEY_Left && pos.col > 0) {
    pos.col -= 1;
  } else if (keyval == GDK_KEY_Right && 
             pos.col < lines[pos.row].size()) {
    pos.col += 1;
  } else if (keyval == GDK_KEY_Down && pos.row + 1 < lines.size()) {
    pos.row += 1;
    clip_col();
  } else if (keyval == GDK_KEY_Down && buffer_id_ + 1 < view.num_buffers()) {
    pos.row = 0;
    buffer_id_ += 1;
    clip_col();
  } else if (keyval == GDK_KEY_Up && pos.row > 0) {
    pos.row -= 1;
    clip_col();
  } else if (keyval == GDK_KEY_Up && buffer_id_ > 0) {
    buffer_id_ -= 1;
    pos.row = view.GetBuffer(buffer_id_)->lines.size() - 1;
    clip_col();
  } else {
    return false;
  }
  if (float_pos_dirty) float_pos = string_view::npos;
  return true;
}

bool DoKeyPress(Buffer& buffer, BufferPos& cursor, size_t& float_pos, uint32_t keyval) {
  bool float_pos_dirty = true;
  const auto& lines = buffer.lines;
  auto& pos = cursor;
  auto clip_col = [&]() {
    // TODO: Do decoration based clipping.
    if (float_pos == string_view::npos) {
      float_pos = pos.col;
    }
    pos.col = std::min(float_pos, buffer.lines[pos.row].size());
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
  } else if (keyval == GDK_KEY_Up && pos.row > 0) {
    pos.row -= 1;
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

bool DoKeyPress(ChunkViewState& view, uint32_t keyval) {
  if (view.num_buffers() == 0) return false;
  auto& buffer_id_ = view.buffer_id_;
  auto& buffer = *view.GetBuffer(view.buffer_id_);
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
    pos.col = std::min(float_pos, view.GetBuffer(buffer_id_)->lines[pos.row].size());
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
  } else if (keyval == GDK_KEY_Down && buffer_id_ + 1 < view.num_buffers()) {
    pos.row = 0;
    buffer_id_ += 1;
    clip_col();
  } else if (keyval == GDK_KEY_Up && pos.row > 0) {
    pos.row -= 1;
    clip_col();
  } else if (keyval == GDK_KEY_Up && buffer_id_ > 0) {
    buffer_id_ -= 1;
    pos.row = view.GetBuffer(buffer_id_)->lines.size() - 1;
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

bool ChunkView::FindChunkViewPos(gui::Point pt, size_t& buffer_id_, BufferPos& cursor) {
  auto* font = gui::DefaultFont();

  size_t num_lines = 0;
  for (size_t i = 0; i < buffers.size(); ++i) {
    num_lines += buffers[i]->lines.size();
  } // + buffers[1].lines.size();
  double width = GetLineNumberRenderingWidth(std::max<size_t>(100, num_lines), font);

  gui::Point st {2 + width, 2 + scroll};

  for (size_t i = 0; i < buffers.size(); ++i) {
    const auto& buffer = *buffers[i];
    // auto& decoration = decorations[i];
    if (i > 0) st.y += gaps[i - 1];
    st.x = 2 + width;
    decorations[i].Adjust(st, font->height(), buffer.lines.size());

    if (FindPos(st, pt, font, cursor, buffer)) { buffer_id_ = i; return true; }
  }
  return false;
}

bool FindPos(gui::Point& st, gui::Point pt, gui::FontLayoutFace* font, BufferPos& cursor, const Buffer& buffer) {
  double segment_height = font->height() * buffer.lines.size();
  size_t click_line = 0;
  if (pt.y > st.y) {
    click_line = static_cast<ssize_t>((pt.y - st.y) / font->height());
  }
  if (click_line < buffer.lines.size()) {
    std::vector<cairo_glyph_t> glyphs;
    st.y += font->height() * click_line;
    font->LayoutWrap(st, glyphs, buffer.lines[click_line]);
    cursor.col = glyphs.size();
    for (size_t j = 1; j < glyphs.size(); ++j) {
      // fprintf(stderr, "error: %g %g\n", pt.x, glyphs[j].x);
      if (pt.x < glyphs[j].x) {
        cursor.col = j - 1;
        break;
      }
    }
    cursor.row = click_line;
    return true;
  }
  st.y += segment_height;
  return false;
}
