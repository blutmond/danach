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
