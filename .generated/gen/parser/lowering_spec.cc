#include "parser/parser-support.h"

namespace lowering_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, equalequal, identifier, notequal, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t) {
switch(t) {
case arrow: return "arrow";case close_arr: return "close_arr";case close_brace: return "close_brace";case close_bracket: return "close_bracket";case close_paran: return "close_paran";case colon: return "colon";case coloncolon: return "coloncolon";case comma: return "comma";case dot: return "dot";case eof: return "eof";case equal: return "equal";case equalequal: return "equalequal";case identifier: return "identifier";case notequal: return "notequal";case number: return "number";case open_arr: return "open_arr";case open_brace: return "open_brace";case open_bracket: return "open_bracket";case open_paran: return "open_paran";case percent: return "percent";case pipe: return "pipe";case plus: return "plus";case semi: return "semi";case star: return "star";case str: return "str";}
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
  if (c =='!') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='"') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  if (c =='%') { 
 ++cur; 
c = *cur; 
 goto bb6; }
  if (c =='(') { 
 ++cur; 
c = *cur; 
 goto bb7; }
  if (c ==')') { 
 ++cur; 
c = *cur; 
 goto bb8; }
  if (c =='*') { 
 ++cur; 
c = *cur; 
 goto bb9; }
  if (c =='+') { 
 ++cur; 
c = *cur; 
 goto bb10; }
  if (c ==',') { 
 ++cur; 
c = *cur; 
 goto bb11; }
  if (c =='-') { 
 ++cur; 
c = *cur; 
 goto bb12; }
  if (c =='.') { 
 ++cur; 
c = *cur; 
 goto bb13; }
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb14; }
  if (c ==':') { 
 ++cur; 
c = *cur; 
 goto bb15; }
  if (c ==';') { 
 ++cur; 
c = *cur; 
 goto bb16; }
  if (c =='<') { 
 ++cur; 
c = *cur; 
 goto bb17; }
  if (c =='=') { 
 ++cur; 
c = *cur; 
 goto bb18; }
  if (c =='>') { 
 ++cur; 
c = *cur; 
 goto bb19; }
  if (c >= 'A' && c <= 'Z') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c =='[') { 
 ++cur; 
c = *cur; 
 goto bb21; }
  if (c ==']') { 
 ++cur; 
c = *cur; 
 goto bb22; }
  if (c =='_') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c >= 'a' && c <= 'z') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c =='{') { 
 ++cur; 
c = *cur; 
 goto bb23; }
  if (c =='|') { 
 ++cur; 
c = *cur; 
 goto bb24; }
  if (c =='}') { 
 ++cur; 
c = *cur; 
 goto bb25; }
  unexpected(c);
bb25:
  return MakeToken(tok::close_brace, st, cur);
bb24:
  return MakeToken(tok::pipe, st, cur);
bb23:
  return MakeToken(tok::open_brace, st, cur);
bb22:
  return MakeToken(tok::close_bracket, st, cur);
bb21:
  return MakeToken(tok::open_bracket, st, cur);
bb20:
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c >= 'A' && c <= 'Z') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c =='_') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c >= 'a' && c <= 'z') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  return MakeToken(tok::identifier, st, cur);
bb19:
  return MakeToken(tok::close_arr, st, cur);
bb18:
  if (c =='=') { 
 ++cur; 
c = *cur; 
 goto bb26; }
  return MakeToken(tok::equal, st, cur);
bb26:
  return MakeToken(tok::equalequal, st, cur);
bb17:
  return MakeToken(tok::open_arr, st, cur);
bb16:
  return MakeToken(tok::semi, st, cur);
bb15:
  if (c ==':') { 
 ++cur; 
c = *cur; 
 goto bb27; }
  return MakeToken(tok::colon, st, cur);
bb27:
  return MakeToken(tok::coloncolon, st, cur);
bb14:
  if (c >= '0' && c <= '9') { 
 ++cur; 
c = *cur; 
 goto bb14; }
  return MakeToken(tok::number, st, cur);
bb13:
  return MakeToken(tok::dot, st, cur);
bb12:
  if (c =='>') { 
 ++cur; 
c = *cur; 
 goto bb28; }
  unexpected(c);
bb28:
  return MakeToken(tok::arrow, st, cur);
bb11:
  return MakeToken(tok::comma, st, cur);
bb10:
  return MakeToken(tok::plus, st, cur);
