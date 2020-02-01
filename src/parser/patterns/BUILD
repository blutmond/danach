import "src/parser" as parser;
import 2 of ".gui/parser" as new_parser;

let emit_parser_lowering_spec = new_parser.emit_parser_lowering_spec;
let type_check_lowering_spec = new_parser.type_check_lowering_spec;
let lower_and_merge_lowering_spec = new_parser.lower_and_merge_lowering_spec;

lib emit_parser {
  srcs = ["emit_parser.cc"];
  deps = [parser.parser_spec, emit_parser_lowering_spec];
}

lib type_check {
  srcs = ["type_check.cc"];
  deps = [parser.parser_spec, type_check_lowering_spec];
}

lib lower_and_merge {
  srcs = ["lower_and_merge.cc"];
  deps = [parser.parser_spec, lower_and_merge_lowering_spec];
}
