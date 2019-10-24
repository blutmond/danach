old_parser lowering_spec {
  parser = "lowering_spec-parser";
  tokens = "lowering_spec-tokenizer";
  cc_out = "lowering_spec.cc";
}

old_lowering_spec emit_lowering_spec {
  src = "emit_lowering_spec";
  cc_out = "emit_lowering_spec.cc";
}

lib lowering_spec_tool {
  srcs = ["lowering-spec-tool.cc"];
  deps = [lowering_spec, emit_lowering_spec];
}

link "lowering-spec-tool-dynamic" {
  deps = [lowering_spec_tool];
}




old_parser parser_spec {
  parser = "parser_spec-parser";
  tokens = "parser_spec-tokenizer";
  cc_out = "parser-spec.cc";
}

old_parser tokenizer_spec {
  parser = "tokenizer_spec-parser";
  tokens = "tokenizer_spec-tokenizer";
  cc_out = "tokenizer-spec.cc";
}

old_lowering_spec lower_to_nfa {
  src = "lower_to_nfa"; cc_out = "lower_to_nfa.cc";
}

old_lowering_spec lower_nfa_to_dfa {
  src = "lower_nfa_to_dfa"; cc_out = "lower_nfa_to_dfa.cc";
}

old_lowering_spec lower_parser_to_functions {
  src = "lower_parser_to_functions"; cc_out = "lower_parser_to_functions.cc";
}

lib tool {
  srcs = ["tool.cc"];
  deps = [
    parser_spec, tokenizer_spec, lower_to_nfa, lower_nfa_to_dfa,
    lower_parser_to_functions
  ];
}

link "parser-dynamic" {
  deps = [tool];
}
