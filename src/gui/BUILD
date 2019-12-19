import "src/rules" as rules;

lib core_gtk_utils {
  srcs = ["font-face.cc"];
  hdrs = ["font-face.h", "cairo-bindings.h", "point.h"];
  deps = [gtk];
}

lib tool {
  srcs = ["tool.cc"];
  deps = [gtk, rules.process, rules.clang_error_parse, dl, core_gtk_utils];
}

link "ide-dynamic" { deps = [tool]; }
