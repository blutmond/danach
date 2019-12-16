#include "rules/emit-passes.h"
#include "parser/parser-support.h"
#include "rules/template-support.h"

#include <fstream>
#include <sstream>
#include <iostream>

// #define TURN_OFF
#ifndef TURN_OFF
#include "gen/rules/emit-passes-meta.h"
#else

namespace passes {

struct ContextDef;

struct IndexComponent {
  ContextDef* parent;
  virtual void EmitPublicDecls(std::ostream& stream) {}
  virtual void EmitPrivateDecls(std::ostream& stream) {}
  virtual void EmitImpls(std::ostream& stream) {}
};

struct TopLevelDecl {
  virtual void EmitStructFwdDeclare(std::ostream& stream) {}
  virtual void EmitFwdDeclare(std::ostream& stream) {}
  virtual void EmitDefinitions(std::ostream& stream) {}
};

struct ContextDef : public TopLevelDecl {
  string_view name;
  ContextDef* context = nullptr;

  virtual void EmitStructFwdDeclare(std::ostream& stream);
  virtual void EmitFwdDeclare(std::ostream& stream);
  virtual void EmitDefinitions(std::ostream& stream);
  void add_decls(IndexComponent* decl);
 private:
  std::vector<IndexComponent*> decls;
};

}  // namespace passes

#endif

namespace passes {

void ContextDef::EmitStructFwdDeclare(std::ostream& stream) {
  stream << "class " << name << ";\n";
}

void ContextDef::EmitFwdDeclare(std::ostream& stream) {
  stream << "\n";
  stream << "class " << name << " {\n";
  stream << " public:\n";
  for (auto* item : decls) {
    item->EmitPublicDecls(stream);
  }
  stream << " private:\n";
  for (auto* item : decls) {
    item->EmitPrivateDecls(stream);
  }
  stream << "};\n";
}

void ContextDef::EmitDefinitions(std::ostream& stream) {
  for (auto* item : decls) {
    stream << "\n";
    item->EmitImpls(stream);
  }
}

void ContextDef::add_decls(IndexComponent* decl) {
  decls.push_back(decl);
  decl->parent = this;
}

struct Module {
  string_view ns;
  std::vector<TopLevelDecl*> decls;
};

struct MemoizedFunction : public IndexComponent {
  string_view result_type;
  string_view name;
  string_view fn_name;
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  " << result_type << "* " << name << "(string_view key);\n";
  }
  virtual void EmitPrivateDecls(std::ostream& stream) {
    stream << "  std::unordered_map<std::string, " << result_type << "*> "
        << name << "_storage;\n";
  }
  virtual void EmitImpls(std::ostream& stream) {
    // Check for possible cycles...
    stream << result_type << "* " << parent->name << "::"
        << name << "(string_view key) {\n";
    stream << "  auto key_copy = std::string(key);\n";
    stream << "  auto it = " << name << "_storage.find(key_copy);\n";
    stream << "  if (it != " << name << "_storage.end()) return it->second;\n";
    stream << "  auto* result = " << fn_name << "(this, key);\n";
    stream << "  " << name << "_storage[key_copy] = result;\n";
    stream << "  return result;\n";
    stream << "}\n";
  }
};

struct MemoizedCycleDetector : public IndexComponent {
  string_view result_type;
  string_view name;
  string_view fn_name;
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  " << result_type << (result_type == "void" ? " " : "* ")
        << name << "(string_view key);\n";
  }
  virtual void EmitPrivateDecls(std::ostream& stream) {
    stream << "  struct " << name << "_CacheState {\n";
    stream << "    bool finished = false;\n";
    stream << "    bool started = false;\n";
    if (result_type != "void")
      stream << "    " << result_type << "* result = nullptr;\n";
    stream << "  };\n";
    stream << "  std::unordered_map<std::string, " << name << "_CacheState> "
        << name << "_storage;\n";
  }

  virtual void EmitImpls(std::ostream& stream) {
    stream << result_type << (result_type == "void" ? " " : "* ")
        << parent->name << "::" << name << "(string_view key) {\n";
    stream << "  auto& state = " << name << "_storage[std::string(key)];\n";
//    stream << "  std::cout << \"Running(" << name << ", \" << key << \")\\n\";";
    stream << "  if (!state.finished) {\n";
    stream << "    assert(!state.started);\n";
    stream << "    state.started = true;\n";
    if (result_type != "void")
      stream << "    state.result = " << fn_name << "(this, key);\n";
    else
      stream << "    " << fn_name << "(this, key);\n";
    stream << "    state.finished = true;\n";
    stream << "  }\n";
    if (result_type != "void")
      stream << "  return state.result;\n";
    stream << "}\n";
  }
};

