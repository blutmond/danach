#include <string>
#include <experimental/string_view>
#include <iostream>
#include <vector>
#include <fstream>

using std::experimental::string_view;

#include "tokens/line_number_helper.cc"
#include "gen/parser/tokenizer.cc"
#include "gen/parser/parser.cc"

namespace parser_spec {
struct DumpASTTypes {
  std::ostream& stream;
  explicit DumpASTTypes(std::ostream& stream) : stream(stream) {}

  void visit(Module* m) {
    stream << "namespace " << m->mod_name.str << " {\n\n";
    for (auto* decl_ : m->decls) {
      if (decl_->getKind() == Decl::Kind::Type) { auto* decl = reinterpret_cast<TypeDecl*>(decl_);
        stream << "struct " << decl->name.str << ";\n";
        if (decl->type->getKind() == TypeDeclExpr::Kind::Sum) {
          auto* sumDecl = reinterpret_cast<SumTypeDeclExpr*>(decl->type);
          for (auto* subDecl : sumDecl->decls) {
            stream << "struct " << subDecl->name.str << decl->name.str << ";\n";
          }
        }
      }
    }
    for (auto* decl_ : m->decls) {
      if (decl_->getKind() == Decl::Kind::Type) { auto* decl = reinterpret_cast<TypeDecl*>(decl_);
        if (decl->type->getKind() == TypeDeclExpr::Kind::Sum) {
          auto* sumDecl = reinterpret_cast<SumTypeDeclExpr*>(decl->type);
          stream << "\nstruct " << decl->name.str << " {\n";
          stream << "  enum class Kind {\n   ";
          for (auto* subDecl : sumDecl->decls) {
            stream << " " << subDecl->name.str << ",";
          }
          stream << "\n  };\n";
          stream << "  " << decl->name.str << "(Kind kind) : kind_(kind) {}\n";
          stream << "  " << "Kind getKind() { return kind_; }\n";
          stream << " private:\n";
          stream << "  Kind kind_;\n";
          stream << "};\n";
          for (auto* subDecl : sumDecl->decls) {
            if (subDecl->type->getKind() == TypeDeclExpr::Kind::Product) {
            stream << "\nstruct " << subDecl->name.str << decl->name.str << " : public " << decl->name.str << " {\n";
            stream << "  " << subDecl->name.str << decl->name.str << "() : " << decl->name.str
                << "(Kind::" << subDecl->name.str << ") {}\n";
            auto* prodDecl = reinterpret_cast<ProductTypeDeclExpr*>(subDecl->type);
            for (auto* grandSubDecl : prodDecl->decls) {
              stream << "  ";
              visitTypeExpr(grandSubDecl->type);
              stream << " " << grandSubDecl->name.str << ";\n";
            }
            stream << "};\n";
            } else {
          stream << "\n\n#error Named: " << subDecl->name.str << " Do not understand " << __LINE__ << "\n"; return;
            }
          }
        } else if (decl->type->getKind() == TypeDeclExpr::Kind::Product) {
          stream << "\nstruct " << decl->name.str << " {\n";
          auto* prodDecl = reinterpret_cast<ProductTypeDeclExpr*>(decl->type);
          for (auto* subDecl : prodDecl->decls) {
            stream << "  ";
            visitTypeExpr(subDecl->type);
            stream << " " << subDecl->name.str << ";\n";
          }
          stream << "};\n";
        } else {
          stream << "\n\n#error Named: " << decl->name.str << " Do not understand " << __LINE__ << "\n"; return;
        }
      }
    }
    stream << "\n}  // namespace " << m->mod_name.str << "\n";
  }

  void visitTypeExpr(TypeDeclExpr* expr) {
    switch (expr->getKind()) {
    case TypeDeclExpr::Kind::Parametric: { auto* self = static_cast<ParametricTypeDeclExpr*>(expr);
      visitTypeExpr(self->base);
      stream << "<";
      bool first = true;
      for (TypeDeclExpr* param : self->params) {
        if (first) {
          first = false;
        } else {
          stream << ", ";
        }
        visitTypeExpr(param);
      }
      stream << ">";
      return;
    } case TypeDeclExpr::Kind::Named: {
      auto name = static_cast<NamedTypeDeclExpr*>(expr)->name.str;
      if (name == "Token") stream << "tok::Token";
      else if (name == "Array") stream << "std::vector";
      else if (name == "Map") stream << "std::map";
      else if (name == "String") stream << "string_view";
      else {
        stream << name << "*";
      }
      return;
    } default: stream << "\n\n#error Named:  Do not understand " << __LINE__ << "\n"; return;
    }
  }
};

struct DumpRawFunc {
  std::ostream& stream;
  explicit DumpRawFunc(std::ostream& stream) : stream(stream) {}

