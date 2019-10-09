import "src/new_parser" as parser;

old_parser rule_spec {
  parser = "rule_spec-parser";
  tokens = "rule_spec-tokenizer";
  cc_out = "rule-spec.cc";
}

lib process {
  srcs = ["process.cc"];
  hdrs = ["process.h"];
}

lib tool {
  srcs = ["tool.cc"];
  deps = [rule_spec, process];
}

link "rules-dynamic" { deps = [tool]; }
