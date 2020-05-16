#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace chunk_manifest {
namespace tok {
enum T {arrow, bang, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t) {
switch(t) {
case arrow: return "arrow";case bang: return "bang";case close_arr: return "close_arr";case close_brace: return "close_brace";case close_bracket: return "close_bracket";case close_paran: return "close_paran";case colon: return "colon";case coloncolon: return "coloncolon";case comma: return "comma";case dot: return "dot";case eof: return "eof";case equal: return "equal";case identifier: return "identifier";case number: return "number";case open_arr: return "open_arr";case open_brace: return "open_brace";case open_bracket: return "open_bracket";case open_paran: return "open_paran";case percent: return "percent";case pipe: return "pipe";case plus: return "plus";case semi: return "semi";case star: return "star";case str: return "str";}
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
  return MakeToken(tok::equal, st, cur);
bb17:
  return MakeToken(tok::open_arr, st, cur);
bb16:
  return MakeToken(tok::semi, st, cur);
bb15:
  if (c ==':') { 
 ++cur; 
c = *cur; 
 goto bb26; }
  return MakeToken(tok::colon, st, cur);
bb26:
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
 goto bb27; }
  unexpected(c);
bb27:
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
 goto bb28; }
  if (c >= '#' && c <= '[') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  if (c =='\\') { 
 ++cur; 
c = *cur; 
 goto bb29; }
  if (c >= ']' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  unexpected(c);
bb29:
  if (c >= ' ' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb5; }
  unexpected(c);
bb28:
  return MakeToken(tok::str, st, cur);
bb4:
  return MakeToken(tok::bang, st, cur);
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
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
Dep* _production_Dep(Tokenizer& tokens) {
if (tokens.peak_check_str("h")) {
tokens.expect("h");
auto result = ({
auto __current_self = tokens.New<HDep>();tokens.expect(":");
__current_self->name = tokens.expect(tok::identifier);
__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = tokens.New<CCDep>();__current_self->name = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("cc_binary")) {
tokens.expect("cc_binary");
auto result = ({
auto __current_self = tokens.New<BinaryDecl>();__current_self->fname = tokens.expect(tok::str);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("deps")) {
tokens.expect("deps");
auto result = ({
auto __current_self = tokens.New<DepsDecl>();__current_self->deps = ([&]{
std::vector<Dep*> __current_vector__;
   if (!tokens.peak_check_str(";")) {
    while (true) {
 __current_vector__.push_back([&]{auto result = _production_Dep(tokens);
return result;
 }()); if (tokens.peak_check(tok::comma)) {
   tokens.expect(tok::comma);
 } else { break; }
  }}return __current_vector__;
}())
;
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("name")) {
tokens.expect("name");
auto result = ({
auto __current_self = tokens.New<NameDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = tokens.New<Module>();__current_self->decls = ([&]{
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
}  // namespace chunk_manifest
