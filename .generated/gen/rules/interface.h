#pragma once
#include "parser/parser-support.h"

namespace interface_spec {
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
}  // namespace parser
}  // namespace interface_spec
