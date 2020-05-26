#include "wm/sub-window.h"
#include "gui/so-handle.h"
#include "gui/so-handoff-lib.h"
#include "wm/cross-process-transfer.h"
#include "parser/ast-context.h"

#include <iostream>
#include <stdlib.h>

ADD_TRANSFER_TYPE(SubWindow, 3);

void DrawDecorations(gui::DrawCtx& cr, gui::Rectangle pos) {
  cr.set_source_rgb(1.0, 1.0, 1.0);
  cr.rectangle(pos.st, {static_cast<double>(pos.shape.w), static_cast<double>(pos.shape.h)});

  double x1 = pos.st.x + 3;
  double x2 = pos.st.x + static_cast<double>(pos.shape.w - 3);
  double y1 = pos.st.y + 25;
  double y2 = pos.st.y + static_cast<double>(pos.shape.h - 3);
  cr.move_to({x1, y1});
  cr.line_to({x1, y2});
  cr.line_to({x2, y2});
  cr.line_to({x2, y1});
  cr.line_to({x1, y1});
  cr.fill();
}

gui::Rectangle InsideDecoration(gui::Rectangle pos) {
  return {{pos.st.x + 3, pos.st.y + 25}, {pos.shape.w - 6, pos.shape.h - 28}};
}

gui::Rectangle TranslateIntoRect(gui::Rectangle rect) {
  return {{0,0}, rect.shape};
}

bool GetResizeHover(gui::Rectangle rect, gui::Point pt_orig, size_t& dir) {
  size_t bw = 3; // Border width
  size_t cw = 12; // Corner width
  gui::Point pt = pt_orig - rect.st;
  gui::Point shape = rect.shape.AsPoint();
  if (pt.x >= 0 && pt.y >= 0 && pt.x < shape.x && pt.y < shape.y) {
    if (pt.x < bw || pt.y < bw || pt.x >= shape.x - bw || pt.y >= shape.y - bw) {
      if (pt.x < cw && pt.y < cw) { dir = 0; }
      else if (pt.x >= shape.x - cw && pt.y < cw) { dir = 2; }
      else if (pt.x >= shape.x - cw && pt.y >= shape.y - cw) { dir = 4; }
      else if (pt.x < cw && pt.y >= shape.y - cw) { dir = 6; }
      else if (pt.y < bw) { dir = 1; }
      else if (pt.x >= shape.x - bw) { dir = 3; }
      else if (pt.y >= shape.y - bw) { dir = 5; }
      else if (pt.x < bw) { dir = 7; }
      else {
        std::cerr << "GetResizeHover logic error.";
        __builtin_trap();
      }
      return true;
    }
  }
  return false;
}

struct UnrefCursor {
  void operator()(GdkCursor* cursor) {
    if (cursor) g_object_unref(cursor);
  }
};

struct CursorSet {
  CursorSet() { for (size_t i = 0; i < 8; ++i) { resize.emplace_back(nullptr); } }
  std::vector<std::unique_ptr<GdkCursor, UnrefCursor>> resize;
  std::unique_ptr<GdkCursor, UnrefCursor> grabbing;
  std::unique_ptr<GdkCursor, UnrefCursor> wait;

  void Init(GdkDisplay* window) {
    grabbing.reset(gdk_cursor_new_from_name(window, "grabbing"));
    wait.reset(gdk_cursor_new_from_name(window, "wait"));
    resize.clear();
    for (const char* name : {
         "nw-resize", "n-resize", "ne-resize", "e-resize",
         "se-resize", "s-resize", "sw-resize", "w-resize"
      }) {
      resize.emplace_back(gdk_cursor_new_from_name(window, name));
    }
  }
};

gui::Rectangle SubWindow::rect() const { return TranslateIntoRect(InsideDecoration(decorated_rect)); }

