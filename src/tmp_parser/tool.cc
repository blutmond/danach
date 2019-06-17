#include "tokens/tokenizer_helper.cc"
#include "gen/tmp_parser/tokenizer.cc"
#include "gen/tmp_parser/parser.cc"
#include "gen/new_parser/tokenizer.cc"
#include "gen/new_parser/parser.cc"
#include "new_parser/regex_nfa_to_dfa.cc"

#include <assert.h>
#include <memory>
#include <map>

namespace parser_spec {

struct GotoDFACPPEmitter {
  int id_assign = 0;
  std::vector<std::pair<Node*, int>> work_list;
  std::map<Node*, int> assigned;

  int getId(Node* node) {
    auto it = assigned.find(node);
    if (it != assigned.end()) return it->second;
    int id = id_assign;
    ++id_assign;
    assigned[node] = id;
    work_list.push_back({node, id});
    return id;
  }
  void prettyEmitChar(char c) {
    if (c == '\\') {
      std::cout << "'\\\\'";
      return;
    }
    if (std::isprint(c)) {
      std::cout << "'" << c << "'";
      return;
    } else {
      std::cout << (int)c;
    }
  }
  void emitRoot(Node* node) {
    getId(node);
    emitAll();
  }
  void emitAll() {
    while (!work_list.empty()) {
      auto* node = work_list.back().first;
      std::cout << "bb" << work_list.back().second << ":\n";
      work_list.pop_back();
      for (auto* edge: node->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range: {
          auto* tmp = reinterpret_cast<RangeEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  if (c >= ";
          prettyEmitChar(*tmp->start);
          std::cout << " && c <= ";
          prettyEmitChar(*tmp->end);
          std::cout << ") { \n";
          std::cout << " ++cur; \n";
          std::cout << "c = *cur; \n";
          std::cout << " goto bb" << id << "; }\n";
          break;
        } case Edge::Kind::Unary: {
          auto* tmp = reinterpret_cast<UnaryEdge*>(edge);
          int id = getId(tmp->next);
          std::cout << "  if (c ==";
          prettyEmitChar(*tmp->match);
          std::cout << ") { \n";
          std::cout << " ++cur; \n";
          std::cout << "c = *cur; \n";
          std::cout << " goto bb" << id << "; }\n";
          break;
        } case Edge::Kind::SkipTo:
          fprintf(stderr, "Un-eliminated skip-to\n");
          exit(-1);
          break;
        case Edge::Kind::Emit:
          std::cout << "  return MakeToken(tok::" <<
              reinterpret_cast<EmitEdge*>(edge)->name.str <<
              ", st, cur);\n";
          break;
        case Edge::Kind::Ignore:
          std::cout << "  goto start;\n";
          break;
        case Edge::Kind::Unexpected:
          std::cout << "  unexpected(c);\n";
          break;
        }
      }
    }
  }
};

struct TokenizerIndex {
  NFAGraphDecl* decl;

  std::set<string_view> all_tokens;
  TokenizerIndex(NFAGraphDecl* decl) : decl(decl) {
    std::set<Node*> visited;
    std::vector<Node*> work_list = {decl->root};
    while (!work_list.empty()) {
      auto* nxt = work_list.back();
      work_list.pop_back();
      auto add_item = [&](Node* n) {
        if (visited.find(n) == visited.end()) {
          work_list.push_back(n);
          visited.insert(n);
        }
      };
      for (auto* edge : nxt->edges) {
        switch (edge->getKind()) {
        case Edge::Kind::Range:
          add_item(reinterpret_cast<RangeEdge*>(edge)->next);
          break;
        case Edge::Kind::Unary:
          add_item(reinterpret_cast<UnaryEdge*>(edge)->next);
          break;
        case Edge::Kind::Emit:
          all_tokens.insert(reinterpret_cast<EmitEdge*>(edge)->name.str);
          break;
        default:
          break;
        }
      }
    }
//    for (auto token : all_tokens) { std::cout << "Found token: \"" << token << "\"\n"; }
  }

  bool isToken(string_view str) const { return all_tokens.find(str) != all_tokens.end(); }

