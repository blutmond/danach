#pragma once

#include "gui/widget-helper.h"
#include "gui/buffer-view.h"
#include "notes/type-support.h"

struct TypeRef;
class GenericCursor {
 public:
  virtual ~GenericCursor() {}
  virtual bool IsCursorFor(tptr other) { return false; }
  virtual void FoundTypeRef(TypeRef* ref) { }
  virtual void FoundPointerType(tptr ptr) {
    if (ptr.is_a<TypeRef>()) FoundTypeRef(&ptr.get<TypeRef>());
  }
  virtual void press(uint32_t keyval) {}
  virtual void sanitize() {}
  virtual BufferPos* GetCursor(Buffer& ref) { return nullptr; }
};

struct Cursor : public GenericCursor {
  // TODO: Make this some sort of string lens...
  std::string& ref;
  size_t pos;
  Cursor(std::string& ref, size_t pos) : ref(ref), pos(pos) {}

  bool IsCursorFor(tptr other) override { return &ref == &other.get<std::string>(); }
  void press(uint32_t keyval) override {
    if (keyval >= ' ' && keyval <= '~') {
      ref.insert(ref.begin() + pos, keyval);
      pos += 1;
    } else if (keyval == GDK_KEY_Left && pos > 0) {
      pos -= 1;
    } else if (keyval == GDK_KEY_BackSpace && pos > 0) {
      pos -= 1;
      ref.erase(ref.begin() + pos);
    } else if (keyval == GDK_KEY_Delete && pos < ref.size()) {
      ref.erase(ref.begin() + pos);
    } else if (keyval == GDK_KEY_Right && pos < ref.size()) {
      pos += 1;
    }
  }
};

struct BufferCursor : public GenericCursor {
  Buffer& buffer;
  BufferPos cursor;
  size_t float_pos = string_view::npos;
  BufferCursor(Buffer& ref, BufferPos cursor) : buffer(ref), cursor(cursor) {}
  
  BufferPos* GetCursor(Buffer& oref) override { return &oref == &buffer ? &cursor : nullptr; }

  void press(uint32_t keyval) override {
    DoKeyPress(buffer, cursor, float_pos, keyval);
  }
};

class Tool : public GenericCursor {};

struct CursorState {
 public:
  std::unique_ptr<GenericCursor> active = nullptr;
};

struct LayoutLineState {
  gui::Shape shape;
  LayoutLineState(gui::Shape shape) : shape(shape) {}
  gui::FontLayoutFace* font = gui::DefaultFont();
  std::vector<cairo_glyph_t> glyphs;
  gui::Point pt{2,2};
  double reset_point = 2;
  bool just_reset = true;
  void nl() {
    just_reset = true;
    pt.x = reset_point;
    pt.y += font->height();
  }
  void layout(string_view str) {
    just_reset = false;
    font->LayoutWrap(pt, glyphs, str);
  }
  virtual ~LayoutLineState() {}
  virtual void flush() { glyphs.clear(); }
};

struct ResetScope {
  ResetScope(LayoutLineState& s) : s(s) {
    old_pos = s.reset_point;
    s.reset_point = s.pt.x;
  }
  ~ResetScope() {
    s.reset_point = old_pos;
    if (s.just_reset) s.pt.x = old_pos;
  }
  LayoutLineState& s;
  double old_pos;
};

struct DrawLineState : public LayoutLineState {
  gui::DrawCtx& cr;
  DrawLineState(gui::DrawCtx& cr, gui::Shape shape) : LayoutLineState(shape), cr(cr) {}
  void flush() override {
    font->Flush(cr, glyphs);
    glyphs.clear();
  }
};
