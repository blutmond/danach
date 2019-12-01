#include "rules/emit-passes-template.h"
#include "rules/emit-passes.h"
#include "parser/parser-support.h"
#include "rules/template-support.h"
#include "gen/rules/interface.h"
#include "rules/string-utils.h"

#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace passes_template {

struct TopLevelDecl {
  virtual void EmitStructFwdDeclare(std::ostream& stream) {}
  virtual void EmitFwdDeclare(std::ostream& stream) {}
  virtual void EmitDefinitions(std::ostream& stream) {}
};

struct Module {
  string_view ns;
  std::vector<TopLevelDecl*> decls;
};

void EmitModuleHeader(Module* module, std::ostream& stream) {
  stream << "#pragma once\n";
  stream << "namespace " << module->ns << " {\n\n";

  for (auto* context : module->decls) {
    context->EmitStructFwdDeclare(stream);
  }
  for (auto* context : module->decls) {
    context->EmitFwdDeclare(stream);
  }

  stream << "}  // namespace " << module->ns << "\n";
}

void EmitModuleCC(Module* module, std::ostream& stream) {
  stream << "namespace " << module->ns << " {\n";
  for (auto* context : module->decls) {
    context->EmitDefinitions(stream);
  }
  stream << "}  // namespace " << module->ns << "\n";
}
void EmitModule(Module* module, std::ostream& cc_stream, std::ostream& h_stream) {
  EmitModuleHeader(module, h_stream);
  EmitModuleCC(module, cc_stream);
}

struct Type {
  virtual void Emit(std::ostream& stream) {}
};

struct OpaqueType : public Type {
  virtual void Emit(std::ostream& stream) { stream << value; }
  static OpaqueType* get(string_view value) {
    auto* result = new OpaqueType;
    result->value = value;
    return result;
  }
  string_view value;
};

struct VectorType : public Type {
  virtual void Emit(std::ostream& stream) { stream << "std::vector<"; element->Emit(stream); stream << ">"; }
  static VectorType* get(Type* element) {
    auto* result = new VectorType;
    result->element = element;
    return result;
  }
  Type* element;
};

struct FunctionType : public Type {
  Type* ret_t;
  std::vector<Type*> arg_ts;
};

struct MethodInterface {
  FunctionType type;
  std::vector<string_view> arg_names;
  string_view name;
  bool is_const = false;
  void EmitBase(std::ostream& stream, string_view prefix) {
    type.ret_t->Emit(stream);
    stream << " ";
    stream << prefix;
    if (!prefix.empty()) stream << "::";
    stream << name << "(";
    for (size_t i = 0; i < type.arg_ts.size(); ++i) {
      if (i != 0) stream << ", ";
      type.arg_ts[i]->Emit(stream);
      stream << " ";
      if (i < arg_names.size()) {
        stream << arg_names[i];
      } else {
        stream << "arg" << i;
      }
    }
    stream << ")";
  }
};

struct ConcreteType;

struct Interface : public TopLevelDecl {
  struct Method {
    MethodInterface signature;
    bool mandatory = false;
    string_view default_impl;
  };

  string_view name;
  std::vector<Method*> methods;
  ConcreteType* parent = nullptr;

  void EmitStructFwdDeclare(std::ostream& stream) override {
    stream << "struct " << name << ";\n";
  }
  void EmitFwdDeclare(std::ostream& stream) override;
  void EmitDefinitions(std::ostream& stream) override {
    for (Method* method : methods) {
      if (!method->mandatory) {
        method->signature.EmitBase(stream, name);
        stream << " {\n" << method->default_impl << "}\n";
      }
    }
  }
};

