#include "parser/parser-support.h"

namespace rule_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_bracket, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_bracket, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t) {
switch(t) {
case arrow: return "arrow";case close_arr: return "close_arr";case close_brace: return "close_brace";case close_bracket: return "close_bracket";case close_paran: return "close_paran";case colon: return "colon";case coloncolon: return "coloncolon";case comma: return "comma";case dot: return "dot";case eof: return "eof";case equal: return "equal";case identifier: return "identifier";case number: return "number";case open_arr: return "open_arr";case open_brace: return "open_brace";case open_bracket: return "open_bracket";case open_paran: return "open_paran";case percent: return "percent";case pipe: return "pipe";case plus: return "plus";case semi: return "semi";case star: return "star";case str: return "str";}
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
  if (c =='[') { 
 ++cur; 
c = *cur; 
 goto bb20; }
  if (c ==']') { 
 ++cur; 
c = *cur; 
 goto bb21; }
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
 goto bb22; }
  if (c =='|') { 
 ++cur; 
c = *cur; 
 goto bb23; }
  if (c =='}') { 
 ++cur; 
c = *cur; 
 goto bb24; }
  unexpected(c);
bb24:
  return MakeToken(tok::close_brace, st, cur);
bb23:
  return MakeToken(tok::pipe, st, cur);
bb22:
  return MakeToken(tok::open_brace, st, cur);
bb21:
  return MakeToken(tok::close_bracket, st, cur);
bb20:
  return MakeToken(tok::open_bracket, st, cur);
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
 goto bb25; }
  return MakeToken(tok::colon, st, cur);
bb25:
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
 goto bb26; }
  unexpected(c);
bb26:
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
 goto bb27; }
  if (c >= '#' && c <= '[') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='\\') { 
 ++cur; 
c = *cur; 
 goto bb28; }
  if (c >= ']' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb28:
  if (c >= ' ' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb27:
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
struct OldParserDecl;
struct OldLoweringSpecDecl;
struct LibraryDecl;
struct PassesDecl;
struct PassesTemplateDecl;
struct LinkDecl;
struct Expr;
struct NameExpr;
struct StringLiteralExpr;
struct ArrayLiteralExpr;
struct DotExpr;
struct Module;
struct Option;

struct Decl {
  enum class Kind {
    Import, OldParser, OldLoweringSpec, Library, Passes, PassesTemplate, Link,
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

struct Expr {
  enum class Kind {
    Name, StringLiteral, ArrayLiteral, Dot,
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
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
Expr* _production_Expr_group_0(Tokenizer& tokens) {
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = new NameExpr;__current_self->name = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check(tok::str)) {
auto _tmp_0 = tokens.expect(tok::str);
auto result = ({
auto __current_self = new StringLiteralExpr;__current_self->value = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check_str("[")) {
tokens.expect("[");
auto result = ({
auto __current_self = new ArrayLiteralExpr;__current_self->values = ([&]{
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
return expr_result;
}
tokens.unexpected();
}
Expr* _production_Expr(Tokenizer& tokens) {
auto result = _production_Expr_group_1(tokens);
return result;
}
Option* _production_Option(Tokenizer& tokens) {
auto result = ({
auto __current_self = new Option;__current_self->key = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->value = _production_Expr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("import")) {
tokens.expect("import");
auto result = ({
auto __current_self = new ImportDecl;__current_self->path = tokens.expect(tok::str);
tokens.expect("as");
__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("old_parser")) {
tokens.expect("old_parser");
auto result = ({
auto __current_self = new OldParserDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
if (tokens.peak_check_str("old_lowering_spec")) {
tokens.expect("old_lowering_spec");
auto result = ({
auto __current_self = new OldLoweringSpecDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
if (tokens.peak_check_str("lib")) {
tokens.expect("lib");
auto result = ({
auto __current_self = new LibraryDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
if (tokens.peak_check_str("passes")) {
tokens.expect("passes");
auto result = ({
auto __current_self = new PassesDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
if (tokens.peak_check_str("passes_meta")) {
tokens.expect("passes_meta");
auto result = ({
auto __current_self = new PassesTemplateDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
if (tokens.peak_check_str("link")) {
tokens.expect("link");
auto result = ({
auto __current_self = new LinkDecl;__current_self->fname = tokens.expect(tok::str);
tokens.expect("{");
__current_self->options = ([&]{
std::vector<Option*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Option(tokens);
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
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = new Module;__current_self->decls = ([&]{
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
}  // namespace rule_spec
