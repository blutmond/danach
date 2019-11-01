import "src/parser" as parser;

old_lowering_spec emit_parser_lowering_spec {
  src = "emit_parser"; cc_out = "emit_parser.cc";
}

lib emit_parser {
  srcs = ["emit_parser.cc"];
  deps = [parser.parser_spec, emit_parser_lowering_spec];
}

old_lowering_spec type_check_lowering_spec {
  src = "type_check"; cc_out = "type_check.cc";
}

lib type_check {
  srcs = ["type_check.cc"];
  deps = [parser.parser_spec, type_check_lowering_spec];
}

old_lowering_spec lower_and_merge_lowering_spec {
  src = "lower_and_merge"; cc_out = "lower_and_merge.cc";
}

lib lower_and_merge {
  srcs = ["lower_and_merge.cc"];
  deps = [parser.parser_spec, lower_and_merge_lowering_spec];
}
