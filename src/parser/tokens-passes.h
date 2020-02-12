#pragma once

#include <set>
#include "gen/parser/tokenizer-spec.h"

namespace parser_spec {

struct TokenizerPreEmit {
  NFAGraphDecl* decl;

  std::set<string_view> all_tokens;
};

Module* LowerToNFA(ASTContext& ast_ctx, Module* m);

TokenizerPreEmit* FetchTokenizer(Module* m, string_view name);

void EmitTokenizer(TokenizerPreEmit* tokens, std::ostream& stream, bool is_header);

}  // namespace parser_spec
