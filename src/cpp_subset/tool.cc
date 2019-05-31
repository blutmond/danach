#include <string>
#include <assert.h>
#include <experimental/string_view>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>

using std::experimental::string_view;

#include "tokens/line_number_helper.cc"
#include "gen/cpp_subset/tokenizer.cc"
#include "gen/cpp_subset/parser.cc"

void UnescapedDump(std::ostream& stream, string_view data) {
  // TODO: This is bad.
  data.remove_prefix(1);
  data.remove_suffix(1);
  size_t pos = data.find('\\');
  while (pos != string_view::npos) {
    stream << data.substr(0, pos);
    data.remove_prefix(pos);
    data.remove_prefix(1);
    if (data[0] == 'n') {
      stream << "\n";
      data.remove_prefix(1);
    }
    pos = data.find('\\');
  }
  stream << data;
}

// /tmp/tokenizer-template data/cpp_subset_tok cpp_subset > gen/cpp_subset_tok.cc
// /tmp/gen-parser2 data/cpp_subset_parser | clang-format-3.8 > gen/cpp_subset_parser.cc

namespace cpp_subset {

struct LowerEverything {
  std::ostream& stream;
  explicit LowerEverything(std::ostream& stream) : stream(stream) {}

  void visit(Module* m) {
    for (auto* decl : m->decls) { visitNames(decl); }
    for (auto* decl : m->decls) { visit(decl); }
  }
  void visit(Decl* m) {
    switch (m->getKind()) {
    case Decl::Kind::InlineCpp: { auto* decl = reinterpret_cast<InlineCppDecl*>(m); (void)decl;
      UnescapedDump(stream, decl->value.str);
      stream << "\n";
      return;
    } case Decl::Kind::KnownCppType: { auto* decl = reinterpret_cast<KnownCppTypeDecl*>(m); (void)decl;
      stream << "using " << decl->name.str << " = ";
      UnescapedDump(stream, decl->value.str);
      stream << ";\n";
      return;
    } case Decl::Kind::FnExpr: { auto* decl = reinterpret_cast<FnExprDecl*>(m); (void)decl;
      return;
    } case Decl::Kind::KnownTemplate: { auto* decl = reinterpret_cast<KnownTemplateDecl*>(m); (void)decl;
      return;
    } case Decl::Kind::RawFunc: { auto* decl = reinterpret_cast<RawFuncDecl*>(m); (void)decl;
      // TODO: dump things... 
      visit(decl->ret_t);
      stream << " " << decl->name.str << "(";
      int i = 0;
      for (auto* arg : decl->args) {
        if (i != 0) stream << ", ";
        ++i;
        visit(arg->type);
        stream << " " << arg->name.str;
      }
      stream << ") {\n";
      visit(decl->body);
      stream << "}\n";
      return;
    }
    }
  }
  void visit(TypeRef* _type) {
    switch (_type->getKind()) {
    case TypeRef::Kind::Named: { auto* type = reinterpret_cast<NamedTypeRef*>(_type); (void)(type);
      stream << type->name.str;
      return;
    } case TypeRef::Kind::Template: { auto* type = reinterpret_cast<TemplateTypeRef*>(_type); (void)(type);
      visit(type->base);
      stream << "<";
      int i = 0;
      for (auto& item : type->args) {
        if (i != 0) stream << ", ";
        ++i;
        visit(item);
      }
      stream << ">";
      return;
    } case TypeRef::Kind::Void: { auto* type = reinterpret_cast<VoidTypeRef*>(_type); (void)(type);
      stream << "void";
      return;
    }
    }
  }
  void visitNames(Decl* m) {
    switch (m->getKind()) {
    case Decl::Kind::InlineCpp: { auto* decl = reinterpret_cast<InlineCppDecl*>(m); (void)decl;
      return;
    } case Decl::Kind::KnownCppType: { auto* decl = reinterpret_cast<KnownCppTypeDecl*>(m); (void)decl;
      return;
    } case Decl::Kind::FnExpr: { auto* decl = reinterpret_cast<FnExprDecl*>(m); (void)decl;
      fn_exprs[decl->name.str] = decl;
      return;
    } case Decl::Kind::KnownTemplate: { auto* decl = reinterpret_cast<KnownTemplateDecl*>(m); (void)decl;
      return;
    } case Decl::Kind::RawFunc: { auto* decl = reinterpret_cast<RawFuncDecl*>(m); (void)decl;
      return;
    }
    }
  }
  std::map<string_view, FnExprDecl*> fn_exprs;
  void visit(Expr* _expr) {
    switch (_expr->getKind()) {
    case Expr::Kind::Named: { auto* expr = reinterpret_cast<NamedExpr*>(_expr); (void)expr;
      stream << expr->name.str;
      return;
    } case Expr::Kind::InlineCpp: { auto* expr = reinterpret_cast<InlineCppExpr*>(_expr); (void)expr;
      UnescapedDump(stream, expr->value.str);
      return;
    } case Expr::Kind::Dot: { auto* expr = reinterpret_cast<DotExpr*>(_expr); (void)expr;
      visit(expr->base);
      stream << "." << expr->name.str;
      return;
    } case Expr::Kind::IntegerLiteral: { auto* expr = reinterpret_cast<IntegerLiteralExpr*>(_expr); (void)expr;
      stream << expr->value.str;
      return;
    } case Expr::Kind::StringLiteral: { auto* expr = reinterpret_cast<StringLiteralExpr*>(_expr); (void)expr;
      stream << expr->value.str;
      return;
    } case Expr::Kind::Call: { auto* expr = reinterpret_cast<CallExpr*>(_expr); (void)expr;
      if (expr->base->getKind() == Expr::Kind::Named) {
        string_view base = reinterpret_cast<NamedExpr*>(expr->base)->name.str;
        auto it = fn_exprs.find(base);
        if (it != fn_exprs.end()) {
          stream << "(";
          int i = 0;
          auto& items = it->second->items;
          i = 0;
          for (auto* item : items) {
            if (item->getKind() == FnExprItem::Kind::Slot) {
              assert(i < expr->args.size());
              visit(expr->args[i]);
              ++i;
            } else {
              UnescapedDump(stream, reinterpret_cast<StrFnExprItem*>(item)->value.str);
            }
          }
          // Better diagnostics here.
          assert(i == expr->args.size());
          stream << ")";
          return;
        }
      }
      visit(expr->base);
      stream << "(";
      int i = 0;
      for (auto* expr : expr->args) {
        if (i != 0) stream << ", ";
        ++i;
        visit(expr);
      }
      stream << ")";
      return;
    }
    }
  }
  void visit(Stmt* _stmt) {
    switch (_stmt->getKind()) {
    case Stmt::Kind::Case: { auto* stmt = reinterpret_cast<CaseStmt*>(_stmt); (void)stmt;
      stream << "if (";
      visit(stmt->cond);
      stream << ") {\n";
      visit(stmt->true_stmt);
      if (stmt->false_stmt->getKind() == Stmt::Kind::Case) {
        stream << "} else ";
        visit(stmt->false_stmt);
      } else if (stmt->false_stmt->getKind() == Stmt::Kind::Compound) {
        auto* false_stmt = reinterpret_cast<CompoundStmt*>(stmt->false_stmt);
        if (!false_stmt->stmts.empty()) {
          stream << "} else {\n";
          visit(stmt->false_stmt);
        }
        stream << "}\n";
      }
      return;
    } case Stmt::Kind::Var: { auto* stmt = reinterpret_cast<VarStmt*>(_stmt); (void)stmt;
      visit(stmt->type);
      stream << " " << stmt->name.str << ";\n";
      return;
    } case Stmt::Kind::Break: { auto* stmt = reinterpret_cast<BreakStmt*>(_stmt); (void)stmt;
      stream << "break;\n";
      return;
    } case Stmt::Kind::Loop: { auto* stmt = reinterpret_cast<LoopStmt*>(_stmt); (void)stmt;
      stream << "while (true) {\n";
      visit(stmt->body);
      stream << "}\n";
      return;
    } case Stmt::Kind::VarCall: { auto* stmt = reinterpret_cast<VarCallStmt*>(_stmt); (void)stmt;
      visit(stmt->type);
      stream << " " << stmt->name.str << "(";
      int i = 0;
      for (auto* expr : stmt->args) {
        if (i != 0) stream << ", ";
        ++i;
        visit(expr);
      }
      stream << ");\n";
      return;
    } case Stmt::Kind::VarAssign: { auto* stmt = reinterpret_cast<VarAssignStmt*>(_stmt); (void)stmt;
      visit(stmt->type);
      stream << " " << stmt->name.str << " = ";
      visit(stmt->value);
      stream << ";\n";
      return;
    } case Stmt::Kind::Return: { auto* stmt = reinterpret_cast<ReturnStmt*>(_stmt); (void)stmt;
      stream << "return ";
      visit(stmt->expr);
      stream << ";\n";
      return;
    } case Stmt::Kind::Discard: { auto* stmt = reinterpret_cast<DiscardStmt*>(_stmt); (void)stmt;
      visit(stmt->expr);
      stream << ";\n";
      return;
    } case Stmt::Kind::Compound: { auto* stmt = reinterpret_cast<CompoundStmt*>(_stmt); (void)stmt;
      for (auto* sub : stmt->stmts) {
        visit(sub);
      }
      return;
    }
    }
  }
};

}  // namespace cpp_subset

int main(int argc, char **argv){
  using namespace cpp_subset;
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

  LowerEverything(std::cout).visit(m);
  /*
//  DumpTypeDecls(m, argv[2]);
  while (true) {
    auto token = tokens.next();
    if (token.type == tok::eof) break;
    PrintToken(token);
  }
  */
}