  void Emit(std::ostream& stream) const {
    stream << "namespace tok {\n";
    stream << "enum T {";
    bool first = false;
    for (auto& name : all_tokens) {
      if (first) stream << ", ";
      stream << name;
      first = true;
    }
    stream << "};\n";
    stream << "const char* StringifyType(T t) {\n";
    stream << "switch(t) {\n";
    for (auto& name : all_tokens) {
      stream << "case " << name << ": return \"" << name << "\";";
    }
    stream << "}\n";
    stream << "}\n";
    stream << R"(
struct Token {
  T type = tok::eof;
  string_view str;
};

Token MakeToken(T t, const char* st, const char* ed) {
  return Token{t, string_view(st, ed - st)};
}

void PrintToken(Token t) {
  std::cout << "tok::" << StringifyType(t.type) << " : \"" << t.str << "\"\n";
}

void unexpected(char c) {
  fprintf(stderr, "unexpected: \"%c\"\n", c);
  exit(-1);
}

Token GetNext(const char*& cur) {
    const char* st;
    int c;
  start:
    st = cur;
    c = *cur;
    goto bb0;
)";

    GotoDFACPPEmitter().emitRoot(decl->root);

    stream << "}\n";

    stream << "} // namespace tok\n";
    stream << R"(
struct Tokenizer {
  explicit Tokenizer(const char* cursor_inp) : cursor(cursor_inp) {
    start = cursor;
    current = tok::GetNext(cursor);
  }
  tok::Token peak() {
    return current;
  }
  tok::Token next() {
    auto res = current;
    current = tok::GetNext(cursor);
    return res;
  }
  tok::Token expect(tok::T t) {
    auto res = next();
    if (t != res.type) {
      auto pos = GetLineInfo(start, res.str.data());
      fprintf(stderr, "error:%d:%d: expected: tok::%s but got:", pos.line, pos.col, tok::StringifyType(t));
      std::cerr << "tok::" << StringifyType(res.type) << " : \"" << res.str << "\"\n";
      exit(-1);
    }
    return res;
  }
  tok::Token expect(const char* c) {
    auto res = next();
    if (c != res.str) {
      auto pos = GetLineInfo(start, res.str.data());
      fprintf(stderr, "error:%d:%d: expected: \"%s\" but got:", pos.line, pos.col, c);
      std::cerr << "tok::" << StringifyType(res.type) << " : \"" << res.str << "\"\n";
      exit(-1);
    }
    return res;
  }
  bool peak_check_str(const char* str) {
    return peak().str == str;
  }
  bool peak_check(tok::T type) {
    return peak().type == type;
  }
  void unexpected() __attribute__ ((__noreturn__)) {
    unexpected(peak());
  }
  void unexpected(tok::Token tok) __attribute__ ((__noreturn__)) {
    auto pos = GetLineInfo(start, tok.str.data());
    fprintf(stderr, "error:%d:%d: unexpected:", pos.line, pos.col);
    std::cerr << "tok::" << StringifyType(tok.type) << " : \"" << tok.str << "\"\n";
    exit(-1);
  }

 private:
  const char* start;
  const char* cursor;
  tok::Token current;
};
)";
  }
};

struct TokenizerModuleIndex {
  TokenizerModuleIndex(Module* m) {
    for (auto* decl : m->decls) {
      if (decl->getKind() == Decl::Kind::NFAGraph) {
        auto* nfa_decl = reinterpret_cast<NFAGraphDecl*>(decl);
        tokenizers[nfa_decl->name.str].reset(new TokenizerIndex(nfa_decl));
      }
    }
  }
  std::map<string_view, std::unique_ptr<TokenizerIndex>> tokenizers;

  void EmitTokenizer(string_view str, std::ostream& stream) const {
    auto it = tokenizers.find(str);
    if (it == tokenizers.end()) {
      std::cerr << "No such tokenizer: " << str << "\n";
      exit(-1);
    }
    it->second->Emit(stream);
  }
};

}  // namespace parser_spec

namespace production_spec {
#include "tmp_parser/dump_types.cc"

NamedTypeDeclExpr* BuiltinName(string_view name) {
  auto* res = new NamedTypeDeclExpr;
  res->name.str = name;
  res->name.type = tok::identifier;
  return res;
}

struct DeclIndex;

struct EmitContext {
  std::ostream& stream;
  DeclIndex* index;
  explicit EmitContext(std::ostream& stream) : stream(stream) {}

};

struct Production {
  virtual ~Production() {}
  tok::Token name;
  virtual void DoEmit(EmitContext& decl_index) const = 0;
  virtual void DoEmitFwd(EmitContext& decl_index) const = 0;
  virtual void CollectType(DeclIndex* decl_index) const = 0;
  virtual TypeDeclExpr* ReferenceTypeDeclExpr(DeclIndex* decl_index) const; 
};

PatternExpr* GetValue(PatternStmt* stmt) {
  if (stmt->getKind() == PatternStmt::Kind::Assign) {
    return reinterpret_cast<AssignPatternStmt*>(stmt)->value;
  }
  if (stmt->getKind() == PatternStmt::Kind::Wrap) {
    return reinterpret_cast<WrapPatternStmt*>(stmt)->value;
  }
  return nullptr;
}

struct DeclIndex {
  const parser_spec::TokenizerModuleIndex* token_index = nullptr;
  tok::Token tokenizer;
  tok::Token entry;
  tok::Token mod_name;
  std::map<string_view, Production*> productions;
  std::map<string_view, TypeDecl*> type_decls;

  const parser_spec::TokenizerIndex& getTokenizer() {
    if (tokenizer.str.empty()) {
      fprintf(stderr, "No tokenizer set.\n");
      exit(-1);
    }
    auto it = token_index->tokenizers.find(tokenizer.str);
    if (it == token_index->tokenizers.end()) {
      std::cerr << "No such tokenizer " << tokenizer.str << "\n";
      exit(-1);
    }
    return *it->second;
  }
  void Register(Production* prod) {
    auto it = productions.find(prod->name.str);
    if (it != productions.end()) {
      std::cerr << "Duplicate name: " << it->second->name.str << "\n";
      exit(-1);
    } else {
      productions[prod->name.str] = prod;
    }
  }

  void EmitModule() {
    EmitContext ctx(std::cout);
    ctx.stream << "namespace " << mod_name.str << " {\n";
    token_index->EmitTokenizer(tokenizer.str, ctx.stream);
    ctx.stream << "}  // namespace " << mod_name.str << "\n";

    for (auto& prod : productions) {
      prod.second->CollectType(this);
    }
    // Emit all types...
    {
      auto* m = new Module;
      m->mod_name = mod_name;
      for (auto& type : type_decls) {
        m->decls.push_back(type.second);
      }
      DumpASTTypes(ctx.stream).visit(m);
    }
    ctx.stream << "namespace " << mod_name.str << " {\n";
    ctx.stream << "namespace parser {\n";
    ctx.index = this;
    // Emit productions here...
    for (auto& prod : productions) {
      prod.second->DoEmitFwd(ctx);
    }
    for (auto& prod : productions) {
      prod.second->DoEmit(ctx);
    }

    ctx.stream << entry.str << "* DoParse(Tokenizer& tokens) { return _production_" << entry.str << "(tokens); }\n";
    ctx.stream << "}  // namespace parser\n";
    ctx.stream << "}  // namespace " << mod_name.str << "\n";
  }
  
