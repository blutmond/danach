#include <gtk/gtk.h>

#include "rules/process.h"
#include "rules/clang-error-parse.h"
#include "gui/font-face.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

void clang_example() {
  std::string out;
  // TODO: clang utility that parses diagnostics...
  int wstatus = RunWithPipe({
                            "/usr/bin/clang-6.0", "-ferror-limit=1000",
                            "-Wall", "-fdiagnostics-parseable-fixits",
                            "-fno-diagnostics-fixit-info", "-fno-caret-diagnostics",
                            "-fdiagnostics-print-source-range-info", "/tmp/toy.cc"
                    }, &out, &out);

  clang_util::ParseErrorMessages(out);

  // printf("result: \"%s\"\n", out.c_str());
  // printf("status: %d\n", wstatus);
  // AssertWStatus(wstatus);
}

int __attribute__((visibility("default"))) helper_fn(int v) {
  fprintf(stderr, "Here\n");
  return v + 19;
}

class SoHandle {
 public:
  SoHandle() {}
  SoHandle(const char* filename, int flags) {
    handle = dlopen(filename, flags);
    if (!handle) { fprintf(stderr, "dlopen(%s): %s\n", filename, dlerror()); exit(EXIT_FAILURE); }
  }
  SoHandle(void* handle) : handle(handle) {}

  SoHandle(const SoHandle& o) = delete;
  SoHandle& operator=(const SoHandle& o) = delete;

  template <typename ffi_function_type>
  ffi_function_type* get_sym(const char* symname) {
    void* sym = dlsym(handle, symname);
    if (!sym) { fprintf(stderr, "dlsym: %s\n", dlerror()); exit(EXIT_FAILURE);  }
    return (ffi_function_type*)sym; 
  }
  ~SoHandle() {
    if (dlclose(handle) != 0) {
      fprintf(stderr, "dlclose: %s\n", dlerror());
      exit(EXIT_FAILURE);
    }
  }
 private:
  void* handle;
};

// Current build rules:
// clang-6.0 -shared -fpic -Wl,-z,defs test.cc -o test3.so test2.so -L . -Wl,-rpath='$ORIGIN'
void blah_example() {
//  SoHandle handle_pre("/tmp/dltest/libtest2.so", RTLD_NOW | RTLD_GLOBAL);
  SoHandle handle("/tmp/dltest/test.so", RTLD_NOW | RTLD_LOCAL);
  SoHandle handle2("/tmp/dltest/test3.so", RTLD_NOW | RTLD_LOCAL);
  auto fn = handle.get_sym<int(int)>(("my_example"));
  auto fn2 = handle2.get_sym<int(int)>(("my_example"));

  std::cout << "fn: " << (*fn)(3) << std::endl;
  std::cout << "fn: " << (*fn2)(3) << std::endl;
}

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

struct BufferPos {
  size_t row = 0;
  size_t col = 0;
};

struct Buffer {
  std::vector<std::string> lines{{""}};

  void Print() {
    for (const auto& ln : lines) {
      printf("ln: \"%s\"\n", ln.c_str());
    }
  }
};

void Insert(Buffer& buffer, BufferPos& cursor, char c) {
  if (c == '\n') {
    buffer.lines.insert(buffer.lines.begin() + (cursor.row + 1), 
                         buffer.lines[cursor.row].substr(cursor.col));
    buffer.lines[cursor.row] = buffer.lines[cursor.row].substr(0, cursor.col);
    cursor.row += 1;
    cursor.col = 0;
  } else {
    auto& str = buffer.lines[cursor.row];
    str.insert(str.begin() + cursor.col, c);
    cursor.col += 1;
  }
}

void Insert(Buffer& buffer, BufferPos& cursor, string_view text) {
  for (char c : text) {
    Insert(buffer, cursor, c);
  }
}

