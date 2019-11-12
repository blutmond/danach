#include "parser/parser_lowering.h"

int main(int argc, char **argv){
  if (argc <= 2) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  auto contents = LoadFile(argv[1]);
  auto contents_tok = LoadFile(argv[2]);

  bool is_header = argc > 3 && argv[3] == string_view("header");

  production_spec::Tokenizer tokens(contents.c_str());
  auto* m = production_spec::parser::DoParse(tokens);

  parser_spec::Tokenizer tokens_tok(contents_tok.c_str());
  auto* m2 = parser_spec::parser::DoParse(tokens_tok);

  auto* ctx = parser::DoAnalysis(m, m2);
  
  // Actual Emit...
  parser::EmitParser(std::cout, m, m2, ctx, is_header);
}
