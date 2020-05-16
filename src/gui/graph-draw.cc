#include "gui/so-handle.h"
#include "gui/so-handoff-lib.h"
#include "gui/cairo-bindings.h"
#include "gui/font-face.h"

#include <iostream>
#include <sstream>
#include <assert.h>
#include <gtk/gtk.h>


struct DagNode {
  std::string short_name;
  std::string desc;
  std::vector<DagNode*> deps;
  std::vector<DagNode*> uses;
};

DagNode* MakeNode(std::vector<std::unique_ptr<DagNode>>& resl,
                  const std::string& short_name,
                  const std::string& desc) {
  auto* res = new DagNode;
  resl.emplace_back(res);
  res->short_name = short_name;
  res->desc = desc;
  return res;
}

void AddDep(DagNode* a, DagNode* b) {
  a->deps.push_back(b);
  b->uses.push_back(a);
}

std::vector<std::unique_ptr<DagNode>> ExampleDag() {
  std::vector<std::unique_ptr<DagNode>> res;
  auto* node1 = MakeNode(res, "A", "Build rule 1");
  auto* node2 = MakeNode(res, "B", "Build rule 2");
  auto* node3 = MakeNode(res, "C", "Build rule 3");
  auto* node4 = MakeNode(res, "D", "Build rule 4");

  AddDep(node1, node2);
  AddDep(node1, node4);
  AddDep(node2, node3);
  AddDep(node3, node4);

  return res;
}

struct WindowState {
  GtkWidget* window;
  GtkWidget* drawing_area;

  size_t window_width_ = 1024 * 3 / 2;
  size_t window_height_ = 680 * 3 / 2;

  gulong sig1;
  gulong sig2;
  gulong sig3;
  gulong sig4;
  gulong sig5;
  gulong sig6;
  gulong sig7;

  int64_t hover_id = 2;

  explicit WindowState();
  explicit WindowState(GtkWidget* window, GtkWidget* drawing_area,
              size_t width, size_t height);

  gboolean MotionNotify(GdkEventMotion* motion);
  gboolean ScrollEvent(GdkEventScroll* event);
  gboolean Draw(gui::DrawCtx& cr);
  gboolean configure_event(GdkEventConfigure* config) {
    if (window_width_ != config->width || window_height_ != config->height) {
      window_width_ = config->width;
      window_height_ = config->height;
      redraw();
    }
    return TRUE;
  }

  gboolean key_press(GdkEventKey* event) {
    auto keyval = event->keyval;
    if (keyval == GDK_KEY_F5) {
      fprintf(stderr, "Doing recompile\n");
      int status = system(".build/rules src/gui ide-graph.so");
      if (status == 0) {
        g_signal_handler_disconnect(window, sig1);
        g_signal_handler_disconnect(window, sig2);
        g_signal_handler_disconnect(drawing_area, sig3);
        g_signal_handler_disconnect(window, sig4);
        g_signal_handler_disconnect(window, sig5);
        g_signal_handler_disconnect(window, sig6);
        g_signal_handler_disconnect(window, sig7);

        fprintf(stderr, "[so-reloader] ?? doing load number: %zu\n", main::GetJumpId());
        const char* so_name = (main::GetJumpId() % 2) == 0 ? "/tmp/gui-so-red.so" : "/tmp/gui-so-black.so";
        const char* base_so_name = ".build/ide-graph.so";
        fprintf(stderr, "[so-reloader] Loading from: %s\n", so_name);

        link(base_so_name, so_name);
        SoHandle handle(so_name, RTLD_NOW | RTLD_LOCAL);
        unlink(so_name);

        handle.get_sym<void(GtkWidget*,GtkWidget*,size_t,size_t, const char*)>("gtk_transfer_window")(window, drawing_area,
                                                                window_width_, window_height_, "silly");
        main::SwapToNewSoFile(std::move(handle));
        delete this;
      } else {
        fprintf(stderr, "Could not compile....\n");
      }
      return FALSE;
    }
    return TRUE;
  }

  uint32_t press_time = -1;
  gboolean ButtonPress(GdkEventButton* button) {
    gui::Point pt{button->x, button->y};

    return FALSE;
  }