void EmitModule(Module* module, std::ostream& cc_stream, std::ostream& h_stream) {
  {
    auto& stream = h_stream;
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
  {
    auto& stream = cc_stream;
    stream << "namespace " << module->ns << " {\n";
    for (auto* context : module->decls) {
      context->EmitDefinitions(stream);
    }
    stream << "}  // namespace " << module->ns << "\n";
  }
}

struct PathLookup : public IndexComponent {
  string_view name;
  string_view result_type;
  string_view set_name;
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  " << result_type << " " << name << "(string_view path);\n";
  }
  virtual void EmitImpls(std::ostream& stream) {
    stream << result_type << " " << parent->name << "::" << name << "(string_view path) {\n";
    stream << "  auto it = " << set_name << ".find(std::string(path));\n";
    stream << "  if (it == " << set_name << ".end()) {\n";
    stream << "    std::cerr << \"No such " << name << ": \" << path << \"\\n\";\n";
    stream << "    exit(EXIT_FAILURE);\n";
    stream << "  }\n";
    stream << "  return it->second;\n";
    stream << "}\n";
  }
};

struct TopLevelDeclHack : public TopLevelDecl {
  virtual void EmitFwdDeclare(std::ostream& stream) {
    stream << "  RuleFile* LoadRuleFile(RuleModuleContext* ctx, string_view path);\n";
  }
  virtual void EmitDefinitions(std::ostream& stream) {
    stream << "RuleFile* LoadRuleFile(RuleModuleContext* ctx, string_view path) {\n";
    stream << "  auto* result = new RuleFile;\n";
    stream << "  result->parent = ctx;\n";
    stream << "  result->filename = std::string(path);\n";
    stream << "  result->module = ReadRuleFile(path);\n";
    stream << "  result->DoIndex();\n";
    stream << "  return result;\n";
    stream << "}\n";
  }
};

struct RuleSetHack : public IndexComponent {
  struct CachedKnownFlags {
    const char* field_name;
    const char* factory_name;
  };
  std::vector<CachedKnownFlags> flags_to_emit() {
    return {{"gtk_flags", "MakeGtkFlags"},
            {"dl_flags", "MakeDLFlags"}};
  }
  virtual void EmitPublicDecls(std::ostream& stream) override {
    stream << "  LibraryBuildResult* default_flags = MakeDefaultFlags();\n";
    for (auto& fl : flags_to_emit()) {
      stream << "  LibraryBuildResult* " << fl.field_name << "();\n";
    }
  }
  virtual void EmitPrivateDecls(std::ostream& stream) override {
    for (auto& fl : flags_to_emit()) {
      stream << "  LibraryBuildResult* _cache_" << fl.field_name << " = nullptr;\n";
    }
  }
  virtual void EmitImpls(std::ostream& stream) override {
    for (auto& fl : flags_to_emit()) {
      stream << "  LibraryBuildResult* " << parent->name << "::" << fl.field_name << "() {\n";
      stream << "    if (_cache_" << fl.field_name << ") {\n";
      stream << "      return _cache_" << fl.field_name << ";\n";
      stream << "    }\n";
      stream << "    return _cache_" << fl.field_name << " = " << fl.factory_name << "(this);\n";
      stream << "  }\n";
    }
  }
};

