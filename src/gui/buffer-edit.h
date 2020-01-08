#pragma once

#include "gui/font-face.h"
#include "gui/buffer.h"

double GetLineNumberRenderingWidth(size_t n_lines, gui::FontLayoutFace* font);
void LayoutLineNumbers(gui::Point st, std::vector<cairo_glyph_t>& glyphs,
                       gui::FontLayoutFace* font, size_t i, double width);
void LayoutTwidle(gui::Point st, std::vector<cairo_glyph_t>& glyphs,
                  gui::FontLayoutFace* font);
void DoDrawLines(gui::DrawCtx& cr, gui::Point st,
                 double width, size_t start_line, size_t num_lines, size_t window_height,
                 gui::FontLayoutFace* font);
void DoDrawTwidle(gui::DrawCtx& cr, gui::Point st,
                  size_t window_height, gui::FontLayoutFace* font);
void FlushLineWithCursor(gui::DrawCtx& cr, gui::Point st, gui::FontLayoutFace* font,
                         std::vector<cairo_glyph_t>& glyphs, size_t col);
void DrawBuffer(gui::DrawCtx& cr, gui::Point st, const Buffer& buffer, BufferPos* cursor);