struct ResizeDragger : public Dragger {
 public:
  ResizeDragger(SubWindow* window, size_t dir, gui::Point pt) : window(window), dir(dir), pt(pt) {}
  SubWindow* window;
  size_t dir;
  gui::Point pt;
  void Drag(gui::Point pt2) override {
    gui::Point diff = pt2.Floor() - pt.Floor();
    pt = pt2.Floor();
    if (dir >= 2 && dir <= 4) {
      window->decorated_rect.shape.w += diff.x;
    }
    if (dir >= 4 && dir <= 6) {
      window->decorated_rect.shape.h += diff.y;
      if (window->decorated_rect.shape.h <= 30) {
        int error = window->decorated_rect.shape.h - 30;
        window->decorated_rect.shape.h = 30;
        pt.y -= error;
      }
    }
    if (dir >= 0 && dir <= 2) {
      window->decorated_rect.st.y += diff.y;
      window->decorated_rect.shape.h -= diff.y;
    }
    if (dir >= 6 || dir == 0) {
      window->decorated_rect.st.x += diff.x;
      window->decorated_rect.shape.w -= diff.x;
    }
  }
};

gui::Rectangle DefaultRectangle() {
  gui::Rectangle rect0;
      rect0.st.x = 300;
      rect0.st.y = 300;

      rect0.shape.w = 500;
      rect0.shape.h = 400;
      return rect0;
}

class WindowDragger : public Dragger {
 public:
  SubWindow* window;
  gui::Point pt;
  WindowDragger(SubWindow* window, gui::Point pt) : window(window), pt(pt) {}
  void Drag(gui::Point pt2) override {
    gui::Point diff = pt2.Floor() - pt.Floor();
    pt = pt2.Floor();
    window->decorated_rect.st += diff;
  }
};

class WindowState : public BasicWindowState {
 public:
  CursorSet cursors;

  std::vector<std::unique_ptr<SubWindow>> windows;
  std::unique_ptr<Dragger> dragger;
  
  void set_cursor(GdkCursor* cursor) {
    gdk_window_set_cursor(gtk_widget_get_window(window), cursor);
  }
  WindowState() {
    InitBasicState();
    InitEvents();
    cursors.Init(gtk_widget_get_display(window));
  }
  WindowState(GtkWidget* window, GtkWidget* drawing_area, size_t window_width, size_t window_height, bool is_fullscreen_, GdkSeat *seat) {
    this->window = window;
    this->drawing_area = drawing_area;
    this->window_width_ = window_width;
    this->window_height_ = window_height;
    this->is_fullscreen_ = is_fullscreen_;
    this->seat = seat;
    InitEvents();
    cursors.Init(gtk_widget_get_display(window));
  }
  gui::Point last_hover_pt;
  size_t GetHoverPt() {
    auto pt = last_hover_pt;
    for (size_t i = 0; i < windows.size(); ++i) {
      if (gui::TestInside(windows[i]->decorated_rect, pt)) {
        return i;
      }
    }
    return 0;
  }
  SubWindow* GetWindow() const {
    auto pt = last_hover_pt;
    for (size_t i = 0; i < windows.size(); ++i) {
      if (gui::TestInside(windows[i]->decorated_rect, pt)) {
        return windows[i].get();
      }
    }
    return nullptr;
  }

