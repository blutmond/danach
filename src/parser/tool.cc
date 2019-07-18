#include <assert.h>
// Most of this file can and should be folded back in as
// "lowering" code, but might need some features for that...
#include "parser/tokenizer_helper.cc"
#include "gen/parser/parser-spec.cc"
#include "gen/parser/tokenizer-spec.cc"
#include <unordered_map>
#include <unordered_set>
#include <assert.h>
#include <memory>
#include <map>
#include <set>
#include "parser/regex_nfa_to_dfa.cc"
#include "parser/goto_dfa_emitter.cc"
namespace production_spec {

PatternExpr* getValue(PatternStmt* s);
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

PatternStmt* RotateFront(CompoundPatternStmt* stmt);

PatternStmt* RotateFront(PatternExpr*& expr) {
  switch (expr->getKind()) {
  case PatternExpr::Kind::CommaConcat:
  case PatternExpr::Kind::Concat:
    std::cerr << "Not supported rotating Concat...\n";
    exit(-1);
  case PatternExpr::Kind::Named:
  case PatternExpr::Kind::Self: {
    auto* res = new PushPatternStmt;
    res->value = expr;
    expr = new PopPatternExpr;
    return res;
  } case PatternExpr::Kind::New: {
    auto* cexpr = reinterpret_cast<NewPatternExpr*>(expr);
    assert(cexpr->value->getKind() == PatternStmt::Kind::Compound);
    return RotateFront(
        reinterpret_cast<CompoundPatternStmt*>(cexpr->value));
  } case PatternExpr::Kind::Pop:
    return nullptr;
  }
}

PatternStmt* RotateFront(CompoundPatternStmt* stmt) {
  // if isCleanSuccessor(stmt->items[0]) return stmt;
  for (size_t i = 0; i < stmt->items.size(); ++i) {
    auto* item = stmt->items[i];
    switch (item->getKind()) {
    case PatternStmt::Kind::Compound:
      return RotateFront(reinterpret_cast<CompoundPatternStmt*>(item));
      break;
    case PatternStmt::Kind::String:
    case PatternStmt::Kind::Push:
      stmt->items.erase(stmt->items.begin() + i);
      return item;
      break;
    case PatternStmt::Kind::Assign: {
      auto* cstmt = reinterpret_cast<AssignPatternStmt*>(item);
      if (auto* res = RotateFront(cstmt->value)) return res;
      break;
    } case PatternStmt::Kind::Wrap: {
      auto* cstmt = reinterpret_cast<WrapPatternStmt*>(item);
      if (auto* res = RotateFront(cstmt->value)) return res;
      break;
    } case PatternStmt::Kind::Merge:
      std::cerr << "Not supported!!\n";
      exit(-1);
      break;
    case PatternStmt::Kind::ExprTailLoop:
      std::cerr << "Not supported rotating ExprTailLoop\n";
      exit(-1);
      break;
    case PatternStmt::Kind::Conditional:
      std::cerr << "Not supported rotating Conditional\n";
      exit(-1);
      break;
    }
  }
  return nullptr;
}

PatternStmt* RotateFrontTry(PatternStmt* stmt) {
  if (stmt->getKind() != PatternStmt::Kind::Conditional) return nullptr;
  auto* cstmt = reinterpret_cast<ConditionalPatternStmt*>(stmt);
  assert(cstmt->value->getKind() == PatternStmt::Kind::Compound);
  return RotateFront(reinterpret_cast<CompoundPatternStmt*>(cstmt->value));
}

CompoundPatternStmt* RotateCompound(CompoundPatternStmt* stmt) {
  auto* child = RotateFront(stmt);
  assert(child && "Could not properly rotate Compound...");
  stmt->items.insert(stmt->items.begin(), child);
  return stmt;
}

CompoundPatternStmt* getTryChild(PatternStmt* cstmt) {
  assert(cstmt->getKind() == PatternStmt::Kind::Conditional);
  auto* cstmt2 = reinterpret_cast<ConditionalPatternStmt*>(cstmt);
  assert(cstmt2->value->getKind() == PatternStmt::Kind::Compound);
  return *reinterpret_cast<CompoundPatternStmt**>(&cstmt2->value);
}

// group(n) -> std::vector<PatternStmt*, std::vector<PatternStmt*>>;
//
// cases:
//  - only one group with more than one element:
//   -> Keep unwrapping.
//  - many groups
//   -> for each prefix:
//   -> try { [prefix] + trys }
//      -> recurse on each body...
//
//   nullptr group allowed but MUST be singular.
//   non-conditional becomes nullptr group...
//
// Could simply introduce more nested "try" blocks...

bool isEqualTry(PatternExpr* a, PatternExpr* b) {
  if (a->getKind() != b->getKind()) return false;
  if (a->getKind() == PatternExpr::Kind::Named) {
    return reinterpret_cast<NamedPatternExpr*>(a)->name.str ==
        reinterpret_cast<NamedPatternExpr*>(b)->name.str;
  }
  if (a->getKind() == PatternExpr::Kind::Self) return true;
  return false;
}

bool isEqualTry(PatternStmt* a, PatternStmt* b) {
  if (a->getKind() != b->getKind()) return false;
  if (a->getKind() == PatternStmt::Kind::String) {
    return reinterpret_cast<StringPatternStmt*>(a)->value.str
        == reinterpret_cast<StringPatternStmt*>(b)->value.str;
  } else if (a->getKind() == PatternStmt::Kind::Push) {
    return isEqualTry(
        reinterpret_cast<PushPatternStmt*>(a)->value,
        reinterpret_cast<PushPatternStmt*>(b)->value);
  }
  return false;
}

struct PatternGroup {
  PatternStmt* common = nullptr;
  std::vector<PatternStmt*> items;
};

struct Grouping {
  PatternStmt* unrotated = nullptr;
  std::vector<PatternGroup> groups;
};

void InsertIntoGroups(PatternStmt* key, PatternStmt* item,
                      Grouping& out) {
  if (!key) {
    assert(!out.unrotated && "duplicate nullptr groups...");
    out.unrotated = item;
    return;
  }
  for (auto& group : out.groups) {
    if (isEqualTry(group.common, key)) {
      group.items.push_back(item);
      return;
    }
  }
  out.groups.emplace_back();
  out.groups.back().common = key;
  out.groups.back().items.push_back(item);
}
Grouping doGrouping(ModuleContext* globals, const std::vector<PatternStmt*>& items) {
  Grouping out;
  for (auto* item : items) {
    InsertIntoGroups(RotateFront(getTryChild(item)), item, out);
  }
  return out;
}

bool isProduction(ModuleContext* globals, PatternStmt* stmt) {
  if (stmt->getKind() != PatternStmt::Kind::Push) return false;
  auto* value = reinterpret_cast<PushPatternStmt*>(stmt)->value;
  if (value->getKind() != PatternExpr::Kind::Named) return false;
  return !globals->isToken(reinterpret_cast<NamedPatternExpr*>(value)->name.str);
}

void Distinguish(ModuleContext* globals, std::vector<PatternStmt*> prefix,
                 std::vector<PatternStmt*>& items) {
  if (items.empty()) return;
  while (true) {
    auto groupings = doGrouping(globals, items);
    if (groupings.groups.size() == 1 && !groupings.unrotated) {
      auto& group = groupings.groups.front();
      if (group.items.size() == 1) {
        auto* stmt = getTryChild(group.items.front());
        stmt->items.insert(stmt->items.begin(), group.common);
        items = {group.items.front()};
        break;
      } else {
        prefix.push_back(groupings.groups.front().common);
        items = std::move(groupings.groups.front().items);
      }
    } else {
      items.clear();
      for (auto& group : groupings.groups) {
        if (group.items.size() == 1) {
          if (isProduction(globals, group.common)) {
            auto* stmt = getTryChild(group.items.front());
            stmt->items.insert(stmt->items.begin(), group.common);
            assert(!groupings.unrotated && "Duplicate unconsumable");
            groupings.unrotated = group.items.front();
          } else {
            auto* stmt = getTryChild(group.items.front());
            stmt->items.insert(stmt->items.begin(), group.common);
            items.push_back(group.items.front());
          }
        } else {
          Distinguish(globals, {group.common}, group.items);
          auto* tmp1 = new ConditionalPatternStmt;
          auto* tmp2 = new CompoundPatternStmt;
          tmp1->value = tmp2;
          tmp2->items = std::move(group.items);
          items.push_back(tmp1);
        }
      }
      if (groupings.unrotated) {
        items.push_back(getTryChild(groupings.unrotated));
      }
      break;
    }
  }
  items.insert(items.begin(), prefix.begin(), prefix.end());
}

CompoundPatternStmt* RotateAndVerifyTrys(ModuleContext* globals, CompoundPatternStmt* stmt) {
  Distinguish(globals, {}, stmt->items);
  return stmt;
}

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

tok::Token WrapToken(tok::Token base, size_t id) {
  auto* tmp = 
      new std::string(std::string(base.str) + "_group_" + std::to_string(id));
  tok::Token res;
  res.str = string_view(*tmp);
  return res;
}

struct ExprGroupedByType {
  std::vector<PatternDecl*> binary;
  std::vector<PatternDecl*> prefix;
  std::vector<PatternDecl*> postfix;
  std::vector<PatternDecl*> literal;
};
struct ExprGroup {
  enum Associtivity {
    LeftToRight, // Evaluation order of tree 
    RightToLeft,
    Unknown, // Binaries will not be able to self-refer.
  };
  enum OperatorType {
    Binary,
    Prefix,
    Postfix,
    Literal
  };
  static const char* OperatorName(OperatorType t) {
    switch (t) {
    case Binary: return "Binary";
    case Prefix: return "Prefix";
    case Postfix: return "Postfix";
    case Literal: return "Literal";
    }
  }
  Associtivity assoc = Unknown;
  std::vector<PatternDecl*> patterns;