  void RegisterType(TypeDecl* t) {
    auto it = type_decls.find(t->name.str);
    if (it == type_decls.end()) {
      type_decls[t->name.str] = t;
    } else {
      if (t->name.str == "TypeLetDecl") {
        // Whitelist
        return;
      }
      // CompareForEquality(it->second, t);
      std::cerr << "Duplicate type decls: " << t->name.str << ".\n";
      exit(-1);
    }
  }

  TypeDeclExpr* TypeCheck(PatternExpr* _expr, string_view self_type_name) {
    switch (_expr->getKind()) {
    case PatternExpr::Kind::CommaConcat: {
      auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(_expr);
      auto* res = new ParametricTypeDeclExpr;
      res->base = BuiltinName("Array");
      res->params.push_back(TypeCheck(expr->element, self_type_name));
      return res;
    } case PatternExpr::Kind::Concat: {
      auto* expr = reinterpret_cast<ConcatPatternExpr*>(_expr);
      auto* res = new ParametricTypeDeclExpr;
      res->base = BuiltinName("Array");
      res->params.push_back(TypeCheck(expr->element, self_type_name));
      return res;
    } case PatternExpr::Kind::Self: {
      if (self_type_name.empty()) {
        std::cerr << "Self not available in context.\n";
        exit(-1);
      }
      return BuiltinName(self_type_name);
    } case PatternExpr::Kind::Named: {
      auto* expr = reinterpret_cast<NamedPatternExpr*>(_expr);
      if (getTokenizer().isToken(expr->name.str)) {
        return BuiltinName("Token");
      } else {
        auto it = productions.find(expr->name.str);
        if (it == productions.end()) {
          std::cerr << "Could not find: " << expr->name.str << "\n";
          exit(-1);
        }
        return it->second->ReferenceTypeDeclExpr(this);
      }
    } case PatternExpr::Kind::New: {
      auto* expr = reinterpret_cast<NewPatternExpr*>(_expr);
      auto* res = new NamedTypeDeclExpr;
      res->name = expr->type_name;
      auto* new_type = new TypeDecl;
      new_type->name = expr->type_name;
      new_type->type = TypeCheck(expr->value, self_type_name);
      RegisterType(new_type);
      return res;
    }
    }
  }

  TypeDeclExpr* TypeCheck(PatternStmt* stmt, string_view self_type_name) {
    switch (stmt->getKind()) {
    case PatternStmt::Kind::Compound: {
      auto* mstmt = reinterpret_cast<CompoundPatternStmt*>(stmt);
      TypeDeclExpr* wrapped_t = nullptr;
      auto* res = new ProductTypeDeclExpr;
      for (auto* item : mstmt->items) {
        switch (item->getKind()) {
        case PatternStmt::Kind::Compound: {
          fprintf(stderr, "Compound Type unhandled.\n");
          exit(-1);
          break;
        } case PatternStmt::Kind::String: {
          break;
        } case PatternStmt::Kind::Assign: {
          auto* stmt = reinterpret_cast<AssignPatternStmt*>(item);
          auto* nt = new TypeLetDecl;
          nt->name = stmt->name;
          nt->type = TypeCheck(stmt->value, self_type_name);
          res->decls.push_back(nt);
          break;
        } case PatternStmt::Kind::Wrap: {
          if (wrapped_t) {
            std::cerr << "There can only be 1 Wrapped type!\n";
            exit(-1);
          }
          wrapped_t = TypeCheck(reinterpret_cast<WrapPatternStmt*>(item)->value, self_type_name);
          break;
        }
        }
      }
      if (wrapped_t != nullptr && !res->decls.empty()) {
        std::cerr << "There can only be wrapped type or assign building a struct.\n";
        exit(-1);
      }
      if (wrapped_t) { return wrapped_t; }
      return res;
    } case PatternStmt::Kind::String: {
      fprintf(stderr, "Raw String Type unhandled.\n");
      exit(-1);
    } case PatternStmt::Kind::Assign: {
      fprintf(stderr, "Raw Assign Type unhandled.\n");
      exit(-1);
    } case PatternStmt::Kind::Wrap: {
      fprintf(stderr, "Raw Wrap Type unhandled.\n");
      exit(-1);
    }
    }
  }

  ProductTypeDeclExpr* PatternToStructType(PatternStmt* stmt, string_view self_type_name) {
    auto* res = TypeCheck(stmt, self_type_name);
    if (res->getKind() != TypeDeclExpr::Kind::Product) {
      fprintf(stderr, "Really wanted product here..\n");
      exit(-1);
    }
    return reinterpret_cast<ProductTypeDeclExpr*>(res);
  }
  
