#pragma once

#include "parser/tokens-passes.h"
#include "parser/patterns/pattern-passes.h"
#include "gen/parser/parser-spec.h"
#include "gen/parser/tokenizer-spec.h"

namespace parser {

production_spec::ModuleContext* DoAnalysis(ASTContext& ast_ctx, production_spec::Module*& m_parser,
                                           parser_spec::Module*& m_tokens);

void EmitParser(std::ostream& stream, production_spec::Module* m_parser,
                parser_spec::Module* m_tokens,
                production_spec::ModuleContext* ctx, bool is_header);

}  // namespace parser