  static bool IsSelf(PatternStmt* stmt) {
    auto* expr = getValue(stmt);
    if (!expr) return false;
    return expr->getKind() == PatternExpr::Kind::Self;
  }

  void Populate(const std::vector<Decl*>& decls) {
    for (auto* decl : decls) {
      switch (decl->getKind()) {
      case Decl::Kind::Pattern:
        patterns.push_back(reinterpret_cast<PatternDecl*>(decl));
        break;
      default:
        std::cerr << "Unknown Decl not handled in expr scope\n";
        exit(-1);
      }
    }
  }

  static OperatorType AnalyizeType(PatternDecl* decl) {
    auto* stmt = reinterpret_cast<CompoundPatternStmt*>(decl->value);
    assert(stmt->items.size() > 0);
    bool st_self = IsSelf(stmt->items[0]);
    bool ed_self = IsSelf(stmt->items[stmt->items.size() - 1]);
    for (size_t i = 1; i < stmt->items.size(); ++i) {
      if (IsSelf(stmt->items[i]) && IsSelf(stmt->items[i - 1])) {
        std::cerr << "No two selfs in a row\n";
        exit(-1);
      }
    }
    if (st_self && ed_self) {
      assert(stmt->items.size() > 1);
      return Binary;
    }
    if (st_self && !ed_self) return Postfix;
    if (!st_self && ed_self) return Prefix;
    return Literal;
  }

