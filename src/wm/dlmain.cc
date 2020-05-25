#include "gui/widget-helper.h"
#include "wm/sub-window.h"
#include "gui/buffer.h"
#include "gui/buffer-view.h"
#include "gui/editor-widget.h"
#include "rules/process.h"
#include "rules/template-support.h"
#include "rules/clang-error-parse.h"
#include <iostream>
#include <stdlib.h>
#include <exception>

struct NoMoreKeyvalException : public std::exception {
  const char * what () const throw () {
    return "No more keyval. (This is not an error)";
  }
};

struct KeyFeed {
  uint32_t Next() {
    if (i < command_history.size()) return command_history[i++];
    throw NoMoreKeyvalException();
  }
  std::vector<uint32_t>& command_history;
  size_t i = 0;
};

bool LineEdit(uint32_t keyval, std::string& line, size_t& cursor) {
  if (keyval >= ' ' && keyval <= '~') {
    line.insert(line.begin() + cursor, keyval);
    cursor += 1;
  } else if (keyval == GDK_KEY_BackSpace && cursor > 0) {
    cursor -= 1;
    line.erase(line.begin() + cursor);
  } else if (keyval == GDK_KEY_Left && cursor > 0) {
    cursor -= 1;
  } else if (keyval == GDK_KEY_Right && cursor < line.size()) {
    cursor += 1;
  } else {
    return false;
  }
  return true;
}

class BasicEditorWindow : public SubWindow {
 public:
  BasicEditorWindow() {
    view.buffers.push_back(&buffer);
    view.decorations.push_back(Decoration());
  }