bb9:
  return MakeToken(tok::star, st, cur);
bb8:
  return MakeToken(tok::close_paran, st, cur);
bb7:
  return MakeToken(tok::open_paran, st, cur);
bb6:
  return MakeToken(tok::percent, st, cur);
bb5:
  if (c >= ' ' && c <= '!') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  if (c =='"') { 
 ++cur; 
c = *cur; 
 goto bb29; }
  if (c >= '#' && c <= '[') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  if (c =='\\') { 
 ++cur; 
c = *cur; 
 goto bb30; }
  if (c >= ']' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  unexpected(c);
bb30:
  if (c >= ' ' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  unexpected(c);
bb29:
  return MakeToken(tok::str, st, cur);
bb4:
  if (c =='=') { 
 ++cur; 
c = *cur; 
 goto bb31; }
  unexpected(c);
bb31:
  return MakeToken(tok::notequal, st, cur);
bb3:
  goto start;
bb2:
  goto start;
bb1:
  return MakeToken(tok::eof, st, cur);
}
} // namespace tok

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
}  // namespace lowering_spec
namespace lowering_spec{
struct Stmt;
struct CompoundStmt;
struct ReturnVoidStmt;
struct ReturnStmt;
struct LetStmt;
struct VarStmt;
struct OpenWithTypeStmt;
struct OpenStmt;
struct CaseStmt;
struct ForStmt;
struct LoopStmt;
struct IfElseStmt;
struct IfStmt;
struct ScopeStmt;
struct DefaultStmt;
struct EmitterStmt;
struct DbgEmitterStmt;
struct BreakStmt;
struct DiscardStmt;
struct Decl;
struct ContextDecl;
struct FuncDecl;
struct Expr;
struct NewExpr;
struct NumberExpr;
struct StrExpr;
struct NamedExpr;
struct DotExpr;
struct ArrowExpr;
struct IndexExpr;
struct ColonColonExpr;
struct CallExpr;
struct CompEqEqExpr;
struct AssignExpr;
struct FuncArg;
struct Module;
struct TypeRef;
struct VoidTypeRef;
struct NamedTypeRef;
struct MemberTypeRef;
struct TemplateTypeRef;

struct Stmt {
  enum class Kind {
    Compound, ReturnVoid, Return, Let, Var, OpenWithType, Open, Case, For, Loop, IfElse, If, Scope, Default, Emitter, DbgEmitter, Break, Discard,
  };
  Stmt(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct CompoundStmt: public Stmt {
  CompoundStmt() : Stmt(Kind::Compound) {}
  std::vector<Stmt*> stmts;
};

struct ReturnVoidStmt: public Stmt {
  ReturnVoidStmt() : Stmt(Kind::ReturnVoid) {}
};

struct ReturnStmt: public Stmt {
  ReturnStmt() : Stmt(Kind::Return) {}
  Expr* expr;
};

struct LetStmt: public Stmt {
  LetStmt() : Stmt(Kind::Let) {}
  tok::Token name;
  Expr* expr;
};

struct VarStmt: public Stmt {
  VarStmt() : Stmt(Kind::Var) {}
  tok::Token name;
  TypeRef* type;
};

struct OpenWithTypeStmt: public Stmt {
  OpenWithTypeStmt() : Stmt(Kind::OpenWithType) {}
  tok::Token name;
  TypeRef* type;
  Stmt* body;
};

struct OpenStmt: public Stmt {
  OpenStmt() : Stmt(Kind::Open) {}
  tok::Token name;
  Stmt* body;
};

struct CaseStmt: public Stmt {
  CaseStmt() : Stmt(Kind::Case) {}
  tok::Token name;
};

struct ForStmt: public Stmt {
  ForStmt() : Stmt(Kind::For) {}
  tok::Token name;
  Expr* sequence;
  Stmt* body;
};

struct LoopStmt: public Stmt {
  LoopStmt() : Stmt(Kind::Loop) {}
  Stmt* body;
};

struct IfElseStmt: public Stmt {
  IfElseStmt() : Stmt(Kind::IfElse) {}
  Expr* cond;
  Stmt* body;
  Stmt* else_body;
};

struct IfStmt: public Stmt {
  IfStmt() : Stmt(Kind::If) {}
  Expr* cond;
  Stmt* body;
};

struct ScopeStmt: public Stmt {
  ScopeStmt() : Stmt(Kind::Scope) {}
  tok::Token name;
  Expr* expr;
  Stmt* body;
};

struct DefaultStmt: public Stmt {
  DefaultStmt() : Stmt(Kind::Default) {}
};

struct EmitterStmt: public Stmt {
  EmitterStmt() : Stmt(Kind::Emitter) {}
  Stmt* body;
};

struct DbgEmitterStmt: public Stmt {
  DbgEmitterStmt() : Stmt(Kind::DbgEmitter) {}
  Stmt* body;
};

struct BreakStmt: public Stmt {
  BreakStmt() : Stmt(Kind::Break) {}
};

struct DiscardStmt: public Stmt {
  DiscardStmt() : Stmt(Kind::Discard) {}
  Expr* expr;
};

struct Decl {
  enum class Kind {
    Context, Func,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct ContextDecl: public Decl {
  ContextDecl() : Decl(Kind::Context) {}
  tok::Token name;
  TypeRef* type;
};

struct FuncDecl: public Decl {
  FuncDecl() : Decl(Kind::Func) {}
  tok::Token name;
  std::vector<FuncArg*> args;
  TypeRef* ret_t;
  Stmt* body;
};

struct Expr {
  enum class Kind {
    New, Number, Str, Named, Dot, Arrow, Index, ColonColon, Call, CompEqEq, Assign,
  };
  Expr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct NewExpr: public Expr {
  NewExpr() : Expr(Kind::New) {}
  TypeRef* type;
  Stmt* body;
};

struct NumberExpr: public Expr {
  NumberExpr() : Expr(Kind::Number) {}
  tok::Token value;
};

struct StrExpr: public Expr {
  StrExpr() : Expr(Kind::Str) {}
  tok::Token value;
};

struct NamedExpr: public Expr {
  NamedExpr() : Expr(Kind::Named) {}
  tok::Token name;
};

struct DotExpr: public Expr {
  DotExpr() : Expr(Kind::Dot) {}
  Expr* base;
  tok::Token name;
};

struct ArrowExpr: public Expr {
  ArrowExpr() : Expr(Kind::Arrow) {}
  Expr* base;
  tok::Token name;
};

struct IndexExpr: public Expr {
  IndexExpr() : Expr(Kind::Index) {}
  Expr* base;
  std::vector<Expr*> args;
};

struct ColonColonExpr: public Expr {
  ColonColonExpr() : Expr(Kind::ColonColon) {}
  Expr* base;
  tok::Token name;
};

struct CallExpr: public Expr {
  CallExpr() : Expr(Kind::Call) {}
  Expr* base;
  std::vector<Expr*> args;
};

struct CompEqEqExpr: public Expr {
  CompEqEqExpr() : Expr(Kind::CompEqEq) {}
  Expr* lhs;
  Expr* rhs;
};

struct AssignExpr: public Expr {
  AssignExpr() : Expr(Kind::Assign) {}
  Expr* lhs;
  Expr* rhs;
};

struct FuncArg {
  tok::Token name;
  TypeRef* type;
};

struct Module {
  tok::Token mod_name;
  std::vector<Decl*> decls;
};

struct TypeRef {
  enum class Kind {
    Void, Named, Member, Template,
  };
  TypeRef(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct VoidTypeRef: public TypeRef {
  VoidTypeRef() : TypeRef(Kind::Void) {}
};

struct NamedTypeRef: public TypeRef {
  NamedTypeRef() : TypeRef(Kind::Named) {}
  tok::Token name;
};

struct MemberTypeRef: public TypeRef {
  MemberTypeRef() : TypeRef(Kind::Member) {}
  TypeRef* base;
  tok::Token name;
};

struct TemplateTypeRef: public TypeRef {
  TemplateTypeRef() : TypeRef(Kind::Template) {}
  TypeRef* base;
  std::vector<TypeRef*> args;
};
}  // namespace lowering_spec
namespace lowering_spec{
namespace parser {
Module* DoParse(Tokenizer& tokens);
TypeRef* _production_TypeRef_group_0(Tokenizer& tokens);
TypeRef* _production_TypeRef_group_1(Tokenizer& tokens);
TypeRef* _production_TypeRef(Tokenizer& tokens);
Expr* _production_Expr_group_0(Tokenizer& tokens);
Expr* _production_Expr_group_1(Tokenizer& tokens);
Expr* _production_Expr_group_2(Tokenizer& tokens);
Expr* _production_Expr_group_3(Tokenizer& tokens);
Expr* _production_Expr(Tokenizer& tokens);
Stmt* _production_CompoundStmt(Tokenizer& tokens);
Stmt* _production_Stmt(Tokenizer& tokens);
FuncArg* _production_FuncArg(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
TypeRef* _production_TypeRef_group_0(Tokenizer& tokens) {
if (tokens.peak_check_str("unit")) {
tokens.expect("unit");
auto result = ({
auto __current_self = new VoidTypeRef;__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = new NamedTypeRef;__current_self->name = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
TypeRef* _production_TypeRef_group_1(Tokenizer& tokens) {
TypeRef* expr_result = _production_TypeRef_group_0(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str("::")) {
tokens.expect("::");
auto result = ({
auto __current_self = new MemberTypeRef;__current_self->base = _tmp_0;
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("<")) {
tokens.expect("<");
auto result = ({
auto __current_self = new TemplateTypeRef;__current_self->base = _tmp_0;
__current_self->args = ([&]{
std::vector<TypeRef*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str(">")) { break; }
 __current_vector__.push_back([&]{auto result = _production_TypeRef(tokens);
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
TypeRef* _production_TypeRef(Tokenizer& tokens) {
auto result = _production_TypeRef_group_1(tokens);
return result;
}
Expr* _production_Expr_group_0(Tokenizer& tokens) {
if (tokens.peak_check_str("new")) {
tokens.expect("new");
auto result = ({
auto __current_self = new NewExpr;__current_self->type = _production_TypeRef(tokens);
__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check(tok::number)) {
auto _tmp_0 = tokens.expect(tok::number);
auto result = ({
auto __current_self = new NumberExpr;__current_self->value = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check(tok::str)) {
auto _tmp_0 = tokens.expect(tok::str);
auto result = ({
auto __current_self = new StrExpr;__current_self->value = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = new NamedExpr;__current_self->name = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
Expr* _production_Expr_group_1(Tokenizer& tokens) {
Expr* expr_result = _production_Expr_group_0(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str(".")) {
tokens.expect(".");
auto result = ({
auto __current_self = new DotExpr;__current_self->base = _tmp_0;
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("->")) {
tokens.expect("->");
auto result = ({
auto __current_self = new ArrowExpr;__current_self->base = _tmp_0;
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("[")) {
tokens.expect("[");
auto result = ({
auto __current_self = new IndexExpr;__current_self->base = _tmp_0;
__current_self->args = ([&]{
std::vector<Expr*> __current_vector__;
   if (!tokens.peak_check_str("]")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = _production_Expr(tokens);
return result;
 }()); if (tokens.peak_check(tok::comma)) {
   tokens.expect(tok::comma);
 } else { break; }
  }}return __current_vector__;
}())
;
tokens.expect("]");
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("::")) {
tokens.expect("::");
auto result = ({
auto __current_self = new ColonColonExpr;__current_self->base = _tmp_0;
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("(")) {
tokens.expect("(");
auto result = ({
auto __current_self = new CallExpr;__current_self->base = _tmp_0;
__current_self->args = ([&]{
std::vector<Expr*> __current_vector__;
   if (!tokens.peak_check_str(")")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = _production_Expr(tokens);
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
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
Expr* _production_Expr_group_2(Tokenizer& tokens) {
Expr* expr_result = _production_Expr_group_1(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str("==")) {
tokens.expect("==");
auto result = ({
auto __current_self = new CompEqEqExpr;__current_self->lhs = _tmp_0;
__current_self->rhs = _production_Expr_group_1(tokens);
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
Expr* _production_Expr_group_3(Tokenizer& tokens) {
auto _tmp_0 = _production_Expr_group_2(tokens);
if (tokens.peak_check_str("=")) {
tokens.expect("=");
auto result = ({
auto __current_self = new AssignExpr;__current_self->lhs = _tmp_0;
__current_self->rhs = _production_Expr_group_3(tokens);
__current_self;
});
return result;
}
auto result = _tmp_0;
return result;
}
Expr* _production_Expr(Tokenizer& tokens) {
auto result = _production_Expr_group_3(tokens);
return result;
}
Stmt* _production_CompoundStmt(Tokenizer& tokens) {
if (tokens.peak_check_str("{")) {
tokens.expect("{");
auto result = ({
auto __current_self = new CompoundStmt;__current_self->stmts = ([&]{
std::vector<Stmt*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Stmt(tokens);
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
Stmt* _production_Stmt(Tokenizer& tokens) {
if (tokens.peak_check_str("return")) {
tokens.expect("return");
if (tokens.peak_check_str(";")) {
tokens.expect(";");
auto result = ({
auto __current_self = new ReturnVoidStmt;__current_self;
});
return result;
}
auto _tmp_0 = _production_Expr(tokens);
auto result = ({
auto __current_self = new ReturnStmt;__current_self->expr = _tmp_0;
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("let")) {
tokens.expect("let");
auto result = ({
auto __current_self = new LetStmt;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->expr = _production_Expr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("var")) {
tokens.expect("var");
auto result = ({
auto __current_self = new VarStmt;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeRef(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("open")) {
tokens.expect("open");
auto _tmp_0 = tokens.expect(tok::identifier);
if (tokens.peak_check_str(":")) {
tokens.expect(":");
auto result = ({
auto __current_self = new OpenWithTypeStmt;__current_self->name = _tmp_0;
__current_self->type = _production_TypeRef(tokens);
__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
auto _tmp_1 = _production_CompoundStmt(tokens);
auto result = ({
auto __current_self = new OpenStmt;__current_self->name = _tmp_0;
__current_self->body = _tmp_1;
__current_self;
});
return result;
}
if (tokens.peak_check_str("case")) {
tokens.expect("case");
auto result = ({
auto __current_self = new CaseStmt;tokens.expect(".");
__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self;
});
return result;
}
if (tokens.peak_check_str("for")) {
tokens.expect("for");
auto result = ({
auto __current_self = new ForStmt;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("in");
__current_self->sequence = _production_Expr(tokens);
__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("loop")) {
tokens.expect("loop");
auto result = ({
auto __current_self = new LoopStmt;__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("if")) {
tokens.expect("if");
tokens.expect("(");
auto _tmp_0 = _production_Expr(tokens);
tokens.expect(")");
auto _tmp_1 = _production_CompoundStmt(tokens);
if (tokens.peak_check_str("else")) {
tokens.expect("else");
auto result = ({
auto __current_self = new IfElseStmt;__current_self->cond = _tmp_0;
__current_self->body = _tmp_1;
__current_self->else_body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
auto result = ({
auto __current_self = new IfStmt;__current_self->cond = _tmp_0;
__current_self->body = _tmp_1;
__current_self;
});
return result;
}
if (tokens.peak_check_str("scope")) {
tokens.expect("scope");
auto result = ({
auto __current_self = new ScopeStmt;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->expr = _production_Expr(tokens);
__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("default")) {
tokens.expect("default");
auto result = ({
auto __current_self = new DefaultStmt;tokens.expect(":");
__current_self;
});
return result;
}
if (tokens.peak_check_str("emit")) {
tokens.expect("emit");
auto result = ({
auto __current_self = new EmitterStmt;__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("dbg_emit")) {
tokens.expect("dbg_emit");
auto result = ({
auto __current_self = new DbgEmitterStmt;__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
if (tokens.peak_check_str("break")) {
tokens.expect("break");
auto result = ({
auto __current_self = new BreakStmt;tokens.expect(";");
__current_self;
});
return result;
}
auto _tmp_0 = _production_Expr(tokens);
auto result = ({
auto __current_self = new DiscardStmt;__current_self->expr = _tmp_0;
tokens.expect(";");
__current_self;
});
return result;
}
FuncArg* _production_FuncArg(Tokenizer& tokens) {
auto result = ({
auto __current_self = new FuncArg;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeRef(tokens);
__current_self;
});
return result;
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("context")) {
tokens.expect("context");
auto result = ({
auto __current_self = new ContextDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeRef(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("func")) {
tokens.expect("func");
auto result = ({
auto __current_self = new FuncDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("(");
__current_self->args = ([&]{
std::vector<FuncArg*> __current_vector__;
   if (!tokens.peak_check_str(")")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = _production_FuncArg(tokens);
return result;
 }()); if (tokens.peak_check(tok::comma)) {
   tokens.expect(tok::comma);
 } else { break; }
  }}return __current_vector__;
}())
;
tokens.expect(")");
tokens.expect("->");
__current_self->ret_t = _production_TypeRef(tokens);
__current_self->body = _production_CompoundStmt(tokens);
__current_self;
});
return result;
}
tokens.unexpected();
}
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = new Module;tokens.expect("module");
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
}  // namespace lowering_spec
