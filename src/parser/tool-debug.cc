#include "gen/parser/parser-spec.h"
#include "gen/parser/tokenizer-spec.h"

#include <unordered_map>
#include <unordered_set>
#include "parser/tokenizer_helper.cc"
#include "parser/regex_nfa_to_dfa.cc"
#include "parser/goto_dfa_emitter.cc"
#include <assert.h>

int main(int argc, char **argv){
  if (argc <= 2) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }
  
  bool is_header = false;

  auto contents = LoadFile(argv[1]);
  auto contents_tok = LoadFile(argv[2]);

  production_spec::Tokenizer tokens(contents.c_str());
  auto* m = production_spec::parser::DoParse(tokens);

  parser_spec::Tokenizer tokens_tok(contents_tok.c_str());
  auto* m2 = parser_spec::parser::DoParse(tokens_tok);

  m2 = parser_spec::LoweringToNFA().visit(m2);
  parser_spec::TokenizerModuleIndex idx(m2);
  
  auto& stream = std::cout;
  stream << "namespace " << m->mod_name.str << " {\n";
  idx.EmitTokenizer("basic", stream, is_header);
  stream << "}  // namespace " << m->mod_name.str << "\n";

  //  auto* ctx = new production_spec::ModuleContext;
  //  ctx->all_tokens = idx.getTokenSet("basic");

  /*
     m = production_spec::lowerProductionToMerge(ctx, m);

  ctx->m = m;
  production_spec::doModuleTypeCheck(ctx, m);
  
  production_spec::ImplicitDumpTypes(m);

  production_spec::emitBasics(ctx, m, is_header);

  // Emit other things??
  // production_spec::Emit(m, idx);
  */
}
