#include "gui/so-handle.h"
#include "gui/so-handoff-lib.h"
#include "gui/font-face.h"
#include "gui/buffer.h"
#include "rules/template-support.h"
#include "parser/parser-support.h"
#include "gui/buffer-edit.h"
#include "gui/buffer-view.h"
#include "gen/gui/view_manifest.h"

#include <iostream>
#include <sstream>
#include <assert.h>
#include <gtk/gtk.h>

void SaveFile(const std::string& filename, const std::vector<IdBuffer>& buffers) {
  EmitStream out2;
  SaveFile(out2.stream(), buffers);
  out2.write(filename);
}

struct WindowState {
  GtkWidget* window;
  GtkWidget* drawing_area;

  size_t window_width_ = 1024 * 3 / 2;
  size_t window_height_ = 680 * 3 / 2;

  enum class Mode {
    INSERT,
    ESCAPE,
    COLON,
  };
  Mode mode = Mode::INSERT;

  std::string colon_text;
  size_t col_col = 0;

  std::vector<std::unique_ptr<Buffer>> buffers;
  size_t view_id_ = 0;
  std::vector<ChunkView> views;

  std::string filename_;

  gulong sig1;
  gulong sig2;
  gulong sig3;
  gulong sig4;
  gulong sig5;
  gulong sig6;

