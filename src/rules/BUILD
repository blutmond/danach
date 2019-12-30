import "src/parser" as parser;

old_parser rule_spec {
  parser = "rule_spec-parser";
  tokens = "rule_spec-tokenizer";
  cc_out = "rule-spec.cc";
  h_out = "rule-spec.h";
}

lib process {
  srcs = ["process.cc"];
  hdrs = ["process.h"];
}

lib compiler {
  srcs = ["compiler.cc"];
  hdrs = ["compiler.h"];
  deps = [process];
}

lib string_utils {
  srcs = ["string-utils.cc"];
  hdrs = ["string-utils.h"];
}

lib clang_error_parse {
  srcs = ["clang-error-parse.cc"];
  hdrs = ["clang-error-parse.h"];
}

lib emit_passes {
  srcs = ["emit-passes.cc"];
  hdrs = ["emit-passes.h"];
  deps = [emit_passes_meta];
}

lib emit_passes_template {
  srcs = ["emit-passes-template.cc"];
  hdrs = ["emit-passes-template.h"];
  deps = [interface];
}

old_parser interface {
  parser = "syntax/interface-parser";
  tokens = "syntax/interface-tokenizer";
  cc_out = "interface.cc";
  h_out = "interface.h";
}

lib rule_invoker {
  srcs = ["rule-invoker.cc"];
  hdrs = ["rule-invoker.h"];
  deps = [string_utils, compiler,
          parser.parser_lowering,
          parser.lowering_spec_lowering,
          rule_spec];
}

lib tool {
  srcs = ["tool.cc"];
  deps = [rule_invoker];
}

link "rules-dynamic" { deps = [tool]; }
