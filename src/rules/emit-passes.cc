#include "rules/emit-passes.h"
#include "parser/parser-support.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace passes {

struct ContextDef;

struct IndexComponent {
  ContextDef* context;
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
  ContextDef* parent;

  virtual void EmitStructFwdDeclare(std::ostream& stream) {
    stream << "class " << name << ";\n";
  }
  virtual void EmitFwdDeclare(std::ostream& stream) {
    stream << "\n";
    stream << "class " << name << " {\n";
    stream << " public:\n";
    for (auto* item : items) {
      item->EmitPublicDecls(stream);
    }
    stream << " private:\n";
    for (auto* item : items) {
      item->EmitPrivateDecls(stream);
    }
    stream << "};\n";
  }
  virtual void EmitDefinitions(std::ostream& stream) {
    for (auto* item : items) {
      stream << "\n";
      item->EmitImpls(stream);
    }
  }
  void AddDecl(IndexComponent* decl) {
    items.push_back(decl);
    decl->context = this;
  }
 private:
  std::vector<IndexComponent*> items;
};

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
    stream << result_type << "* " << context->name << "::"
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
        << context->name << "::" << name << "(string_view key) {\n";
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
    stream << result_type << " " << context->name << "::" << name << "(string_view path) {\n";
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
  virtual void EmitPublicDecls(std::ostream& stream) {
    stream << "  LibraryBuildResult* default_flags = MakeDefaultFlags();\n";
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
    stream << "void " << context->name << "::DoIndex() {\n";
    stream << "  for (auto* decl_ : module->decls) {\n";
    stream << "    switch (decl_->getKind()) {\n";
    for (string_view decl_type : {"Import", "OldParser", "OldLoweringSpec", "Library", "Passes"}) {
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
    stream << "void " << context->name << "::LinkOrTrigger(string_view rule_name) {\n";
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
    root_def->AddDecl(idx);
  }
  {
    auto* idx = new RuleSetHack;
    root_def->AddDecl(idx);
  }
  auto* def = new ContextDef;
  module->decls.push_back(def);
  def->name = "RuleFile";
  def->parent = root_def;
  def->AddDecl(new RuleFileHack);
  def->AddDecl(new RuleFileLinkOrTrigger);
  {
    auto* idx = new MemoizedCycleDetector;
    idx->result_type = "LibraryBuildResult";
    idx->name = "GetAndRunRule";
    idx->fn_name = "DoGetAndRunRule";
    def->AddDecl(idx);
  }
  {
    auto* idx = new MemoizedCycleDetector;
    idx->result_type = "void";
    idx->name = "Link";
    idx->fn_name = "DoLink";
    def->AddDecl(idx);
  }
  {
    auto* idx = new PathLookup;
    idx->result_type = "rule_spec::LinkDecl*";
    idx->name = "GetLinkDecl";
    idx->set_name = "links";
    def->AddDecl(idx);
  }
  {
    auto* idx = new PathLookup;
    idx->result_type = "rule_spec::Decl*";
    idx->name = "GetRuleDecl";
    idx->set_name = "libs";
    def->AddDecl(idx);
  }

  std::string cc_output;
  std::stringstream cc_stream(cc_output);
  std::string h_output;
  std::stringstream h_stream(h_output);

  EmitModule(module, cc_stream, h_stream);
  {
    cc_stream.flush();
    std::ofstream ofs(cc_fname, std::ofstream::out | std::ofstream::trunc);
    ofs << cc_stream.str();
    ofs.close();
  }
  {
    h_stream.flush();
    std::ofstream ofs(h_fname, std::ofstream::out | std::ofstream::trunc);
    ofs << h_stream.str();
    ofs.close();
  }
}

}  // namespace passes