  std::map<PatternExpr*, PatternStmt*> concat_successors; 
  void IndexConcatSuccessors(PatternExpr* expr) {
    switch (expr->getKind()) {
    case PatternExpr::Kind::Concat:
      IndexConcatSuccessors(reinterpret_cast<ConcatPatternExpr*>(expr)->element);
      break;
    case PatternExpr::Kind::CommaConcat:
      IndexConcatSuccessors(reinterpret_cast<CommaConcatPatternExpr*>(expr)->element);
      break;
    case PatternExpr::Kind::New:
      IndexConcatSuccessors(reinterpret_cast<NewPatternExpr*>(expr)->value);
      break;
    }
  }
  void IndexConcatSuccessors(PatternStmt* stmt) {
    switch (stmt->getKind()) {
    case PatternStmt::Kind::Compound: {
      auto& items = reinterpret_cast<CompoundPatternStmt*>(stmt)->items;
      for (size_t i = 0; i < items.size(); ++i) {
        auto* item = items[i];
        auto* value = GetValue(item);
        if (value && (value->getKind() == PatternExpr::Kind::Concat
            || value->getKind() == PatternExpr::Kind::CommaConcat) &&
            i < items.size() - 1) {
          concat_successors[value] = items[i + 1];
        }
        IndexConcatSuccessors(item);
      }
      break;
    }
    case PatternStmt::Kind::Wrap:
      IndexConcatSuccessors(reinterpret_cast<WrapPatternStmt*>(stmt)->value);
      break;
    case PatternStmt::Kind::Assign:
      IndexConcatSuccessors(reinterpret_cast<AssignPatternStmt*>(stmt)->value);
      break;
    case PatternStmt::Kind::String:
      break;
    }
  }
  void IndexConcatSuccessors(Decl* decl) {
    switch (decl->getKind()) {
    case Decl::Kind::Expr:
      for (auto* stmt : reinterpret_cast<ExprDecl*>(decl)->stmts) IndexConcatSuccessors(stmt);
      break;
    case Decl::Kind::ProductionAndType:
      for (auto* stmt : reinterpret_cast<ProductionAndTypeDecl*>(decl)->stmts) IndexConcatSuccessors(stmt);
      break;
    case Decl::Kind::Production:
      for (auto* stmt : reinterpret_cast<ProductionDecl*>(decl)->stmts) IndexConcatSuccessors(stmt);
      break;
    case Decl::Kind::Pattern:
      IndexConcatSuccessors(reinterpret_cast<PatternDecl*>(decl)->value);
      break;
    case Decl::Kind::Define:
      IndexConcatSuccessors(reinterpret_cast<DefineDecl*>(decl)->value);
      break;
    default:
      break;
    }
  }
};

TypeDeclExpr* Production::ReferenceTypeDeclExpr(DeclIndex* decl_index) const {
  auto* res = new NamedTypeDeclExpr;
  res->name = name;
  res->name.type = tok::identifier;
  return res;
}

struct PatternEmitContext : public EmitContext {
  explicit PatternEmitContext(const EmitContext& o) : EmitContext(o) {}

