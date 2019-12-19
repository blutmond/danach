#pragma once

#include "gui/cairo-bindings.h"
#include <ft2build.h>
#include <vector>
#include "rules/string-utils.h"

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H

namespace gui {

class FontLayoutFace {
 public:
  explicit FontLayoutFace(const char* name);
  ~FontLayoutFace();

  FT_UInt GetGlyph(FT_ULong c);

  double height() const { return height_; }

  double GetWidth(FT_UInt glyph_index);

  double GetWidth(string_view text);

  void Draw(gui::DrawCtx& ctx, gui::Point& pt, cairo_glyph_t* glyphs, size_t lens);
  
  void Layout(gui::Point& pt, cairo_glyph_t* glyphs, size_t lens);
  
  void LayoutWrap(gui::Point& pt, std::vector<cairo_glyph_t>& glyphs,
                  string_view text, double sx = 0.0, double wrap_x = 1e300);
  // , size_t lens);

  void SetFace(gui::DrawCtx& ctx);

  void Flush(gui::DrawCtx& ctx, const std::vector<cairo_glyph_t>& glyphs) {
    Flush(ctx.cr(), glyphs);
  }

  void Flush(cairo_t* ctx, const std::vector<cairo_glyph_t>& glyphs);

  void Flush(cairo_t* ctx, const cairo_glyph_t* data, size_t len);
  void Flush(gui::DrawCtx& ctx, const cairo_glyph_t* data, size_t len) {
    Flush(ctx.cr(), data, len);
  }

 private:
  FT_Glyph GetCachedGlyph(FT_UInt glyph_id);
  std::vector<FT_Glyph> cached_glyphs_;
  FT_Library  library_;   /* handle to library     */
  FT_Face     face_;      /* handle to face object */
  double height_;
  cairo_font_face_t* cr_face_;
};

FontLayoutFace* DefaultFont();
FontLayoutFace* DefaultBoldFont();

}  // namespace gui