  void MoveToTop(size_t i) {
    if (windows.empty()) return;
    auto tmp = std::move(windows[i]);
    windows.erase(windows.begin() + i);
    windows.insert(windows.begin(), std::move(tmp));
  }
  void MoveToBottom(size_t i) {
    if (windows.empty()) return;
    auto tmp = std::move(windows[i]);
    windows.erase(windows.begin() + i);
    windows.insert(windows.end(), std::move(tmp));
  }
  bool HandleWmEvents(GdkEventKey* event) {
    if (event->state & GDK_MOD1_MASK) {
      size_t last_hover = GetHoverPt();
      if (event->keyval == 'r') {
        MoveToBottom(last_hover);
        return true;
      } else if (event->keyval == 'e') {
        MoveToTop(last_hover);
        return true;
      } else if (event->keyval == 's') {
        UngrabSeat();
        system("xdotool set_desktop 1");
        return true;
      } else if (event->keyval == 'd') {
        UngrabSeat();
        system("xdotool set_desktop 2");
        return true;
      } else if (event->keyval == 'f') {
        UngrabSeat();
        system("xdotool set_desktop 3");
        return true;
      }
    }
    if ((event->state & GDK_SHIFT_MASK) && (event->state & GDK_CONTROL_MASK) && event->keyval == 'N') {
      auto window = MakeCommandWindow();
      window->wm = this;
      auto& rect0 = window->decorated_rect;
      rect0.st.x = 300;
      rect0.st.y = 300;

      rect0.shape.w = 500;
      rect0.shape.h = 400;

      AddWindow(std::move(window));
      return true;
    }
    return false;
  }

  void AddWindow(std::unique_ptr<SubWindow> window) {
    windows.insert(windows.begin(), std::move(window));
  }

  void DoActualHandoff() {
    fprintf(stderr, "[so-reloader] ?? doing load number: %zu\n", main::GetJumpId());
    const char* so_name = (main::GetJumpId() % 2) == 0 ? "/tmp/gui-so-red.so" : "/tmp/gui-so-black.so";
    const char* base_so_name = ".build/ide-wm.so";
    fprintf(stderr, "[so-reloader] Loading from: %s\n", so_name);

    link(base_so_name, so_name);
    SoHandle handle(so_name, RTLD_NOW | RTLD_LOCAL);
    unlink(so_name);

    BufferContext ctx;
    std::vector<OpaqueTransferRef> items;
    for (auto& window : windows) items.push_back(window->encode(ctx));

    handle.get_sym<void(GtkWidget*,GtkWidget*,size_t,size_t, bool, GdkSeat*, const std::vector<OpaqueTransferRef>&
                        )>("gtk_transfer_window")(window, drawing_area,
                                              window_width_, window_height_, is_fullscreen_, seat, items);
    main::SwapToNewSoFile(std::move(handle));
  }

  void RefreshBinary() {
    g_idle_add((GSourceFunc)((+[](WindowState* state) {
      state->redraw();
      state->DeregisterEvents();
      state->DoActualHandoff();
      delete state;
      return FALSE;
    })), this);
  }

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

    for (size_t i = state->windows.size(); i > 0;) { 
      --i;
      auto& window = state->windows[i];
      auto rect = window->decorated_rect;
      DrawDecorations(cr, rect);
      SaveClipTranslate(cr, InsideDecoration(rect));
      window->Draw(cr);
      cr.restore();
    }

    state->needs_redraw = false;

