#include "rules/process.h"
#include <stdio.h>
#include <string.h>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <sstream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include "gen/rules/rule-spec.h"
#include "rules/string-utils.h"
#include "parser/lowering_spec_lowering.h"
#include "parser/parser_lowering.h"
#include "rules/emit-passes.h"
#include "rules/emit-passes-template.h"

struct MkdirCache {
  std::unordered_set<std::string> mkdirs;
  void Ensure(const char* fname) {
    auto key = std::string(fname);
    if (mkdirs.find(key) == mkdirs.end()) {
      mkdirs.insert(key);
      Run({"/bin/mkdir", "-p", fname});
    }
  }
};
static MkdirCache mkdir;

const char* strdup(std::string str) {
  return strdup(str.c_str());
}

const char* strdup(string_view str) {
  return strdup(std::string(str).c_str());
}

string_view RemoveExt(string_view filename) {
  return filename.substr(0, filename.find_last_of("."));
}

struct LibraryBuildResult {
  std::vector<const char*> object_files;
  std::vector<const char*> exposed_headers;
  std::vector<const char*> link_flags;
  std::vector<const char*> cxx_flags;
  std::vector<LibraryBuildResult*> deps;
};

struct LinkCommand {
  const char* output_name;
  std::vector<LibraryBuildResult*> deps;
};

struct LibraryCommand {
  std::vector<const char*> object_files;
  std::vector<const char*> exposed_headers;
  std::vector<const char*> link_flags;
  std::vector<const char*> cxx_flags;
  std::vector<LibraryBuildResult*> deps;
};

struct VisitorState {
  std::unordered_set<LibraryBuildResult*> visited;
  struct WorkItem {
    LibraryBuildResult* result;
    size_t idx;
  };
  std::vector<LibraryBuildResult*> postorder;

  void add(LibraryBuildResult* item) {
    if (visited.insert(item).second) {
      add(item->deps);
      postorder.push_back(item);
    }
  }
  void add(const std::vector<LibraryBuildResult*>& deps) {
    for (auto* dep : deps) add(dep);
  }
};

std::vector<LibraryBuildResult*> postorder(const std::vector<LibraryBuildResult*>& deps) {
  VisitorState result;
  result.add(deps);
  return std::move(result.postorder);
}

std::vector<LibraryBuildResult*> toposort(const std::vector<LibraryBuildResult*>& deps) {
  VisitorState result;
  result.add(deps);
  std::reverse(result.postorder.begin(), result.postorder.end());
  return std::move(result.postorder);
}

void CollectObjects(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out) {
  for (auto* dep : deps) {
    for (auto* object : dep->object_files) {
      out->push_back(object);
    }
  }
}

void CollectLinkFlags(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out) {
  for (auto* dep : deps) {
    for (auto* link_flag : dep->link_flags) {
      out->push_back(link_flag);
    }
  }
}

void CollectCXXFlags(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out) {
  for (auto* dep : deps) {
    for (auto* cxx_flag : dep->cxx_flags) {
      out->push_back(cxx_flag);
    }
  }
}

void BuildLinkCommand(LinkCommand* cmd) {
  std::vector<const char*> eval = {"/usr/bin/clang-6.0", "-Wall"};
  auto toposort_deps = toposort(cmd->deps);
  CollectObjects(toposort_deps, &eval);
  CollectLinkFlags(toposort_deps, &eval);
  eval.push_back("-o");
  eval.push_back(cmd->output_name);
  Run(eval);
}

LibraryBuildResult *SimpleCompileCXXFile(const std::vector<LibraryBuildResult*>& deps,
                                         string_view folder, std::vector<string_view> srcs) {
  auto* res = new LibraryBuildResult;
  for (string_view cxx_name : srcs) {
    std::vector<const char*> eval = {"/usr/bin/ccache", "/usr/bin/clang-6.0", "-Wall", "-std=c++17"};
    CollectCXXFlags(postorder(deps), &eval);
    std::string base = std::string(RemoveExt(cxx_name));
    mkdir.Ensure(strdup((".build/objects/" + std::string(folder))));
    const char* object_filename = strdup((".build/objects/" + std::string(folder) + "/"
                                          + base + ".o"));
    eval.push_back("-MF");
    eval.push_back(strdup((".build/objects/" + std::string(folder) + "/"
                           + base + ".d")));
    eval.push_back("-MD");
    eval.push_back("-c");
    eval.push_back(strdup(std::string(folder) + "/" + std::string(cxx_name)));
    eval.push_back("-o");
    eval.push_back(object_filename);
    Run(std::move(eval));
    res->object_files.push_back(object_filename);
  }
  res->deps = deps;
  return res;
}

