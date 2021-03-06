import "src/parser/types" as types;
import "src/parser/patterns" as patterns;
import 2 of ".gui/parser" as new_parser;

let lowering_spec = new_parser.lowering_spec;
let parser_spec = new_parser.parser_spec;
let tokenizer_spec = new_parser.tokenizer_spec;
let emit_lowering_spec = new_parser.emit_lowering_spec;
let lower_to_nfa = new_parser.lower_to_nfa;
let lower_nfa_to_dfa = new_parser.lower_nfa_to_dfa;
let lower_parser_to_functions = new_parser.lower_parser_to_functions;

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

lib tokenizer_helper {
  srcs = ["tokenizer_helper.cc", "ast-context.cc"];
}

lib regex_passes {
  srcs = ["tokens/regex_nfa_to_dfa.cc", "tokens/goto_dfa_emitter.cc"];
  deps = [tokenizer_spec, lower_to_nfa, lower_nfa_to_dfa];
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
