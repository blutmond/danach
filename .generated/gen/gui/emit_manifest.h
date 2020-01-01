#pragma once
#include "parser/parser-support.h"

namespace emit_manifest {
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
}  // namespace emit_manifest
namespace emit_manifest{
struct Action;
struct PragmaOnceAction;
struct ImportAction;
struct HdrChunkAction;
struct FwdDeclareFuncAction;
struct DefineFuncAction;
struct ChunkSrc;
struct EmitFileDecl;
struct Module;

struct Action {
  enum class Kind {
    PragmaOnce, Import, HdrChunk, FwdDeclareFunc, DefineFunc,
  };
  Action(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct PragmaOnceAction: public Action {
  PragmaOnceAction() : Action(Kind::PragmaOnce) {}
};

struct ImportAction: public Action {
  ImportAction() : Action(Kind::Import) {}
  tok::Token filename;
};

struct HdrChunkAction: public Action {
  HdrChunkAction() : Action(Kind::HdrChunk) {}
  ChunkSrc* id;
};

struct FwdDeclareFuncAction: public Action {
  FwdDeclareFuncAction() : Action(Kind::FwdDeclareFunc) {}
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
  tok::Token filename;
  std::vector<Action*> actions;
};

struct Module {
  std::vector<EmitFileDecl*> decls;
};
}  // namespace emit_manifest
namespace emit_manifest{
namespace parser {
Module* DoParse(Tokenizer& tokens);
ChunkSrc* _production_ChunkSrc(Tokenizer& tokens);
Action* _production_Action(Tokenizer& tokens);
EmitFileDecl* _production_EmitFileDecl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
}  // namespace parser
}  // namespace emit_manifest