LibraryBuildResult *SimpleOldParserGen(const std::vector<LibraryBuildResult*>& deps,
                                       const char* parser, const char* tokenizer,
                                       const char* outname, const char* h_outname,
                                       string_view folder, string_view cxx_name) {
  auto contents = LoadFile(parser);
  auto contents_tok = LoadFile(tokenizer);

  production_spec::Tokenizer tokens(contents.c_str());
  auto* m = production_spec::parser::DoParse(tokens);

  parser_spec::Tokenizer tokens_tok(contents_tok.c_str());
  auto* m2 = parser_spec::parser::DoParse(tokens_tok);

  auto* ctx = parser::DoAnalysis(m, m2);
  
  // Actual Emit...
  {
    std::string output;
    std::stringstream stream(output);
    parser::EmitParser(stream, m, m2, ctx, /* is_header= */ false);

    std::ofstream ofs(outname, std::ofstream::out | std::ofstream::trunc);
    ofs << stream.str();
    ofs.close();
  }
  
  if (h_outname) {
    std::string output;
    std::stringstream stream(output);
    parser::EmitParser(stream, m, m2, ctx, /* is_header= */ true);

    std::ofstream ofs(h_outname, std::ofstream::out | std::ofstream::trunc);
    ofs << stream.str();
    ofs.close();
    return SimpleCompileCXXFile(deps, folder, {cxx_name}); 
  }

  auto* res = new LibraryBuildResult;
  res->deps = deps;
  return res;
}

LibraryBuildResult *SimpleOldLoweringSpecGen(const std::vector<LibraryBuildResult*>& deps,
                                             const char* lowering_spec, const char* outname) {
  auto contents = LoadFile(lowering_spec);
  lowering_spec::Tokenizer tokens(contents.c_str());
  auto* m = lowering_spec::parser::DoParse(tokens);

  std::string output;
  std::stringstream stream(output);
  lowering_spec::Emit(stream, m);
  stream.flush();

  std::ofstream ofs(outname, std::ofstream::out | std::ofstream::trunc);
  ofs << stream.str();
  ofs.close();

  auto* res = new LibraryBuildResult;
  res->deps = deps;
  return res;
}


LibraryBuildResult *MakeDefaultFlags() {
  auto* res = new LibraryBuildResult;
  res->link_flags = {"-lstdc++"};
  res->cxx_flags = {"-I", ".generated/", "-I", "src", "-I", ".build/"};
  return res;
}

std::unordered_map<string_view, rule_spec::Option*> IndexOptionSet(
    string_view filename,
    string_view rule_name,
    const std::vector<rule_spec::Option*>& options,
    const std::vector<string_view>& known_keys) {
  std::unordered_map<string_view, rule_spec::Option*> result;
  for (auto key : known_keys) { result[key]; }
  for (auto* option : options) {
    auto it = result.find(option->key.str);
    if (it == result.end()) {
      std::cerr << filename << ":" << rule_name << ": No such option " << option->key.str << "\n";
      exit(EXIT_FAILURE);
    }
    if (it->second) {
      std::cerr << filename << ":" << rule_name << ": Duplicate option: " << option->key.str << "\n";
      exit(EXIT_FAILURE);
    }
    it->second = option;
  }
  return result;
}

std::vector<std::string> StringArgList(rule_spec::Option* option) {
  using namespace rule_spec;
  std::vector<std::string> out;
  if (!option) return out;
  assert(option->value->getKind() == Expr::Kind::ArrayLiteral && "Must be array literal...");
  auto* value = reinterpret_cast<ArrayLiteralExpr*>(option->value);
  for (auto* child : value->values) {
    assert(child->getKind() == Expr::Kind::StringLiteral && "Must be a string literal...");
    out.push_back(Unescaped(reinterpret_cast<StringLiteralExpr*>(child)->value.str));
  }
  return out;
}

std::string GetStringOption(rule_spec::Option* option) {
  using namespace rule_spec;
  assert(option && option->value->getKind() == Expr::Kind::StringLiteral && "Must be a string literal...");
  return Unescaped(reinterpret_cast<StringLiteralExpr*>(option->value)->value.str);
}

