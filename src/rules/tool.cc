#include "rules/process.h"
#include <stdio.h>
#include <string.h>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <assert.h>
#include <algorithm>
#include "parser/tokenizer_helper.cc"

std::string Unescaped(string_view data) {
  std::string out;
  // TODO: This is bad (unsafe)
  data.remove_prefix(1);
  data.remove_suffix(1);
  size_t pos = data.find('\\');
  while (pos != string_view::npos) {
    out.append(data.data(), pos);
    data.remove_prefix(pos);
    data.remove_prefix(1);
    if (data[0] == 'n') {
      out.append(1, '\n');
      data.remove_prefix(1);
    } else if (data[0] == '\\') {
      out.append(1, '\\');
      data.remove_prefix(1);
    }
    pos = data.find('\\');
  }
  out.append(data.data(), data.size());
  return out;
}

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
                                         string_view folder, string_view cxx_name) {
  std::vector<const char*> eval = {"/usr/bin/ccache", "/usr/bin/clang-6.0", "-Wall", "-std=c++17"};
  CollectCXXFlags(postorder(deps), &eval);
  std::string base = std::string(RemoveExt(cxx_name));
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
  auto* res = new LibraryBuildResult;
  res->object_files = {object_filename};
  res->deps = deps;
  return res;
}

LibraryBuildResult *SimpleOldParserGen(const std::vector<LibraryBuildResult*>& deps,
                                       const char* parser, const char* tokenizer,
                                       const char* outname) {
  std::vector<const char*> eval = {".build/parser", parser, tokenizer};
  RunWithPipe(std::move(eval), outname);
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

#include "gen/rules/rule-spec.cc"

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

class RuleSet;

class RuleFile {
 public:
  RuleSet* parent;
  std::string filename;
  rule_spec::Module* module;

  struct LibCache {
    rule_spec::Decl* decl;
    // Unused for imports...
    LibraryBuildResult* result = nullptr;
    // For cycle detection.
    bool started = false;
  };

  struct LinkCache {
    rule_spec::LinkDecl* decl;
    bool finished = false;
    bool started = false;
  };

  void IndexDecl(rule_spec::Decl* decl_) {
    using namespace rule_spec;
    switch (decl_->getKind()) {
    case Decl::Kind::Import: {
      auto* decl = reinterpret_cast<ImportDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", std::string(decl->name.str).c_str());
        exit(EXIT_FAILURE);
      }
      libs[key].decl = decl;
      break;
    } case Decl::Kind::OldParser: {
      auto* decl = reinterpret_cast<OldParserDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", std::string(decl->name.str).c_str());
        exit(EXIT_FAILURE);
      }
      libs[key].decl = decl;
      break;
    } case Decl::Kind::Library: {
      auto* decl = reinterpret_cast<LibraryDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", std::string(decl->name.str).c_str());
        exit(EXIT_FAILURE);
      }
      libs[key].decl = decl;
      break;
    } case Decl::Kind::Link: {
      auto* decl = reinterpret_cast<LinkDecl*>(decl_);
      auto key = Unescaped(decl->fname.str);
      if (links.find(key) != links.end()) {
        fprintf(stderr, "Duplicate link rule: %s\n", std::string(decl->fname.str).c_str());
        exit(EXIT_FAILURE);
      }
      links[key].decl = decl;
      break;
    }
    }
  }

  std::unordered_map<std::string, LibCache> libs;
  std::unordered_map<std::string, LinkCache> links;

  LibraryBuildResult* GetAndRunRule(string_view rule_name);

  LibraryBuildResult* ProcessLibraryBuildResult(rule_spec::Expr* expr);
  std::vector<LibraryBuildResult*> ProcessLibraryBuildResultList(rule_spec::Option* option);
  void Link(string_view rule_name);
};

class RuleSet {
 public:
  std::unordered_map<std::string, RuleFile> rule_files;

  RuleFile* GetFile(string_view path) {
    auto it = rule_files.find(std::string(path));
    if (it != rule_files.end()) return &it->second;
    rule_spec::Tokenizer tokens((new std::string{LoadFile(std::string(path) + "/BUILD")})->c_str());
    auto* result = &rule_files[std::string(path)];
    result->module = rule_spec::parser::DoParse(tokens);
    result->parent = this;
    result->filename = std::string(path);
    for (auto* decl : result->module->decls) {
      result->IndexDecl(decl);
    }
    return result;
  }
  LibraryBuildResult* default_flags = MakeDefaultFlags();
};

