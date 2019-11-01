import "src/parser" as parser;

old_lowering_spec emit_type_decls_lowering_spec {
  src = "emit_type_decls"; cc_out = "emit_type_decls.cc";
}

lib emit_type_decls {
  srcs = ["emit_type_decls.cc"];
  deps = [parser.parser_spec, emit_type_decls_lowering_spec];
}