struct RuleFileHack : public IndexComponent {
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  rule_spec::Module* module;\n";
    stream << "  RuleModuleContext* parent;\n";
    stream << "  std::string filename;\n";
    stream << "  void DoIndex();\n";
//  }
//  virtual void EmitPrivateDecls(std::ostream& stream) {
    stream << "  std::unordered_map<std::string, rule_spec::LinkDecl*> links;\n";
    stream << "  std::unordered_map<std::string, rule_spec::Decl*> libs;\n";
  }
  virtual void EmitImpls(std::ostream& stream) {
    stream << "void " << parent->name << "::DoIndex() {\n";
    stream << "  for (auto* decl_ : module->decls) {\n";
    stream << "    switch (decl_->getKind()) {\n";
    for (string_view decl_type : {"Import", "OldParser", "OldLoweringSpec", "Library", "Passes", "PassesTemplate"}) {
    stream << "    case rule_spec::Decl::Kind::" << decl_type << ": {\n";
    stream << "      auto* decl = reinterpret_cast<rule_spec::" << decl_type << "Decl*>(decl_);\n";
    stream << "      auto key = std::string(decl->name.str);\n";
    stream << "      if (libs.find(key) != libs.end()) {\n";
    stream << "        fprintf(stderr, \"Duplicate rule: %s\\n\", key.c_str());\n";
    stream << "        exit(EXIT_FAILURE);\n";
    stream << "      }\n";
    stream << "      libs[key] = decl;\n";
    stream << "      break;\n";
    stream << "    }\n";
    }
    stream << "    case rule_spec::Decl::Kind::Link: {\n";
    stream << "      auto* decl = reinterpret_cast<rule_spec::LinkDecl*>(decl_);\n";
    stream << "      auto key = Unescaped(decl->fname.str);\n";
    stream << "      if (links.find(key) != links.end()) {\n";
    stream << "        fprintf(stderr, \"Duplicate link rule: %s\\n\", key.c_str());\n";
    stream << "        exit(EXIT_FAILURE);\n";
    stream << "      }\n";
    stream << "      links[key] = decl;\n";
    stream << "      break;\n";
    stream << "    }\n";
    stream << "    }\n";
    stream << "  }\n";
    stream << "}\n";
  }
};

struct RuleFileLinkOrTrigger : public IndexComponent {
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  void LinkOrTrigger(string_view rule_name);\n";
  }
  virtual void EmitImpls(std::ostream& stream) {
    stream << "void " << parent->name << "::LinkOrTrigger(string_view rule_name) {\n";
    stream << "  if (links.find(std::string(rule_name)) != links.end()) {\n";
    stream << "    return Link(rule_name);\n";
    stream << "  }\n";
    stream << "  GetAndRunRule(rule_name);\n";
    stream << "}\n";
  }
};

void EmitPassesToFilename(const char* cc_fname,
                          const char* h_fname) {
  auto* module = new Module;
  module->ns = "rules";

  auto* root_def = new ContextDef;
  module->decls.push_back(root_def);
  module->decls.push_back(new TopLevelDeclHack);
  root_def->name = "RuleModuleContext";
  {
    auto* idx = new MemoizedFunction;
    idx->result_type = "RuleFile";
    idx->name = "GetFile";
    idx->fn_name = "LoadRuleFile";
    root_def->add_decls(idx);
  }
  {
    auto* idx = new RuleSetHack;
    root_def->add_decls(idx);
  }
  auto* def = new ContextDef;
  module->decls.push_back(def);
  def->name = "RuleFile";
  def->context = root_def;
  def->add_decls(new RuleFileHack);
  def->add_decls(new RuleFileLinkOrTrigger);
  {
    auto* idx = new MemoizedCycleDetector;
    idx->result_type = "LibraryBuildResult";
    idx->name = "GetAndRunRule";
    idx->fn_name = "DoGetAndRunRule";
    def->add_decls(idx);
  }
  {
    auto* idx = new MemoizedCycleDetector;
    idx->result_type = "void";
    idx->name = "Link";
    idx->fn_name = "DoLink";
    def->add_decls(idx);
  }
  {
    auto* idx = new PathLookup;
    idx->result_type = "rule_spec::LinkDecl*";
    idx->name = "GetLinkDecl";
    idx->set_name = "links";
    def->add_decls(idx);
  }
  {
    auto* idx = new PathLookup;
    idx->result_type = "rule_spec::Decl*";
    idx->name = "GetRuleDecl";
    idx->set_name = "libs";
    def->add_decls(idx);
  }

  EmitStream cc;
  EmitStream h;
  EmitModule(module, cc.stream(), h.stream());
  h.write(h_fname);
  cc.write(cc_fname);
}

}  // namespace passes

#ifndef TURN_OFF
#include "gen/rules/emit-passes-meta.cc"
#endif
