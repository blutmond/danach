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

lib tool {
  srcs = ["tool.cc"];
  deps = [rule_spec, process,
          parser.parser_lowering,
          parser.lowering_spec_lowering,
          parser.tokenizer_helper];
}

link "rules-dynamic" { deps = [tool]; }