  void visit(Module* m) {
    stream << "namespace " << m->mod_name.str << " {\n";
    stream << "namespace parser {\n\n";
    for (auto* decl_ : m->decls) {
      if (decl_->getKind() == Decl::Kind::RawFunc) { auto* decl = reinterpret_cast<RawFuncDecl*>(decl_);
        DumpASTTypes(stream).visitTypeExpr(decl->type);
        stream << " " << decl->name.str << "(Tokenizer& tokens);\n";
      }
    }
    stream << "\n";
    for (auto* decl_ : m->decls) {
      if (decl_->getKind() == Decl::Kind::RawFunc) { auto* decl = reinterpret_cast<RawFuncDecl*>(decl_);
        DumpASTTypes(stream).visitTypeExpr(decl->type);
        stream << " " << decl->name.str << "(Tokenizer& tokens) {\n";
        dumpStmt(decl->body);
        stream << "}\n";
      }
    }
    stream << "\n}  // namespace parser\n";
    stream << "}  // namespace " << m->mod_name.str << "\n";
  }

  void dumpExpr(Expr* expr_) {
    switch (expr_->getKind()) {
    case Expr::Kind::RawNew: { auto* expr = reinterpret_cast<RawNewExpr*>(expr_); (void)expr;
      stream << "new " << expr->name.str;
      return;
    } case Expr::Kind::New: { auto* expr = reinterpret_cast<NewExpr*>(expr_); (void)expr;
      // TODO: make into re-write rule...
      auto key = "__current_self__";
      stream << "([&]() {\n";
      stream << "auto* " << key << " = new " << expr->name.str << ";\n";
      dumpStmt(expr->body);
      stream << "return " << key << ";\n";
      stream << "}())\n";
      return;
    } case Expr::Kind::TypeNew: { auto* expr = reinterpret_cast<TypeNewExpr*>(expr_); (void)expr;
      printf("\n#error unexpected:%s:%d\n", __FILE__, __LINE__);
      return;
    } case Expr::Kind::StringExpect: { auto* expr = reinterpret_cast<StringExpectExpr*>(expr_); (void)expr;
      stream << "tokens.expect(" << expr->value.str << ")";
      return;
    } case Expr::Kind::TokenExpect: { auto* expr = reinterpret_cast<TokenExpectExpr*>(expr_); (void)expr;
      stream << "tokens.expect(tok::" << expr->value.str << ")";
      return;
    } case Expr::Kind::Name: { auto* expr = reinterpret_cast<NameExpr*>(expr_); (void)expr;
      stream << expr->name.str;
      return;
    } case Expr::Kind::SelfGet: { auto* expr = reinterpret_cast<SelfGetExpr*>(expr_); (void)expr;
      stream << "__current_self__->" << expr->name.str;
      return;
    } case Expr::Kind::ConcatArray: { auto* expr = reinterpret_cast<ConcatArrayExpr*>(expr_); (void)expr;
      stream << "([&]() {\n";
      stream << "std::vector<";
      DumpASTTypes(stream).visitTypeExpr(expr->element);
      stream << "> __current_vector__;\n";
      stream << "while (tokens.peak().type != tok::" << expr->endtok.str << ") {\n";
      stream << "__current_vector__.push_back([&]() {";
      dumpStmt(expr->body);
      stream << "}());\n";
      stream << "}\n";
      stream << "return __current_vector__;\n";
      stream << "}())\n";
      return;
    } case Expr::Kind::CommaArray: { auto* expr = reinterpret_cast<CommaArrayExpr*>(expr_); (void)expr;
      stream << "([&]() {\n";
      stream << "std::vector<";
      DumpASTTypes(stream).visitTypeExpr(expr->element);
      stream << "> __current_vector__;\n";
      stream << "if (tokens.peak().type != tok::" << expr->endtok.str << ") {\n";
      stream << "while (true) {\n";
      stream << "__current_vector__.push_back([&]() {";
      dumpStmt(expr->body);
      stream << "}());\n";
      stream << "if(tokens.peak().type == tok::" << expr->commatok.str << ") {\n";
      stream << "tokens.expect(tok::" << expr->commatok.str << ");\n";
      stream << "} else {\n";
      stream << "break;\n";
      stream << "}\n";
      stream << "}\n";
      stream << "}\n";
      stream << "return __current_vector__;\n";
      stream << "}())\n";
      return;
    } case Expr::Kind::LetLoad: { auto* expr = reinterpret_cast<LetLoadExpr*>(expr_); (void)expr;
      printf("\n#error unexpected:%s:%d\n", __FILE__, __LINE__);
      return;
    } case Expr::Kind::DotGetRef: { auto* expr = reinterpret_cast<DotGetRefExpr*>(expr_); (void)expr;
      dumpExpr(expr->base);
      stream << "->" << expr->name.str;
      return;
    } case Expr::Kind::Call: { auto* expr = reinterpret_cast<CallExpr*>(expr_); (void)expr;
      stream << expr->name.str << "(tokens)";
      return;
    } case Expr::Kind::CallRawFunc: { auto* expr = reinterpret_cast<CallRawFuncExpr*>(expr_); (void)expr;
      // Not needed, can really just check the above is correct and leave it at that.
      printf("\n#error unexpected:%s:%d\n", __FILE__, __LINE__);
      return;
    }
    }
  }