  ExprGroupedByType DoGrouping() const {
    ExprGroupedByType res;
    for (auto* pattern : patterns) {
      switch(AnalyizeType(pattern)) {
      case Binary:
        res.binary.push_back(pattern);
        break;
      case Prefix:
        res.prefix.push_back(pattern);
        break;
      case Postfix:
        res.postfix.push_back(pattern);
        break;
      case Literal:
        res.literal.push_back(pattern);
        break;
      }
    }
    return res;
  }
};

PatternStmt* makeTryStmtFromPattern(PatternDecl* subdecl, TypeDeclExpr* base_type);

void DebugPrintStmt(PatternStmt*);

PatternExpr* getExprProd(tok::Token base, size_t id) {
  auto* res = new NamedPatternExpr;
  res->name = WrapToken(base, id);
  return res;
}

tok::Token DoExprAnalysis(Module* m, ModuleContext* globals, ExprDecl* decl, TypeDeclExpr* base_type) {
  std::vector<ExprGroup> groups;
  ExprGroup tmp_group;
  for (auto* decl : decl->stmts) {
    switch (decl->getKind()) {
    case Decl::Kind::Pattern:
      tmp_group.patterns.push_back(reinterpret_cast<PatternDecl*>(decl));
      break;
    case Decl::Kind::LeftAssoc: {
      if (!tmp_group.patterns.empty()) {
        groups.push_back(std::move(tmp_group));
      }
      ExprGroup lhs_group;
      lhs_group.assoc = ExprGroup::LeftToRight;
      lhs_group.Populate(reinterpret_cast<LeftAssocDecl*>(decl)->stmts);
      assert(!lhs_group.patterns.empty());
      groups.push_back(std::move(lhs_group));
      tmp_group = ExprGroup();
      break;
    }
    case Decl::Kind::RightAssoc: {
      if (!tmp_group.patterns.empty()) {
        groups.push_back(std::move(tmp_group));
      }
      ExprGroup rhs_group;
      rhs_group.assoc = ExprGroup::RightToLeft;
      rhs_group.Populate(reinterpret_cast<RightAssocDecl*>(decl)->stmts);
      assert(!rhs_group.patterns.empty());
      groups.push_back(std::move(rhs_group));
      tmp_group = ExprGroup();
      break;
    }
    default:
      std::cerr << "Unknown Decl not handled in expr scope\n";
      exit(-1);
    }
  }
  if (!tmp_group.patterns.empty()) {
    groups.push_back(std::move(tmp_group));
  }

  size_t i = 0;
  for (auto& group : groups) {
    auto grouped = group.DoGrouping();
    assert((i != 0 || grouped.literal.size() > 0) && "Base level must contain literals.");
    if (!grouped.binary.empty()) {
      assert(i > 0 && "Binary ops nonsensical on first level");
      assert(grouped.binary.size() == group.patterns.size() && "Must be all binary or nothing");
      auto child_i = group.assoc == ExprGroup::RightToLeft ? i : i - 1;
      auto* lit_body = new CompoundPatternStmt;
      for (auto& binary : grouped.binary) {
        assert(binary->getKind() == Decl::Kind::Pattern);
        auto* subdecl = reinterpret_cast<PatternDecl*>(binary);
        auto* copy = new PatternDecl(*subdecl);
        auto* bodycopy = new CompoundPatternStmt(*reinterpret_cast<CompoundPatternStmt*>(copy->value));
        copy->value = bodycopy;

        {
          auto* tmp = bodycopy->items[0];
          assert(tmp->getKind() == PatternStmt::Kind::Assign);
          auto* assign_tmp = reinterpret_cast<AssignPatternStmt*>(tmp);
          assign_tmp->value = new PopPatternExpr;
        }
        
        {
          auto* tmp = bodycopy->items.back();
          assert(tmp->getKind() == PatternStmt::Kind::Assign);
          auto* assign_tmp = reinterpret_cast<AssignPatternStmt*>(tmp);
          assign_tmp->value = getExprProd(decl->name, child_i);
        }

        lit_body->items.push_back(makeTryStmtFromPattern(copy, base_type));
      }
      if (group.assoc == ExprGroup::RightToLeft) {
        if (i != 0) {
          auto* tmp1 = new ConditionalPatternStmt;
          auto* tmp2 = new CompoundPatternStmt;
          tmp1->value = tmp2;
          auto* res = new WrapPatternStmt;
          res->value = new PopPatternExpr;
          tmp2->items.push_back(res);
          lit_body->items.push_back(tmp1);
        }

        auto* cdecl = new DefineWithTypeDecl;
        cdecl->name = WrapToken(decl->name, i);
        cdecl->type = base_type; 
        lit_body = RotateAndVerifyTrys(globals, lit_body);
        auto* push_prefix = new PushPatternStmt;
        push_prefix->value = getExprProd(decl->name, i - 1);
        lit_body->items.insert(lit_body->items.begin(), push_prefix);
        cdecl->value = lit_body;
        m->decls.push_back(cdecl);
        ++i;
      } else {
        assert(group.assoc == ExprGroup::LeftToRight && "Binary ops must have associativity");
        auto* cdecl = new DefineWithTypeDecl;
        cdecl->name = WrapToken(decl->name, i);
        cdecl->type = base_type; 
        auto* tmp = new CompoundPatternStmt;
        auto* expr_tail_expr = new ExprTailLoopPatternStmt;
        expr_tail_expr->type = base_type;
        expr_tail_expr->base = getExprProd(decl->name, i - 1);
        expr_tail_expr->value = RotateAndVerifyTrys(globals, lit_body);
        tmp->items.push_back(expr_tail_expr);
        cdecl->value = tmp;
        m->decls.push_back(cdecl);
        ++i;
      }
    } else {
      assert(grouped.binary.size() == 0 && "Binary ops not handled\n");
      auto* lit_body = new CompoundPatternStmt;
      for (auto& prefix : grouped.prefix) {
        assert(prefix->getKind() == Decl::Kind::Pattern);
        auto* subdecl = reinterpret_cast<PatternDecl*>(prefix);
        auto* copy = new PatternDecl(*subdecl);
        auto* bodycopy = new CompoundPatternStmt(*reinterpret_cast<CompoundPatternStmt*>(copy->value));
        copy->value = bodycopy;
        auto* tmp = bodycopy->items.back();
        assert(tmp->getKind() == PatternStmt::Kind::Assign);
        auto* assign_tmp = reinterpret_cast<AssignPatternStmt*>(tmp);
        assign_tmp->value = getExprProd(decl->name, i);
        lit_body->items.push_back(makeTryStmtFromPattern(copy, base_type));
      }

      for (auto& literal : grouped.literal) {
        assert(literal->getKind() == Decl::Kind::Pattern);
        lit_body->items.push_back(makeTryStmtFromPattern(reinterpret_cast<PatternDecl*>(literal), base_type));
      }
      
      if (!lit_body->items.empty()) {
        if (i != 0) {
          auto* tmp1 = new ConditionalPatternStmt;
          auto* tmp2 = new CompoundPatternStmt;
          tmp1->value = tmp2;
          auto* res = new WrapPatternStmt;
          res->value = getExprProd(decl->name, i - 1);
          tmp2->items.push_back(res);
          lit_body->items.push_back(tmp1);
        }
//        assert(i == 0 && "TODO: non 0 literals...");
        auto* cdecl = new DefineWithTypeDecl;
        cdecl->name = WrapToken(decl->name, i);
        cdecl->type = base_type; 
        cdecl->value = RotateAndVerifyTrys(globals, lit_body);
        m->decls.push_back(cdecl);
        ++i;
      }

      lit_body = new CompoundPatternStmt;
      for (auto& postfix : grouped.postfix) {
        assert(postfix->getKind() == Decl::Kind::Pattern);
        auto* subdecl = reinterpret_cast<PatternDecl*>(postfix);
        auto* copy = new PatternDecl(*subdecl);
        auto* bodycopy = new CompoundPatternStmt(*reinterpret_cast<CompoundPatternStmt*>(copy->value));
        copy->value = bodycopy;
        auto* tmp = bodycopy->items[0];
        assert(tmp->getKind() == PatternStmt::Kind::Assign);
        auto* assign_tmp = reinterpret_cast<AssignPatternStmt*>(tmp);
        assign_tmp->value = new PopPatternExpr;
        lit_body->items.push_back(makeTryStmtFromPattern(copy, base_type));
      }

      if (!lit_body->items.empty()) {
        auto* cdecl = new DefineWithTypeDecl;
        cdecl->name = WrapToken(decl->name, i);
        cdecl->type = base_type; 
        auto* tmp = new CompoundPatternStmt;
        auto* expr_tail_expr = new ExprTailLoopPatternStmt;
        expr_tail_expr->type = base_type;
        expr_tail_expr->base = getExprProd(decl->name, i - 1);
        expr_tail_expr->value = RotateAndVerifyTrys(globals, lit_body);
        tmp->items.push_back(expr_tail_expr);
        cdecl->value = tmp;
        m->decls.push_back(cdecl);
        ++i;
      }
    }
  }
  return WrapToken(decl->name, i - 1);
}

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

PatternStmt* getFirstItem(PatternStmt* stmt) {
  assert(stmt->getKind() == PatternStmt::Kind::Compound);
  auto& items = reinterpret_cast<CompoundPatternStmt*>(stmt)->items;
  assert(items.size() > 0);
  return items[0];
}

}  // namespace production_spec

