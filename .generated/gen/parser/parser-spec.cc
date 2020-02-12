#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace production_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t) {
switch(t) {
case arrow: return "arrow";case close_arr: return "close_arr";case close_brace: return "close_brace";case close_paran: return "close_paran";case colon: return "colon";case coloncolon: return "coloncolon";case comma: return "comma";case dot: return "dot";case eof: return "eof";case equal: return "equal";case identifier: return "identifier";case number: return "number";case open_arr: return "open_arr";case open_brace: return "open_brace";case open_paran: return "open_paran";case percent: return "percent";case pipe: return "pipe";case plus: return "plus";case semi: return "semi";case star: return "star";case str: return "str";}
}

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
bb0:
  if (c ==0) { 
 goto bb1; }
  if (c ==10) { 
 ++cur; 
c = *cur; 
 goto bb2; }
  if (c ==' ') { 
 ++cur; 
c = *cur; 
 goto bb3; }
  if (c =='"') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='%') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  if (c =='(') { 
 ++cur; 
c = *cur; 
 goto bb6; }
  if (c ==')') { 
 ++cur; 
c = *cur; 
 goto bb7; }
  if (c =='*') { 
 ++cur; 
c = *cur; 
 goto bb8; }
  if (c =='+') { 
 ++cur; 
c = *cur; 
 goto bb9; }
  if (c ==',') { 
 ++cur; 
c = *cur; 
 goto bb10; }
  if (c =='-') { 
 ++cur; 
c = *cur; 
 goto bb11; }
  if (c =='.') { 
 ++cur; 
c = *cur; 
 goto bb12; }
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb13; }
  if (c ==':') { 
 ++cur; 
c = *cur; 
 goto bb14; }
  if (c ==';') { 
 ++cur; 
c = *cur; 
 goto bb15; }
  if (c =='<') { 
 ++cur; 
c = *cur; 
 goto bb16; }
  if (c =='=') { 
 ++cur; 
c = *cur; 
 goto bb17; }
  if (c =='>') { 
 ++cur; 
c = *cur; 
 goto bb18; }
  if (c >= 'A' && c <= 'Z') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c =='_') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c >= 'a' && c <= 'z') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c =='{') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c =='|') { 
 ++cur; 
c = *cur; 
 goto bb21; }
  if (c =='}') { 
 ++cur; 
c = *cur; 
 goto bb22; }
  unexpected(c);
bb22:
  return MakeToken(tok::close_brace, st, cur);
bb21:
  return MakeToken(tok::pipe, st, cur);
bb20:
  return MakeToken(tok::open_brace, st, cur);
bb19:
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c >= 'A' && c <= 'Z') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c =='_') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c >= 'a' && c <= 'z') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  return MakeToken(tok::identifier, st, cur);
bb18:
  return MakeToken(tok::close_arr, st, cur);
bb17:
  return MakeToken(tok::equal, st, cur);
bb16:
  return MakeToken(tok::open_arr, st, cur);
bb15:
  return MakeToken(tok::semi, st, cur);
bb14:
  if (c ==':') { 
 ++cur; 
c = *cur; 
 goto bb23; }
  return MakeToken(tok::colon, st, cur);
bb23:
  return MakeToken(tok::coloncolon, st, cur);
bb13:
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb13; }
  return MakeToken(tok::number, st, cur);
bb12:
  return MakeToken(tok::dot, st, cur);
bb11:
  if (c =='>') { 
 ++cur; 
c = *cur; 
 goto bb24; }
  unexpected(c);
bb24:
  return MakeToken(tok::arrow, st, cur);
bb10:
  return MakeToken(tok::comma, st, cur);
bb9:
  return MakeToken(tok::plus, st, cur);
bb8:
  return MakeToken(tok::star, st, cur);
bb7:
  return MakeToken(tok::close_paran, st, cur);
bb6:
  return MakeToken(tok::open_paran, st, cur);
bb5:
  return MakeToken(tok::percent, st, cur);
bb4:
  if (c >= ' ' && c <= '!') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='"') { 
 ++cur; 
c = *cur; 
 goto bb25; }
  if (c >= '#' && c <= '[') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='\\') { 
 ++cur; 
c = *cur; 
 goto bb26; }
  if (c >= ']' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb26:
  if (c >= ' ' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb25:
  return MakeToken(tok::str, st, cur);
bb3:
  goto start;
bb2:
  goto start;
bb1:
  return MakeToken(tok::eof, st, cur);
}
} // namespace tok

