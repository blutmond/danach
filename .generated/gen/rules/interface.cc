#include "parser/parser-support.h"

namespace interface_spec {
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
}  // namespace interface_spec
namespace interface_spec{
struct ArgSignature;
struct Attr;
struct MandatoryAttr;
struct IsChildListAttr;
struct DefaultAttr;
struct ConcreteBodyDecl;
struct FuncConcreteBodyDecl;
struct VarConcreteBodyDecl;
struct Decl;
struct InterfaceDecl;
struct ConcreteDecl;
struct ConcreteWithoutInterfaceDecl;
struct TypeAliasDecl;
struct FuncSignature;
struct InterfaceBodyDecl;
struct FuncInterfaceBodyDecl;
struct ParentInterfaceBodyDecl;
struct Module;
struct TypeExpr;
struct OpaqueTypeExpr;
struct VectorTypeExpr;
struct NominalTypeExpr;

struct ArgSignature {
  tok::Token name;
  TypeExpr* type;
};

struct Attr {
  enum class Kind {
    Mandatory, IsChildList, Default,
  };
  Attr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct MandatoryAttr: public Attr {
  MandatoryAttr() : Attr(Kind::Mandatory) {}
};

struct IsChildListAttr: public Attr {
  IsChildListAttr() : Attr(Kind::IsChildList) {}
};

struct DefaultAttr: public Attr {
  DefaultAttr() : Attr(Kind::Default) {}
  tok::Token value;
};

struct ConcreteBodyDecl {
  enum class Kind {
    Func, Var,
  };
  ConcreteBodyDecl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct FuncConcreteBodyDecl: public ConcreteBodyDecl {
  FuncConcreteBodyDecl() : ConcreteBodyDecl(Kind::Func) {}
  tok::Token name;
  FuncSignature* sig;
};

struct VarConcreteBodyDecl: public ConcreteBodyDecl {
  VarConcreteBodyDecl() : ConcreteBodyDecl(Kind::Var) {}
  tok::Token name;
  TypeExpr* type;
  std::vector<Attr*> attrs;
};

struct Decl {
  enum class Kind {
    Interface, Concrete, ConcreteWithoutInterface, TypeAlias,
  };
  Decl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct InterfaceDecl: public Decl {
  InterfaceDecl() : Decl(Kind::Interface) {}
  tok::Token name;
  std::vector<InterfaceBodyDecl*> body;
};

struct ConcreteDecl: public Decl {
  ConcreteDecl() : Decl(Kind::Concrete) {}
  tok::Token name;
  tok::Token implements;
  std::vector<ConcreteBodyDecl*> body;
};

struct ConcreteWithoutInterfaceDecl: public Decl {
  ConcreteWithoutInterfaceDecl() : Decl(Kind::ConcreteWithoutInterface) {}
  tok::Token name;
  std::vector<ConcreteBodyDecl*> body;
};

struct TypeAliasDecl: public Decl {
  TypeAliasDecl() : Decl(Kind::TypeAlias) {}
  tok::Token name;
  TypeExpr* type;
};

struct FuncSignature {
  std::vector<ArgSignature*> args;
  TypeExpr* ret_t;
};

struct InterfaceBodyDecl {
  enum class Kind {
    Func, Parent,
  };
  InterfaceBodyDecl(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct FuncInterfaceBodyDecl: public InterfaceBodyDecl {
  FuncInterfaceBodyDecl() : InterfaceBodyDecl(Kind::Func) {}
  tok::Token name;
  FuncSignature* sig;
  std::vector<Attr*> attrs;
};

struct ParentInterfaceBodyDecl: public InterfaceBodyDecl {
  ParentInterfaceBodyDecl() : InterfaceBodyDecl(Kind::Parent) {}
  tok::Token name;
};

struct Module {
  tok::Token ns;
  std::vector<Decl*> decls;
};

struct TypeExpr {
  enum class Kind {
    Opaque, Vector, Nominal,
  };
  TypeExpr(Kind kind) : kind_(kind) {}
 Kind getKind() { return kind_; }
 private:
  Kind kind_;
};

struct OpaqueTypeExpr: public TypeExpr {
  OpaqueTypeExpr() : TypeExpr(Kind::Opaque) {}
  tok::Token value;
};

struct VectorTypeExpr: public TypeExpr {
  VectorTypeExpr() : TypeExpr(Kind::Vector) {}
  TypeExpr* element;
};

struct NominalTypeExpr: public TypeExpr {
  NominalTypeExpr() : TypeExpr(Kind::Nominal) {}
  tok::Token value;
};
}  // namespace interface_spec
namespace interface_spec{
namespace parser {
Module* DoParse(Tokenizer& tokens);
TypeExpr* _production_TypeExpr_group_0(Tokenizer& tokens);
TypeExpr* _production_TypeExpr(Tokenizer& tokens);
ArgSignature* _production_ArgSignature(Tokenizer& tokens);
FuncSignature* _production_FuncSignature(Tokenizer& tokens);
Attr* _production_Attr(Tokenizer& tokens);
InterfaceBodyDecl* _production_InterfaceBodyDecl(Tokenizer& tokens);
ConcreteBodyDecl* _production_ConcreteBodyDecl(Tokenizer& tokens);
Decl* _production_Decl(Tokenizer& tokens);
Module* _production_Module(Tokenizer& tokens);
Module* DoParse(Tokenizer& tokens) {
  return _production_Module(tokens);
}
TypeExpr* _production_TypeExpr_group_0(Tokenizer& tokens) {
if (tokens.peak_check_str("!")) {
tokens.expect("!");
auto result = ({
auto __current_self = new OpaqueTypeExpr;tokens.expect("<");
__current_self->value = tokens.expect(tok::str);
tokens.expect(">");
__current_self;
});
return result;
}
if (tokens.peak_check_str("vector")) {
tokens.expect("vector");
auto result = ({
auto __current_self = new VectorTypeExpr;tokens.expect("<");
__current_self->element = _production_TypeExpr(tokens);
tokens.expect(">");
__current_self;
});
return result;
}
if (tokens.peak_check(tok::identifier)) {
auto _tmp_0 = tokens.expect(tok::identifier);
auto result = ({
auto __current_self = new NominalTypeExpr;__current_self->value = _tmp_0;
__current_self;
});
return result;
}
tokens.unexpected();
}
TypeExpr* _production_TypeExpr(Tokenizer& tokens) {
auto result = _production_TypeExpr_group_0(tokens);
return result;
}
ArgSignature* _production_ArgSignature(Tokenizer& tokens) {
auto result = ({
auto __current_self = new ArgSignature;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeExpr(tokens);
__current_self;
});
return result;
}
FuncSignature* _production_FuncSignature(Tokenizer& tokens) {
auto result = ({
auto __current_self = new FuncSignature;tokens.expect("(");
__current_self->args = ([&]{
std::vector<ArgSignature*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str(")")) { break; }
 __current_vector__.push_back([&]{auto result = _production_ArgSignature(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect(")");
tokens.expect("->");
__current_self->ret_t = _production_TypeExpr(tokens);
__current_self;
});
return result;
}
Attr* _production_Attr(Tokenizer& tokens) {
if (tokens.peak_check_str("mandatory")) {
tokens.expect("mandatory");
auto result = ({
auto __current_self = new MandatoryAttr;__current_self;
});
return result;
}
if (tokens.peak_check_str("is_child_list")) {
tokens.expect("is_child_list");
auto result = ({
auto __current_self = new IsChildListAttr;__current_self;
});
return result;
}
if (tokens.peak_check_str("default_value")) {
tokens.expect("default_value");
auto result = ({
auto __current_self = new DefaultAttr;tokens.expect("(");
__current_self->value = tokens.expect(tok::str);
tokens.expect(")");
__current_self;
});
return result;
}
tokens.unexpected();
}
InterfaceBodyDecl* _production_InterfaceBodyDecl(Tokenizer& tokens) {
if (tokens.peak_check_str("func")) {
tokens.expect("func");
auto result = ({
auto __current_self = new FuncInterfaceBodyDecl;__current_self->name = tokens.expect(tok::identifier);
__current_self->sig = _production_FuncSignature(tokens);
__current_self->attrs = ([&]{
std::vector<Attr*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str(";")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Attr(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("parent")) {
tokens.expect("parent");
auto result = ({
auto __current_self = new ParentInterfaceBodyDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
ConcreteBodyDecl* _production_ConcreteBodyDecl(Tokenizer& tokens) {
if (tokens.peak_check_str("override")) {
tokens.expect("override");
auto result = ({
auto __current_self = new FuncConcreteBodyDecl;__current_self->name = tokens.expect(tok::identifier);
__current_self->sig = _production_FuncSignature(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
if (tokens.peak_check_str("var")) {
tokens.expect("var");
auto result = ({
auto __current_self = new VarConcreteBodyDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect(":");
__current_self->type = _production_TypeExpr(tokens);
__current_self->attrs = ([&]{
std::vector<Attr*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str(";")) { break; }
 __current_vector__.push_back([&]{auto result = _production_Attr(tokens);
return result;
 }());  }
return __current_vector__;
}())
;
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Decl* _production_Decl(Tokenizer& tokens) {
if (tokens.peak_check_str("interface")) {
tokens.expect("interface");
auto result = ({
auto __current_self = new InterfaceDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->body = ([&]{
std::vector<InterfaceBodyDecl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_InterfaceBodyDecl(tokens);
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
if (tokens.peak_check_str("concrete")) {
tokens.expect("concrete");
auto _tmp_0 = tokens.expect(tok::identifier);
if (tokens.peak_check_str(":")) {
tokens.expect(":");
auto result = ({
auto __current_self = new ConcreteDecl;__current_self->name = _tmp_0;
__current_self->implements = tokens.expect(tok::identifier);
tokens.expect("{");
__current_self->body = ([&]{
std::vector<ConcreteBodyDecl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_ConcreteBodyDecl(tokens);
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
if (tokens.peak_check_str("{")) {
tokens.expect("{");
auto result = ({
auto __current_self = new ConcreteWithoutInterfaceDecl;__current_self->name = _tmp_0;
__current_self->body = ([&]{
std::vector<ConcreteBodyDecl*> __current_vector__;
    while (true) {
   if (tokens.peak_check_str("}")) { break; }
 __current_vector__.push_back([&]{auto result = _production_ConcreteBodyDecl(tokens);
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
if (tokens.peak_check_str("alias")) {
tokens.expect("alias");
auto result = ({
auto __current_self = new TypeAliasDecl;__current_self->name = tokens.expect(tok::identifier);
tokens.expect("=");
__current_self->type = _production_TypeExpr(tokens);
tokens.expect(";");
__current_self;
});
return result;
}
tokens.unexpected();
}
Module* _production_Module(Tokenizer& tokens) {
auto result = ({
auto __current_self = new Module;tokens.expect("namespace");
__current_self->ns = tokens.expect(tok::identifier);
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
}  // namespace interface_spec
