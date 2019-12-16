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

passes_meta emit_passes_meta {
  src = "emit-passes-meta";
  cc_out = "emit-passes-meta.cc";
  h_out = "emit-passes-meta.h";
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

passes rules_passes {
  cc_out = "rules-passes.cc";
  h_out = "rules-passes.h";
}

lib tool {
  srcs = ["tool.cc"];
  deps = [rule_spec, process, emit_passes, rules_passes, emit_passes_template,
          string_utils,
          parser.parser_lowering,
          parser.lowering_spec_lowering,
          parser.tokenizer_helper];
}

link "rules-dynamic" { deps = [tool]; }
link "rules-passes" { deps = [tool2]; }