LibraryBuildResult* RuleFile::GetAndRunRule(string_view rule_name) {
  using namespace rule_spec;
  auto it = libs.find(std::string(rule_name));
  if (it == libs.end()) {
    std::cerr << "No such library: \"" << rule_name << "\"\n";
    exit(EXIT_FAILURE);
  }
  if (it->second.result) return it->second.result;
  if (it->second.started) {
    std::cerr << "Cycle detected while building: \"" << rule_name << "\"\n";
    exit(EXIT_FAILURE);
  }
  it->second.started = true;

  auto* decl_ = it->second.decl;
  switch (decl_->getKind()) {
  case Decl::Kind::OldParser: {
    auto* decl = reinterpret_cast<OldParserDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"parser", "tokens", "cc_out"});

    auto parser = GetStringOption(options["parser"]);
    auto tokenizer = GetStringOption(options["tokens"]);
    auto cc_out = GetStringOption(options["cc_out"]);

    std::string filename_gen;
    if (string_view(filename).substr(0, 4) == "src/") {
      filename_gen = "gen/" + filename.substr(4);
    } else {
      filename_gen = filename;
    }

    SimpleOldParserGen({parent->default_flags}, strdup(filename + "/" + parser),
                       strdup(filename + "/" + tokenizer),
                       strdup(".generated/" + filename_gen + "/" + cc_out));

    it->second.result = new LibraryBuildResult; 
    break;
  } case Decl::Kind::Library: {
    auto* decl = reinterpret_cast<LibraryDecl*>(decl_);
    auto options = IndexOptionSet(filename, rule_name, decl->options, {"deps", "srcs", "hdrs"});
    auto srcs = StringArgList(options["srcs"]);
    StringArgList(options["hdrs"]);
    if (srcs.size() != 1) {
      std::cerr << "Must have 1 and only 1 srcs: \"" << rule_name << "\"\n";
      exit(EXIT_FAILURE);
    }
    assert(srcs.size() == 1 && "Can only currently handle one src per library.");
    auto deps = ProcessLibraryBuildResultList(options["deps"]);
    deps.push_back(parent->default_flags);
    it->second.result = SimpleCompileCXXFile(deps, filename, srcs[0]); 
    break;
  }
  default:
  std::cerr << "Not a normal rule!: \"" << rule_name << "\"\n";
  exit(EXIT_FAILURE);
  }

  if (it->second.result) return it->second.result;

  std::cerr << "Problem while building: \"" << rule_name << "\"\n";
  exit(EXIT_FAILURE);
}

void RuleFile::Link(string_view rule_name) {
  auto it = links.find(std::string(rule_name));
  if (it == links.end()) {
    std::cerr << "No such link rule: \"" << rule_name << "\"\n";
    exit(EXIT_FAILURE);
  }
  if (it->second.finished) return;
  if (it->second.started) {
    std::cerr << "Cycle while linking: \"" << rule_name << "\"\n";
    exit(EXIT_FAILURE);
  }
  it->second.started = true;
  LinkCommand out;
  out.output_name = strdup(".build/" + std::string(rule_name));
  auto options = IndexOptionSet(filename, rule_name, it->second.decl->options, {"deps"});
  out.deps = ProcessLibraryBuildResultList(options["deps"]);
  BuildLinkCommand(&out);
  it->second.finished = true;
}

LibraryBuildResult* RuleFile::ProcessLibraryBuildResult(rule_spec::Expr* expr) {
  using namespace rule_spec;
  switch (expr->getKind()) {
  case Expr::Kind::Dot: {
    auto* base = reinterpret_cast<DotExpr*>(expr)->base;
    assert(base->getKind() == Expr::Kind::Name && "Base of dot must be a name...");

    auto key = reinterpret_cast<NameExpr*>(base)->name.str;
    auto it = libs.find(std::string(key));
    if (libs.end() == it || it->second.decl->getKind() != Decl::Kind::Import) {
      std::cerr << "No such import " << key << "\n";
      exit(EXIT_FAILURE);
    }
    auto* new_file = parent->GetFile(Unescaped(reinterpret_cast<ImportDecl*>(it->second.decl)->path.str));
    return new_file->GetAndRunRule(reinterpret_cast<DotExpr*>(expr)->name.str);
  } case Expr::Kind::Name:
    return GetAndRunRule(reinterpret_cast<NameExpr*>(expr)->name.str);
  default:
    std::cerr << "Unexpected in ProcessLibraryBuildResult\n";
    exit(EXIT_FAILURE);
  }
}

std::vector<LibraryBuildResult*> RuleFile::ProcessLibraryBuildResultList(rule_spec::Option* option) {
  using namespace rule_spec;
  std::vector<LibraryBuildResult*> out;
  if (!option) return out;
  assert(option->value->getKind() == Expr::Kind::ArrayLiteral && "Must be array literal...");
  auto* value = reinterpret_cast<ArrayLiteralExpr*>(option->value);
  for (auto* child : value->values) {
    out.push_back(ProcessLibraryBuildResult(child));
  }
  return out;
}


int main(int argc, char **argv) {
  using namespace rule_spec;
  
  if (argc <= 2) {
    fprintf(stderr, "Not enough arguments: %d.\n", argc);
    exit(-1);
  }

  RuleSet rule_set;
  Run({"/bin/mkdir", "-p", ".build/objects/src/rules/"});
  Run({"/bin/mkdir", "-p", ".generated/gen/rules/"});
  rule_set.GetFile(argv[1])->Link(argv[2]);
  // TODO: Remove this at some point
  if (argv[2] == string_view("rules-dynamic")) {
    Run({"/bin/mv", ".build/rules-dynamic", ".build/rules"});
  }

  printf("\e[32mSuccess!\e[m\n");
}