  string_view self_type;
  PatternEmitContext Clone() {
    auto result = *this;
    result.tmpi = 0;
    result.emit_tmpi = 0;
    return result;
  }
  bool EmitPeakCheck(PatternStmt* stmt) {
    if (stmt->getKind() == PatternStmt::Kind::String) {
      stream << "  if (tokens.peak_check_str(" <<
          reinterpret_cast<StringPatternStmt*>(stmt)->value.str << ")) {\n";
      return true;
    } else {
      auto* _expr = GetValue(stmt);
      if (_expr && _expr->getKind() == PatternExpr::Kind::Named) {
        auto* expr = reinterpret_cast<NamedPatternExpr*>(_expr);
        if (index->getTokenizer().isToken(expr->name.str)) {
          stream << "  if (tokens.peak_check(tok::" << expr->name.str << ")) {\n";
          return true;
        }
      }
    }
    return false;
  }
  void EmitValue(PatternExpr* _expr) {
    switch (_expr->getKind()) {
    case PatternExpr::Kind::CommaConcat: {
      auto* expr = reinterpret_cast<CommaConcatPatternExpr*>(_expr);
      auto* type = index->TypeCheck(expr->element, "");
      stream << "  ([&] {\n   std::vector<";
      DumpASTTypes(stream).visitTypeExpr(type);
      stream << "> __current_vector__;\n";
      stream << "    while (true) {\n";
      auto it = index->concat_successors.find(_expr);
      if (it != index->concat_successors.end()) {
        auto* stmt = it->second;
        if (stmt->getKind() == PatternStmt::Kind::String) {
          stream << "    if (!tokens.peak_check_str(" <<
              reinterpret_cast<StringPatternStmt*>(stmt)->value.str << "))";
        } else {
          std::cerr << "Unhandled stmt...\n";
          exit(-1);
        }
      } else {
        stream << "    if (!tokens.peak_check(tok::eof))";
      }
      stream << "{\n";
      stream << " __current_vector__.push_back([&] {";
      {
        PatternEmitContext ext_ctx = Clone();
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(expr->element);
        for (auto* stmt : stmt->items) {
          ext_ctx.EmitToTmp(stmt);
        }
        for (auto* stmt : stmt->items) {
          ext_ctx.CopyFromTmp(stmt);
        }
      }
      stream << " }());";
      stream << " if (tokens.peak_check(tok::" << expr->comma.str << ")) {\n";
      stream << "   tokens.expect(tok::" << expr->comma.str << ");\n";
      stream << " } else { return __current_vector__; }\n";
      stream << "  }}}())\n";
      break;
    } case PatternExpr::Kind::Concat: {
      auto* expr = reinterpret_cast<ConcatPatternExpr*>(_expr);
      auto* type = index->TypeCheck(expr->element, self_type);
      stream << "  ([&] {\n   std::vector<";
      DumpASTTypes(stream).visitTypeExpr(type);
      stream << "> __current_vector__;\n";
      stream << "    while (true) {\n";
      auto it = index->concat_successors.find(_expr);
      if (it != index->concat_successors.end()) {
        auto* stmt = it->second;
        if (stmt->getKind() == PatternStmt::Kind::String) {
          stream << "    if (tokens.peak_check_str(" <<
              reinterpret_cast<StringPatternStmt*>(stmt)->value.str << "))";
        } else {
          std::cerr << "Unhandled stmt...\n";
          exit(-1);
        }
      } else {
        stream << "    if (tokens.peak_check(tok::eof))";
      }
      stream << "    { return __current_vector__; }";
      stream << " __current_vector__.push_back([&] {";
      {
        PatternEmitContext ext_ctx = Clone();
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(expr->element);
        for (auto* stmt : stmt->items) {
          ext_ctx.EmitToTmp(stmt);
        }
        for (auto* stmt : stmt->items) {
          ext_ctx.CopyFromTmp(stmt);
        }
      }
      stream << " }());";
      stream << "  }}())\n";
      break;
    } case PatternExpr::Kind::Named: {
      auto* expr = reinterpret_cast<NamedPatternExpr*>(_expr);
      if (index->getTokenizer().isToken(expr->name.str)) {
        stream << "tokens.expect(tok::" << expr->name.str << ")";
      } else {
        stream << "_production_" << expr->name.str << "(tokens)";
      }
      break;
    } case PatternExpr::Kind::Self:
      if (self_type.empty()) {
        stream << "unhandled(\"self\")";
      } else {
        stream << "_production_" << self_type << "(tokens)";
      }
      break;
    case PatternExpr::Kind::New: {
      auto* expr = reinterpret_cast<NewPatternExpr*>(_expr);
      stream << "([&] {\n";
      stream << "    auto* result = new " << expr->type_name.str << ";\n";
      {
        PatternEmitContext ext_ctx = Clone();
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(expr->value);
        for (auto* stmt : stmt->items) {
          ext_ctx.EmitToTmp(stmt);
        }
        for (auto* stmt : stmt->items) {
          ext_ctx.CopyFromTmp(stmt);
        }
      }
      stream << "    return result;\n";
      stream << "  }())";
      break;
    }
    }
  }
  void EmitToTmp(PatternStmt* astmt) {
    switch (astmt->getKind()) {
    case PatternStmt::Kind::String: {
      stream << "  tokens.expect(" << 
          reinterpret_cast<StringPatternStmt*>(astmt)->value.str << ");\n";
      return;
    } case PatternStmt::Kind::Assign: {
    } case PatternStmt::Kind::Wrap: {
      auto* value = GetValue(astmt);
      stream << "  auto tmp" << tmpi << " = ";
      EmitValue(value);
      stream << ";\n";
      ++tmpi;
      break;
    }
    }
  }
  int tmpi = 0;
  int emit_tmpi = 0;
  void CopyFromTmp(PatternStmt* astmt) {
    if (astmt->getKind() == PatternStmt::Kind::Wrap) {
      stream << "  return tmp0;\n";
    }
    if (astmt->getKind() == PatternStmt::Kind::Assign) {
      auto* stmt = reinterpret_cast<AssignPatternStmt*>(astmt);
      stream << "  result->" << stmt->name.str << " = tmp" << emit_tmpi << ";\n";
      ++emit_tmpi;
    }
  }
};

struct ExprProduction : public Production {
  ExprDecl* decl;
  ExprProduction(ExprDecl* decl) : decl(decl) {}
  struct GroupdByType {
    std::vector<PatternDecl*> binary;
    std::vector<PatternDecl*> prefix;
    std::vector<PatternDecl*> postfix;
    std::vector<PatternDecl*> literal;
  };
  struct Group {
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
      auto* expr = GetValue(stmt);
      if (!expr) return false;
      return expr->getKind() == PatternExpr::Kind::Self;
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

