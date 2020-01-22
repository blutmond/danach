import "src/parser/types" as types;
import "src/parser/patterns" as patterns;
import 2 of ".gui/parser" as new_parser;

let lowering_spec = new_parser.lowering_spec;
old_lowering_spec emit_lowering_spec {
  src = "emit_lowering_spec";
  cc_out = "emit_lowering_spec.cc";
}

lib lowering_spec_lowering {
  srcs = ["lowering_spec_lowering.cc"];
  hdrs = ["lowering_spec_lowering.h"];
  deps = [lowering_spec, emit_lowering_spec];
}

lib lowering_spec_tool {
  srcs = ["lowering-spec-tool.cc"];
  deps = [lowering_spec_lowering];
}

link "lowering-spec-tool-dynamic" {
  deps = [lowering_spec_tool];
}

old_parser parser_spec {
  parser = "syntax/parser_spec-parser";
  tokens = "syntax/parser_spec-tokenizer";
  cc_out = "parser-spec.cc";
  h_out = "parser-spec.h";
}

old_parser tokenizer_spec {
  parser = "syntax/tokenizer_spec-parser";
  tokens = "syntax/tokenizer_spec-tokenizer";
  cc_out = "tokenizer-spec.cc";
  h_out = "tokenizer-spec.h";
}

lib tokenizer_helper {
  srcs = ["tokenizer_helper.cc"];
}

lib regex_passes {
  srcs = ["tokens/regex_nfa_to_dfa.cc", "tokens/goto_dfa_emitter.cc"];
  deps = [tokenizer_spec, lower_to_nfa, lower_nfa_to_dfa];
}

old_lowering_spec lower_to_nfa {
  src = "tokens/lower_to_nfa"; cc_out = "lower_to_nfa.cc";
}

old_lowering_spec lower_nfa_to_dfa {
  src = "tokens/lower_nfa_to_dfa"; cc_out = "lower_nfa_to_dfa.cc";
}

old_lowering_spec lower_parser_to_functions {
  src = "lower_parser_to_functions"; cc_out = "lower_parser_to_functions.cc";
}

lib parser_lowering {
  srcs = ["parser_lowering.cc"];
  hdrs = ["parser_lowering.h"];
  deps = [
    parser_spec, tokenizer_spec, types.emit_type_decls,
    patterns.emit_parser,
    patterns.type_check,
    patterns.lower_and_merge,
    lower_parser_to_functions, regex_passes
  ];
}

lib tool {
  srcs = ["tool.cc"];
  deps = [parser_lowering];
}

link "parser-dynamic" {
  deps = [tool];
}

lib dbg_tool {
  srcs = ["tool-debug.cc"];
  deps = [
    parser_spec, tokenizer_spec, lower_to_nfa, lower_nfa_to_dfa,
    lower_parser_to_functions
  ];
}

link "dbg-parser" {
  deps = [dbg_tool];
}
