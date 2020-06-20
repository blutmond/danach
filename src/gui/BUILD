import "src/rules" as rules;
import "src/parser" as parser;
import 2 of ".gui/gui-gen" as gui_gen;

lib core_gtk_utils {
  srcs = ["font-face.cc"];
  hdrs = ["font-face.h", "cairo-bindings.h", "point.h"];
  deps = [gtk];
}

lib buffer {
  srcs = ["buffer.cc"];
  hdrs = ["buffer.h"];
  deps = [
    parser.tokenizer_helper
  ];
}

lib emit_buffer {
  srcs = ["emit-buffer.cc"];
  hdrs = ["emit-buffer.h"];
  deps = [
    buffer, emit_manifest
  ];
}

lib buffer_edit {
  srcs = ["buffer-edit.cc"];
  hdrs = ["buffer-edit.h"];
  deps = [
    buffer, core_gtk_utils
  ];
}

lib buffer_view {
  srcs = ["buffer-view.cc"];
  hdrs = ["buffer-view.h"];
  deps = [buffer_edit];
}

old_parser emit_manifest {
  parser = "syntax/manifest-parser";
  tokens = "syntax/manifest-tokenizer";
  cc_out = "emit_manifest.cc";
  h_out = "emit_manifest.h";
}

old_parser view_manifest {
  parser = "syntax/view-manifest-parser";
  tokens = "syntax/view-manifest-tokenizer";
  cc_out = "view_manifest.cc";
  h_out = "view_manifest.h";
}

lib so_handoff {
  srcs = ["so-handoff-lib.cc"];
  hdrs = ["so-handoff-lib.h"];
  deps = [so_handle, core_gtk_utils];
}

lib so_handle {
  srcs = ["so-handle.cc"];
  hdrs = ["so-handle.h"];
  deps = [dl];
}

old_parser chunk_manifest {
  parser = "syntax/chunk-manifest-parser";
  tokens = "syntax/chunk-manifest-tokenizer";
  cc_out = "chunk-manifest.cc";
  h_out = "chunk-manifest.h";
}

lib dlmain {
  srcs = ["dlmain.cc"];
  deps = [gtk, rules.process, rules.clang_error_parse, core_gtk_utils, view_manifest,
          parser.tokenizer_helper, buffer, so_handoff, buffer_edit, buffer_view];
}

solink "ide-dynamic.so" { deps = [dlmain]; }

lib graphdraw {
  srcs = ["graph-draw.cc"];
  deps = [gtk, core_gtk_utils, so_handoff];
}

lib editor_widget {
  srcs = ["editor-widget.cc", "editor-widget-build.cc"];
  hdrs = ["editor-widget.h"];
  deps = [gtk, core_gtk_utils, view_manifest, rules.compiler, chunk_manifest,
          rules.string_utils,
          parser.tokenizer_helper, buffer, buffer_edit, buffer_view];
}

lib escape_commands {
  srcs = ["escape-commands.cc"];
  hdrs = ["escape-commands.h"];
  deps = [buffer, buffer_edit];
}

solink "ide-graph.so" { deps = [graphdraw]; }

lib main {
  srcs = ["main.cc"];
  deps = [dl];
}

lib widget_helper {
  srcs = ["widget-helper.cc"];
  hdrs = ["widget-helper.h"];
  deps = [core_gtk_utils];
}

link "gui" { deps = [main]; }

lib widget_spec_lowering {
  srcs = ["widget-spec-lowering.cc"];
  hdrs = ["widget-spec-lowering.h"];
  deps = [parser.tokenizer_helper, gui_gen.widgets_spec];
}

link "widget-example" { deps = [gui_gen.example_widget]; }