  bool needs_redraw = true;
  void redraw() {
    if (!needs_redraw) {
      needs_redraw = true;
      gtk_widget_queue_draw(window);
    }
  }
  void InitEvents();
};

extern "C" void gtk_transfer_window(GtkWidget* window, GtkWidget* drawing_area,
                                    size_t width, size_t height, const char* filename) {
  new WindowState(window, drawing_area, width, height);
}

WindowState::WindowState() {
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawing_area, window_width_, window_height_);

  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_, window_height_);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  gtk_widget_show_all(window);
  InitEvents();
}

WindowState::WindowState(GtkWidget* window_inp, GtkWidget* drawing_area_inp,
                         size_t width, size_t height) {
  window = window_inp;
  drawing_area = drawing_area_inp;
  window_width_ = width;
  window_height_ = height;
  InitEvents();
  gtk_widget_queue_draw(window);
}

gboolean WindowState::Draw(gui::DrawCtx& cr) {
  cr.set_source_rgb(0.0, 0.0, 0.0);
  cr.paint();

  cr.set_source_rgb(1.0, 0.0, 0.0);
  auto* font = gui::DefaultFont();

  {
    std::vector<cairo_glyph_t> glyphs;
    gui::Point opt {2, 2};
    {
      gui::Point st = opt;
      cr.set_source_rgb(0.5, 1.0, 1.0);
      
      font->LayoutWrap(st, glyphs, "func ", 12, 300.0 - 2);
      font->LayoutWrap(st, glyphs, "something or another make a dag somehow", 12, 600.0 - 2);

      font->Flush(cr, glyphs);
      glyphs.clear();
      opt.y += font->height();
    }
    {
      gui::Point st = opt;
      st.x += 10;
      cr.set_source_rgb(0.5, 1.0, 1.0);
      
      font->LayoutWrap(st, glyphs, "%0 = ", 12, 300.0 - 2);
      font->LayoutWrap(st, glyphs, "int(0)", 12, 600.0 - 2);

      font->Flush(cr, glyphs);
      glyphs.clear();
      opt.y += font->height();
    }

    opt.y += font->height();


    auto dag = ExampleDag();
    size_t i = 0;
    DagNode* hover_node = nullptr;
    for (const auto& node : dag) {
      if ((i++) == hover_id) { hover_node = node.get(); }
    }
    for (const auto& node : dag) {
      gui::Point st = opt;
      cr.set_source_rgb(0.5, 1.0, 1.0);
      
      font->LayoutWrap(st, glyphs, node->short_name, 12, 300.0 - 2);
      if (hover_node == node.get()) {
        cr.set_source_rgb(1.0, 0.0, 0.0);
        font->Flush(cr, glyphs);
        glyphs.clear();
        cr.set_source_rgb(0.5, 1.0, 1.0);
      }

      font->LayoutWrap(st, glyphs, " : ", 12, 300.0 - 2);
      font->LayoutWrap(st, glyphs, node->desc, 12, 300.0 - 2);
      
      opt.y = st.y + font->height();
      st = opt;
      font->LayoutWrap(st, glyphs, " deps = [", 12, 300.0 - 2);
      size_t i = 0;
      for (auto* dep : node->deps) {
        if ((i++) != 0) {
          font->LayoutWrap(st, glyphs, ", ", 12, 300.0 - 2);
        }
        if (hover_node == dep) {
          font->Flush(cr, glyphs);
          glyphs.clear();
          font->LayoutWrap(st, glyphs, dep->short_name, 12, 300.0 - 2);
          cr.set_source_rgb(1.0, 0.0, 0.0);
          font->Flush(cr, glyphs);
          glyphs.clear();
          cr.set_source_rgb(0.5, 1.0, 1.0);
        } else {
        font->LayoutWrap(st, glyphs, dep->short_name, 12, 300.0 - 2);
        }
      }
      font->LayoutWrap(st, glyphs, "];", 12, 300.0 - 2);

      font->Flush(cr, glyphs);
      glyphs.clear();
      opt.y += font->height();
    }
  }

  needs_redraw = false;

  return TRUE;
}

