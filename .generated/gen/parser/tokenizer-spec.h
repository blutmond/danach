#pragma once
#include "parser/parser-support.h"

namespace parser_spec {
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
}  // namespace parser_spec
namespace parser_spec{
struct Edge;
struct RangeEdge;
struct UnaryEdge;
struct SkipToEdge;
struct EmitEdge;
struct IgnoreEdge;
struct UnexpectedEdge;
struct NodePair;
struct Decl;
struct RegexDecl;
struct NFAGraphDecl;
struct Module;
struct Node;
struct RegexExpr;
struct IntegerRegexExpr;
struct StringRegexExpr;
struct RangeRegexExpr;
struct NamedRegexExpr;
struct WrappedRegexExpr;
struct StarRegexExpr;
struct PlusRegexExpr;
struct JuxtaRegexExpr;
struct AltRegexExpr;
struct TokenDecl;
struct LetTokenDecl;
struct EmitTokenDecl;
struct IgnoreTokenDecl;
struct ImportTokenDecl;

struct Edge {
  enum class Kind {
    Range, Unary, SkipTo, Emit, Ignore, Unexpected,
  };
  Edge(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct RangeEdge: public Edge {
  RangeEdge() : Edge(Kind::Range) {}
  char start;
  char end;
  Node* next;
};

struct UnaryEdge: public Edge {
  UnaryEdge() : Edge(Kind::Unary) {}
  char match;
  Node* next;
};

struct SkipToEdge: public Edge {
  SkipToEdge() : Edge(Kind::SkipTo) {}
  Node* next;
};

struct EmitEdge: public Edge {
  EmitEdge() : Edge(Kind::Emit) {}
  tok::Token name;
};

struct IgnoreEdge: public Edge {
  IgnoreEdge() : Edge(Kind::Ignore) {}
};

struct UnexpectedEdge: public Edge {
  UnexpectedEdge() : Edge(Kind::Unexpected) {}
};

struct NodePair {
  Node* st;
  Node* ed;
};

struct Decl {
  enum class Kind {
    Regex, NFAGraph,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct RegexDecl: public Decl {
  RegexDecl() : Decl(Kind::Regex) {}
  tok::Token name;
  std::vector<TokenDecl*> items;
};

struct NFAGraphDecl: public Decl {
  NFAGraphDecl() : Decl(Kind::NFAGraph) {}
  tok::Token name;
  Node* root;
};

struct Module {
  tok::Token mod_name;
  std::vector<Decl*> decls;
};

struct Node {
  std::vector<Edge*> edges;
};

struct RegexExpr {
  enum class Kind {
    Integer, String, Range, Named, Wrapped, Star, Plus, Juxta, Alt,
  };
  RegexExpr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct IntegerRegexExpr: public RegexExpr {
  IntegerRegexExpr() : RegexExpr(Kind::Integer) {}
  tok::Token value;
};

struct StringRegexExpr: public RegexExpr {
  StringRegexExpr() : RegexExpr(Kind::String) {}
  tok::Token value;
};

struct RangeRegexExpr: public RegexExpr {
  RangeRegexExpr() : RegexExpr(Kind::Range) {}
  RegexExpr* st;
  RegexExpr* ed;
};

struct NamedRegexExpr: public RegexExpr {
  NamedRegexExpr() : RegexExpr(Kind::Named) {}
  tok::Token name;
};

struct WrappedRegexExpr: public RegexExpr {
  WrappedRegexExpr() : RegexExpr(Kind::Wrapped) {}
  RegexExpr* value;
};

struct StarRegexExpr: public RegexExpr {
  StarRegexExpr() : RegexExpr(Kind::Star) {}
  RegexExpr* base;
};

struct PlusRegexExpr: public RegexExpr {
  PlusRegexExpr() : RegexExpr(Kind::Plus) {}
  RegexExpr* base;
};

struct JuxtaRegexExpr: public RegexExpr {
  JuxtaRegexExpr() : RegexExpr(Kind::Juxta) {}
  RegexExpr* lhs;
  RegexExpr* rhs;
};

struct AltRegexExpr: public RegexExpr {
  AltRegexExpr() : RegexExpr(Kind::Alt) {}
  RegexExpr* lhs;
  RegexExpr* rhs;
};

struct TokenDecl {
  enum class Kind {
    Let, Emit, Ignore, Import,
  };
  TokenDecl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct LetTokenDecl: public TokenDecl {
  LetTokenDecl() : TokenDecl(Kind::Let) {}
  tok::Token name;
  RegexExpr* value;
};

struct EmitTokenDecl: public TokenDecl {
  EmitTokenDecl() : TokenDecl(Kind::Emit) {}
  tok::Token name;
  RegexExpr* value;
};

struct IgnoreTokenDecl: public TokenDecl {
  IgnoreTokenDecl() : TokenDecl(Kind::Ignore) {}
  RegexExpr* value;
};

struct ImportTokenDecl: public TokenDecl {
  ImportTokenDecl() : TokenDecl(Kind::Import) {}
  tok::Token module;
  tok::Token name;
};
}  // namespace parser_spec
namespace parser_spec{
namespace parser {
Module* DoParse(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_0(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_1(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_2(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_3(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_4(Tokenizer& tokens);
RegexExpr* _production_RegexExpr_group_5(Tokenizer& tokens);
RegexExpr* _production_RegexExpr(Tokenizer& tokens);
TokenDecl* _production_TokenDecl(Tokenizer& tokens);
Edge* _production_Edge(Tokenizer& tokens);
Node* _production_Node(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
}  // namespace parser
}  // namespace parser_spec