struct Tokenizer {
  explicit Tokenizer(ASTContext& ctx, const char* cursor_inp) : ctx_(ctx), cursor(cursor_inp) {
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

  template <typename T>
  T* New() { return ctx_.New<T>(); }
 private:
  ASTContext& ctx_;
  const char* start;
  const char* cursor;
  tok::Token current;
};
}  // namespace production_spec
namespace production_spec{
struct PatternStmt;
struct CompoundPatternStmt;
struct StringPatternStmt;
struct AssignPatternStmt;
struct PushPatternStmt;
struct MergePatternStmt;
struct ExprTailLoopPatternStmt;
struct ConditionalPatternStmt;
struct WrapPatternStmt;
struct Decl;
struct TypeDecl;
struct ExprDecl;
struct ProductionAndTypeDecl;
struct ProductionDecl;
struct PatternDecl;
struct LeftAssocDecl;
struct RightAssocDecl;
struct DefineWithTypeDecl;
struct DefineDecl;
struct EntryDecl;
struct TokenizerDecl;
struct Module;
struct PatternExpr;
struct CommaConcatPatternExpr;
struct ConcatPatternExpr;
struct SelfPatternExpr;
struct NewPatternExpr;
struct PopPatternExpr;
struct NamedPatternExpr;
struct TypeLetDecl;
struct TypeDeclExpr;
struct ProductTypeDeclExpr;
struct SumTypeDeclExpr;
struct NamedTypeDeclExpr;
struct ColonTypeDeclExpr;
struct ParametricTypeDeclExpr;

struct PatternStmt {
  enum class Kind {
    Compound, String, Assign, Push, Merge, ExprTailLoop, Conditional, Wrap,
  };
  PatternStmt(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct CompoundPatternStmt: public PatternStmt {
  CompoundPatternStmt() : PatternStmt(Kind::Compound) {}
  std::vector<PatternStmt*> items;
};

struct StringPatternStmt: public PatternStmt {
  StringPatternStmt() : PatternStmt(Kind::String) {}
  tok::Token value;
};

struct AssignPatternStmt: public PatternStmt {
  AssignPatternStmt() : PatternStmt(Kind::Assign) {}
  tok::Token name;
  PatternExpr* value;
};

struct PushPatternStmt: public PatternStmt {
  PushPatternStmt() : PatternStmt(Kind::Push) {}
  PatternExpr* value;
};

struct MergePatternStmt: public PatternStmt {
  MergePatternStmt() : PatternStmt(Kind::Merge) {}
  std::vector<PatternStmt*> items;
};

struct ExprTailLoopPatternStmt: public PatternStmt {
  ExprTailLoopPatternStmt() : PatternStmt(Kind::ExprTailLoop) {}
  PatternExpr* base;
  TypeDeclExpr* type;
  PatternStmt* value;
};

struct ConditionalPatternStmt: public PatternStmt {
  ConditionalPatternStmt() : PatternStmt(Kind::Conditional) {}
  PatternStmt* value;
};

struct WrapPatternStmt: public PatternStmt {
  WrapPatternStmt() : PatternStmt(Kind::Wrap) {}
  PatternExpr* value;
};

struct Decl {
  enum class Kind {
    Type, Expr, ProductionAndType, Production, Pattern, LeftAssoc, RightAssoc, DefineWithType, Define, Entry, Tokenizer,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct TypeDecl: public Decl {
  TypeDecl() : Decl(Kind::Type) {}
  tok::Token name;
  TypeDeclExpr* type;
};

struct ExprDecl: public Decl {
  ExprDecl() : Decl(Kind::Expr) {}
  tok::Token name;
  std::vector<Decl*> stmts;
};

struct ProductionAndTypeDecl: public Decl {
  ProductionAndTypeDecl() : Decl(Kind::ProductionAndType) {}
  tok::Token name;
  TypeDeclExpr* type;
  std::vector<Decl*> stmts;
};

struct ProductionDecl: public Decl {
  ProductionDecl() : Decl(Kind::Production) {}
  tok::Token name;
  std::vector<Decl*> stmts;
};

struct PatternDecl: public Decl {
  PatternDecl() : Decl(Kind::Pattern) {}
  tok::Token name;
  PatternStmt* value;
};

struct LeftAssocDecl: public Decl {
  LeftAssocDecl() : Decl(Kind::LeftAssoc) {}
  std::vector<Decl*> stmts;
};

struct RightAssocDecl: public Decl {
  RightAssocDecl() : Decl(Kind::RightAssoc) {}
  std::vector<Decl*> stmts;
};

struct DefineWithTypeDecl: public Decl {
  DefineWithTypeDecl() : Decl(Kind::DefineWithType) {}
  tok::Token name;
  TypeDeclExpr* type;
  PatternStmt* value;
};

struct DefineDecl: public Decl {
  DefineDecl() : Decl(Kind::Define) {}
  tok::Token name;
  PatternStmt* value;
};

struct EntryDecl: public Decl {
  EntryDecl() : Decl(Kind::Entry) {}
  tok::Token name;
};

struct TokenizerDecl: public Decl {
  TokenizerDecl() : Decl(Kind::Tokenizer) {}
  tok::Token name;
};

struct Module {
  tok::Token mod_name;
  std::vector<Decl*> decls;
};

struct PatternExpr {
  enum class Kind {
    CommaConcat, Concat, Self, New, Pop, Named,
  };
  PatternExpr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct CommaConcatPatternExpr: public PatternExpr {
  CommaConcatPatternExpr() : PatternExpr(Kind::CommaConcat) {}
  tok::Token comma;
  PatternStmt* element;
};

struct ConcatPatternExpr: public PatternExpr {
  ConcatPatternExpr() : PatternExpr(Kind::Concat) {}
  PatternStmt* element;
};

struct SelfPatternExpr: public PatternExpr {
  SelfPatternExpr() : PatternExpr(Kind::Self) {}
};

struct NewPatternExpr: public PatternExpr {
  NewPatternExpr() : PatternExpr(Kind::New) {}
  TypeDeclExpr* type;
  PatternStmt* value;
};

struct PopPatternExpr: public PatternExpr {
  PopPatternExpr() : PatternExpr(Kind::Pop) {}
};

struct NamedPatternExpr: public PatternExpr {
  NamedPatternExpr() : PatternExpr(Kind::Named) {}
  tok::Token name;
};

struct TypeLetDecl {
  tok::Token name;
  TypeDeclExpr* type;
};

struct TypeDeclExpr {
  enum class Kind {
    Product, Sum, Named, Colon, Parametric,
  };
  TypeDeclExpr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct ProductTypeDeclExpr: public TypeDeclExpr {
  ProductTypeDeclExpr() : TypeDeclExpr(Kind::Product) {}
  std::vector<TypeLetDecl*> decls;
};

struct SumTypeDeclExpr: public TypeDeclExpr {
  SumTypeDeclExpr() : TypeDeclExpr(Kind::Sum) {}
  std::vector<TypeLetDecl*> decls;
};

struct NamedTypeDeclExpr: public TypeDeclExpr {
  NamedTypeDeclExpr() : TypeDeclExpr(Kind::Named) {}
  tok::Token name;
};

struct ColonTypeDeclExpr: public TypeDeclExpr {
  ColonTypeDeclExpr() : TypeDeclExpr(Kind::Colon) {}
  TypeDeclExpr* base;
  tok::Token name;
};

struct ParametricTypeDeclExpr: public TypeDeclExpr {
  ParametricTypeDeclExpr() : TypeDeclExpr(Kind::Parametric) {}
  TypeDeclExpr* base;
  std::vector<TypeDeclExpr*> params;
};
}  // namespace production_spec
namespace production_spec{
namespace parser {
Module* DoParse(Tokenizer& tokens);
TypeDeclExpr* _production_TypeDeclExpr_group_0(Tokenizer& tokens);
TypeDeclExpr* _production_TypeDeclExpr_group_1(Tokenizer& tokens);
TypeDeclExpr* _production_TypeDeclExpr(Tokenizer& tokens);
PatternExpr* _production_PatternExpr(Tokenizer& tokens);
PatternStmt* _production_CompoundPatternStmt(Tokenizer& tokens);
PatternStmt* _production_PatternStmt(Tokenizer& tokens);
std::vector<Decl*> _production_DeclList(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
TypeDeclExpr* _production_TypeDeclExpr_group_0(Tokenizer& tokens) {
if (tokens.peak_check_str("(")) {
tokens.expect("(");
auto result = ({
auto __current_self = tokens.New<ProductTypeDeclExpr>();__current_self->decls = ([&]{
std::vector<TypeLetDecl*> __current_vector__;
   if (!tokens.peak_check_str(")")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = ({
auto __current_self = tokens.New<TypeLetDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeDeclExpr(tokens);
__current_self;
});
return result;
 }()); if (tokens.peak_check(tok::comma)) {
   tokens.expect(tok::comma);
 } else { break; }
  }}return __current_vector__;
}())
;
tokens.expect(")");
__current_self;
});
return result;
}
if (tokens.peak_check_str("{")) {
tokens.expect("{");
auto result = ({
auto __current_self = tokens.New<SumTypeDeclExpr>();__current_self->decls = ([&]{
std::vector<TypeLetDecl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = ({
auto __current_self = tokens.New<TypeLetDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->type = _production_TypeDeclExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect("}");
__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = tokens.New<NamedTypeDeclExpr>();__current_self->name = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
TypeDeclExpr* _production_TypeDeclExpr_group_1(Tokenizer& tokens) {
TypeDeclExpr* expr_result = _production_TypeDeclExpr_group_0(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str("::")) {
tokens.expect("::");
auto result = ({
auto __current_self = tokens.New<ColonTypeDeclExpr>();__current_self->base = _tmp_0;
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("<")) {
tokens.expect("<");
auto result = ({
auto __current_self = tokens.New<ParametricTypeDeclExpr>();__current_self->base = _tmp_0;
__current_self->params = ([&]{
std::vector<TypeDeclExpr*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str(">")) { break; }
 __current_vector__.push_back([&]{auto result = _production_TypeDeclExpr(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect(">");
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
TypeDeclExpr* _production_TypeDeclExpr(Tokenizer& tokens) {
auto result = _production_TypeDeclExpr_group_1(tokens);
return result;
}
PatternExpr* _production_PatternExpr(Tokenizer& tokens) {
if (tokens.peak_check_str("comma_array")) {
tokens.expect("comma_array");
auto result = ({
auto __current_self = tokens.New<CommaConcatPatternExpr>();tokens.expect("(");
__current_self->comma = tokens.expect(tok::identifier);
tokens.expect(")");
__current_self->element = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("concat")) {
tokens.expect("concat");
auto result = ({
auto __current_self = tokens.New<ConcatPatternExpr>();__current_self->element = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("_")) {
tokens.expect("_");
auto result = ({
auto __current_self = tokens.New<SelfPatternExpr>();__current_self;
});
return result;
}
if (tokens.peak_check_str("new")) {
tokens.expect("new");
auto result = ({
auto __current_self = tokens.New<NewPatternExpr>();__current_self->type = _production_TypeDeclExpr(tokens);
__current_self->value = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("pop")) {
tokens.expect("pop");
auto result = ({
auto __current_self = tokens.New<PopPatternExpr>();__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = tokens.New<NamedPatternExpr>();__current_self->name = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
PatternStmt* _production_CompoundPatternStmt(Tokenizer& tokens) {
if (tokens.peak_check_str("{")) {
tokens.expect("{");
auto result = ({
auto __current_self = tokens.New<CompoundPatternStmt>();__current_self->items = ([&]{
std::vector<PatternStmt*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_PatternStmt(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect("}");
__current_self;
});
return result;
}
tokens.unexpected();
}
PatternStmt* _production_PatternStmt(Tokenizer& tokens) {
if (tokens.peak_check(tok::str)) {
auto _tmp_0 = tokens.expect(tok::str);
auto result = ({
auto __current_self = tokens.New<StringPatternStmt>();__current_self->value = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check_str("%")) {
tokens.expect("%");
auto result = ({
auto __current_self = tokens.New<AssignPatternStmt>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->value = _production_PatternExpr(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("push")) {
tokens.expect("push");
auto result = ({
auto __current_self = tokens.New<PushPatternStmt>();__current_self->value = _production_PatternExpr(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("merge")) {
tokens.expect("merge");
auto result = ({
auto __current_self = tokens.New<MergePatternStmt>();tokens.expect("(");
__current_self->items = ([&]{
std::vector<PatternStmt*> __current_vector__;
   if (!tokens.peak_check_str(")")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = _production_PatternStmt(tokens);
return result;
 }()); if (tokens.peak_check(tok::comma)) {
   tokens.expect(tok::comma);
 } else { break; }
  }}return __current_vector__;
}())
;
tokens.expect(")");
__current_self;
});
return result;
}
if (tokens.peak_check_str("expr_tail_loop")) {
tokens.expect("expr_tail_loop");
auto result = ({
auto __current_self = tokens.New<ExprTailLoopPatternStmt>();tokens.expect("(");
__current_self->base = _production_PatternExpr(tokens);
tokens.expect(")");
tokens.expect(":");
__current_self->type = _production_TypeDeclExpr(tokens);
__current_self->value = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("try")) {
tokens.expect("try");
auto result = ({
auto __current_self = tokens.New<ConditionalPatternStmt>();__current_self->value = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
auto _tmp_0 = _production_PatternExpr(tokens);
auto result = ({
auto __current_self = tokens.New<WrapPatternStmt>();__current_self->value = _tmp_0;
__current_self;
});
return result;
}
std::vector<Decl*> _production_DeclList(Tokenizer& tokens) {
tokens.expect("{");
auto result = ([&]{
std::vector<Decl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Decl(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect("}");
return result;
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("type")) {
tokens.expect("type");
auto result = ({
auto __current_self = tokens.New<TypeDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->type = _production_TypeDeclExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("expr")) {
tokens.expect("expr");
auto result = ({
auto __current_self = tokens.New<ExprDecl>();__current_self->name = tokens.expect(tok::identifier);
__current_self->stmts = _production_DeclList(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("production")) {
tokens.expect("production");
auto _tmp_0 = tokens.expect(tok::identifier);
if (tokens.peak_check_str(":")) {
tokens.expect(":");
auto result = ({
auto __current_self = tokens.New<ProductionAndTypeDecl>();__current_self->name = _tmp_0;
__current_self->type = _production_TypeDeclExpr(tokens);
__current_self->stmts = _production_DeclList(tokens);
__current_self;
});
return result;
}
auto _tmp_1 = _production_DeclList(tokens);
auto result = ({
auto __current_self = tokens.New<ProductionDecl>();__current_self->name = _tmp_0;
__current_self->stmts = _tmp_1;
__current_self;
});
return result;
}
if (tokens.peak_check_str("pattern")) {
tokens.expect("pattern");
auto result = ({
auto __current_self = tokens.New<PatternDecl>();__current_self->name = tokens.expect(tok::identifier);
__current_self->value = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("left")) {
tokens.expect("left");
auto result = ({
auto __current_self = tokens.New<LeftAssocDecl>();__current_self->stmts = _production_DeclList(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("right")) {
tokens.expect("right");
auto result = ({
auto __current_self = tokens.New<RightAssocDecl>();__current_self->stmts = _production_DeclList(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("define")) {
tokens.expect("define");
auto _tmp_0 = tokens.expect(tok::identifier);
if (tokens.peak_check_str(":")) {
tokens.expect(":");
auto result = ({
auto __current_self = tokens.New<DefineWithTypeDecl>();__current_self->name = _tmp_0;
__current_self->type = _production_TypeDeclExpr(tokens);
__current_self->value = _production_CompoundPatternStmt(tokens);
__current_self;
});
return result;
}
auto _tmp_1 = _production_CompoundPatternStmt(tokens);
auto result = ({
auto __current_self = tokens.New<DefineDecl>();__current_self->name = _tmp_0;
__current_self->value = _tmp_1;
__current_self;
});
return result;
}
if (tokens.peak_check_str("entry")) {
tokens.expect("entry");
auto result = ({
auto __current_self = tokens.New<EntryDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("tokenizer")) {
tokens.expect("tokenizer");
auto result = ({
auto __current_self = tokens.New<TokenizerDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = tokens.New<Module>();tokens.expect("module");
__current_self->mod_name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self->decls = ([&]{
std::vector<Decl*> __current_vector__;
    while (true) {
   if (tokens.peak_check(tok::eof)) { break; }
 __current_vector__.push_back([&]{auto result = _production_Decl(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
__current_self;
});
return result;
}
}  // namespace parser
}  // namespace production_spec