    return TRUE;
  })), this);
  SigConnect(window, "button-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* button, WindowState* state) -> gboolean {
    bool is_event = (button->time != state->press_time);
    state->press_time = button->time;
    if (is_event) {
      RestoreEventXY button_restore(button);
      gui::Point pt{button->x, button->y};

      size_t num_windows = state->windows.size();
      for (size_t i = 0; i < num_windows; ++i) {
        auto& window = state->windows[i];
        gui::Rectangle rect = window->decorated_rect;
        if (gui::TestInside(rect, pt)) {
          size_t dir;
          if (GetResizeHover(rect, pt, dir)) {
            state->dragger.reset(new ResizeDragger(window.get(), dir, pt));
            break;
          } else if (pt.y < rect.st.y + 25) {
            state->dragger.reset(new WindowDragger(window.get(), pt));
            state->set_cursor(state->cursors.grabbing.get());
            break;
          }
          if (i != 0) {
            state->MoveToTop(i);
            state->redraw();
          }
          auto tmp = InsideDecoration(rect).st;
          button->x -= tmp.x;
          button->y -= tmp.y;
          
          window->ButtonPress(button);
          break;
        }
      }
    }
    return TRUE;
  })), this);
  SigConnect(window, "button-release-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* event, WindowState* state) -> gboolean {
    state->dragger = nullptr;
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

    if (state->HandleSpecialEvents(event) || state->HandleWmEvents(event)) {
      state->redraw();
    } else if (auto* window = state->GetWindow()) {
      window->KeyPress(event);
    }

    return TRUE;
  })), this);
  SigConnect(window, "scroll-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventScroll* event, WindowState* state) -> gboolean {
    if (auto* window = state->GetWindow()) {
      window->ScrollEvent(event);
    }

    return TRUE;
  })), this);
  SigConnect(window, "motion-notify-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventMotion* event, WindowState* state) -> gboolean {
    gui::Point pt{event->x, event->y};


    if (state->dragger) {
      state->dragger->Drag(pt);
      state->redraw();
    } else {
      bool found = false;
      size_t num_windows = state->windows.size();
      for (size_t i = 0; i < num_windows; ++i) {
        auto& window = state->windows[i];
        gui::Rectangle rect = window->decorated_rect;
        if (gui::TestInside(rect, pt)) {
          found = true;
          size_t dir;
          if (GetResizeHover(rect, pt, dir)) {
            state->set_cursor(state->cursors.resize[dir].get());
          } else {
            state->set_cursor(nullptr);
          }
          break;
        }
      }
      if (!found) {
        state->set_cursor(nullptr);
      }
      state->last_hover_pt = pt;
    }

    return TRUE;
  })), this);

  }

  uint32_t press_time = -1;
};

void SubWindow::AddWindow(std::unique_ptr<SubWindow> window) {
  window->wm = wm;
  wm->AddWindow(std::move(window));
}

void SubWindow::redraw() {
  wm->redraw();
}

void SubWindow::RefreshBinary() {
  wm->RefreshBinary();
}

extern "C" void gtk_transfer_window(GtkWidget* window, GtkWidget* drawing_area,
                                    size_t width, size_t height, bool is_fullscreen, GdkSeat* seat,
                                    const std::vector<OpaqueTransferRef>& items) {
  auto* window2 = new WindowState(window, drawing_area, width, height, is_fullscreen, seat);

  for (auto& t : items) {
    auto tmp = t.Load<SubWindow>();
    if (tmp) {
      tmp->wm = window2;
      window2->windows.push_back(std::move(tmp));
    }
  }
}

struct SillyBase {
  virtual const char* name() const = 0;
  virtual ~SillyBase() {}
  virtual void doThing() const = 0;

  virtual OpaqueTransferRef encode(BufferContext& context) = 0;
};

struct SillyTypeContext {
  int silly;
};

struct SillyType : public SillyBase {
  explicit SillyType(int i) : i_(i) {} 
  explicit SillyType(const SillyTypeContext& ctx) : i_(ctx.silly) {}
  ~SillyType() override {}
  const char* name() const override { return "SillyType"; }
  void doThing() const override { printf("Silly: %d\n", i_); }

  OpaqueTransferRef encode(BufferContext& context) override;

  int i_;
};

ADD_TRANSFER_TYPE(SillyBase, 0);
ADD_TRANSFER_TYPE(SillyType, 1);
ADD_SUBCLASS(SillyType, SillyBase);
ADD_BASIC_DECODER(SillyTypeContext, SillyType, 0);

OpaqueTransferRef SillyType::encode(BufferContext& context) {
  return context.encode(SillyTypeContext{i_});
}

extern "C" void dl_plugin_entry(int argc, char **argv) {
  /*
  auto* silly = new SillyType(10);
  BufferContext ctx;
  auto tmp = silly->encode(ctx);

  delete silly;
  tmp.Load<SillyType>()->doThing();

  exit(0);
  */
  if (argc > 1) {
    fprintf(stderr, "calling(%s): %s\n", argv[0], argv[1]);
    new WindowState(); // argv[1]);
  } else {
    new WindowState(); // ".gui/data");
  }
}