    GroupdByType DoGrouping() const {
      GroupdByType res;
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
  std::vector<Group> groups;

  void EmitSelfSetRef(EmitContext& ctx, PatternStmt* stmt) const {
    assert(stmt->getKind() == PatternStmt::Kind::Assign);
    ctx.stream << "result->" << reinterpret_cast<AssignPatternStmt*>(stmt)->name.str;
  }
  void DoEmit(EmitContext& ctx) const override {
    auto* type = ReferenceTypeDeclExpr(ctx.index);
    // Do Expr analysis...

    for (size_t i = 0; i < groups.size(); ++i) {
      DumpASTTypes(ctx.stream).visitTypeExpr(type);
      ctx.stream << " _production_" << name.str << "_group_" << i << "(Tokenizer& tokens) {  \n";
      DumpASTTypes(ctx.stream).visitTypeExpr(type);
      ctx.stream << " expr = nullptr;";
      auto grouped = groups[i].DoGrouping();
      assert(grouped.binary.size() == 0 && "Binary ops not handled\n");

      for (auto& prefix : grouped.prefix) {
        PatternEmitContext ext_ctx(ctx);
        ext_ctx.self_type = name.str;
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(prefix->value);
        assert(ext_ctx.EmitPeakCheck(stmt->items[0]) && "Exhaustive exprs not allowed");
        std::vector<PatternStmt*> nitems(stmt->items.begin(), stmt->items.end() - 1);
        for (auto* stmt : nitems) {
          ext_ctx.EmitToTmp(stmt);
        }
        ctx.stream << "  auto* result = new " << prefix->name.str << name.str << ";\n";
        for (auto* stmt : nitems) {
          ext_ctx.CopyFromTmp(stmt);
        }
        EmitSelfSetRef(ctx, stmt->items[stmt->items.size() - 1]);
        ctx.stream << " = ";
        ctx.stream << " _production_" << name.str << "_group_" << (i - 1) << "(tokens);\n";
        ctx.stream << "  expr = result;\n";
        ctx.stream << " continue;\n";
        ctx.stream << " }\n";
      }

      bool first_literal = true;
      for (auto& literal : grouped.literal) {
        PatternEmitContext ext_ctx(ctx);
        ext_ctx.self_type = name.str;
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(literal->value);
        if (!first_literal) ext_ctx.stream << " else ";
        assert(ext_ctx.EmitPeakCheck(stmt->items[0]) && "Exhaustive exprs not allowed");
        for (auto* stmt : stmt->items) {
          ext_ctx.EmitToTmp(stmt);
        }
        ctx.stream << "  auto* result = new " << literal->name.str << name.str << ";\n";
        for (auto* stmt : stmt->items) {
          ext_ctx.CopyFromTmp(stmt);
        }
        ctx.stream << "  expr = result;\n";
        ctx.stream << " }\n";
        first_literal = false;
      }

      if (!first_literal) ctx.stream << " else ";
      if (i == 0) {
        ctx.stream << "{ tokens.unexpected(); }\n";
      } else {
        ctx.stream << "  {\n";
        ctx.stream << "     expr = _production_" << name.str << "_group_" << (i - 1) << "(tokens);\n";
        ctx.stream << "  }\n";
      }

      ctx.stream << "  while (true) {\n";
      for (auto& postfix : grouped.postfix) {
        PatternEmitContext ext_ctx(ctx);
        ext_ctx.self_type = name.str;
        auto* stmt = reinterpret_cast<CompoundPatternStmt*>(postfix->value);
        assert(ext_ctx.EmitPeakCheck(stmt->items[1]) && "Exhaustive exprs not allowed");
        std::vector<PatternStmt*> nitems(stmt->items.begin() + 1, stmt->items.end());
        for (auto* stmt : nitems) {
          ext_ctx.EmitToTmp(stmt);
        }
        ctx.stream << "  auto* result = new " << postfix->name.str << name.str << ";\n";
        for (auto* stmt : nitems) {
          ext_ctx.CopyFromTmp(stmt);
        }
        EmitSelfSetRef(ctx, stmt->items[0]);
        ctx.stream << " = expr;\n";
        ctx.stream << "  expr = result;\n";
        ctx.stream << " continue;\n";
        ctx.stream << " }\n";
      }
      ctx.stream << "  }\n";
      ctx.stream << "  return expr;\n}\n";
    }
    DumpASTTypes(ctx.stream).visitTypeExpr(type);
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens) {\n";
    ctx.stream << "  return _production_" << name.str << "_group_" << (groups.size() - 1) << "(tokens);\n";
    ctx.stream << "}\n";
  }
  void DoEmitFwd(EmitContext& ctx) const override {
    auto* type = ReferenceTypeDeclExpr(ctx.index);
    // Do Expr analysis...
    for (size_t i = 0; i < groups.size(); ++i) {
      DumpASTTypes(ctx.stream).visitTypeExpr(type);
      ctx.stream << " _production_" << name.str << "_group_" << i << "(Tokenizer& tokens);\n";
    }
    DumpASTTypes(ctx.stream).visitTypeExpr(type);
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens);\n";
  }
  void CollectType(DeclIndex* decl_index) const override {
    auto* type_decls = &decl_index->type_decls;
    // Need a function which gives the TypeDeclExpr for a PatternStmt.
    auto it = type_decls->find(name.str);
    SumTypeDeclExpr* type = nullptr;
    if (it == type_decls->end()) {
      auto* type_decl = (*type_decls)[name.str] = new TypeDecl;
      type_decl->name = name;
      type_decl->type = type = new SumTypeDeclExpr;
    } else {
      std::cerr << "Type name " << name.str << " already defined.\n";
      exit(-1);
    }
    for (auto& group : groups) {
      for (auto* pattern : group.patterns) {
        auto* nv = new TypeLetDecl;
        type->decls.push_back(nv);
        nv->name = pattern->name;
        nv->type = decl_index->PatternToStructType(pattern->value, name.str);
      }
    }
  }
};

bool IsSame(PatternExpr* aexpr, PatternExpr* bexpr) {
  if (aexpr->getKind() != bexpr->getKind()) return false;
  switch (aexpr->getKind()) {
  case PatternExpr::Kind::Self:
    return true;
  case PatternExpr::Kind::Named: {
    return reinterpret_cast<NamedPatternExpr*>(aexpr)->name.str == 
    reinterpret_cast<NamedPatternExpr*>(bexpr)->name.str;
  } default:
    return false;
  }
}

bool IsSame(PatternStmt* astmt, PatternStmt* bstmt) {
  switch (astmt->getKind()) {
  case PatternStmt::Kind::String: {
    if (bstmt->getKind() != PatternStmt::Kind::String) return false;
    return reinterpret_cast<StringPatternStmt*>(astmt)->value.str ==
        reinterpret_cast<StringPatternStmt*>(bstmt)->value.str;
  } case PatternStmt::Kind::Assign:
  case PatternStmt::Kind::Wrap: {
    auto* avalue = GetValue(astmt);
    auto* bvalue = GetValue(bstmt);
    if (!avalue || !bvalue) return false;
    return IsSame(avalue, bvalue);
  } 
  }
  return false;
}

std::vector<PatternStmt*> FindCommon(const std::vector<PatternStmt*>& a,
                                     const std::vector<PatternStmt*>& b,
                                     size_t st_a = 0, size_t st_b = 0) {
  std::vector<PatternStmt*> out;
  auto n = std::min(a.size() - st_a, b.size() - st_b);
  for (size_t i = 0; i < n; ++i) {
    auto* astmt = a[i + st_a];
    if (!IsSame(astmt, b[i + st_b])) {
      break;
    }
    out.push_back(astmt);
  }
  return out;
}

struct UnionAnalysis {
  // Can only get smaller.
  std::vector<PatternStmt*> common;
  std::vector<UnionAnalysis> children;
  PatternDecl* emit = nullptr;

