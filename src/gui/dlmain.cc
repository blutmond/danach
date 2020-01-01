#include "gui/so-handle.h"
#include "gui/so-handoff-lib.h"
#include "gui/font-face.h"
#include "gui/buffer.h"
#include "rules/template-support.h"
#include "parser/parser-support.h"

#include <iostream>
#include <sstream>
#include <gtk/gtk.h>

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

void DoDrawLines(gui::DrawCtx& cr, double scroll,
                 double width, size_t num_lines, size_t window_height,
                 gui::FontLayoutFace* font) {
  gui::Point st {2,2};
  std::vector<cairo_glyph_t> glyphs;
  st.y += scroll;
  size_t i = 1;
  if (st.y < 0) {
    size_t n_hidden = static_cast<size_t>(-st.y / font->height()); 
    st.y += font->height() * n_hidden;
    i += n_hidden;
  }
  while (st.y < window_height) {
    glyphs.clear();
    if (i <= num_lines) LayoutLineNumbers(st, glyphs, font, i, width);
    else LayoutTwidle(st, glyphs, font);
    font->Flush(cr, glyphs);
    st.y += font->height();
    i += 1;
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
  const auto& lines = buffer.lines;
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

void SaveFile(const std::vector<IdBuffer>& buffers) {
  std::string out;
  std::stringstream ss(out);
  SaveFile(ss, buffers);
  ss.flush();
  // std::cerr << ss.str();
  auto tmp = ParseMultiBuffer(ss.str());
  std::vector<ParsedIdBuffer> copy;
  for (auto& t : buffers) {
    copy.push_back({t.id, *t.buffer});
  }
  if (tmp != copy) {
    std::cerr << "Could not roundtrip!\n";
    std::cerr << ss.str();
  }

  EmitStream out2;
  SaveFile(out2.stream(), buffers);
  out2.write(".gui/data");
}

struct WindowState {
  GtkWidget* window;
  GtkWidget* drawing_area;

  size_t window_width_ = 1024 * 3 / 2;
  size_t window_height_ = 680 * 3 / 2;

  double scroll = 0.0;

  enum class Mode {
    INSERT,
    ESCAPE,
    COLON,
  };
  Mode mode = Mode::INSERT;

  BufferPos cursor {0, 0};
  size_t buffer_id_ = 0;
  std::string colon_text;
  size_t col_col = 0;
  std::vector<Buffer> buffers;

  gulong sig1;
  gulong sig2;
  gulong sig3;
  gulong sig4;
  gulong sig5;

  WindowState();
  WindowState(GtkWidget* window, GtkWidget* drawing_area,
              size_t width, size_t height);

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

        fprintf(stderr, "[so-reloader] ?? doing load number: %zu\n", main::GetJumpId());
        const char* so_name = (main::GetJumpId() % 2) == 0 ? "/tmp/gui-so-red.so" : "/tmp/gui-so-black.so";
        const char* base_so_name = ".build/ide-dynamic.so";
        fprintf(stderr, "[so-reloader] Loading from: %s\n", so_name);

        link(base_so_name, so_name);
        SoHandle handle(so_name, RTLD_NOW | RTLD_LOCAL);
        unlink(so_name);

        handle.get_sym<void(GtkWidget*,GtkWidget*,size_t,size_t)>("gtk_transfer_window")(window, drawing_area,
                                                                window_width_, window_height_);
        main::SwapToNewSoFile(std::move(handle));
        delete this;
      } else {
        fprintf(stderr, "Could not compile....\n");
      }
      return FALSE;
    }

    if (mode == Mode::INSERT) {
    auto& buffer = buffers[buffer_id_];
    bool float_pos_dirty = true;
    const auto& lines = buffer.lines;
    auto& pos = cursor;
    auto clip_col = [&]() {
      // TODO: Do decoration based clipping.
      if (float_pos == string_view::npos) {
        float_pos = pos.col;
      }
      pos.col = std::min(float_pos, buffers[buffer_id_].lines[pos.row].size());
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
    } else if (keyval == GDK_KEY_Down && buffer_id_ + 1 < buffers.size()) {
      pos.row = 0;
      buffer_id_ += 1;
      clip_col();
    } else if (keyval == GDK_KEY_Up && pos.row > 0) {
      pos.row -= 1;
      clip_col();
    } else if (keyval == GDK_KEY_Up && buffer_id_ > 0) {
      buffer_id_ -= 1;
      pos.row = buffers[buffer_id_].lines.size() - 1;
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
    } else if (keyval == GDK_KEY_Escape) {
      mode = Mode::ESCAPE;
    } else {
      fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return FALSE;
    }
    if (float_pos_dirty) float_pos = string_view::npos;
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
            buffers_out.push_back({i, &buffers[i]});
          }
          SaveFile(buffers_out);
        } else if (colon_text == "t") {
          fprintf(stderr, "--------Eval--------\n");
          std::vector<IdBuffer> buffers_out;
          for (size_t i = 0; i < buffers.size(); ++i) {
            buffers_out.push_back({i, &buffers[i]});
          }
          SaveFile(buffers_out);

          if (system(".build/run-buffer-tests") == 0) {
            fprintf(stderr, "... Test Success ...\n");
          } else {
            fprintf(stderr, "... Test Failure ...\n");
          }
        } else if (colon_text == "s") {
          buffer_id_ = (buffer_id_ + 1) % buffers.size();;
          cursor = {0, 0};
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

  size_t float_pos = string_view::npos;
  bool needs_redraw = true;
  void redraw() {
    if (!needs_redraw) {
      needs_redraw = true;
      gtk_widget_queue_draw(window);
    }
  }
  void InitEvents();
};

