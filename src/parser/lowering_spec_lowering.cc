#include "parser/lowering_spec_lowering.h"
#include "gen/parser/lowering_spec.h"

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <assert.h>

namespace lowering_spec {

  class ContextFinderContext {
 public:

  struct State {
    std::vector<ContextDecl*> contexts;
  };
  std::map<string_view, ContextDecl*> contexts;
  std::map<string_view, FuncDecl*> funcs;
  std::unordered_map<FuncDecl*, State> edges;

  const std::vector<ContextDecl*>& GetHiddenParams(string_view name) {
    static const std::vector<ContextDecl*> nil_ctx_set;
    auto* func = isFuncUsage(name);
    if (!func) return nil_ctx_set;
    return edges[func].contexts;
  }

  void RegisterContext(ContextDecl* decl) {
    assert(contexts.find(decl->name.str) == contexts.end());
    contexts[decl->name.str] = decl;
  }
  void RegisterFunc(FuncDecl* decl) {
    assert(funcs.find(decl->name.str) == funcs.end());
    funcs[decl->name.str] = decl;
  }

  ContextDecl* isContextUsage(string_view name) {
    auto it = contexts.find(name);
    if (it == contexts.end()) return nullptr;
    return it->second;
  }
  FuncDecl* isFuncUsage(string_view name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) return nullptr;
    return it->second;
  }

  std::string GetStdoutContext() {
    if (isContextUsage("stream")) {
      return "stream"; // GetStream(stream)";
    }
    return "std::cout";
  }

  void HardSetContext(FuncDecl* decl, ContextDecl* ctx) {
    edges[decl].contexts.push_back(ctx);
  }
};

void increment(int& i) { ++i; }

CompoundStmt* AsCompound(Stmt* stmt) {
  assert(stmt->getKind() == Stmt::Kind::Compound);
  return reinterpret_cast<CompoundStmt*>(stmt);
}

// Do analysis:
// - Find functions + contexts.
// - Find all calls to other funcs from funcs.
//  -> each function will have a list of call-sites.
// - Find all usages of contexts (basically name_expr of context).
//
// add context parameters to all calls and defines according to the spec...
//

}  // namespace lowering_spec

#include "gen/parser/emit_lowering_spec.cc"
