#include "simdb/base-widgets.h"
#include "simdb/reflect.h"
#include "gui/font-face.h"
#include <iostream>

void line_insert(size_t& cursor, std::string& text, uint32_t keyval) {
  text.insert(text.begin() + cursor, keyval);
  cursor += 1;
}

void line_delete(size_t& cursor, std::string& text) {
  if (cursor > 0) {
    cursor -= 1;
    text.erase(text.begin() + cursor);
  }
}

void line_clear(size_t& cursor, std::string& text) {
  text.clear();
  cursor = 0;
}

bool test_insert_char(uint32_t keyval) { return keyval >= ' ' && keyval <= '~'; }
bool test2_delete_char(uint32_t keyval) { return keyval == GDK_KEY_BackSpace; }
bool test2_return_char(uint32_t keyval) { return keyval == GDK_KEY_Return; }

struct LayoutLineState {
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
  void layout(string_view str) {
    just_reset = false;
    font->LayoutWrap(pt, glyphs, str);
  }
  void flush() {
    font->Flush(cr, glyphs);
    glyphs.clear();
  }
};

void integer_draw(int value, DrawLineState& s) {
  auto tmp = std::to_string(value);
  s.layout("int(");
  s.layout(tmp);
  s.layout(");");
  s.flush();
  s.nl();
}

void integer_set(int& out, int value) { out = value; }

struct ExampleStruct {
  int v1 = 0;
  int v2 = 0;
  void draw(DrawLineState& s) {
    s.layout("v1: ");
    integer_draw(v1, s);
    s.layout("v2: ");
    integer_draw(v2, s);
  }
};

ExampleStruct get_example() {
  return {40, 35};
}

void modify_thing(ExampleStruct& silly) {
  fprintf(stderr, "silly: %d, %d ## %p\n", silly.v1, silly.v2, &silly);
  silly.v2 = 70;
}

class V1BoundLens : public TypedObjectRefSource<int> {
 public:
  explicit V1BoundLens(any_ptr base) : base(base) {}
  any_ptr base;
  int* typed_unwrap() override { return &base->unwrap<ExampleStruct>()->v1; }
  const char* type_name() const override { return typeid(int).name(); }
};

class ExampleStructLens : public ApplyFnWrapperBase {
 public:
  any_ptr apply(const std::vector<any_ptr>& args) override {
    return std::make_shared<V1BoundLens>(args[0]);
  }
  const char* type_name() const override { return "\ExampleStruct.v1"; }
  size_t size() const override { return 1; }
};

void any_ptr_draw(any_ptr drawable, DrawLineState& s) {
  if (drawable == nullptr) {
    s.layout("nullptr");
    s.nl(); }
  else if (auto* ptr = dynamic_cast<V1BoundLens*>(drawable.get())) {
    s.layout("lens: ");
    integer_draw(*ptr->typed_unwrap(), s);
  }
  else if (auto* ptr = drawable->unwrap_or_nullptr<int>()) { integer_draw(*ptr, s); }
  else if (auto* ptr = drawable->unwrap_or_nullptr<ExampleStruct>()) { ptr->draw(s); }
  else if (auto* ptr = dynamic_cast<ExampleStructLens*>(drawable.get())) {
    s.layout("\\ExampleStruct.v1");
    s.nl();
  }
  else {
    s.layout("Can't draw: \"");
    s.layout(demangle(drawable->type_name()));
    s.layout("\"");
    s.nl();
  }
}

int do_add(int a, int b) {
  return a + b;
}

int do_silly() {
  return 1;
}

void basic_draw(const std::string& base, DrawLineState& s) {
  s.layout("unknown: ");
  s.layout(base);
  s.flush();
  s.nl();
};

void error_string_draw(const std::string& base, DrawLineState& s) {
  s.layout("error, cannot evaluate: \"");
  s.layout(base);
  s.layout("\"!!");
  s.flush();
  s.nl();
};

class Base {
 public:
  std::string tag;
};