#include "gen/parser/lower_parser_to_functions.cc"

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

int main(int argc, char **argv){
  if (argc <= 2) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  auto contents = LoadFile(argv[1]);
  auto contents_tok = LoadFile(argv[2]);

  production_spec::Tokenizer tokens(contents.c_str());
  auto* m = production_spec::parser::DoParse(tokens);

  parser_spec::Tokenizer tokens_tok(contents_tok.c_str());
  auto* m2 = parser_spec::parser::DoParse(tokens_tok);

  auto* ctx = new production_spec::ModuleContext;
  {
    m2 = parser_spec::LoweringToNFA().visit(m2);
    parser_spec::TokenizerModuleIndex idx(m2);
    ctx->all_tokens = idx.getTokenSet(getTokenizerName(m));
    auto& stream = std::cout;
    if (true) {
      stream << "namespace " << m->mod_name.str << " {\n";
      idx.EmitTokenizer("basic", stream);
      stream << "}  // namespace " << m->mod_name.str << "\n";
    }
  }

  m = production_spec::lowerProductionToMerge(ctx, m);

  ctx->m = m;
  production_spec::doModuleTypeCheck(ctx, m);
  
  production_spec::ImplicitDumpTypes(m);

  production_spec::emitBasics(ctx, m);

  // Emit other things??
  // production_spec::Emit(m, idx);
}
