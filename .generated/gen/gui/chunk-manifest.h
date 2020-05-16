#pragma once
#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace chunk_manifest {
namespace tok {
enum T {arrow, bang, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
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
}  // namespace chunk_manifest
namespace chunk_manifest{
struct Decl;
struct BinaryDecl;
struct DepsDecl;
struct NameDecl;
struct Dep;
struct HDep;
struct CCDep;
struct Module;

struct Decl {
  enum class Kind {
    Binary, Deps, Name,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct BinaryDecl: public Decl {
  BinaryDecl() : Decl(Kind::Binary) {}
  tok::Token fname;
};

struct DepsDecl: public Decl {
  DepsDecl() : Decl(Kind::Deps) {}
  std::vector<Dep*> deps;
};

struct NameDecl: public Decl {
  NameDecl() : Decl(Kind::Name) {}
  tok::Token name;
};

struct Dep {
  enum class Kind {
    H, CC,
  };
  Dep(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct HDep: public Dep {
  HDep() : Dep(Kind::H) {}
  tok::Token name;
};

struct CCDep: public Dep {
  CCDep() : Dep(Kind::CC) {}
  tok::Token name;
};

struct Module {
  std::vector<Decl*> decls;
};
}  // namespace chunk_manifest
namespace chunk_manifest{
namespace parser {
Module* DoParse(Tokenizer& tokens);
Dep* _production_Dep(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
}  // namespace parser
}  // namespace chunk_manifest
