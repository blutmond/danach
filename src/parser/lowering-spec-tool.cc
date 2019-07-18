#include <assert.h>
#include "parser/tokenizer_helper.cc"
#include "gen/parser/lowering_spec.cc"

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

  void HardSetContext(FuncDecl* decl, ContextDecl* ctx) {
    edges[decl].contexts.push_back(ctx);
  }
};

// Do analysis:
// - Find functions + contexts.
// - Find all calls to other funcs from funcs.
//  -> each function will have a list of call-sites.
// - Find all usages of contexts (basically name_expr of context).
//
// add context parameters to all calls and defines according to the spec...
//

}  // namespace lowering_spec

#include "parser/emit-lowering-spec.cc"

int main(int argc, char **argv){
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  auto contents = LoadFile(argv[1]);
  lowering_spec::Tokenizer tokens(contents.c_str());
  auto* m = lowering_spec::parser::DoParse(tokens);

  lowering_spec::Emit(m);
}
