#include "parser/parser-support.h"
#include "parser/ast-context.h"

namespace parser_spec {
namespace tok {
enum T {arrow, close_arr, close_brace, close_paran, colon, coloncolon, comma, dot, eof, equal, identifier, number, open_arr, open_brace, open_paran, percent, pipe, plus, semi, star, str};
const char* StringifyType(T t) {
switch(t) {
case arrow: return "arrow";case close_arr: return "close_arr";case close_brace: return "close_brace";case close_paran: return "close_paran";case colon: return "colon";case coloncolon: return "coloncolon";case comma: return "comma";case dot: return "dot";case eof: return "eof";case equal: return "equal";case identifier: return "identifier";case number: return "number";case open_arr: return "open_arr";case open_brace: return "open_brace";case open_paran: return "open_paran";case percent: return "percent";case pipe: return "pipe";case plus: return "plus";case semi: return "semi";case star: return "star";case str: return "str";}
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
 goto bb20; }
  if (c =='|') { 
 ++cur; 
c = *cur; 
 goto bb21; }
  if (c =='}') { 
 ++cur; 
c = *cur; 
 goto bb22; }
  unexpected(c);
bb22:
  return MakeToken(tok::close_brace, st, cur);
bb21:
  return MakeToken(tok::pipe, st, cur);
bb20:
  return MakeToken(tok::open_brace, st, cur);
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
 goto bb23; }
  return MakeToken(tok::colon, st, cur);
bb23:
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
 goto bb24; }
  unexpected(c);
bb24:
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
 goto bb25; }
  if (c >= '#' && c <= '[') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  if (c =='\\') { 
 ++cur; 
c = *cur; 
 goto bb26; }
  if (c >= ']' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb26:
  if (c >= ' ' && c <= '~') { 
 ++cur; 
c = *cur; 
 goto bb4; }
  unexpected(c);
bb25:
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
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
RegexExpr* _production_RegexExpr_group_0(Tokenizer& tokens) {
if (tokens.peak_check(tok::number)) {
auto _tmp_0 = tokens.expect(tok::number);
auto result = ({
auto __current_self = tokens.New<IntegerRegexExpr>();__current_self->value = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check(tok::str)) {
auto _tmp_0 = tokens.expect(tok::str);
auto result = ({
auto __current_self = tokens.New<StringRegexExpr>();__current_self->value = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
RegexExpr* _production_RegexExpr_group_1(Tokenizer& tokens) {
RegexExpr* expr_result = _production_RegexExpr_group_0(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str(":")) {
tokens.expect(":");
auto result = ({
auto __current_self = tokens.New<RangeRegexExpr>();__current_self->st = _tmp_0;
__current_self->ed = _production_RegexExpr_group_0(tokens);
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
RegexExpr* _production_RegexExpr_group_2(Tokenizer& tokens) {
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = tokens.New<NamedRegexExpr>();__current_self->name = _tmp_0;
__current_self;
});
return result;
}
if (tokens.peak_check_str("(")) {
tokens.expect("(");
auto result = ({
auto __current_self = tokens.New<WrappedRegexExpr>();__current_self->value = _production_RegexExpr(tokens);
tokens.expect(")");
__current_self;
});
return result;
}
auto _tmp_0 = _production_RegexExpr_group_1(tokens);
auto result = _tmp_0;
return result;
}
RegexExpr* _production_RegexExpr_group_3(Tokenizer& tokens) {
RegexExpr* expr_result = _production_RegexExpr_group_2(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str("*")) {
tokens.expect("*");
auto result = ({
auto __current_self = tokens.New<StarRegexExpr>();__current_self->base = _tmp_0;
__current_self;
});
expr_result = result;
continue;
}
if (tokens.peak_check_str("+")) {
tokens.expect("+");
auto result = ({
auto __current_self = tokens.New<PlusRegexExpr>();__current_self->base = _tmp_0;
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
RegexExpr* _production_RegexExpr_group_4(Tokenizer& tokens) {
RegexExpr* expr_result = _production_RegexExpr_group_3(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str(".")) {
tokens.expect(".");
auto result = ({
auto __current_self = tokens.New<JuxtaRegexExpr>();__current_self->lhs = _tmp_0;
__current_self->rhs = _production_RegexExpr_group_3(tokens);
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
RegexExpr* _production_RegexExpr_group_5(Tokenizer& tokens) {
RegexExpr* expr_result = _production_RegexExpr_group_4(tokens);
while (true) {
auto _tmp_0 = expr_result;if (tokens.peak_check_str("|")) {
tokens.expect("|");
auto result = ({
auto __current_self = tokens.New<AltRegexExpr>();__current_self->lhs = _tmp_0;
__current_self->rhs = _production_RegexExpr_group_4(tokens);
__current_self;
});
expr_result = result;
continue;
}
return expr_result;
}
tokens.unexpected();
}
RegexExpr* _production_RegexExpr(Tokenizer& tokens) {
auto result = _production_RegexExpr_group_5(tokens);
return result;
}
TokenDecl* _production_TokenDecl(Tokenizer& tokens) {
if (tokens.peak_check_str("let")) {
tokens.expect("let");
auto result = ({
auto __current_self = tokens.New<LetTokenDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->value = _production_RegexExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("emit")) {
tokens.expect("emit");
auto result = ({
auto __current_self = tokens.New<EmitTokenDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->value = _production_RegexExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("ignore")) {
tokens.expect("ignore");
auto result = ({
auto __current_self = tokens.New<IgnoreTokenDecl>();__current_self->value = _production_RegexExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("import")) {
tokens.expect("import");
auto result = ({
auto __current_self = tokens.New<ImportTokenDecl>();__current_self->module = tokens.expect(tok::identifier);
tokens.expect(".");
__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Edge* _production_Edge(Tokenizer& tokens) {
tokens.unexpected();
}
Node* _production_Node(Tokenizer& tokens) {
auto result = ({
auto __current_self = tokens.New<Node>();__current_self->edges = ([&]{
std::vector<Edge*> __current_vector__;
    while (true) {
   if (tokens.peak_check(tok::eof)) { break; }
 __current_vector__.push_back([&]{auto result = _production_Edge(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
__current_self;
});
return result;
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("regex")) {
tokens.expect("regex");
auto result = ({
auto __current_self = tokens.New<RegexDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->items = ([&]{
std::vector<TokenDecl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_TokenDecl(tokens);
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
if (tokens.peak_check_str("nfa_grap")) {
tokens.expect("nfa_grap");
auto result = ({
auto __current_self = tokens.New<NFAGraphDecl>();__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->root = _production_Node(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = tokens.New<Module>();tokens.expect("module");
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
}  // namespace parser_spec