  void add(PatternDecl* decl, size_t offset = 0) {
    assert(decl->value->getKind() == PatternStmt::Kind::Compound);
    auto* stmt = reinterpret_cast<CompoundPatternStmt*>(decl->value);
    if (children.size() == 0 && emit == nullptr) {
      common = std::vector<PatternStmt*>(stmt->items.begin() + offset,
                                         stmt->items.end());
      emit = decl;
    } else {
      std::vector<PatternStmt*> ncommon = FindCommon(stmt->items, common, offset);
      if (ncommon.size() == common.size()) {
        if (stmt->items.size() == ncommon.size() + offset) {
          if (emit) {
            std::cerr << "Duplicate problem!\n";
            exit(-1);
          } else {
            emit = decl;
          }
        } else {
          for (auto& child: children) {
            if (!FindCommon(stmt->items, child.common, offset + ncommon.size()).empty()) {
              child.add(decl, ncommon.size() + offset);
              return;
            }
          }
          // TODO: Find a proper home...
          UnionAnalysis new_home;
          new_home.add(decl, ncommon.size() + offset);
          children.push_back(new_home);
        }
      } else {
        std::vector<PatternStmt*> tail(common.begin() + ncommon.size(), common.end());
        auto s_copy = std::move(*this);
        *this = UnionAnalysis();
        s_copy.common = tail;
        common = ncommon;
        children.push_back(std::move(s_copy));
        if (stmt->items.size() == ncommon.size() + offset) {
          if (emit) {
            std::cerr << "Duplicate problem!\n";
            exit(-1);
          } else {
            emit = decl;
          }
        } else {
          UnionAnalysis new_home;
          new_home.add(decl, ncommon.size() + offset);
          children.push_back(new_home);
        }
      }
    }
  }
  void EmitCommon(PatternEmitContext& ctx, tok::Token type_name) {
    for (auto* stmt : common) {
      ctx.EmitToTmp(stmt);
    }
    bool exhausted = false;
    for (auto& child : children) {
      PatternEmitContext ctx_copy = ctx;
      assert(!child.common.empty());
      assert(!exhausted);
      auto* stmt = child.common[0];
      if (!ctx_copy.EmitPeakCheck(stmt)) {
        exhausted = true;
      }
      child.EmitCommon(ctx_copy, type_name);
      if (!exhausted) ctx.stream << "  }\n";
    }
    if (exhausted) {
      assert(emit == nullptr);
      return;
    }
    if (emit) {
      ctx.stream << "  auto* result = new " 
          << emit->name.str
          << type_name.str
          << ";\n";
      assert(emit->value->getKind() == PatternStmt::Kind::Compound);
      auto* stmt = reinterpret_cast<CompoundPatternStmt*>(emit->value);
      for (auto* stmt : stmt->items) {
        ctx.CopyFromTmp(stmt);
      }
      ctx.stream << "  return result;\n";
    } else {
      ctx.stream << "  tokens.unexpected();\n";
    }
  }
  void DoEmit(EmitContext& ctx, tok::Token type_name) {
    PatternEmitContext ext_ctx(ctx);
    EmitCommon(ext_ctx, type_name);
  }
};

struct UnionProduction : public Production {
  tok::Token type_name;
  Decl* decl;
  std::vector<PatternDecl*> patterns;
  void DoEmit(EmitContext& ctx) const override {
    UnionAnalysis analysis{};
    for (auto* pattern : patterns) {
      analysis.add(pattern);
    }
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens) {\n";
    analysis.DoEmit(ctx, type_name);
    ctx.stream << "}\n\n";
  }
  void DoEmitFwd(EmitContext& ctx) const override {
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens);\n";
  }
  void CollectType(DeclIndex* decl_index) const override {
    auto* type_decls = &decl_index->type_decls;
    // Need a function which gives the TypeDeclExpr for a PatternStmt.
    auto it = type_decls->find(type_name.str);
    SumTypeDeclExpr* type = nullptr;
    if (it == type_decls->end()) {
      auto* type_decl = (*type_decls)[type_name.str] = new TypeDecl;
      type_decl->name = type_name;
      type_decl->type = type = new SumTypeDeclExpr;
    } else {
      if (it->second->type->getKind() == TypeDeclExpr::Kind::Sum) {
        type = reinterpret_cast<SumTypeDeclExpr*>(it->second->type);
      }
    }
    if (type == nullptr) {
      std::cerr << "Type name " << type_name.str << " already defined.\n";
      exit(-1);
    }
    for (auto* pattern : patterns) {
      auto* nv = new TypeLetDecl;
      type->decls.push_back(nv);
      nv->name = pattern->name;
      nv->type = decl_index->PatternToStructType(pattern->value, type_name.str);
    }
  }
  TypeDeclExpr* ReferenceTypeDeclExpr(DeclIndex* decl_index) const override {
    auto* res = new NamedTypeDeclExpr;
    res->name = type_name;
    res->name.type = tok::identifier;
    return res;
  }
};

struct LooseProduction : public Production {
  PatternDecl* decl;
  void DoEmit(EmitContext& ctx) const override {
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens) {\n";
    assert(decl->value->getKind() == PatternStmt::Kind::Compound);
    PatternEmitContext ext_ctx(ctx);
    auto* stmt = reinterpret_cast<CompoundPatternStmt*>(decl->value);
    for (auto* stmt : stmt->items) {
      ext_ctx.EmitToTmp(stmt);
    }
    for (auto* stmt : stmt->items) {
      ext_ctx.CopyFromTmp(stmt);
    }
    ctx.stream << "}\n";
  }
  void DoEmitFwd(EmitContext& ctx) const override {
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens);\n";
  }
  void CollectType(DeclIndex* decl_index) const override {
  }
  TypeDeclExpr* ReferenceTypeDeclExpr(DeclIndex* decl_index) const override {
    return decl_index->TypeCheck(decl->value, "");
  }
};