gboolean WindowState::ScrollEvent(GdkEventScroll* event) {
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

  size_t num_lines = 0;
  for (size_t i = 0; i < buffers.size(); ++i) {
    num_lines += buffers[i].lines.size();
  } // + buffers[1].lines.size();
  double width = GetLineNumberRenderingWidth(std::max<size_t>(100, num_lines), font);

  DoDrawLines(cr, scroll, width, num_lines, window_height_, font);
  gui::Point st {2 + width, 2 + scroll};

  for (size_t i = 0; i < buffers.size(); ++i) {
    if ((i % 2) == 1) {
      // st.y += font->height() / 2;
      st.x = 2 + width + 20;
      cr.move_to({width + 15, st.y + 6});
      cr.line_to({width + 15, st.y + font->height() * buffers[i].lines.size()});
      cr.stroke();
    } else {
      st.x = 2 + width;
    }
    
    DrawBuffer(cr, st, buffers[i], buffer_id_ == i ? &cursor : nullptr);
    st.y += font->height() * buffers[i].lines.size();
  }

  // Status line:
  {
    cr.set_source_rgb(0.1, 0.1, 0.1);

    double h = font->height() + 4;
    auto st_pt = gui::Point{0, window_height_ - h};
    cr.rectangle(st_pt,
                 gui::Point{static_cast<double>(window_width_), h});

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
                                    size_t width, size_t height) {
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

void WindowState::InitEvents() {
  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);

  sig4 = g_signal_connect(window, "scroll-event", G_CALLBACK((
              +[](GtkWidget* widget, GdkEventScroll* event, WindowState* state) -> gboolean {
              return state->ScrollEvent(event);
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

  if (false) {

  buffers.emplace_back();
  buffers.emplace_back();
  buffers.emplace_back();
  buffers.emplace_back();
  Init(buffers[0], "WindowState::WindowState(GtkWidget* window_inp, GtkWidget* drawing_area_inp,\n                         size_t width, size_t height)");
  Init(buffers[1], 
R"(window = window_inp;
drawing_area = drawing_area_inp;
window_width_ = width;
window_height_ = height;
InitEvents();
gtk_widget_queue_draw(drawing_area);)");
  Init(buffers[2], "extern \"C\" void dl_plugin_entry()");
  Init(buffers[3], "new WindowState();");

  } else {
  auto tmp = ParseMultiBuffer(LoadFile(".gui/data"));

  for (auto& id_b : tmp) {
    buffers.emplace_back(std::move(id_b.buffer));
  }

  }
  // What are the things, and how can you compose them?
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

extern "C" void dl_plugin_entry() {
  new WindowState();
}