  explicit WindowState(const char* filename);
  WindowState(GtkWidget* window, GtkWidget* drawing_area,
              size_t width, size_t height, const char* filename);

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
      int status = system(".build/rules src/gui ide-dynamic.so");
      /* clang-6.0 -o \
                          /tmp/trampoline_test/entry.so \
                          -shared -fpic -Wl,-z,defs \
                          -lstdc++ -ldl -L . -Wl,-rpath='$ORIGIN' \
                          src/gui/so-handoff-lib.cc \
                          src/gui/so-handle.cc \
                          src/gui/dlmain.cc \
                          src/gui/buffer.cc \
                          src/gui/font-face.cc \
                          src/parser/tokenizer_helper.cc \
                          -I src `pkg-config gtk+-3.0 --cflags --libs` -lfontconfig -lfreetype -g");
                          */
      if (status == 0) {
        // Invoke compile here...
        // Should probably delay this until the end of the event so it can happen
        // at any point.
        g_signal_handler_disconnect(window, sig1);
        g_signal_handler_disconnect(window, sig2);
        g_signal_handler_disconnect(drawing_area, sig3);
        g_signal_handler_disconnect(window, sig4);
        g_signal_handler_disconnect(window, sig5);
        g_signal_handler_disconnect(window, sig6);

        fprintf(stderr, "[so-reloader] ?? doing load number: %zu\n", main::GetJumpId());
        const char* so_name = (main::GetJumpId() % 2) == 0 ? "/tmp/gui-so-red.so" : "/tmp/gui-so-black.so";
        const char* base_so_name = ".build/ide-dynamic.so";
        fprintf(stderr, "[so-reloader] Loading from: %s\n", so_name);

        link(base_so_name, so_name);
        SoHandle handle(so_name, RTLD_NOW | RTLD_LOCAL);
        unlink(so_name);

        handle.get_sym<void(GtkWidget*,GtkWidget*,size_t,size_t, const char*)>("gtk_transfer_window")(window, drawing_area,
                                                                window_width_, window_height_, filename_.c_str());
        main::SwapToNewSoFile(std::move(handle));
        delete this;
      } else {
        fprintf(stderr, "Could not compile....\n");
      }
      return FALSE;
    }

    if (mode == Mode::INSERT) {
      if (DoKeyPress(views[view_id_], keyval)) {
      } else if (keyval == GDK_KEY_Escape) {
        mode = Mode::ESCAPE;
      } else {
        fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
        return FALSE;
      }
      redraw();
      return FALSE;
    } else if (mode == Mode::ESCAPE) {
      if (keyval == 'i') {
        mode = Mode::INSERT;
      } else if (keyval == ':') {
        mode = Mode::COLON;
        colon_text = "";
        col_col = 0;
      } else {
        fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
        return FALSE;
      }
      redraw();
      return FALSE;
    } else if (mode == Mode::COLON) {
      if (keyval >= ' ' && keyval <= '~') {
        colon_text.insert(colon_text.begin() + col_col, keyval);
        col_col += 1;
      } else if (keyval == GDK_KEY_Left && col_col > 0) {
        col_col -= 1;
      } else if (keyval == GDK_KEY_Right && col_col < colon_text.size()) {
        col_col += 1;
      } else if (keyval == GDK_KEY_Return) {
        if (colon_text == "w") {
          std::vector<IdBuffer> buffers_out;
          for (size_t i = 0; i < buffers.size(); ++i) {
            buffers_out.push_back({i, buffers[i].get()});
          }
          SaveFile(filename_, buffers_out);
        } else if (colon_text == "t") {
          fprintf(stderr, "--------Eval--------\n");
          std::vector<IdBuffer> buffers_out;
          for (size_t i = 0; i < buffers.size(); ++i) {
            buffers_out.push_back({i, buffers[i].get()});
          }
          SaveFile(filename_, buffers_out);

          if (system(".build/run-buffer-tests") == 0) {
            fprintf(stderr, "... Test Success ...\n");
          } else {
            fprintf(stderr, "... Test Failure ...\n");
          }
        } else {
          fprintf(stderr, "Unknown command: %s\n", colon_text.c_str());
        }
        mode = Mode::ESCAPE;
      } else if (keyval == GDK_KEY_Escape) {
        mode = Mode::ESCAPE;
      } else {
        fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
        return FALSE;
      }
      redraw();
      return FALSE;
    } else {
      fprintf(stderr, "unknown mode and keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return FALSE;
    }
  }

  uint32_t press_time = -1;
  gboolean ButtonPress(GdkEventButton* button) {
    gui::Point pt{button->x, button->y};

    if (pt.x < 300) {
    auto* font = gui::DefaultFont();

    std::vector<cairo_glyph_t> glyphs;
    gui::Point opt {2, 2};
    
    for (size_t i = 0; i < views.size(); ++i) {
      gui::Point st = opt;
      glyphs.clear();
      font->LayoutWrap(st, glyphs, "- ", 12, 300.0 - 2);
      font->LayoutWrap(st, glyphs, views[i].title, 12, 300.0 - 2);
      opt.y = st.y + font->height() + 10;
      if (pt.y < opt.y) {
        fprintf(stderr, "Going to view %zu\n", i);
        view_id_ = i;
        views[view_id_].SanitizeCursor();
        redraw();
        return TRUE;
      }
    }

      return FALSE;
    }
    auto* font = gui::DefaultFont();

    auto& view = views[view_id_];
    auto& buffer_id_ = view.buffer_id_;
    auto& buffers = view.buffers;
    auto& gaps = view.gaps;
    auto& decorations = view.decorations;
    auto& cursor = view.cursor;
    auto& scroll = view.scroll;

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
      double segment_height = font->height() * buffer.lines.size();

      size_t click_line = 0;
      if (pt.y > st.y) {
        click_line = static_cast<ssize_t>((pt.y - st.y) / font->height());
      }
      if (click_line < buffer.lines.size()) {
        cursor.col = 0;
        cursor.row = click_line;
        buffer_id_ = i;
        redraw();
        return TRUE;
      }
      st.y += segment_height;
    }
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
  void InitBuffers();
};

gboolean WindowState::ScrollEvent(GdkEventScroll* event) {
  auto& scroll = views[view_id_].scroll;
  auto* font = gui::DefaultFont();
  if (event->direction == GDK_SCROLL_UP) {
    scroll += font->height() * 3;
  } else if (event->direction == GDK_SCROLL_DOWN) {
    scroll -= font->height() * 3;
  }
  if (scroll > 0) { scroll = 0; }
  redraw();
  return TRUE;
}

