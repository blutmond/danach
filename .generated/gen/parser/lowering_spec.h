#pragma once
#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace lowering_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, equalequal, identifier, notequal, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t);

struct Token {
  T type = tok::eof;
  string_view str;
};

Token MakeToken(T t, const char* st, const char* ed);

void PrintToken(Token t);

void unexpected(char c);

Token GetNext(const char*& cur);
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
struct NewArenaExpr;
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
    NewArena, New, Number, Str, Named, Dot, Arrow, Index, ColonColon, Call, CompEqEq, Assign,
  };
  Expr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct NewArenaExpr: public Expr {
  NewArenaExpr() : Expr(Kind::NewArena) {}
  tok::Token arena_name;
  TypeRef* type;
  Stmt* body;
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
}  // namespace parser
}  // namespace lowering_spec
