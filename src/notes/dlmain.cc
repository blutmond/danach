#include "gui/so-handle.h"
#include "gui/widget-helper.h"
#include "gui/so-handoff-lib.h"
#include "parser/ast-context.h"

#include <iostream>
#include <algorithm>
#include <stdlib.h>

// Progression:
//  Some basic draw tree components (layouts etc)
//  - xml is basically a layout / draw tree. We want the same here.
//   (Basically want to construct new draw tree nodes types as functions).

#define notimpl(name) throw std::logic_error(#name " not implemented")
struct DrawEvalContext {};

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
  virtual void flush() {}
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

class Obj {
 public:
  using C = DrawEvalContext;
  virtual ~Obj() {}
  virtual void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) { notimpl(draw); }
  virtual void button_press(C ectx, gui::Shape shape, GdkEventButton* button) {}
  virtual void layout_text(C ectx, LayoutLineState& state) { notimpl(layout_text); }

  virtual void visit(int ocolor) {}
  void tick(int ncolor) {
    if (color == ncolor) return;
    color = ncolor;
    visit(color);
  }
  struct TraceContext {
    int color = 0;
    std::vector<std::unique_ptr<Obj>> objs;
    void DoGC(std::vector<Obj*> roots) {
      color += 1;
      for (auto* root : roots) {
        root->tick(color);
      }
      objs.erase(std::remove_if(objs.begin(), objs.end(), [this](std::unique_ptr<Obj>& o) -> bool {
        return o->color != color;
      }), objs.end());
    }
    template<class T, class... Args >
    T* make(Args&&... args) {
      auto* out = new T(std::forward<Args>(args)...);
      objs.emplace_back(out);
      out->ctx = this;
      return out;
    }
  };
  template<class T, class... Args >
  T* make(Args&&... args) {
    auto* out = new T(std::forward<Args>(args)...);
    ctx->objs.emplace_back(out);
    out->ctx = ctx;
    return out;
  }
 private:
  int color = 0;
  TraceContext* ctx;
};

class BoxItem : public Obj {
 public:
  gui::ColorRGB color = {0, 1, 0};
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    cr.set_source_rgb(color);
    cr.paint();
  }
};

class TextLayout : public Obj {
 public:
  std::vector<Obj*> children;
  void visit(int color) override { for (auto* child: children) child->tick(color); }
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    DrawLineState state(cr, shape);
    for (auto* child : children) {
      child->layout_text(ectx, state);
    }
  }
};

class ConstantString : public Obj {
 public:
  explicit ConstantString(std::string value) : value(std::move(value)) {}
  std::string value;
  void layout_text(C ectx, LayoutLineState& state) override {
    state.layout(value);
    state.flush();
  }
};

class PrintfClickBox : public Obj {
 public:
  PrintfClickBox(Obj* child) : child(child) {}
  Obj* child;
  void visit(int color) override { child->tick(color); }
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    child->draw(ectx, cr, shape);  
  }
  void button_press(C ectx, gui::Shape shape, GdkEventButton* button) override {
    fprintf(stderr, "On click: %g %g\n", button->x, button->y);
  }
};

class RenderListThing : public Obj {
 public:
  std::vector<size_t> items;
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    DrawLineState state(cr, shape);
    cr.set_source_rgb(1, 1, 0);
    for (size_t v : items) {
      state.layout("-- silly: ");
      char txt[30];
      size_t i = 30;
      while (true) {
        i -= 1;
        txt[i] = (v % 10) + '0';
        v /= 10;
        if (v == 0) break;
      }
      state.layout(string_view(&txt[i], 30 - i));
      state.flush();
      state.nl();
      state.nl();
    }
  }
  void button_press(C ectx, gui::Shape shape, GdkEventButton* button) override {
    items.push_back(items.size());
  }
};

class LayoutNl : public Obj {
 public:
  LayoutNl() {}
  void layout_text(C ectx, LayoutLineState& state) override {
    state.nl();
  }
};

class ColorScope : public Obj {
 public:
  ColorScope(Obj* child, gui::ColorRGB color) : child(child), color(color) {}
  Obj* child;
  gui::ColorRGB color;
  void visit(int color) override { child->tick(color); }
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    cr.save();
    cr.set_source_rgb(color);
    child->draw(ectx, cr, shape);
    cr.restore();
  }
};

