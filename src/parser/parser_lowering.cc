#include "parser/parser_lowering.h"

#include "gen/parser/lower_parser_to_functions.cc"

namespace parser {

production_spec::ModuleContext* DoAnalysis(ASTContext& ast_ctx, production_spec::Module*& m_parser,
                                           parser_spec::Module*& m_tokens) {
  auto* ctx = new production_spec::ModuleContext;
  ctx->ast_context = &ast_ctx;
  m_tokens = parser_spec::LowerToNFA(ast_ctx, m_tokens);
  auto* tokenizer = parser_spec::FetchTokenizer(m_tokens, getTokenizerName(m_parser));
  ctx->all_tokens = tokenizer->all_tokens;

  m_parser = production_spec::lowerProductionToMerge(ctx, m_parser);

  ctx->m = m_parser;
  production_spec::doModuleTypeCheck(ctx, m_parser);

  delete tokenizer;
  return ctx;
}

void EmitParser(std::ostream& stream, production_spec::Module* m_parser,
                parser_spec::Module* m_tokens,
                production_spec::ModuleContext* ctx, bool is_header) {
  if (is_header) {
    stream << "#pragma once\n";
  }
  stream << "#include \"parser/parser-support.h\"\n";
  stream << "#include \"parser/ast-context.h\"\n\n";
  stream << "namespace " << m_parser->mod_name.str << " {\n";
  auto* tokenizer = parser_spec::FetchTokenizer(m_tokens, getTokenizerName(m_parser));
  EmitTokenizer(tokenizer, stream, is_header);
  stream << "}  // namespace " << m_parser->mod_name.str << "\n";
  production_spec::ImplicitDumpTypes(stream, m_parser);

  production_spec::emitBasics(stream, ctx, m_parser, is_header);
  delete tokenizer;
}

}  // namespace parser