gboolean WindowState::MotionNotify(GdkEventMotion* motion) {
  gui::Point pt{motion->x, motion->y};

  DagNode* hover_node = nullptr;

  auto* font = gui::DefaultFont();

    std::vector<cairo_glyph_t> glyphs;
    gui::Point opt {2, 2 + font->height() * 3};

    auto dag = ExampleDag();
    for (const auto& node : dag) {
      gui::Point st = opt;
      
      gui::Point a = st;
      font->LayoutWrap(st, glyphs, node->short_name, 12, 300.0 - 2);
      auto b = st + gui::Point{0, font->height()};
      if (a.x <= pt.x && pt.x <= b.x &&
          a.y <= pt.y && pt.y <= b.y) {
        hover_node = node.get();
      }

      font->LayoutWrap(st, glyphs, " : ", 12, 300.0 - 2);
      font->LayoutWrap(st, glyphs, node->desc, 12, 300.0 - 2);
      
      opt.y = st.y + font->height();
      st = opt;
      font->LayoutWrap(st, glyphs, " deps = [", 12, 300.0 - 2);
      size_t i = 0;
      for (auto* dep : node->deps) {
        if ((i++) != 0) {
          font->LayoutWrap(st, glyphs, ", ", 12, 300.0 - 2);
        }
      gui::Point a = st;
      font->LayoutWrap(st, glyphs, node->short_name, 12, 300.0 - 2);
      auto b = st + gui::Point{0, font->height()};
      if (a.x <= pt.x && pt.x <= b.x &&
          a.y <= pt.y && pt.y <= b.y) {
        hover_node = dep;
      }
      }
      font->LayoutWrap(st, glyphs, "];", 12, 300.0 - 2);

      glyphs.clear();
      opt.y += font->height();
    }

    size_t i = 0;
    int64_t old_hover_id = hover_id;
    hover_id = -1;
    for (const auto& node : dag) {
      if (node.get() == hover_node) { hover_id = i; }
      ++i;
    }
    if (old_hover_id != hover_id) {
      redraw();
    }

//  ... highlight ...

  return TRUE;
}

gboolean WindowState::ScrollEvent(GdkEventScroll* event) {
  redraw();
  return TRUE;
}

void WindowState::InitEvents() {
  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);

  sig4 = g_signal_connect(window, "scroll-event", G_CALLBACK((
              +[](GtkWidget* widget, GdkEventScroll* event, WindowState* state) -> gboolean {
              return state->ScrollEvent(event);
              })), this);

  sig6 = g_signal_connect(window, "button-press-event", G_CALLBACK((
              +[](GtkWidget* widget, GdkEventButton* button, WindowState* state) -> gboolean {
                bool is_event = (button->time != state->press_time);
                state->press_time = button->time;
                if (is_event) return state->ButtonPress(button);
                return TRUE;
              })), this);

  sig3 = g_signal_connect(drawing_area, "draw", G_CALLBACK((
              +[](GtkWidget*, cairo_t* cr, WindowState* state) -> gboolean {
              return state->Draw(*gui::DrawCtx::wrap(cr));
              })), this);

  sig5 = g_signal_connect(window, "configure-event", G_CALLBACK((
              +[](GtkWidget*, GdkEventConfigure* event, WindowState* state) -> gboolean {
              return state->configure_event(event);
              })), this);

  sig1 = g_signal_connect(window, "key-press-event", G_CALLBACK((
              +[](GtkWidget*, GdkEventKey* event, WindowState* state) -> gboolean {
              return state->key_press(event);
              })), this);

  sig2 = g_signal_connect(window, "destroy", G_CALLBACK(
          +[](GtkWidget*, WindowState* state) -> gboolean {
          gtk_main_quit();
          delete state;
          return FALSE;
  }), this);

  sig7 = g_signal_connect(window, "motion-notify-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventMotion* motion, WindowState* state) -> gboolean {
          return state->MotionNotify(motion);
          })), this);
}

extern "C" void dl_plugin_entry(int argc, char **argv) {
  if (argc > 1) {
    fprintf(stderr, "calling(%s): %s\n", argv[0], argv[1]);
    new WindowState();
  } else {
    new WindowState();
  }
}
