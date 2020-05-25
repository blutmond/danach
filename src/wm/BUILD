import "src/rules" as rules;
import "src/parser" as parser;
import "src/gui" as gui;

lib sub_window {
  srcs = ["sub-window.cc"];
  hdrs = ["sub-window.h"];
  deps = [gtk, gui.so_handoff, gui.widget_helper];
}

lib dlmain {
  srcs = ["dlmain.cc"];
  deps = [gtk, gui.so_handoff, gui.widget_helper, sub_window,
          gui.buffer_edit, gui.buffer_view, rules.clang_error_parse,
          rules.process];
}

solink "ide-wm.so" { deps = [dlmain]; }