class HSplit : public Obj {
 public:
  std::vector<Obj*> children;
  void visit(int color) override { for (auto* child: children) child->tick(color); }
  void draw(C ectx, gui::DrawCtx& cr, gui::Shape shape) override {
    for (size_t i = 0; i < children.size(); ++i) {
      gui::Rectangle cbox;
      cbox.st.x = float((shape.w * i) / children.size());
      cbox.st.y = 0;
      cbox.shape.w = (shape.w * (i + 1)) / children.size() - (shape.w * i) / children.size();
      cbox.shape.h = shape.h;
      SaveClipTranslate(cr, cbox);
      children[i]->draw(ectx, cr, cbox.shape);
      cr.restore();
    }
  }
  void button_press(C ectx, gui::Shape shape, GdkEventButton* button) override {
    for (size_t i = 0; i < children.size(); ++i) {
      gui::Rectangle cbox;
      cbox.st.x = float((shape.w * i) / children.size());
      cbox.st.y = 0;
      cbox.shape.w = (shape.w * (i + 1)) / children.size() - (shape.w * i) / children.size();
      cbox.shape.h = shape.h;
      if (TestRectangleClick(button, cbox)) children[i]->button_press(ectx, cbox.shape, button);
    }
  }
};

class WindowState : public BasicWindowState {
 public:
  WindowState() {
    auto thing = ctx.make<HSplit>();
    auto thing1 = ctx.make<BoxItem>();
    auto text = ctx.make<TextLayout>();
    auto thing2 = ctx.make<ColorScope>(text, gui::ColorRGB{1, 1, 0});
    text->children.push_back(ctx.make<ConstantString>("hello world: "));
    text->children.push_back(ctx.make<ConstantString>("other item"));
    text->children.push_back(ctx.make<LayoutNl>());
    text->children.push_back(ctx.make<ConstantString>("Bad thing..."));
    text->children.push_back(ctx.make<LayoutNl>());

    thing->children.push_back(thing1);
    thing->children.push_back(ctx.make<PrintfClickBox>(thing2));
    thing->children.push_back(thing1);
    thing->children.push_back(ctx.make<RenderListThing>());
    thing->children.push_back(ctx.make<PrintfClickBox>(thing2));
    root = thing; 
    InitBasicState();
    InitEvents();
  }

  Obj::TraceContext ctx;
  Obj* root;

  void DoGC() {
    ctx.DoGC({root});
  }
  gui::Shape shape() { return {int(window_width_), int(window_height_)}; }
  void InitEvents() {
  SigConnect(window, "destroy", G_CALLBACK((
          +[](GtkWidget*, WindowState* state) -> gboolean {
    gtk_main_quit();
    delete state;
    return TRUE;
  })), this);
  SigConnect(window, "draw", G_CALLBACK((
          +[](GtkWidget*, cairo_t* cr_ptr, WindowState* state) -> gboolean {
    auto& cr = *gui::DrawCtx::wrap(cr_ptr);
    cr.set_source_rgb(0.0, 0.0, 0.2);
    cr.paint();
    state->needs_redraw = false;
    state->root->draw({}, cr, state->shape());
    // auto& ctx = state->ctx;
    return TRUE;
  })), this);
  SigConnect(window, "button-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* button, WindowState* state) -> gboolean {
    bool is_event = (button->time != state->press_time);
    state->press_time = button->time;
    if (is_event) {
      RestoreEventXY button_restore(button);
      gui::Point pt{button->x, button->y};
      state->root->button_press({}, state->shape(), button);
      state->redraw();
    }
    return TRUE;
  })), this);
  SigConnect(window, "button-release-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* event, WindowState* state) -> gboolean {
    return TRUE;
  })), this);
  SigConnect(window, "configure-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventConfigure* config, WindowState* state) -> gboolean {

    if (state->window_width_ != config->width || state->window_height_ != config->height) {
      state->window_width_ = config->width;
      state->window_height_ = config->height;
      state->redraw();
    }
    return FALSE;

    return TRUE;
  })), this);
  SigConnect(window, "key-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventKey* event, WindowState* state) -> gboolean {

    if (state->HandleSpecialEvents(event)) { 
    } else {
      state->redraw();
    }
    return TRUE;
  })), this);
  SigConnect(window, "scroll-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventScroll* event, WindowState* state) -> gboolean {

    return TRUE;
  })), this);
  SigConnect(window, "motion-notify-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventMotion* event, WindowState* state) -> gboolean {
    gui::Point pt{event->x, event->y};

    return TRUE;
  })), this);

  }

  uint32_t press_time = -1;
};

extern "C" void dl_plugin_entry(int argc, char **argv) {
//  silly_test();
//  exit(0);
  if (argc > 1) {
    fprintf(stderr, "calling(%s): %s\n", argv[0], argv[1]);
    new WindowState(); // argv[1]);
  } else {
    new WindowState(); // ".gui/data");
  }
}