void Delete(Buffer& buffer, BufferPos s_pos, BufferPos e_pos) {
  auto& lines_ = buffer.lines;
    if (s_pos.row == e_pos.row) {
      std::string& target = lines_[e_pos.row];
      if (!(s_pos.col < e_pos.col)) {
        fprintf(stderr, "error");
        exit(EXIT_FAILURE);
      }
      if (!(e_pos.col <= target.size())) {
        fprintf(stderr, "error");
        exit(EXIT_FAILURE);
      }
      target.erase(target.begin() + s_pos.col,
                   target.begin() + e_pos.col);
    } else {
      auto tmp = std::move(lines_[e_pos.row]);
      std::string& target = lines_[s_pos.row];
      target.resize(s_pos.col);
      // This also has a copy.
      target += tmp.substr(e_pos.col);

      lines_.erase(lines_.begin() + s_pos.row + 1,
                   lines_.begin() + e_pos.row + 1);
    }
  // buffer.lines.
}

struct WindowState {
  GtkWidget* window;
  GtkWidget* drawing_area;
  
  size_t window_width_ = 1024 * 3 / 2;
  size_t window_height_ = 680 * 3 / 2;
  double scroll = 0.0;

  Buffer buffer;
  BufferPos cursor {0, 0};

  WindowState();

  gboolean ScrollEvent(GdkEventScroll* event);
  gboolean Draw(gui::DrawCtx& cr);
  gboolean configure_event(GdkEventConfigure* config) {
    if (window_width_ != config->width || window_height_ != config->height) {
      window_width_ = config->width;
      window_height_ = config->height;
  //    Resized();
    }
    return FALSE;
  }
  gboolean key_press(GdkEventKey* event) {
    fprintf(stderr, "What: %d\n", event->keyval);
    auto keyval = event->keyval;
    bool float_pos_dirty = true;
    const auto& lines = buffer.lines;
    auto& pos = cursor;
    auto clip_col = [&]() {
      if (float_pos == string_view::npos) {
        float_pos = pos.col;
      }
      pos.col = std::min(float_pos, lines[pos.row].size());
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
    } else if (keyval == GDK_KEY_Up && pos.row > 0) {
      pos.row -= 1;
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
    } else {
      fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return FALSE;
    }
    if (float_pos_dirty) float_pos = string_view::npos;
    redraw();
    return FALSE;
  }
  size_t float_pos = string_view::npos;
  bool needs_redraw = true;
  void redraw() {
    if (!needs_redraw) {
      needs_redraw = true;
      gtk_widget_queue_draw(drawing_area);
    }
  }
};

gboolean WindowState::ScrollEvent(GdkEventScroll* event) {
  auto* font = gui::DefaultFont();
  if (event->direction == GDK_SCROLL_UP) {
    scroll += font->height() * 3;
  } else if (event->direction == GDK_SCROLL_DOWN) {
    scroll -= font->height() * 3;
  }
  // fprintf(stderr, "scroll: %g\n", scroll);
  if (scroll > 0) { scroll = 0; }
  redraw();
  return TRUE;
}

