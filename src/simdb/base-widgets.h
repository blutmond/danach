#pragma once

#include "gui/widget-helper.h"
#include "simdb/reflect.h"
#include <map>

struct Palette {
  Palette();
  std::string dumb;
  size_t dumb_cursor;
  std::string last_error;
  std::map<std::string, any_ptr> vars;
  void RunCommand(string_view cmd);
  void Draw(gui::DrawCtx& cr);
  void KeyPress(GdkEventKey* event);
  any_ptr EvalExpression(string_view cmd);
};
