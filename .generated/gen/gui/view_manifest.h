#pragma once
#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace view_manifest {
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
}  // namespace view_manifest
namespace view_manifest{
struct Action;
struct RawAction;
struct DefineFuncAction;
struct ChunkSrc;
struct EmitFileDecl;
struct Module;

struct Action {
  enum class Kind {
    Raw, DefineFunc,
  };
  Action(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct RawAction: public Action {
  RawAction() : Action(Kind::Raw) {}
  ChunkSrc* id;
};

struct DefineFuncAction: public Action {
  DefineFuncAction() : Action(Kind::DefineFunc) {}
  ChunkSrc* sig_id;
  ChunkSrc* body_id;
};

struct ChunkSrc {
  tok::Token id;
};

struct EmitFileDecl {
  tok::Token name;
  std::vector<Action*> actions;
};

struct Module {
  std::vector<EmitFileDecl*> decls;
};
}  // namespace view_manifest
namespace view_manifest{
namespace parser {
Module* DoParse(Tokenizer& tokens);
ChunkSrc* _production_ChunkSrc(Tokenizer& tokens);
Action* _production_Action(Tokenizer& tokens);
EmitFileDecl* _production_EmitFileDecl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
}  // namespace parser
}  // namespace view_manifest