struct ConcreteType : public TopLevelDecl {
 public:
  struct Field {
    Type* type;
    string_view name;
    bool is_child_list = false;
    string_view default_value;
  };
  struct Override {
    Interface::Method* method;
  };
  Interface* implements = nullptr;
  string_view name;
  std::vector<Field*> fields;
  std::vector<Override> overrides;
  void EmitStructFwdDeclare(std::ostream& stream) override {
    stream << "struct " << name << ";\n";
  }
  void EmitFwdDeclare(std::ostream& stream) override {
    stream << "struct " << name;
    if (implements) {
      stream << " : public " << implements->name; 
    }
    stream << " {\n";
    for (auto* field : fields) {
      stream << "  ";
      field->type->Emit(stream);
      stream << " ";
      stream << field->name;
      stream << field->default_value;
      stream << ";\n";
      if (field->is_child_list) {
        auto* v = dynamic_cast<VectorType*>(field->type);
        assert(v && "must be vector");
        auto* element = v->element;

        stream << "  void add_" << field->name << "(";
        element->Emit(stream);
        stream << " value);\n";
      }
    }
    for (auto& decl : overrides) {
      auto* method = decl.method;
      stream << "  ";
      method->signature.EmitBase(stream, string_view());
      stream << " override;\n";
    }
    stream << "};\n";
  }
};

void Interface::EmitFwdDeclare(std::ostream& stream) {
  stream << "struct " << name << " {\n";
  if (parent != nullptr) {
    stream << "  " << parent->name << "* parent;\n";
  }
  for (Method* method : methods) {
    stream << "  virtual ";
    method->signature.EmitBase(stream, string_view());
    if (!method->mandatory) {
      stream << ";\n";
    } else {
      stream << " = 0;\n";
    }
  }
  stream << "};\n";
}

struct ModuleLoweringContext {
  struct TypeExprCache {
    interface_spec::TypeExpr* decl;
    Type* result = nullptr;
  };
  explicit ModuleLoweringContext(Module* module) : module(module) {}
  Module* module;
  std::map<string_view, TypeExprCache> alias;
  std::map<string_view, TopLevelDecl*> type_name_cache;

  template <typename T, typename InterfaceT>
  T* GetDecl(InterfaceT* decl) {
    return GetDecl<T, InterfaceT>(decl, decl->name.str);
  }
  template <typename T, typename InterfaceT>
  T* GetDecl(InterfaceT* decl, string_view key) {
    auto it = type_name_cache.find(key);
    if (it != type_name_cache.end()) {
      return dynamic_cast<T*>(it->second);
    }
    auto* res = new T;
    res->name = decl->name.str;
    module->decls.push_back(res);
    type_name_cache[key] = res; 
    return res;
  }
  template <typename T>
  T* GetDecl(string_view key) {
    auto it = type_name_cache.find(key);
    if (it != type_name_cache.end()) {
      return dynamic_cast<T*>(it->second);
    }
    fprintf(stderr, "Could not find: %s\n", std::string(key).c_str());
    exit(-1);
  }

  Type* GetType(interface_spec::TypeExpr* type) {
    using namespace interface_spec;
    switch (type->getKind()) {
    case TypeExpr::Kind::Opaque:
      return OpaqueType::get(
          *new std::string(Unescaped(reinterpret_cast<OpaqueTypeExpr*>(type)->value.str)));
    case TypeExpr::Kind::Nominal: {
      auto key = reinterpret_cast<NominalTypeExpr*>(type)->value.str;
      auto it = alias.find(key);
      if (it == alias.end()) {
        fprintf(stderr, "Could not find: %s\n", std::string(key).c_str());
        exit(-1);
      }
      if (!it->second.result) {
        it->second.result = GetType(it->second.decl);
      }
      return it->second.result;
    }
    case TypeExpr::Kind::Vector: {
      auto* type_ = reinterpret_cast<VectorTypeExpr*>(type);
      return VectorType::get(GetType(type_->element));
    }
    }
  }

  FunctionType GetType(interface_spec::FuncSignature* sig) {
    FunctionType result;
    for (auto* arg : sig->args) {
      result.arg_ts.push_back(GetType(arg->type));
    }
    result.ret_t = GetType(sig->ret_t);
    return result;
  }

  std::vector<string_view> GetArgNames(interface_spec::FuncSignature* sig) {
    std::vector<string_view> result;
    for (auto* arg : sig->args) {
      result.push_back(arg->name.str);
    }
    return result;
  }

