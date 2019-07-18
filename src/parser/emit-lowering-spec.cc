#include <assert.h>
namespace lowering_spec {

void EmitType(TypeRef* t) {
  switch (t->getKind()) {
  case TypeRef::Kind::Void:
    std::cout << "void";
    return;
  case TypeRef::Kind::Member: {
    auto* type = reinterpret_cast<MemberTypeRef*>(t);
    EmitType(type->base);
    std::cout << "::" << type->name.str;
    return;
  } case TypeRef::Kind::Named: {
    auto* type = reinterpret_cast<NamedTypeRef*>(t);
    std::cout << type->name.str;
    return;
  } default:
    std::cout << "unknown";
    return;
  }
}

void EmitTypeSignature(TypeRef* t) {
  auto& stream = std::cout;
  switch (t->getKind()) {
  case TypeRef::Kind::Void:
    std::cout << "void";
    return;
  case TypeRef::Kind::Template: { auto* self = static_cast<TemplateTypeRef*>(t);
    EmitTypeSignature(self->base);
    stream << "<";
    bool first = true;
    for (auto* param : self->args) {
      if (first) {
        first = false;
      } else {
        stream << ", ";
      }
      EmitTypeSignature(param);
    }
    stream << ">";
    return;
  } case TypeRef::Kind::Member: {
    auto* type = reinterpret_cast<MemberTypeRef*>(t);
    EmitType(type->base);
    std::cout << "::" << type->name.str;
    return;
  } case TypeRef::Kind::Named: {
    auto name = reinterpret_cast<NamedTypeRef*>(t)->name.str;
    if (name == "Token") stream << "tok::Token";
    else if (name == "Array") stream << "std::vector";
    else if (name == "Map") stream << "std::map";
    else if (name == "String") stream << "string_view";
    else if (name == "char") stream << "char";
    else if (name == "int") stream << "int";
    else if (name == "bool") stream << "bool";
    else {
      stream << name << "*";
    }
    return;
  }
  }
}

void EmitStmt(ContextFinderContext* ctx, Stmt* _stmt);

void EmitExpr(ContextFinderContext* ctx, Expr* _expr) {
  switch (_expr->getKind()) {
  case Expr::Kind::New: {
    auto* expr = reinterpret_cast<NewExpr*>(_expr);
    std::cout << "({\nauto* self = new ";
    EmitType(expr->type);
    std::cout << ";\n";
    EmitStmt(ctx, expr->body);
    std::cout << "self;\n";
    std::cout << "})";
    break;
  } case Expr::Kind::Number: {
    auto* expr = reinterpret_cast<NumberExpr*>(_expr);
    std::cout << expr->value.str;
    break;
  } case Expr::Kind::Str: {
    auto* expr = reinterpret_cast<StrExpr*>(_expr);
    std::cout << expr->value.str;
    break;
  } case Expr::Kind::Dot: {
    auto* expr = reinterpret_cast<DotExpr*>(_expr);
    EmitExpr(ctx, expr->base);
    std::cout << "." << expr->name.str;
    break;
  } case Expr::Kind::Arrow: {
    auto* expr = reinterpret_cast<DotExpr*>(_expr);
    EmitExpr(ctx, expr->base);
    std::cout << "->" << expr->name.str;
    break;
  } case Expr::Kind::Named: {
    auto* expr = reinterpret_cast<NamedExpr*>(_expr);
    std::cout << expr->name.str;
    break;
  } case Expr::Kind::Index: {
    auto* expr = reinterpret_cast<IndexExpr*>(_expr);
    EmitExpr(ctx, expr->base);
    int i = 0;
    std::cout << "[";
    for (auto* arg : expr->args) {
      if (i != 0) { std::cout << ", "; } ++i;
      EmitExpr(ctx, arg);
    }
    std::cout << "]";
    break;
  } case Expr::Kind::ColonColon: {
    auto* expr = reinterpret_cast<ColonColonExpr*>(_expr);
    EmitExpr(ctx, expr->base);
    std::cout << "::" << expr->name.str;
    break;
  } case Expr::Kind::Call: {
    auto* expr = reinterpret_cast<CallExpr*>(_expr);
    if (expr->base->getKind() == Expr::Kind::Named) {
      auto name = reinterpret_cast<NamedExpr*>(expr->base)->name.str;
      if (name == "assert") {
        std::cout << "({\n";
        std::cout << "if (!(";
        EmitExpr(ctx, expr->args[0]);
        std::cout << ")) {\n";
        std::cout << "std::cerr << R\"ASSERT(Assert failed: ";
        int i = 0;
        for (auto* arg : expr->args) {
          if (i != 0) { std::cout << ", "; } ++i;
          EmitExpr(ctx, arg);
        }
        std::cout << "\n)ASSERT\";\n";
        std::cout << "exit(-1);\n";
        std::cout << "}\n})";
        break;
      }
      if (name == "error") {
        std::cout << "({\n";
        std::cout << "std::cerr << R\"ASSERT(Assert failed: ";
        int i = 0;
        for (auto* arg : expr->args) {
          if (i != 0) { std::cout << ", "; } ++i;
          EmitExpr(ctx, arg);
        }
        std::cout << "\n)ASSERT\";\n";
        std::cout << "exit(-1);\n";
        std::cout << "})";
        break;
      }
    }
    EmitExpr(ctx, expr->base);
    std::cout << "(";
    int i = 0;
    if (expr->base->getKind() == Expr::Kind::Named) {
      for (auto* param : ctx->GetHiddenParams(reinterpret_cast<NamedExpr*>(expr->base)->name.str)) {
        if (i != 0) { std::cout << ", "; } ++i;
        std::cout << param->name.str;
      }
    }
    for (auto* arg : expr->args) {
      if (i != 0) { std::cout << ", "; } ++i;
      EmitExpr(ctx, arg);
    }
    std::cout << ")";
    break;
  } case Expr::Kind::CompEqEq: {
    auto* expr = reinterpret_cast<CompEqEqExpr*>(_expr);
    EmitExpr(ctx, expr->lhs);
    std::cout << " == ";
    EmitExpr(ctx, expr->rhs);
    break;
  } case Expr::Kind::Assign: {
    auto* expr = reinterpret_cast<AssignExpr*>(_expr);
    EmitExpr(ctx, expr->lhs);
    std::cout << " = ";
    EmitExpr(ctx, expr->rhs);
    break;
  }
  }
}

void EmitStmt(ContextFinderContext* ctx, Stmt* stmt) {
  auto* _stmt = stmt;
  switch (_stmt->getKind()) {
  case Stmt::Kind::Compound: {
    auto* stmt = reinterpret_cast<CompoundStmt*>(_stmt);
    for (auto* cstmt : stmt->stmts) { EmitStmt(ctx, cstmt); }
    break;
  } case Stmt::Kind::Return: {
    auto* stmt = reinterpret_cast<ReturnStmt*>(_stmt);
    std::cout << "return ";
    EmitExpr(ctx, stmt->expr);
    std::cout << ";\n";
    break;
  } case Stmt::Kind::Let: {
    auto* stmt = reinterpret_cast<LetStmt*>(_stmt);
    std::cout << "auto __tmp__" << stmt->name.str << " = ";
    EmitExpr(ctx, stmt->expr);
    std::cout << ";\n";
    std::cout << "auto " << stmt->name.str << " = std::move(__tmp__" << stmt->name.str << ");\n";
    break;
  } case Stmt::Kind::Var: {
    auto* stmt = reinterpret_cast<VarStmt*>(_stmt);
    EmitTypeSignature(stmt->type);
    std::cout << " " << stmt->name.str << ";";
    break;
  } case Stmt::Kind::OpenWithType: {
    auto* stmt = reinterpret_cast<OpenWithTypeStmt*>(_stmt);
    std::cout << "{\n";
    std::cout << "auto __tmp_switch_name = " << stmt->name.str << ";\n";
    std::cout << "switch (" << stmt->name.str << "->getKind()) {\n";
    auto& stmts = reinterpret_cast<CompoundStmt*>(stmt->body)->stmts;
    bool case_open = false;
    for (auto* cstmt : stmts) {
      if (cstmt->getKind() == Stmt::Kind::Case) {
        auto* mcstmt = reinterpret_cast<CaseStmt*>(cstmt);
        if (case_open) std::cout << "break;\n} ";
        case_open = true;
        std::cout << "case ";
        EmitType(stmt->type);
        std::cout << "::Kind::" << mcstmt->name.str << ": {\n";
        std::cout << "auto* " << stmt->name.str
            << " = reinterpret_cast<" << mcstmt->name.str;
        EmitType(stmt->type);
        std::cout
            << "*>(__tmp_switch_name);\n";
        std::cout << "(void)" << stmt->name.str << ";\n";
      } else if (cstmt->getKind() == Stmt::Kind::Default) {
        if (case_open) std::cout << "break;\n} ";
        case_open = true;
        std::cout << "default: {\n";
      } else {
        EmitStmt(ctx, cstmt);
      }
    }
    if (case_open) std::cout << "}\n";
    std::cout << "}\n";
    std::cout << "}\n";
    break;
  } case Stmt::Kind::Open: {
    auto* stmt = reinterpret_cast<OpenStmt*>(_stmt);
    std::cout << "{\n";
    std::cout << "auto __tmp_switch_name = " << stmt->name.str << ";\n";
    std::cout << "switch (" << stmt->name.str << "->getKind()) {\n";
    auto& stmts = reinterpret_cast<CompoundStmt*>(stmt->body)->stmts;
    bool case_open = false;
    for (auto* cstmt : stmts) {
      if (cstmt->getKind() == Stmt::Kind::Case) {
        auto* mcstmt = reinterpret_cast<CaseStmt*>(cstmt);
        if (case_open) std::cout << "break;\n} ";
        case_open = true;
        std::cout << "case Kind::" << mcstmt->name.str << ": {\n";
        std::cout << "auto* " << stmt->name.str
            << " = reinterpret_cast<" << mcstmt->name.str << ">(__tmp_switch_name);\n";

      } else if (cstmt->getKind() == Stmt::Kind::Default) {
        if (case_open) std::cout << "break;\n} ";
        case_open = true;
        std::cout << "default: {\n";
      } else {
        EmitStmt(ctx, cstmt);
      }
    }
    if (case_open) std::cout << "}\n";
    std::cout << "}\n";
    std::cout << "}\n";
    break;
  } case Stmt::Kind::Case: {
    std::cerr << "Case can only be used as part of a switch...\n";
    exit(-1);
    break;
  } case Stmt::Kind::Emitter: {
    auto* stmt = reinterpret_cast<EmitterStmt*>(_stmt);
    assert(stmt->body->getKind() == Stmt::Kind::Compound);
    auto* bstmt = reinterpret_cast<CompoundStmt*>(stmt->body);
    for (auto* cstmt : bstmt->stmts) {
      if (cstmt->getKind() == Stmt::Kind::Discard) { 
        auto* stmt = reinterpret_cast<DiscardStmt*>(cstmt);
        std::cout << "std::cout << (";
        EmitExpr(ctx, stmt->expr);
        std::cout << ");\n";
      } else {
        EmitStmt(ctx, cstmt);
      }
    }
    break;
  } case Stmt::Kind::DbgEmitter: {
    auto* stmt = reinterpret_cast<DbgEmitterStmt*>(_stmt);
    assert(stmt->body->getKind() == Stmt::Kind::Compound);
    auto* bstmt = reinterpret_cast<CompoundStmt*>(stmt->body);
    for (auto* cstmt : bstmt->stmts) {
      if (cstmt->getKind() == Stmt::Kind::Discard) { 
        auto* stmt = reinterpret_cast<DiscardStmt*>(cstmt);
        std::cout << "std::cerr << (";
        EmitExpr(ctx, stmt->expr);
        std::cout << ");\n";
      } else {
        EmitStmt(ctx, cstmt);
      }
    }
    break;
  } case Stmt::Kind::Break: {
    std::cout << "break;\n";
    break;
  } case Stmt::Kind::ReturnVoid: {
    std::cout << "return;\n";
    break;
  } case Stmt::Kind::If: {
    auto* stmt = reinterpret_cast<IfStmt*>(_stmt);
    std::cout << "if (";
    EmitExpr(ctx, stmt->cond);
    std::cout << ") {\n";
    EmitStmt(ctx, stmt->body);
    std::cout << "}\n";
    break;
  } case Stmt::Kind::IfElse: {
    auto* stmt = reinterpret_cast<IfElseStmt*>(_stmt);
    std::cout << "if (";
    EmitExpr(ctx, stmt->cond);
    std::cout << ") {\n";
    EmitStmt(ctx, stmt->body);
    std::cout << "} else {\n";
    EmitStmt(ctx, stmt->else_body);
    std::cout << "}\n";
    break;
  } case Stmt::Kind::Loop: {
    auto* stmt = reinterpret_cast<LoopStmt*>(_stmt);
    std::cout << "while (true) {\n";
    EmitStmt(ctx, stmt->body);
    std::cout << "}\n";
    break;
  } case Stmt::Kind::For: {
    auto* stmt = reinterpret_cast<ForStmt*>(_stmt);
    std::cout << "for (auto " << stmt->name.str << " : ";
    EmitExpr(ctx, stmt->sequence);
    std::cout << ") {\n";
    EmitStmt(ctx, stmt->body);
    std::cout << "}\n";
    break;
  } case Stmt::Kind::Scope: {
    auto* stmt = reinterpret_cast<ScopeStmt*>(_stmt);
    std::cout << "{\n";
    std::cout << "auto __tmp__" << stmt->name.str << " = ";
    EmitExpr(ctx, stmt->expr);
    std::cout << ";\n";
    std::cout << "auto " << stmt->name.str << " = std::move(__tmp__" << stmt->name.str << ");\n";
    EmitStmt(ctx, stmt->body);
    std::cout << "}\n";
    break;
  } case Stmt::Kind::Default: {
    std::cerr << "Default can only be used as part of a switch...\n";
    exit(-1);
    break;
  } case Stmt::Kind::Discard: {
    auto* stmt = reinterpret_cast<DiscardStmt*>(_stmt);
    EmitExpr(ctx, stmt->expr);
    std::cout << ";\n";
    break;
  }
  }
}

void EmitFuncDeclHeader(FuncDecl* decl) {
  EmitTypeSignature(decl->ret_t);
  std::cout << " " << decl->name.str << "(";
  int i = 0;
  for (auto* arg : decl->args) {
    if (i != 0) { std::cout << ", "; } ++i;
    EmitTypeSignature(arg->type);
    std::cout << " " << arg->name.str;
  }
  std::cout << ")";
}

void EmitFuncDecl(ContextFinderContext* ctx, FuncDecl* decl) {
  EmitFuncDeclHeader(decl);
  std::cout << " {\n";
  EmitStmt(ctx, decl->body);
  std::cout << "}\n";
}

void Emit(Module* m) {
  std::cout << "namespace " << m->mod_name.str << " {\n\n";

  auto* ctx = new ContextFinderContext;

  for (auto* decl : m->decls) {
    if (decl->getKind() == Decl::Kind::Func) {
      EmitFuncDeclHeader(reinterpret_cast<FuncDecl*>(decl)); std::cout << ";\n";
      ctx->RegisterFunc(reinterpret_cast<FuncDecl*>(decl));
    } else if (decl->getKind() == Decl::Kind::Context) {
      ctx->RegisterContext(reinterpret_cast<ContextDecl*>(decl));
    }
  }

  for (auto* decl : m->decls) {
    if (decl->getKind() == Decl::Kind::Func) {
      for (auto* arg : reinterpret_cast<FuncDecl*>(decl)->args) {
        if (auto* sub_ctx = ctx->isContextUsage(arg->name.str)) {
          ctx->HardSetContext(reinterpret_cast<FuncDecl*>(decl), sub_ctx);
        } else {
          break;
        }
      }
    }
  }

  std::cout << "\n";
  for (auto* decl : m->decls) {
    if (decl->getKind() == Decl::Kind::Func) {
      EmitFuncDecl(ctx, reinterpret_cast<FuncDecl*>(decl));
    }
  }

  std::cout << "\n}  // namespace " << m->mod_name.str << "\n";
}

}  // namespace lowering_spec
