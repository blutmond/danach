#include "gui/buffer-edit.h"

double GetLineNumberRenderingWidth(size_t n_lines, gui::FontLayoutFace* font) {
  double max_width = 0.0;
  for (int i = '0'; i <= '9'; ++i) {
    max_width = std::max(max_width, font->GetWidth(font->GetGlyph(i)));
  }
  n_lines = std::max<size_t>(1, n_lines);
  size_t log_10 = 0;
  for (; n_lines > 0; n_lines /= 10) ++log_10;
  return max_width * log_10 + 4;
}

void LayoutLineNumbers(gui::Point st, std::vector<cairo_glyph_t>& glyphs,
                       gui::FontLayoutFace* font, size_t i, double width) {
  char line_numbers[100];
  size_t ln_width = 0;
  for (size_t i_copy = i; i_copy > 0; i_copy /= 10) {
    ++ln_width;
    line_numbers[100 - ln_width] = '0' + (i_copy % 10);
  }

  font->LayoutWrap(st, glyphs, string_view(&line_numbers[100 - ln_width], ln_width));
  double extra = (width - 2) - st.x;
  for (auto& glyph : glyphs) glyph.x += extra;
}

void LayoutTwidle(gui::Point st, std::vector<cairo_glyph_t>& glyphs,
                  gui::FontLayoutFace* font) {
  char twidle[] = {'~'};
  font->LayoutWrap(st, glyphs, string_view(twidle, 1));
}

void DoDrawLines(gui::DrawCtx& cr, gui::Point st,
                 double width, size_t start_line, size_t num_lines, size_t window_height,
                 gui::FontLayoutFace* font) {
  std::vector<cairo_glyph_t> glyphs;
  size_t i = start_line;
  size_t end_line = i + num_lines;
  if (st.y < 0) {
    size_t n_hidden = static_cast<size_t>(-st.y / font->height()); 
    st.y += font->height() * n_hidden;
    i += n_hidden;
  }
  while (st.y < window_height && i < end_line) {
    glyphs.clear();
    if (i < end_line) LayoutLineNumbers(st, glyphs, font, i, width);
    else LayoutTwidle(st, glyphs, font);
    font->Flush(cr, glyphs);
    st.y += font->height();
    i += 1;
  }
}

void DoDrawTwidle(gui::DrawCtx& cr, gui::Point st,
                  size_t window_height, gui::FontLayoutFace* font) {
  std::vector<cairo_glyph_t> glyphs;
  if (st.y < 0) {
    size_t n_hidden = static_cast<size_t>(-st.y / font->height()); 
    st.y += font->height() * n_hidden;
  }
  while (st.y < window_height) {
    glyphs.clear();
    LayoutTwidle(st, glyphs, font);
    font->Flush(cr, glyphs);
    st.y += font->height();
  }
}

void FlushLineWithCursor(gui::DrawCtx& cr, gui::Point st, gui::FontLayoutFace* font,
                         std::vector<cairo_glyph_t>& glyphs, size_t col) {
  font->Flush(cr, glyphs.data(), col);
  if (col < glyphs.size()) {
    font->Flush(cr, glyphs.data() + (col + 1), glyphs.size() - col - 1);

    cr.set_source_rgb(1.0, 1.0, 0.0);
    cr.rectangle(gui::Point{glyphs[col].x, st.y},
                 {(col + 1 == glyphs.size() ? st.x : glyphs[col + 1].x) - glyphs[col].x,
                  font->height()});
    cr.fill();

    cr.set_source_rgb(0.0, 0.0, 0.0);
    cairo_show_glyphs(cr.cr(), glyphs.data() + col, 1);
    cr.set_source_rgb(1.0, 1.0, 0.0);
  } else {
    cr.set_source_rgb(1.0, 1.0, 0.0);
    cr.rectangle(st, {9, font->height()});
    cr.fill();
  }
}

void DrawBuffer(gui::DrawCtx& cr, gui::Point st, const Buffer& buffer, BufferPos* cursor) {
  auto* font = gui::DefaultFont();
  std::vector<cairo_glyph_t> glyphs;
  size_t pos_row = cursor ? cursor->row : 0;
  cr.set_source_rgb(1.0, 1.0, 0.0);
  auto original_x = st.x;
  size_t row = 0;
  // const auto& lines = buffer.lines;
  for (auto& line: buffer.lines) {
    glyphs.clear();
    font->LayoutWrap(st, glyphs, line);
    if (row == pos_row && cursor) {
      FlushLineWithCursor(cr, st, font, glyphs, cursor->col);
    } else {
      font->Flush(cr, glyphs);
    }
    st.y += font->height();
    st.x = original_x;
    ++row;
  }
}
