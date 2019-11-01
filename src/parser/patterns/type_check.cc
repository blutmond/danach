#include "parser/patterns/pattern-passes.h"
#include "gen/parser/parser-spec.h"

namespace production_spec {

struct FieldTypeCheckContext {
  ModuleContext* globals;
  struct Field {
    string_view name;
    TypeDeclExpr* type;
  };
  std::vector<Field> fields;
  void setField(string_view name, TypeDeclExpr* type) {
    fields.push_back(Field{name, type});
  }

  void DoVerify(TypeDeclExpr* struct_typeref) {
    ProductTypeDeclExpr* struct_type = nullptr;
    if (auto* existing = globals->MakeStructType(struct_typeref, &struct_type)) {
      assert(existing);
      // TODO:
      // - check struct type is complete.
    } else {
      // Make new struct type.
      for (auto& field : fields) {
        auto* fdecl = new TypeLetDecl;
        fdecl->name.str = field.name;
        fdecl->type = field.type;
        struct_type->decls.push_back(fdecl);
      }
    }
  }
};

struct TypeCheckContext {
  TypeDeclExpr* self = nullptr;
  ModuleContext* globals;
  std::vector<TypeDeclExpr*> decls;
  int pop_id = 0;
  
  TypeCheckContext* ConcatContext() {
    auto* res = new TypeCheckContext;
    res->globals = globals;
    res->self = self;
    return res;
  }

  static TypeCheckContext* makeRoot(ModuleContext* globals) {
    auto* res = new TypeCheckContext;
    res->globals = globals;
    return res;
  }

  TypeDeclExpr* getSelfType() {
    assert(self);
    return self; 
  }

  void Push(TypeDeclExpr* expr) {
    assert(expr);
    decls.push_back(expr);
  }
  
  FieldTypeCheckContext* newTypeCtx() {
    auto* res = new FieldTypeCheckContext;
    res->globals = globals;
    return res;
  }

  TypeCheckContext* NewConditionalContext() {
    return new TypeCheckContext(*this);
  }

  TypeDeclExpr* Pop() {
    assert(pop_id < decls.size());
    return decls[pop_id++];
  }
};

static TypeDeclExpr* theTokenType = [] {
  auto* res = new NamedTypeDeclExpr;
  res->name.str = "Token";
  return res;
}();

static TypeDeclExpr* theArrayType = [] {
  auto* res = new NamedTypeDeclExpr;
  res->name.str = "Array";
  return res;
}();

void DebugPrintStmt(PatternStmt* e);

}  // namespace production_spec

#include "gen/parser/patterns/type_check.cc"

namespace production_spec {
  
TypeDeclExpr* ModuleContext::getType(string_view name) {
  auto it = productions.find(name);
  if (it != productions.end()) {
    if (!it->second->type) {
      auto* tc_ctx = TypeCheckContext::makeRoot(this);
      it->second->type = doTypeCheck(tc_ctx, it->second->value);
    }
    assert(it->second->type);
    return it->second->type;
  }
  std::cerr << "Could not find production: " << name << "\n";
  exit(-1);
}

void ModuleContext::typeCheckAll() {
  for (auto& decl : productions) {
    auto* tc_ctx = TypeCheckContext::makeRoot(this);
    doTypeCheck(tc_ctx, decl.second->value);
  }
}

}  // namespace production_spec