  void Lower(interface_spec::Module* txtmodule) {
    using namespace interface_spec;
    for (auto* decl_ : txtmodule->decls) {
      switch (decl_->getKind()) {
      case Decl::Kind::Interface: {
        auto* decl = reinterpret_cast<InterfaceDecl*>(decl_);
        GetDecl<Interface>(decl);
        break;
      }
      case Decl::Kind::Concrete: {
        auto* decl = reinterpret_cast<ConcreteDecl*>(decl_);
        GetDecl<ConcreteType>(decl);
        break;
      }
      case Decl::Kind::ConcreteWithoutInterface: {
        auto* decl = reinterpret_cast<ConcreteWithoutInterfaceDecl*>(decl_);
        GetDecl<ConcreteType>(decl);
        break;
      }
      case Decl::Kind::TypeAlias: {
        auto* decl = reinterpret_cast<TypeAliasDecl*>(decl_);
        alias[decl->name.str].decl = decl->type; 
        break;
      }
      }
    }
    for (auto* decl_ : txtmodule->decls) {
      switch (decl_->getKind()) {
      case Decl::Kind::Interface: {
        auto* decl = reinterpret_cast<InterfaceDecl*>(decl_);
        auto* res = GetDecl<Interface>(decl);
        for (auto* body_decl_ : decl->body) {
          switch (body_decl_->getKind()) {
          case InterfaceBodyDecl::Kind::Parent: {
            auto* body_decl = reinterpret_cast<ParentInterfaceBodyDecl*>(body_decl_);
            res->parent = GetDecl<ConcreteType>(body_decl->name.str);
            break;
          }
          case InterfaceBodyDecl::Kind::Func: {
            auto* body_decl = reinterpret_cast<FuncInterfaceBodyDecl*>(body_decl_);
            auto* method = new Interface::Method;
            method->signature.type = GetType(body_decl->sig);
            method->signature.arg_names = GetArgNames(body_decl->sig);
            method->signature.name = body_decl->name.str;
            res->methods.push_back(method);
            break;
          }
          }
        }
        break;
      }
      case Decl::Kind::Concrete: {
        auto* decl = reinterpret_cast<ConcreteDecl*>(decl_);
        auto* res = GetDecl<ConcreteType>(decl);
        res->implements = GetDecl<Interface>(decl->implements.str);
        for (auto* body_decl_ : decl->body) {
          switch (body_decl_->getKind()) {
          case ConcreteBodyDecl::Kind::Func: {
            auto* body_decl = reinterpret_cast<FuncConcreteBodyDecl*>(body_decl_);
            auto* method = new Interface::Method;
            method->signature.type = GetType(body_decl->sig);
            method->signature.arg_names = GetArgNames(body_decl->sig);
            method->signature.name = body_decl->name.str;
            res->overrides.push_back({method});
            break;
          }
          case ConcreteBodyDecl::Kind::Var: {
            auto* body_decl = reinterpret_cast<VarConcreteBodyDecl*>(body_decl_);
            auto* f = new ConcreteType::Field;
            for (auto* attr : body_decl->attrs) {
              switch (attr->getKind()) {
              case Attr::Kind::Default:
                f->default_value = *new std::string(Unescaped(reinterpret_cast<DefaultAttr*>(attr)->value.str));
                break;
              case Attr::Kind::IsChildList:
                f->is_child_list = true;
                break;
              default:
                fprintf(stderr, "Unhandled attr.\n");
                exit(-1);
              }
            }
            f->type = GetType(body_decl->type);
            f->name = body_decl->name.str;
            res->fields.push_back(f);
            break;
          }
          }
        }
        break;
      }
      case Decl::Kind::TypeAlias: {
        auto* decl = reinterpret_cast<TypeAliasDecl*>(decl_);
        alias[decl->name.str].decl = decl->type; 
        break;
      }
      }
    }
  }
};

void EmitPassesTemplateToFilename(const char* src,
                                  const char* cc_fname,
                                  const char* h_fname) {
  auto contents = LoadFile(src);
  interface_spec::Tokenizer tokens(contents.c_str());

  auto* module = new Module;
  auto* txtmodule = interface_spec::parser::DoParse(tokens);
  module->ns = txtmodule->ns.str;
  ModuleLoweringContext ctx(module);
  ctx.Lower(txtmodule);

  EmitStream cc;
  EmitStream h;
  EmitModule(module, cc.stream(), h.stream());
  h.write(h_fname);
  cc.write(cc_fname);
}

}  // namespace passes_template
