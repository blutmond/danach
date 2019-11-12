#include "parser/patterns/pattern-passes.h"
#include "gen/parser/parser-spec.h"

#include <unordered_map>
#include <unordered_set>

namespace production_spec {

PatternExpr* getValue(PatternStmt* s);

struct EmitContext {
  int Push() { return push_id++; }
  int Pop() { return pop_id++; }
  int push_id = 0;
  int pop_id = 0;


  EmitContext* ConcatContext() {
    auto* res = new EmitContext;
    res->globals = globals;
    return res;
  }

  static EmitContext* makeRoot(ModuleContext* globals) {
    auto* res = new EmitContext;
    res->globals = globals;
    return res;
  }

  TypeDeclExpr* TypeCheck(PatternStmt *stmt) {
    auto it = globals->types_cache.find(stmt);
    if (it != globals->types_cache.end()) {
      return it->second;
    }
    auto* res = new NamedTypeDeclExpr;
    res->name.str = "Unknown";
    return res;
  }

  PatternStmt *GetSuccessor(PatternExpr* expr) {
    auto& concat_successors = globals->concat_successors; 
    auto it = concat_successors.find(expr);
    if (it == concat_successors.end()) return nullptr;
    return it->second;
  }

  EmitContext* NewConditionalContext() {
    auto* result = new EmitContext(*this);
    result->has_result = false;
    return result;
  }

  EmitContext* NewExprTailLoopContext() {
    auto* result = new EmitContext(*this);
    result->is_inside_expr = true;
    return result;
  }

  ModuleContext* globals;
  void RegisterConcatSuccessors(const std::vector<PatternStmt*>& items) {
    auto& concat_successors = globals->concat_successors; 
    for (size_t i = 0; i + 1 < items.size(); ++i) {
      auto* item = items[i];
      auto* value = getValue(item);
      if (value && (value->getKind() == PatternExpr::Kind::Concat
                    || value->getKind() == PatternExpr::Kind::CommaConcat)) {
        PatternStmt* succ = nullptr;
        for (size_t j = i + 1; j < items.size() && !succ; ++j) {
          succ = findSuccessor(items[i + 1]);
        }
        if (succ) concat_successors[value] = succ;
      }
    }
  }

  bool is_inside_expr = false;
  
  bool has_result = false;
};

void emitNewType(std::ostream& stream, TypeDeclExpr* t);
void emitTypeExpr(std::ostream& stream, TypeDeclExpr* t);
void productionName(DefineWithTypeDecl* d);
void DebugPrintExpr(PatternExpr* e);

PatternStmt* getFirstItem(PatternStmt* stmt) {
  assert(stmt->getKind() == PatternStmt::Kind::Compound);
  auto& items = reinterpret_cast<CompoundPatternStmt*>(stmt)->items;
  assert(items.size() > 0);
  return items[0];
}

}  // namespace production_spec

#include "gen/parser/patterns/emit_parser.cc"