#include "gen/rules/rules-passes.h"
namespace rules {

LibraryBuildResult *MakeGtkFlags(RuleModuleContext* ctx) {
  auto* res = new LibraryBuildResult;
  res->link_flags = {"-lgtk-3", "-lgdk-3", "-lpangocairo-1.0", "-lpango-1.0",
    "-latk-1.0", "-lcairo-gobject", "-lcairo",
      "-lfontconfig",
      "-lfreetype",
    "-lgdk_pixbuf-2.0", "-lgio-2.0", "-lgobject-2.0", "-lglib-2.0"};
  res->cxx_flags = {"-pthread", "-I/usr/include/gtk-3.0", "-I/usr/include/at-spi2-atk/2.0",
    "-I/usr/include/at-spi-2.0", "-I/usr/include/dbus-1.0",
    "-I/usr/lib/x86_64-linux-gnu/dbus-1.0/include", "-I/usr/include/gtk-3.0",
    "-I/usr/include/gio-unix-2.0/", "-I/usr/include/cairo", "-I/usr/include/pango-1.0",
    "-I/usr/include/harfbuzz", "-I/usr/include/pango-1.0", "-I/usr/include/atk-1.0",
    "-I/usr/include/cairo", "-I/usr/include/pixman-1", "-I/usr/include/freetype2",
    "-I/usr/include/libpng16", "-I/usr/include/gdk-pixbuf-2.0", "-I/usr/include/libpng16",
    "-I/usr/include/glib-2.0", "-I/usr/lib/x86_64-linux-gnu/glib-2.0/include"};
  return res;
}

LibraryBuildResult *MakeDLFlags(RuleModuleContext* ctx) {
  auto* res = new LibraryBuildResult;
  res->link_flags = {"-ldl"};
  return res;
}

LibraryBuildResult* ProcessLibraryBuildResult(RuleFile* context, rule_spec::Expr* expr) {
  using namespace rule_spec;
  switch (expr->getKind()) {
  case Expr::Kind::Dot: {
    auto* base = reinterpret_cast<DotExpr*>(expr)->base;
    assert(base->getKind() == Expr::Kind::Name && "Base of dot must be a name...");

    auto key = reinterpret_cast<NameExpr*>(base)->name.str;
    auto it = context->libs.find(std::string(key));
    if (context->libs.end() == it || it->second->getKind() != Decl::Kind::Import) {
      std::cerr << "No such import " << key << "\n";
      exit(EXIT_FAILURE);
    }
    auto* new_file = context->parent->GetFile(Unescaped(reinterpret_cast<ImportDecl*>(it->second)->path.str));
    return new_file->GetAndRunRule(reinterpret_cast<DotExpr*>(expr)->name.str);
  } case Expr::Kind::Name: {
    auto name = reinterpret_cast<NameExpr*>(expr)->name.str;
    if (name == "gtk") {
      return context->parent->gtk_flags();
    }
    if (name == "dl") {
      return context->parent->dl_flags();
    }
    return context->GetAndRunRule(name);
  } default:
    std::cerr << "Unexpected in ProcessLibraryBuildResult\n";
    exit(EXIT_FAILURE);
  }
}

std::vector<LibraryBuildResult*> ProcessLibraryBuildResultList(RuleFile* context, rule_spec::Option* option) {
  using namespace rule_spec;
  std::vector<LibraryBuildResult*> out;
  if (!option) return out;
  assert(option->value->getKind() == Expr::Kind::ArrayLiteral && "Must be array literal...");
  auto* value = reinterpret_cast<ArrayLiteralExpr*>(option->value);
  for (auto* child : value->values) {
    out.push_back(ProcessLibraryBuildResult(context, child));
  }
  return out;
}

rule_spec::Module* ReadRuleFile(string_view path) {
  rule_spec::Tokenizer tokens((new std::string{LoadFile(std::string(path) + "/BUILD")})->c_str());
  return rule_spec::parser::DoParse(tokens);
}

LibraryBuildResult* DoGetAndRunRule(RuleFile* context, string_view rule_name) {
  using namespace rule_spec;
  std::string filename_gen;
  auto& filename = context->filename;
  auto* parent = context->parent;
  if (string_view(filename).substr(0, 4) == "src/") {
    filename_gen = "gen/" + filename.substr(4);
  } else {
    filename_gen = filename;
  }

  auto* decl_ = context->GetRuleDecl(rule_name);
  switch (decl_->getKind()) {
  case Decl::Kind::OldParser: {
    auto* decl = reinterpret_cast<OldParserDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"parser", "tokens", "cc_out", "h_out"});

    auto parser = GetStringOption(options["parser"]);
    auto tokenizer = GetStringOption(options["tokens"]);
    auto cc_out = GetStringOption(options["cc_out"]);
    std::string h_out = GetStringOption(options["h_out"]);

    mkdir.Ensure(strdup(".generated/" + filename_gen));
    auto* res = SimpleOldParserGen({parent->default_flags}, strdup(filename + "/" + parser),
                                   strdup(filename + "/" + tokenizer),
                                   strdup(".generated/" + filename_gen + "/" + cc_out),
                                   strdup(".generated/" + filename_gen + "/" + h_out),
                                   ".generated/" + filename_gen, cc_out);

    return res;
  } case Decl::Kind::OldLoweringSpec: {
    auto* decl = reinterpret_cast<OldLoweringSpecDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"src", "cc_out"});

    auto src = GetStringOption(options["src"]);
    auto cc_out = GetStringOption(options["cc_out"]);

    mkdir.Ensure(strdup(".generated/" + filename_gen));
    SimpleOldLoweringSpecGen({parent->default_flags}, strdup(filename + "/" + src),
                             strdup(".generated/" + filename_gen + "/" + cc_out));

    return new LibraryBuildResult; 
  } case Decl::Kind::Library: {
    auto* decl = reinterpret_cast<LibraryDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"deps", "srcs", "hdrs"});
    auto srcs = StringArgList(options["srcs"]);
    StringArgList(options["hdrs"]);
    std::vector<string_view> srcs_copy;
    for (auto& src : srcs) {
      srcs_copy.push_back(src);
    }
    auto deps = ProcessLibraryBuildResultList(context, options["deps"]);
    deps.push_back(parent->default_flags);
    return SimpleCompileCXXFile(deps, filename, srcs_copy); 
  } case Decl::Kind::PassesTemplate: {
    auto* decl = reinterpret_cast<PassesDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"cc_out", "h_out", "src"});

    auto cc_out = GetStringOption(options["cc_out"]);
    std::string h_out = GetStringOption(options["h_out"]);
    auto src = GetStringOption(options["src"]);


    mkdir.Ensure(strdup(".generated/" + filename_gen));
    passes_template::EmitPassesTemplateToFilename(
        strdup(filename + "/" + src),
        strdup(".generated/" + filename_gen + "/" + cc_out),
        strdup(".generated/" + filename_gen + "/" + h_out));

    return parent->default_flags;
  } case Decl::Kind::Passes: {
    auto* decl = reinterpret_cast<PassesDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"cc_out", "h_out"});

    auto cc_out = GetStringOption(options["cc_out"]);
    std::string h_out = GetStringOption(options["h_out"]);

    mkdir.Ensure(strdup(".generated/" + filename_gen));
    passes::EmitPassesToFilename(
        strdup(".generated/" + filename_gen + "/" + cc_out),
        strdup(".generated/" + filename_gen + "/" + h_out));

    return parent->default_flags;
  }
  default:
    std::cerr << "Not a normal rule!: \"" << rule_name << "\"\n";
    exit(EXIT_FAILURE);
  }
}

void DoLink(RuleFile* context, string_view rule_name) {
  LinkCommand out;
  out.output_name = strdup(".build/" + std::string(rule_name));
  auto options = IndexOptionSet(context->filename, rule_name,
                                context->GetLinkDecl(rule_name)->options, {"deps"});
  out.deps = ProcessLibraryBuildResultList(context, options["deps"]);
  BuildLinkCommand(&out);
}

}  // namespace rules
#include "gen/rules/rules-passes.cc"

int main(int argc, char **argv) {
  using namespace rule_spec;
  
  if (argc <= 2) {
    fprintf(stderr, "Not enough arguments: %d.\n", argc);
    exit(-1);
  }

  rules::RuleModuleContext rule_set;
  // TODO: This is getting out of hand...
  rule_set.GetFile(argv[1])->LinkOrTrigger(argv[2]);
  // TODO: Remove this at some point (These are for linker errors).
  if (argv[2] == string_view("rules-dynamic")) {
    Run({"/bin/mv", ".build/rules-dynamic", ".build/rules"});
  }

  fprintf(stderr, "\e[32mSuccess!\e[m\n");
}