struct DefineProduction : public Production {
  DefineDecl* decl;
  void DoEmit(EmitContext& ctx) const override {
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens) {\n";
    assert(decl->value->getKind() == PatternStmt::Kind::Compound);
    PatternEmitContext ext_ctx(ctx);
    auto* stmt = reinterpret_cast<CompoundPatternStmt*>(decl->value);
    for (auto* stmt : stmt->items) {
      ext_ctx.EmitToTmp(stmt);
    }
    ctx.stream << "  auto* result = new " << name.str << ";\n";
    for (auto* stmt : stmt->items) {
      ext_ctx.CopyFromTmp(stmt);
    }
    ctx.stream << "  return result;\n";
    ctx.stream << "}\n";
  }
  void DoEmitFwd(EmitContext& ctx) const override {
    DumpASTTypes(ctx.stream).visitTypeExpr(ReferenceTypeDeclExpr(ctx.index));
    ctx.stream << " _production_" << name.str << "(Tokenizer& tokens);\n";
  }
  void CollectType(DeclIndex* decl_index) const override {
    auto* type_decls = &decl_index->type_decls;
    auto it = type_decls->find(name.str);
    if (it != type_decls->end()) {
      std::cerr << "Define type Production emission duplicate\n";
      exit(-1);
    }
    auto* type_decl = (*type_decls)[name.str] = new TypeDecl;
    type_decl->name = name;
    type_decl->type = decl_index->PatternToStructType(decl->value, "");
  }
};

void Emit(Module* m, const parser_spec::TokenizerModuleIndex& token_index) {
  DeclIndex index;
  index.mod_name = m->mod_name;
  index.token_index = &token_index;
  for (auto* decl : m->decls) {
    index.IndexConcatSuccessors(decl);
    switch (decl->getKind()) {
    case Decl::Kind::Tokenizer:
      index.tokenizer = reinterpret_cast<TokenizerDecl*>(decl)->name;
      break;
    case Decl::Kind::Entry:
      index.entry = reinterpret_cast<EntryDecl*>(decl)->name;
      break;
    case Decl::Kind::Expr: {
      auto* expr_decl = reinterpret_cast<ExprDecl*>(decl);
      auto* expr = new ExprProduction(expr_decl);
      expr->name = expr_decl->name;
      std::vector<PatternDecl*> loose_pattern_decls;
      ExprProduction::Group tmp_group;
      for (auto* decl : expr_decl->stmts) {
        switch (decl->getKind()) {
        case Decl::Kind::Pattern:
          tmp_group.patterns.push_back(reinterpret_cast<PatternDecl*>(decl));
          break;
        default:
          std::cerr << "Unknown Decl not handled in expr scope\n";
          exit(-1);
        }
      }
      if (!tmp_group.patterns.empty()) {
        expr->groups.push_back(std::move(tmp_group));
      }
      index.Register(expr);
      break;
    } case Decl::Kind::Pattern: {
      auto* prod = new LooseProduction();
      prod->decl = reinterpret_cast<PatternDecl*>(decl);
      prod->name = prod->decl->name;
      index.Register(prod);
      break;
    } case Decl::Kind::ProductionAndType: {
      auto* prod = new UnionProduction();
      auto* prod_decl = reinterpret_cast<ProductionAndTypeDecl*>(decl);
      prod->decl = decl;
      prod->name = prod_decl->name;
      prod->type_name = prod_decl->type_name;
      for (auto* decl : prod_decl->stmts) {
        switch (decl->getKind()) {
        case Decl::Kind::Pattern:
          prod->patterns.push_back(reinterpret_cast<PatternDecl*>(decl));
          break;
        default:
          std::cerr << "Unknown Decl not handled in expr scope\n";
          exit(-1);
        }
      }
      index.Register(prod);
      break;
    } case Decl::Kind::Production: {
      auto* prod = new UnionProduction();
      auto* prod_decl = reinterpret_cast<ProductionDecl*>(decl);
      prod->decl = decl;
      prod->name = prod_decl->name;
      prod->type_name = prod_decl->name;
      for (auto* decl : prod_decl->stmts) {
        switch (decl->getKind()) {
        case Decl::Kind::Pattern:
          prod->patterns.push_back(reinterpret_cast<PatternDecl*>(decl));
          break;
        default:
          std::cerr << "Unknown Decl not handled in expr scope\n";
          exit(-1);
        }
      }
      index.Register(prod);
      break;
    } case Decl::Kind::Define: {
      auto* prod = new DefineProduction();
      prod->decl = reinterpret_cast<DefineDecl*>(decl);
      prod->name = prod->decl->name;
      index.Register(prod);
      break;
    } default:
      std::cerr << "Unknown Decl not handled\n";
      exit(-1);
      break;
    }
  }
  index.EmitModule();
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

  m2 = parser_spec::LoweringToNFA().visit(m2);

  parser_spec::TokenizerModuleIndex idx(m2);
  production_spec::Emit(m, idx);
}