  void DrawStatus(gui::DrawCtx& cr, gui::Shape shape) {
    auto* font = gui::DefaultFont();
    // Status line:
    {
      cr.set_source_rgb(0.1, 0.1, 0.1);

      double h = font->height() + 4;
      auto st_pt = gui::Point{0, shape.h - h};
      cr.rectangle(st_pt,
                   gui::Point{static_cast<double>(shape.w), h});

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
  }

  void Draw(gui::DrawCtx& cr) override {
    cr.set_source_rgb(0.0, 0.0, 0.0);
    cr.paint();
    gui::Rectangle rect0;
    gui::Rectangle rect1;
    gui::DoVSplit(rect(), rect0, rect1, -int(gui::DefaultFont()->height() + 4));
    SaveClipTranslate(cr, rect0);
    view.Draw(cr, rect0.shape.h, rect0.shape.w);
    cr.restore();
    SaveClipTranslate(cr, rect1);
    DrawStatus(cr, rect1.shape);
    cr.restore();
  }

  bool KeyPressEscape(KeyFeed& feed) {
    auto keyval = feed.Next();
    if (keyval == ':') {
      mode = Mode::COLON;
      colon_text = "";
      col_col = 0;
    }
    if (view.num_buffers() == 0) return true;
    auto& buffer = *view.GetBuffer(view.buffer_id_);
    auto& cursor = view.cursor;
    if (DoEscapeKeyPress(view, keyval)) {
    } else if (keyval == 'i') {
      mode = Mode::INSERT;
    } else if (keyval == 'o') {
      mode = Mode::INSERT;
      cursor.col = buffer.lines[cursor.row].size();
      Insert(buffer, cursor, '\n');
    } else if (keyval == 'd') {
      keyval = feed.Next();
      if (keyval == 'd') {
        fprintf(stderr, "Something\n");
      } else if (keyval == GDK_KEY_apostrophe) {
        keyval = feed.Next();
        if (keyval == GDK_KEY_a) {
          fprintf(stderr, "Delete to mark 'a\n");
        } else {
          fprintf(stderr, "unknown keyval (d'): %d, %s\n", keyval, gdk_keyval_name(keyval));
        }
      } else {
        fprintf(stderr, "unknown keyval (d): %d, %s\n", keyval, gdk_keyval_name(keyval));
      }
    } else {
      fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return false;
    }
    return true;
  }

  bool KeyPressBool(GdkEventKey* event) {
    auto keyval = event->keyval;
    if (mode == Mode::INSERT) {
      if (DoKeyPress(view, keyval)) {
      } else if (keyval == GDK_KEY_Escape) {
        mode = Mode::ESCAPE;
      } else {
        fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
        return false;
      }
      return true;
    } else if (mode == Mode::ESCAPE) {
      command_history.push_back(keyval);
      try {
        KeyFeed feed{command_history};
        bool result = KeyPressEscape(feed);
        command_history.clear();
        return result;
      } catch(NoMoreKeyvalException& e) {}
      return true;
     } else if (mode == Mode::COLON) {
      if (keyval >= ' ' && keyval <= '~') {
        colon_text.insert(colon_text.begin() + col_col, keyval);
        col_col += 1;
      } else if (keyval == GDK_KEY_BackSpace && col_col > 0) {
        col_col -= 1;
        colon_text.erase(colon_text.begin() + col_col);
      } else if (keyval == GDK_KEY_Left && col_col > 0) {
        col_col -= 1;
      } else if (keyval == GDK_KEY_Right && col_col < colon_text.size()) {
        col_col += 1;
      } else if (keyval == GDK_KEY_Return) {
        if (colon_text == "w") {
          EmitStream out;
          out.stream() << Collapse(buffer);
          out.write(filename);
        } else if (colon_text.size() > 5 && colon_text.substr(0, 5) == "open ") {
          string_view data = colon_text;
          data.remove_prefix(5);
          filename = std::string(data); 
          Init(buffer, LoadFile(filename));
        } else if (colon_text == "t") {
          fprintf(stderr, "--------Eval--------\n");
        } else {
          fprintf(stderr, "Unknown command: %s\n", colon_text.c_str());
        }
        mode = Mode::ESCAPE;
      } else if (keyval == GDK_KEY_Escape) {
        mode = Mode::ESCAPE;
      } else {
        fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
        return false;
      }
      return true;
    } else {
      fprintf(stderr, "unknown mode and keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return false;
    }
  }

  void KeyPress(GdkEventKey* event) override {
    if (KeyPressBool(event)) {
    } else {
      return;
    }
    redraw();
  }

  void ScrollEvent(GdkEventScroll* event) override {
    auto& scroll = view.scroll;
    auto* font = gui::DefaultFont();
    if (event->direction == GDK_SCROLL_UP) {
      scroll += font->height() * 3;
    } else if (event->direction == GDK_SCROLL_DOWN) {
      scroll -= font->height() * 3;
    }
    if (scroll > 0) { scroll = 0; }
    redraw();
  }

  void ButtonPress(GdkEventButton* button) override {
    gui::Point pt{button->x, button->y};

    if (button->button == 2) {
      auto* clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
      auto* data = gtk_clipboard_wait_for_text(clipboard);


      if (view.num_buffers() == 0) return;
      auto& cursor = view.cursor;
      auto& buffer = *view.GetBuffer(view.buffer_id_);
      for (char c : string_view(data)) {
        Insert(buffer, cursor, c);
      }

      g_free(data);
      redraw();
      return;
    }
    if (view.FindChunkViewPos(pt, view.buffer_id_, view.cursor))
      redraw();
    return;
  }

  std::string filename;
  Buffer buffer;
  ChunkView view;

  enum class Mode {
    INSERT,
    ESCAPE,
    COLON,
  };  
  Mode mode = Mode::INSERT;
  std::vector<uint32_t> command_history;
  std::string colon_text;
  size_t col_col = 0;
};

size_t GetErrorRenderHeight(clang_util::ErrorMessage* error_ptr) {
  using namespace clang_util;
  switch (error_ptr->getKind()) {
  case ErrorMessage::Kind::UnknownError: {
    auto* error = reinterpret_cast<UnknownError*>(error_ptr);
    return (1 + error->trailing.size());
  }
  default:
    break;
  }
  return 0;
}

void DrawErrors(gui::DrawCtx& cr, gui::Point& cursor, clang_util::ErrorMessage* error_ptr) {
  using namespace clang_util;
  std::vector<cairo_glyph_t> glyphs_;
  auto* font = gui::DefaultFont();
  switch (error_ptr->getKind()) {
  case ErrorMessage::Kind::UnknownError: {
    auto* error = reinterpret_cast<UnknownError*>(error_ptr);
    cr.set_source_rgb(1.0, 1.0, 0.0);
    font->LayoutWrap(cursor, glyphs_, "??");
    font->Flush(cr, glyphs_);
    glyphs_.clear();
    double tmp = cursor.x + 10;
    cr.move_to({cursor.x + 5, cursor.y});
    cr.line_to({cursor.x + 5, cursor.y + font->height() * (1 + error->trailing.size()) - 5});
    cr.stroke();

    cursor.x = tmp;
    font->LayoutWrap(cursor, glyphs_, error->message);
    font->Flush(cr, glyphs_);
    glyphs_.clear();
    cursor.y += font->height();

    for (auto& trailing : error->trailing) {
      cursor.x = tmp;
      font->LayoutWrap(cursor, glyphs_, trailing);
      font->Flush(cr, glyphs_);
      glyphs_.clear();
      cursor.y += font->height();
    }

    cursor.x = 2;
    break;
  }
  default:
    break;
  }
}

class F5RebuildWindow : public SubWindow {
 public:
  std::vector<std::unique_ptr<clang_util::ErrorMessage>> errors;
  std::string output;
  int wstatus = 0;
  void Draw(gui::DrawCtx& cr) override {
    // Draw Prompt?? // Draw output?? // Scroll??
    cr.set_source_rgb(0.0, 0.0, 0.0);
    cr.paint();
    auto* font = gui::DefaultFont();

    gui::Point cursor{2, 2};
    for (auto& error : errors) {
      DrawErrors(cr, cursor, error.get());
    }
    cr.set_source_rgb(1.0, 1.0, 1.0);
    std::vector<cairo_glyph_t> glyphs_;
    for (auto& line : Buffer(output).lines) {
      glyphs_.clear();
      font->LayoutWrap(cursor, glyphs_, line);
      font->Flush(cr, glyphs_);
      cursor.x = 2;
      cursor.y += font->height();
    }
  }
  
  void KeyPress(GdkEventKey* event) override {
    if (event->keyval == GDK_KEY_F5) {
      std::string out;
      wstatus = RunWithPipe({
                                "/usr/bin/clang-6.0", "-ferror-limit=1000",
                                "-Wall", // "-fdiagnostics-parseable-fixits",
                                "-fdiagnostics-print-source-range-info", "/tmp/toy.cc",
                                "-o", "/tmp/silly"
                                }, &out, &out);
      errors = clang_util::ParseErrorMessages(out);
      if (wstatus == 0) {
        std::string out2;
        RunWithPipe({"/tmp/silly"}, &out2, &out2);
        output = std::move(out2);
      }
      redraw();
    }
  }
};

class CommandWindow : public SubWindow {
 public:
  class CommandItem {
   public:
    CommandWindow* window = nullptr;
    virtual ~CommandItem() {
    }
    virtual double GetHeight() = 0;
    virtual void Draw(gui::DrawCtx& cr, gui::Point& cursor) = 0;
  };
  class LsResult : public CommandItem {
   public:
    double GetHeight() override {
      auto* font = gui::DefaultFont();
      return font->height() * items.size();
    }
    void Draw(gui::DrawCtx& cr, gui::Point& cursor) override {
      std::vector<cairo_glyph_t> glyphs_;
      auto* font = gui::DefaultFont();
      for (const auto& item : items) {
        glyphs_.clear();
        font->LayoutWrap(cursor, glyphs_, item);
        cr.set_source_rgb(1.0, 1.0, 0.0);
        font->Flush(cr, glyphs_);
        cursor.y += font->height();
        cursor.x = 2;
      }
    }
    std::vector<std::string> items;
  };
  class BaseCommandEntry : public CommandItem {
   public:
    virtual std::string cwd() = 0;
    virtual string_view command() = 0;
    double GetHeight() override {
      auto* font = gui::DefaultFont();
      return font->height() * 2;
    }
    void Draw(gui::DrawCtx& cr, gui::Point& cursor) override {
      auto& st_pt = cursor;
      std::vector<cairo_glyph_t> glyphs_;
      auto* font = gui::DefaultFont();
      font->LayoutWrap(st_pt, glyphs_, cwd());
      cr.set_source_rgb(1.0, 0.0, 0.0);
      font->Flush(cr, glyphs_);
      glyphs_.clear();
      st_pt.y += font->height();
      st_pt.x = 2;
      font->LayoutWrap(st_pt, glyphs_, "$ ");
      font->LayoutWrap(st_pt, glyphs_, command());

      cr.set_source_rgb(1.0, 1.0, 1.0);
      font->Flush(cr, glyphs_);
      st_pt.y += font->height();
      st_pt.x = 2;
    }
  };
  class CommandEntry : public BaseCommandEntry {
   public:
    virtual std::string cwd() { return window->cwd(); }
    virtual string_view command() { return window->command; }
  };
  class FrozenCommandEntry : public BaseCommandEntry {
   public:
    virtual std::string cwd() { return cwd_; }
    virtual string_view command() { return command_; }
    std::string cwd_;
    std::string command_;
  };
  class ClangErrorMessageList : public CommandItem {
   public:
    double GetHeight() override {
      using namespace clang_util;
      size_t num_lines = 0;
      for (auto& error_uptr : errors) { num_lines += GetErrorRenderHeight(error_uptr.get()); }
      return gui::DefaultFont()->height() * num_lines;
    }
    void Draw(gui::DrawCtx& cr, gui::Point& cursor) override {
      for (auto& error : errors) { DrawErrors(cr, cursor, error.get()); }
    }
    std::vector<std::unique_ptr<clang_util::ErrorMessage>> errors;
  };
  CommandWindow() {
    history.emplace_back(new CommandEntry());
    history.back()->window = this;
  }
  std::vector<std::unique_ptr<CommandItem>> history;
  void Draw(gui::DrawCtx& cr) override {
    // Draw Prompt?? // Draw output?? // Scroll??
    cr.set_source_rgb(0.0, 0.0, 0.0);
    cr.paint();

    double height = 0.0;
    for (size_t i = 0; i < history.size(); ++i) {
      height += history[i]->GetHeight();
    }

    double h = rect().shape.h;

    gui::Point st_pt{2, 2
      + h - 4.0 - std::max<double>(h - 4.0, height)};
    for (size_t i = 0; i < history.size(); ++i) {
      history[i]->Draw(cr, st_pt);
    }
  }
  void KeyPress(GdkEventKey* event) override {
    size_t keyval = event->keyval;
    if (LineEdit(keyval, command, cursor)) {
    } else if (keyval == GDK_KEY_Return) {
      auto cmd_result = 
          RunCommand(command);
      if (cmd_result) {
        cmd_result->window = this;
        auto frozen_command = std::make_unique<FrozenCommandEntry>();
        frozen_command->window = this;
        frozen_command->cwd_ = cwd();
        frozen_command->command_ = command;
        history.insert(std::prev(history.end()), std::move(frozen_command));
        history.insert(std::prev(history.end()), std::move(cmd_result));
      }
      std::cerr << "silly: \"" << command << "\"" << std::endl;
      command.clear();
      cursor = 0;
    } else {
      return;
    }
    redraw();
  }
  std::unique_ptr<CommandItem> RunCommand(string_view command) {
    if (command == "ls" || command == "l") {
      auto result = std::make_unique<LsResult>();
      DIR *dp;
      struct dirent *ep;
      dp = opendir(cwd().c_str());

      if (dp != NULL) {
        while ((ep = readdir (dp)))
          result->items.emplace_back(ep->d_name);

        (void) closedir (dp);
      }
      else {

      }
      return result;
    } else if (command == "edit") {
      auto window = std::make_unique<BasicEditorWindow>();
      window->decorated_rect = DefaultRectangle();
      AddWindow(std::move(window));
    } else if (command == "build") {
      std::cerr << "\n\n\n\n";

      std::string out;
      std::string out2;
      int wstatus = RunWithPipe({
                                "/usr/bin/clang-6.0", "-ferror-limit=1000",
                                "-Wall", // "-fdiagnostics-parseable-fixits",
                                "-fdiagnostics-print-source-range-info", "/tmp/toy.cc"
                                }, &out, &out2);
      RunWithStatus({
                                "/usr/bin/clang-6.0", "-ferror-limit=1000",
                                "-Wall", "/tmp/toy.cc"
                                });

      auto err_msgs = clang_util::ParseErrorMessages(out2);
//      std::cerr << "Blah(" << wstatus << ", " << err_msgs.size() << "): " << out2 << "\n";

      auto result = std::make_unique<ClangErrorMessageList>();

      result->errors = std::move(err_msgs);
      return result;
    } else if (command == "f5") {
      auto window = std::make_unique<F5RebuildWindow>();
      window->decorated_rect = DefaultRectangle();
      AddWindow(std::move(window));
    } else if (command == "clear") {
      history.erase(history.begin(), std::prev(history.end()));
    }
    return nullptr;
  }
  std::string cwd() const {
    auto* tmp = realpath(".", nullptr);
    std::string out(tmp);
    free(tmp);
    return out;
  }
  size_t cursor = 0;
  std::string command;
};

std::unique_ptr<SubWindow> MakeCommandWindow() {
  return std::make_unique<CommandWindow>();
}
