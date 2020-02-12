#pragma once
#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace production_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_paran, percent, pipe, plus, semi, star, str};
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
}  // namespace parser
}  // namespace production_spec