  void dumpStmt(Stmt* stmt_) {
    switch (stmt_->getKind()) {
#define CASE(T) case Stmt::Kind::T: { auto* stmt = reinterpret_cast<T##Stmt*>(stmt_); (void) stmt;
#define ELSE }
    CASE(Let)
      DumpASTTypes(stream).visitTypeExpr(stmt->type);
      stream << " " << stmt->name.str << " = ";
      dumpExpr(stmt->expr);
      stream << ";\n";
      return;
    ELSE CASE(Ignore)
      dumpExpr(stmt->expr);
      stream << ";\n";
      return;
    ELSE CASE(Set)
      dumpExpr(stmt->lhs);
      stream << " = ";
      dumpExpr(stmt->rhs);
      stream << ";\n";
      return;
    ELSE CASE(Return)
      stream << "return ";
      dumpExpr(stmt->expr);
      stream << ";\n";
      return;
    ELSE CASE(Unexpected)
      stream << "tokens.unexpected(tokens.peak());\n";
      stream << "exit(-1);\n";
      return;
    ELSE CASE(Break)
      stream << "break;\n";
      return;
    ELSE CASE(Loop)
      stream << "while (true) {\n";
      dumpStmt(stmt->body);
      stream << "}\n";
      return;
    ELSE CASE(Append)
      dumpExpr(stmt->arr);
      stream  << ".push_back(";
      dumpExpr(stmt->value);
      stream << ");\n";
      return;
    ELSE CASE(Compound)
      for (auto* stmt: stmt->stmts) {
        dumpStmt(stmt);
      }
      return;
    ELSE CASE(Case)
      stream << "if (";
      visitCondition(stmt->cond);
      stream << ") {\n";
      dumpStmt(stmt->true_stmt);
      if (stmt->false_stmt->getKind() == Stmt::Kind::Case) {
        stream << "} else ";
        dumpStmt(stmt->false_stmt);
      } else if (stmt->false_stmt->getKind() == Stmt::Kind::Compound) {
        auto* false_stmt = reinterpret_cast<CompoundStmt*>(stmt->false_stmt);
        if (!false_stmt->stmts.empty()) {
          stream << "} else {\n";
          dumpStmt(stmt->false_stmt);
        }
        stream << "}\n";
      }
      return;

    ELSE CASE(NameLessLet)
      stream << "#error Problem. NameLessLet\n";
      return;
    ELSE
    }
#undef ELSE
#undef CASE
  }
  void visitCondition(Condition* cond_) {
    switch (cond_->getKind()) {
    case Condition::Kind::Type: { auto* cond = reinterpret_cast<TypeCondition*>(cond_);
      stream << "tokens.peak().type == tok::" << cond->token.str;
      break;
    } case Condition::Kind::String: { auto* cond = reinterpret_cast<StringCondition*>(cond_);
      stream << "tokens.peak().str == " << cond->token.str;
      break;
    }
    }
  }
};

}  // namespace parser_spec

int main(int argc, char **argv){
  using namespace parser_spec;
  if (argc <= 1) {
    fprintf(stderr, "Not enough files: %d.\n", argc);
    exit(-1);
  }

  std::ifstream in(argv[1], std::ios::in | std::ios::binary);
  if (!in) {
    fprintf(stderr, "Could not read file: %s\n", argv[1]);
    exit(-1);
  }
  std::string contents;
  in.seekg(0, std::ios::end);
  contents.resize(in.tellg());
  in.seekg(0, std::ios::beg);
  in.read(&contents[0], contents.size());
  in.close();

  Tokenizer tokens(contents.c_str());
  auto* m = parser::DoParse(tokens);

  DumpASTTypes(std::cout).visit(m);

  DumpRawFunc(std::cout).visit(m);
  /*
//  DumpTypeDecls(m, argv[2]);
  while (true) {
    auto token = tokens.next();
    if (token.type == tok::eof) break;
    PrintToken(token);
  }
  */
}