gboolean WindowState::Draw(gui::DrawCtx& cr) {
  cr.set_source_rgb(0.0, 0.0, 0.0);
  cr.paint();

  cr.set_source_rgb(1.0, 0.0, 0.0);
  auto* font = gui::DefaultFont();

  {
    std::vector<cairo_glyph_t> glyphs;
    gui::Point opt {2, 2};
    
    cr.set_source_rgb(0.5, 1.0, 1.0);

    for (size_t i = 0; i < views.size(); ++i) {
      gui::Point st = opt;
      glyphs.clear();
      if (i == view_id_) {
        font->LayoutWrap(st, glyphs, "x ", 12, 300.0 - 2);
      } else {
        font->LayoutWrap(st, glyphs, "- ", 12, 300.0 - 2);
      }
      font->LayoutWrap(st, glyphs, views[i].title, 12, 300.0 - 2);
      font->Flush(cr, glyphs);
      opt.y = st.y + font->height() + 10;
    }
  }

  cr.translate({300, 0});

  const auto& view = views[view_id_];
  DrawView(cr, view, window_height_, window_width_ - 300);
  // Status line:
  {
    cr.set_source_rgb(0.1, 0.1, 0.1);

    double h = font->height() + 4;
    auto st_pt = gui::Point{0, window_height_ - h};
    cr.rectangle(st_pt,
                 gui::Point{static_cast<double>(window_width_ - 300), h});

    cr.fill();

    std::vector<cairo_glyph_t> glyphs_;

    st_pt.y += 2;
    if (mode == Mode::INSERT) {
      cr.set_source_rgb(1.0, 0.0, 0.0);
      font->LayoutWrap(st_pt, glyphs_, "-- INSERT --");
      font->Flush(cr, glyphs_);
    } else if (mode == Mode::COLON) {
      font->LayoutWrap(st_pt, glyphs_, ":");
      font->LayoutWrap(st_pt, glyphs_, colon_text);
      cr.set_source_rgb(1.0, 1.0, 0.0);
      font->Flush(cr, glyphs_);
    }
  }
  cr.translate({900, 0});
  cr.move_to({0, 0});
  cr.line_to({100, 100});
  cr.stroke();
  needs_redraw = false;
  return TRUE;
}

extern "C" void gtk_transfer_window(GtkWidget* window, GtkWidget* drawing_area,
                                    size_t width, size_t height, const char* filename) {
  new WindowState(window, drawing_area, width, height, filename);
}

WindowState::WindowState(const char* filename) : filename_(filename) {
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawing_area, window_width_, window_height_);

  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_, window_height_);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  gtk_widget_show_all(window);
  InitEvents();
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

  InitBuffers();
}

void WindowState::InitBuffers() {

  auto tmp = ParseMultiBuffer(LoadFile(filename_));

  for (auto& id_b : tmp) {
    buffers.emplace_back(std::make_unique<Buffer>(std::move(id_b.buffer)));
  }

  ASTContext ctx;
  auto view_manifest_str = Collapse(*buffers[1]);
  view_manifest::Tokenizer tokens(ctx, view_manifest_str.c_str());
  auto* m = view_manifest::parser::DoParse(tokens);

  auto get_chunk = [&](view_manifest::ChunkSrc* src) -> Buffer* {
    int id = stoi(std::string(src->id.str));
    assert(id >= 0 && id < buffers.size());
    return buffers[id].get();
  };
  for (view_manifest::EmitFileDecl* decl : m->decls) {
    using namespace view_manifest;
    views.push_back({});
    views.back().title = std::string(decl->name.str);
    size_t i = 0;
    auto& view = views.back();
    for (auto* action_ : decl->actions) {
      if (i != 0) { view.gaps.push_back(20); }
      switch (action_->getKind()) {
      case Action::Kind::Raw: {
        auto* action = reinterpret_cast<RawAction*>(action_);
        view.buffers.push_back(get_chunk(action->id));
        view.decorations.push_back({});
        break;
      } case Action::Kind::DefineFunc: {
        auto* action = reinterpret_cast<DefineFuncAction*>(action_);
        view.buffers.push_back(get_chunk(action->sig_id));
        view.buffers.push_back(get_chunk(action->body_id));
        view.decorations.push_back({});
        view.decorations.push_back({false});
        view.gaps.push_back(0);
      }
      }
      ++i;
    }

  }
}

WindowState::WindowState(GtkWidget* window_inp, GtkWidget* drawing_area_inp,
                         size_t width, size_t height, const char* filename) : filename_(filename) {

  window = window_inp;
  drawing_area = drawing_area_inp;
  window_width_ = width;
  window_height_ = height;
  InitEvents();
  gtk_widget_queue_draw(window);
}

extern "C" void dl_plugin_entry(int argc, char **argv) {
  if (argc > 1) {
    fprintf(stderr, "calling(%s): %s\n", argv[0], argv[1]);
    new WindowState(argv[1]);
  } else {
    new WindowState(".gui/data");
  }
}