Palette::Palette() {
  vars["add"] = wrap(do_add);
  vars["silly"] = wrap(do_silly);
  vars["example"] = wrap(get_example);
  vars["integer_set"] = wrap(integer_set);
  vars["get_example_v1"] = std::make_shared<ExampleStructLens>();
  vars["examp_modify"] = wrap(modify_thing);
}

void Palette::Draw(gui::DrawCtx& cr) {
  cr.set_source_rgb(1, 1, 0);
  DrawLineState s{{}, cr};
  for (auto item : vars) {
    s.layout(item.first);
    s.layout(" = ");
    ResetScope scope(s);
    any_ptr_draw(item.second, s);
  }
  if (!last_error.empty()) {
    s.layout("error: ");
    s.layout(last_error);
    s.nl();
  }
  s.layout("silly: ");
  s.layout(dumb);
  s.flush();
  s.nl();
}

struct ParsedCommand {
  string_view cmd;
  std::vector<string_view> args;
};

struct InvalidParsedCommandException : public std::exception {
  const char * what() const throw () {
    return "Invalid parsing of command.";
  }
};

bool ParseCommand(string_view cmd, ParsedCommand& out) {
  size_t pos = cmd.find(' ');
  if (pos == 0) return false;
  out.cmd = cmd.substr(0, pos);
  if (pos == string_view::npos) return true;
  cmd.remove_prefix(pos + 1);
  while (!cmd.empty()) {
    size_t pos = cmd.find(' ');
    if (pos == 0) {
      cmd.remove_prefix(1);
      continue;
    }
    out.args.push_back(cmd.substr(0, pos));
    if (pos == string_view::npos) return true;
    cmd.remove_prefix(pos + 1);
  }
  return true;
}

any_ptr Palette::EvalExpression(string_view cmd) {
  ParsedCommand pcmd;
  if (ParseCommand(cmd, pcmd)) {
    auto it = vars.find(std::string(pcmd.cmd));
    if (it == vars.end()) throw InvalidParsedCommandException();
    auto fn = it->second;
    std::vector<any_ptr> args;
    for (auto arg : pcmd.args) {
      auto it = vars.find(std::string(arg));
      if (it != vars.end()) {
        args.push_back(it->second);
      } else throw InvalidParsedCommandException();
    }
    return apply(fn, args);
  } else throw InvalidParsedCommandException();
}

void Palette::RunCommand(string_view cmd) {
  try {
    // auto cmd_input = cmd;
    std::string result_store;
    if (cmd.size() > 4 && cmd.substr(0, 4) == "let ") {
      cmd.remove_prefix(4);
      size_t pos = cmd.find(' ');
      if (pos == string_view::npos || pos == 0) throw InvalidParsedCommandException();
      result_store = std::string(cmd.substr(0, pos));
      cmd.remove_prefix(pos);
      if (cmd.size() > 3 && cmd.substr(0, 3) == " = ") {
        cmd.remove_prefix(3);
        auto tmp = EvalExpression(cmd);
        vars[result_store] = tmp;
      } else throw InvalidParsedCommandException();
    } else {
      EvalExpression(cmd);
    }
    last_error = "";
  } catch(InvalidParsedCommandException& e) {
    last_error = std::string("cannot eval: \"") + std::string(cmd) + std::string("\"!!");
  }
}

void Palette::KeyPress(GdkEventKey* event) {
  any_ptr keyval = wrap(uint32_t(event->keyval));
  any_ptr test_fn = wrap(test_insert_char);
  any_ptr test2_fn = wrap(test2_delete_char);
  any_ptr test3_fn = wrap(test2_return_char);

  // any_ptr result = apply(test_fn, {keyval});
  if (*apply(test_fn, {keyval})->unwrap<bool>()) {
    line_insert(dumb_cursor, dumb, event->keyval);
  }
  if (*apply(test2_fn, {keyval})->unwrap<bool>()) {
    line_delete(dumb_cursor, dumb);
  }
  if (*apply(test3_fn, {keyval})->unwrap<bool>()) {
    RunCommand(dumb);
    line_clear(dumb_cursor, dumb);
  }
}
