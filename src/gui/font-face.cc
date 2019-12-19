#include "gui/font-face.h"

#include <fontconfig/fontconfig.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include FT_ADVANCES_H

namespace gui {

const char* GetFont(const char* font_description) {
	static FcConfig* config = FcInitLoadConfigAndFonts();
	//make pattern from font name
	FcPattern* pat = FcNameParse((const FcChar8*)font_description);
	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
	const char* fontFile = nullptr; //this is what we'd return if this was a function
	// find the font
	FcResult result;
	FcPattern* font = FcFontMatch(config, pat, &result);
	if (font) {
		FcChar8* file = NULL;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
			// We found the font, now print it.
			// This might be a fallback font.
      fontFile = (const char*)file;
		}
	}
	FcPatternDestroy(pat);
  fprintf(stderr, "font found: %s\n", fontFile);
  return fontFile;
}

const char* GetFTErrorString(FT_Error error) {
  switch (error) {
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s ) \
case e : return s;
#define FT_ERROR_START_LIST
#define FT_ERROR_END_LIST
#include FT_ERRORS_H
default:
  fprintf(stderr, "UNKNOWN FREETYPE_ERROR_CODE: %d", (int)error);
  exit(EXIT_FAILURE);
  }
}

#define FT_CHECK(arg) \
    do { \
      FT_Error error__FT_CHECK_INTERNAL = arg;\
      if (error__FT_CHECK_INTERNAL) { \
        fprintf(stderr, \
          "CHECK(" #arg "): %s:%d: error:%d: \"%s\"", \
                __FILE__, __LINE__, (int)error__FT_CHECK_INTERNAL, GetFTErrorString(error__FT_CHECK_INTERNAL)); \
      } \
    } while (0)

FontLayoutFace::FontLayoutFace(const char* name) {
  FT_CHECK(FT_Init_FreeType(&library_));
  FT_CHECK(FT_New_Face(library_, 
                       name[0] == '/' ? name : GetFont(name),
                       0, &face_));
  if (face_->num_fixed_sizes > 0) {
    const auto& item = face_->available_sizes[0];
    height_ = item.height;
    FT_CHECK(FT_Select_Size(face_, 0));
  } else {
    fprintf(stderr, "Handle non-bitmap");
    exit(EXIT_FAILURE);

    FT_CHECK(FT_Set_Pixel_Sizes(face_, 32, 32));
    height_ = face_->height; 
  }
  // FT_CHECK(FT_Set_Char_Size(face, 0, ptSize, device_hdpi, device_vdpi ));

  cr_face_ = cairo_ft_font_face_create_for_ft_face(face_, 0);
}

FT_UInt FontLayoutFace::GetGlyph(FT_ULong c) {
  return FT_Get_Char_Index(face_, c); }

void FontLayoutFace::Draw(gui::DrawCtx& ctx, gui::Point& pt, cairo_glyph_t* glyphs, size_t lens) {
  Layout(pt, glyphs, lens);

  cairo_show_glyphs(ctx.cr(), glyphs, lens); 
};

double FontLayoutFace::GetWidth(FT_UInt glyph_index) {
  auto* slot = GetCachedGlyph(glyph_index);
  return slot->advance.x / 65536.0;
}

double FontLayoutFace::GetWidth(string_view text) {
  double total = 0;
  for (char c : text) total += GetWidth(GetGlyph(c));
  return total;
}

void FontLayoutFace::Layout(gui::Point& pt, cairo_glyph_t* glyphs, size_t lens) {
  //FT_GlyphSlot slot = face_->glyph;

  for (size_t i = 0; i < lens; ++i) {
	  glyphs[i].x = pt.x;
	  glyphs[i].y = pt.y + height_ - 3;

    auto* glyph = GetCachedGlyph(glyphs[i].index);
    pt.x += glyph->advance.x / 65536.0;
  }
}

void FontLayoutFace::LayoutWrap(gui::Point& pt, std::vector<cairo_glyph_t>& glyphs,
                  string_view text, double sx, double wrap_x) {
  for (size_t i = 0; i < text.size(); ++i) {
    uint32_t c = (uint8_t)(text[i]);
    if (c > 127) {
      int n = __builtin_clz(0xff ^ c) - 25;
      c = c & ((0x1 << (n + 5)) - 1);

      if (i < text.size() - n) {
        for (int j = 0; j < n; ++j,++i) {
          uint32_t c2 = text[i + 1];
          if ((c2 & 0xc0) != 0x80) {
            fprintf(stderr, "problem!\n");
            exit(EXIT_FAILURE);
          }
          c2 = (c2 & 0x3f);
          c = (c << 6) + c2;
        }
      } else {
        c = 0xff;
        i = text.size() - 1;
      }
      // printf("c: %d\n", c);
    }
    glyphs.emplace_back();
    auto& glyph = glyphs.back();
    glyph.index = GetGlyph(c);
    auto* slot = GetCachedGlyph(glyph.index);

    double dx = slot->advance.x / 65536.0;
    if (pt.x + dx > wrap_x) {
      pt.x = sx;
      pt.y += height();
    }
	  glyph.x = pt.x;
	  glyph.y = pt.y + height_ - 3;

    pt.x += dx;
  }
}

void FontLayoutFace::SetFace(gui::DrawCtx& ctx) {
  cairo_set_font_face(ctx.cr(), cr_face_);
  cairo_set_font_size(ctx.cr(), 18.0);
}

void FontLayoutFace::Flush(cairo_t* ctx, const std::vector<cairo_glyph_t>& glyphs) {
  cairo_set_font_face(ctx, cr_face_);
  cairo_set_font_size(ctx, 18.0);
  cairo_show_glyphs(ctx, glyphs.data(), glyphs.size());
}

void FontLayoutFace::Flush(cairo_t* ctx, const cairo_glyph_t* data, size_t len) {
  cairo_set_font_face(ctx, cr_face_);
  cairo_set_font_size(ctx, 18.0);
  cairo_show_glyphs(ctx, data, len);
}

FT_Glyph FontLayoutFace::GetCachedGlyph(FT_UInt glyph_id) {
  if (glyph_id <= 0) {
    fprintf(stderr, "problem!\n");
  }
  if (glyph_id < cached_glyphs_.size()) {
    if (cached_glyphs_[glyph_id] != nullptr) return cached_glyphs_[glyph_id];
  } else {
    if (glyph_id > 2000) {
      fprintf(stderr, "resizing glyph cache to: %d\n", glyph_id);
    }
    cached_glyphs_.resize(glyph_id + 1, nullptr);
  }
  FT_Glyph result;
  FT_CHECK(FT_Load_Glyph(face_, glyph_id, FT_LOAD_DEFAULT ));
  FT_CHECK(FT_Get_Glyph(face_->glyph, &result));
  cached_glyphs_[glyph_id] = result;
  return result;
}

FontLayoutFace::~FontLayoutFace() {
  for (FT_Glyph glyph : cached_glyphs_) {
    if (glyph != nullptr) FT_Done_Glyph(glyph);
  }
}

FontLayoutFace* DefaultFont() {
  static FontLayoutFace face{"/usr/share/fonts/X11/misc/ter-u18n_unicode.pcf.gz"};
  return &face;
}
FontLayoutFace* DefaultBoldFont() {
  static FontLayoutFace face{"/usr/share/fonts/X11/misc/ter-u18b_unicode.pcf.gz"};
  return &face;
}

}  // namespace gui
