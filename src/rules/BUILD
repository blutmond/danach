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

lib emit_passes {
  srcs = ["emit-passes.cc"];
  hdrs = ["emit-passes.h"];
}

passes rules_passes {
  cc_out = "rules-passes.cc";
  h_out = "rules-passes.h";
}

lib tool {
  srcs = ["tool.cc"];
  deps = [rule_spec, process, emit_passes,
          parser.parser_lowering,
          parser.lowering_spec_lowering,
          parser.tokenizer_helper];
}

lib tool2 {
  srcs = ["tool2.cc"];
  deps = [rule_spec, process, emit_passes, rules_passes,
          parser.parser_lowering,
          parser.lowering_spec_lowering,
          parser.tokenizer_helper];
}

link "rules-dynamic" { deps = [tool]; }
link "rules-passes" { deps = [tool2]; }
