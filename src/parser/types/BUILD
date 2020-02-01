import "src/parser" as parser;
import 2 of ".gui/parser" as new_parser;

let emit_type_decls_lowering_spec = new_parser.emit_type_decls_lowering_spec;

lib emit_type_decls {
  srcs = ["emit_type_decls.cc"];
  deps = [parser.parser_spec, emit_type_decls_lowering_spec];
}
