#include "gen/parser/parser-spec.h"
#include "gen/parser/tokenizer-spec.h"
#include "parser/tokens-passes.h"
#include "parser/patterns/pattern-passes.h"
#include "gen/parser/lower_parser_to_functions.cc"

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

  auto* ctx = new production_spec::ModuleContext;
  m2 = parser_spec::LowerToNFA(m2);
  auto* tokenizer = parser_spec::FetchTokenizer(m2, getTokenizerName(m));
  ctx->all_tokens = tokenizer->all_tokens;


  m = production_spec::lowerProductionToMerge(ctx, m);

  ctx->m = m;
  production_spec::doModuleTypeCheck(ctx, m);
  
  // Actual Emit...

  if (is_header) {
    std::cout << "#pragma once\n";
  }
  std::cout << "#include \"parser/parser-support.h\"\n\n";
  auto& stream = std::cout;
  stream << "namespace " << m->mod_name.str << " {\n";
  EmitTokenizer(tokenizer, stream, is_header);
  stream << "}  // namespace " << m->mod_name.str << "\n";
  production_spec::ImplicitDumpTypes(m);

  production_spec::emitBasics(ctx, m, is_header);
}
