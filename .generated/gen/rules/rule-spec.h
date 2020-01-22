#pragma once
#include "parser/parser-support.h"

namespace rule_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
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
}  // namespace rule_spec
namespace rule_spec{
struct Decl;
struct ImportDecl;
struct ImportBufferDecl;
struct LetDecl;
struct BufferParserDecl;
struct OldParserDecl;
struct OldLoweringSpecDecl;
struct LibraryDecl;
struct PassesDecl;
struct PassesTemplateDecl;
struct LinkDecl;
struct SoLinkDecl;
struct Expr;
struct NameExpr;
struct StringLiteralExpr;
struct IntegerLiteralExpr;
struct ArrayLiteralExpr;
struct DotExpr;
struct Module;
struct Option;

struct Decl {
  enum class Kind {
    Import, ImportBuffer, Let, BufferParser, OldParser, OldLoweringSpec, Library, Passes, PassesTemplate, Link, SoLink,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct ImportDecl: public Decl {
  ImportDecl() : Decl(Kind::Import) {}
  tok::Token path;
  tok::Token name;
};

struct ImportBufferDecl: public Decl {
  ImportBufferDecl() : Decl(Kind::ImportBuffer) {}
  tok::Token id;
  tok::Token filename;
  tok::Token name;
};

struct LetDecl: public Decl {
  LetDecl() : Decl(Kind::Let) {}
  tok::Token name;
  Expr* value;
};

struct BufferParserDecl: public Decl {
  BufferParserDecl() : Decl(Kind::BufferParser) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct OldParserDecl: public Decl {
  OldParserDecl() : Decl(Kind::OldParser) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct OldLoweringSpecDecl: public Decl {
  OldLoweringSpecDecl() : Decl(Kind::OldLoweringSpec) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct LibraryDecl: public Decl {
  LibraryDecl() : Decl(Kind::Library) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct PassesDecl: public Decl {
  PassesDecl() : Decl(Kind::Passes) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct PassesTemplateDecl: public Decl {
  PassesTemplateDecl() : Decl(Kind::PassesTemplate) {}
  tok::Token name;
  std::vector<Option*> options;
};

struct LinkDecl: public Decl {
  LinkDecl() : Decl(Kind::Link) {}
  tok::Token fname;
  std::vector<Option*> options;
};

struct SoLinkDecl: public Decl {
  SoLinkDecl() : Decl(Kind::SoLink) {}
  tok::Token fname;
  std::vector<Option*> options;
};

struct Expr {
  enum class Kind {
    Name, StringLiteral, IntegerLiteral, ArrayLiteral, Dot,
  };
  Expr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct NameExpr: public Expr {
  NameExpr() : Expr(Kind::Name) {}
  tok::Token name;
};

struct StringLiteralExpr: public Expr {
  StringLiteralExpr() : Expr(Kind::StringLiteral) {}
  tok::Token value;
};

struct IntegerLiteralExpr: public Expr {
  IntegerLiteralExpr() : Expr(Kind::IntegerLiteral) {}
  tok::Token value;
};

struct ArrayLiteralExpr: public Expr {
  ArrayLiteralExpr() : Expr(Kind::ArrayLiteral) {}
  std::vector<Expr*> values;
};

struct DotExpr: public Expr {
  DotExpr() : Expr(Kind::Dot) {}
  Expr* base;
  tok::Token name;
};

struct Module {
  std::vector<Decl*> decls;
};

struct Option {
  tok::Token key;
  Expr* value;
};
}  // namespace rule_spec
namespace rule_spec{
namespace parser {
Module* DoParse(Tokenizer& tokens);
Expr* _production_Expr_group_0(Tokenizer& tokens);
Expr* _production_Expr_group_1(Tokenizer& tokens);
Expr* _production_Expr(Tokenizer& tokens);
Option* _production_Option(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
}  // namespace parser
}  // namespace rule_spec
