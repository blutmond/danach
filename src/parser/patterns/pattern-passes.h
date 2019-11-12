#pragma once

#include "gen/parser/parser-spec.h"
#include <unordered_map>
#include <unordered_set>
#include <assert.h>

namespace production_spec {

PatternStmt* findSuccessor(PatternStmt* s);

struct ModuleContext {
  std::set<string_view> all_tokens;

  bool isToken(string_view name) {
    return all_tokens.find(name) != all_tokens.end();
  }

  std::unordered_map<PatternExpr*, PatternStmt*> concat_successors; 

  std::map<string_view, DefineWithTypeDecl*> productions;

  void RegisterForTypeChecking(DefineWithTypeDecl* decl) {
    assert(productions.find(decl->name.str) == productions.end());
    productions[decl->name.str] = decl;
  }

  std::unordered_map<PatternStmt*, TypeDeclExpr*> types_cache;

  TypeDeclExpr* CacheType(PatternStmt* s, TypeDeclExpr* t) {
    types_cache[s] = t;
    return t;
  }

  Module* m;

  std::map<string_view, TypeDecl*> known_types;

  ProductTypeDeclExpr* MakeStructType(TypeDeclExpr* struct_typeref,
                                      ProductTypeDeclExpr** new_type) {
    if (struct_typeref->getKind() == TypeDeclExpr::Kind::Named) {
      auto* ref = reinterpret_cast<NamedTypeDeclExpr*>(struct_typeref);
      auto it = known_types.find(ref->name.str);
      if (it == known_types.end()) {
        auto* res = *new_type = new ProductTypeDeclExpr;
        auto* decl = new TypeDecl;
        decl->name = ref->name;
        decl->type = res;
        known_types[ref->name.str] = decl;
        m->decls.push_back(decl);
        return nullptr;
      }
      auto* res = it->second->type;
      assert(res->getKind() == TypeDeclExpr::Kind::Product);
      return reinterpret_cast<ProductTypeDeclExpr*>(res);
    } else if (struct_typeref->getKind() == TypeDeclExpr::Kind::Colon) {
      auto* ref = reinterpret_cast<ColonTypeDeclExpr*>(struct_typeref);
      auto* base = MakeEnumType(ref->base);
      for (auto* subdecl : base->decls) {
        if (subdecl->name.str == ref->name.str) {
          auto* res = subdecl->type;
          assert(res->getKind() == TypeDeclExpr::Kind::Product);
          return reinterpret_cast<ProductTypeDeclExpr*>(res);
        }
      }
      auto* res = *new_type = new ProductTypeDeclExpr;
      auto* entry = new TypeLetDecl;
      entry->name = ref->name;
      entry->type = res;
      base->decls.push_back(entry);
      return nullptr;
    }
    assert(false);
  }
  
  SumTypeDeclExpr* MakeEnumType(TypeDeclExpr* struct_typeref) {
    if (struct_typeref->getKind() == TypeDeclExpr::Kind::Named) {
      auto* ref = reinterpret_cast<NamedTypeDeclExpr*>(struct_typeref);
      auto it = known_types.find(ref->name.str);
      if (it == known_types.end()) {
        auto* res = new SumTypeDeclExpr;
        auto* decl = new TypeDecl;
        decl->name = ref->name;
        decl->type = res;
        known_types[ref->name.str] = decl;
        m->decls.push_back(decl);
        return res;
      }
      auto* res = it->second->type;
      assert(res->getKind() == TypeDeclExpr::Kind::Sum);
      return reinterpret_cast<SumTypeDeclExpr*>(res);
    }
    assert(false);
  }

  void RegisterType(TypeDecl* decl) {
    assert(known_types.find(decl->name.str) == known_types.end());
    known_types[decl->name.str] = decl;
  }

  TypeDeclExpr* getType(string_view name);

  void typeCheckAll();
};

void emitBasics(std::ostream& stream, ModuleContext* globals, Module* m, bool is_header);

Module* lowerProductionToMerge(ModuleContext* globals, Module* module);

void ImplicitDumpTypes(std::ostream& stream, Module* m);

}  // namespace production_spec