gboolean WindowState::Draw(gui::DrawCtx& cr) {
  cr.set_source_rgb(0.0, 0.0, 0.0);
  cr.paint();

  cr.set_source_rgb(1.0, 0.0, 0.0);
  auto* font = gui::DefaultFont();
  std::vector<cairo_glyph_t> glyphs;

  size_t num_lines = buffer.lines.size();
  double width = GetLineNumberRenderingWidth(std::max<size_t>(100, num_lines), font);

  // fprintf(stderr, "scroll: %g\n", scroll);
  DoDrawLines(cr, scroll, width, num_lines, window_height_, font);
  gui::Point st {2 + width, 2 + scroll};

  auto& pos = cursor;
  const auto& lines = buffer.lines;
  size_t pos_row = pos.row;
  cr.set_source_rgb(1.0, 1.0, 0.0);
  size_t row = 0;
  for (auto& line: buffer.lines) {
    glyphs.clear();
    font->LayoutWrap(st, glyphs, line);
    if (row == pos_row) {

      font->Flush(cr, glyphs.data(), pos.col);
      if (pos.col < glyphs.size()) {
        font->Flush(cr, glyphs.data() + (pos.col + 1), glyphs.size() - pos.col - 1);

        cr.set_source_rgb(1.0, 1.0, 0.0);
        cr.rectangle(gui::Point{glyphs[pos.col].x, st.y},
                     {font->GetWidth(lines[row][pos.col]), font->height()});
        cr.fill();

        cr.set_source_rgb(0.0, 0.0, 0.0);
        cairo_show_glyphs(cr.cr(), glyphs.data() + pos.col, 1);
        cr.set_source_rgb(1.0, 1.0, 0.0);
      } else {
        cr.set_source_rgb(1.0, 1.0, 0.0);
        cr.rectangle(st, {9, font->height()});
        cr.fill();
      }
    } else {
      font->Flush(cr, glyphs);
    }
    st.y += font->height();
    st.x = 2 + width;
    ++row;
  }
  cr.translate({700, 0});
  cr.move_to({0, 0});
  cr.line_to({100, 100});
  cr.stroke();
  needs_redraw = false;
  return TRUE;
}

WindowState::WindowState() {
  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawing_area, window_width_, window_height_);
  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(window, "scroll-event", G_CALLBACK((
              +[](GtkWidget* widget, GdkEventScroll* event, WindowState* state) -> gboolean {
              return state->ScrollEvent(event);
              })), this);

  g_signal_connect(drawing_area, "draw", G_CALLBACK((
              +[](GtkWidget*, cairo_t* cr, WindowState* state) -> gboolean {
              return state->Draw(*gui::DrawCtx::wrap(cr));
              })), this);

  g_signal_connect(window, "configure-event", G_CALLBACK((
              +[](GtkWidget*, GdkEventConfigure* event, WindowState* state) -> gboolean {
              return state->configure_event(event);
              })), this);

  g_signal_connect(window, "key-press-event", G_CALLBACK((
              +[](GtkWidget*, GdkEventKey* event, WindowState* state) -> gboolean {
              return state->key_press(event);
              })), this);


  g_signal_connect(window, "destroy", G_CALLBACK(
          +[](GtkWidget*, WindowState* state) -> gboolean {
          gtk_main_quit();
          delete state;
          return FALSE;
  }), this);

  // What are the things, and how can you compose them?
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_, window_height_);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);

  auto* state = new WindowState;
  Insert(state->buffer, state->cursor, R"(// Hello world
// this is an example text that I don't have to load from anywhere.
// whatever

WindowState::WindowState() {
  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawing_area, window_width_, window_height_);
  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect(window, "scroll-event", G_CALLBACK((
  +[](GtkWidget* widget, GdkEventScroll* event, WindowState* state) -> gboolean {
    return state->ScrollEvent(event);
  })), this);

  g_signal_connect(drawing_area, "draw", G_CALLBACK((
  +[](GtkWidget*, cairo_t* cr, WindowState* state) -> gboolean {
    return state->Draw(*gui::DrawCtx::wrap(cr));
  })), this);

  g_signal_connect(window, "configure-event", G_CALLBACK((
  +[](GtkWidget*, GdkEventConfigure* event, WindowState* state) -> gboolean {
    return state->configure_event(event);
  })), this);

  g_signal_connect(window, "key-press-event", G_CALLBACK((
  +[](GtkWidget*, GdkEventKey* event, WindowState* state) -> gboolean {
    return state->key_press(event);
  })), this);


  g_signal_connect(window, "destroy", G_CALLBACK(
  +[](GtkWidget*, WindowState* state) -> gboolean {
    gtk_main_quit();
    delete state;
    return FALSE;
  }), this);

  // What are the things, and how can you compose them?
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_, window_height_);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  gtk_widget_show_all(window);
}

)");

  state->buffer.Print();

  gtk_main();
}
