#include "rules/rule-invoker.h"

#include <assert.h>
#include <unordered_map>
#include "rules/string-utils.h"
#include "parser/parser-support.h"
#include "parser/lowering_spec_lowering.h"
#include "parser/parser_lowering.h"
#include "rules/template-support.h"

namespace {
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

std::string GetName(rule_spec::Decl* decl) {
#define DEFINE_NAME(tname, name_id, unwrap) \
   case rule_spec::Decl::Kind::tname: \
     return unwrap(reinterpret_cast<rule_spec::tname ## Decl*>(decl)->name_id.str);
  switch (decl->getKind()) {
    DEFINE_NAME(OldParser, name, std::string)
    DEFINE_NAME(OldLoweringSpec, name, std::string)
    DEFINE_NAME(Library, name, std::string)
    DEFINE_NAME(PassesTemplate, name, std::string)
    DEFINE_NAME(Passes, name, std::string)
    DEFINE_NAME(Import, name, std::string)
    DEFINE_NAME(Link, fname, Unescaped)
    DEFINE_NAME(SoLink, fname, Unescaped)
  }
}

rule_spec::Module* ReadRuleFile(string_view path) {
  rule_spec::Tokenizer tokens((new std::string{LoadFile(std::string(path) + "/BUILD")})->c_str());
  return rule_spec::parser::DoParse(tokens);
}

LibraryBuildResult *SimpleCompileCXXFile(const std::vector<LibraryBuildResult*>& deps,
                                         string_view folder, const std::vector<string_view>& srcs) {
  return SimpleCompileCXXFile(deps, folder, ".build/objects/" + std::string(folder), srcs);
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
    EmitStream stream;
    parser::EmitParser(stream.stream(), m, m2, ctx, /* is_header= */ false);
    stream.write(outname);
  }
  
  if (h_outname) {
    EmitStream stream;
    parser::EmitParser(stream.stream(), m, m2, ctx, /* is_header= */ true);
    stream.write(h_outname);
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

  EmitStream stream;
  lowering_spec::Emit(stream.stream(), m);
  stream.write(outname);

  auto* res = new LibraryBuildResult;
  res->deps = deps;
  return res;
}

}
namespace rules {

rule_spec::Decl* FileContext::GetDecl(string_view rule) {
  auto it = data.find(std::string(rule));
  if (it == data.end()) {
    fprintf(stderr, "Could not find rule: \"%s\"\n", std::string(rule).c_str());
    exit(-1);
  }
  return it->second;
}

LibraryBuildResult* FileContext::ProcessLibraryBuildResult(rule_spec::Expr* expr) {
  using namespace rule_spec;
  switch (expr->getKind()) {
  case Expr::Kind::Dot: {
    auto* base = reinterpret_cast<DotExpr*>(expr)->base;
    assert(base->getKind() == Expr::Kind::Name && "Base of dot must be a name...");
    auto key = reinterpret_cast<NameExpr*>(base)->name.str;
    return EvalRule<FileContext*>(key, GetDecl(key))->GetLibRule(reinterpret_cast<DotExpr*>(expr)->name.str);
  } case Expr::Kind::Name: {
    auto name = reinterpret_cast<NameExpr*>(expr)->name.str;
    if (name == "gtk") {
      return parent->gtk_flags;
    }
    if (name == "dl") {
      return parent->dl_flags;
    }
    return GetLibRule(name);
  } default:
    std::cerr << "Unexpected in ProcessLibraryBuildResult\n";
    exit(EXIT_FAILURE);
  }
}

FileContext* FileContext::Eval(string_view name, rule_spec::ImportDecl* decl) {
  return parent->GetFile(Unescaped(decl->path.str));
}

std::string FileContext::GetGeneratedFilename() {
  if (string_view(filename).substr(0, 4) == "src/") {
    return "gen/" + filename.substr(4);
  } else {
    return filename;
  }
}

LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::OldParserDecl* decl) {
  auto filename_gen = GetGeneratedFilename();
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"parser", "tokens", "cc_out", "h_out"});

  auto parser = GetStringOption(options["parser"]);
  auto tokenizer = GetStringOption(options["tokens"]);
  auto cc_out = GetStringOption(options["cc_out"]);
  std::string h_out = GetStringOption(options["h_out"]);

  DoMkdir(strdup(".generated/" + filename_gen));
  auto* res = SimpleOldParserGen({parent->default_flags}, strdup(filename + "/" + parser),
                                 strdup(filename + "/" + tokenizer),
                                 strdup(".generated/" + filename_gen + "/" + cc_out),
                                 strdup(".generated/" + filename_gen + "/" + h_out),
                                 ".generated/" + filename_gen, cc_out);

  return res;
}
LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::OldLoweringSpecDecl* decl) {
  auto filename_gen = GetGeneratedFilename();
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"src", "cc_out"});

  auto src = GetStringOption(options["src"]);
  auto cc_out = GetStringOption(options["cc_out"]);

  DoMkdir(strdup(".generated/" + filename_gen));
  SimpleOldLoweringSpecGen({parent->default_flags}, strdup(filename + "/" + src),
                           strdup(".generated/" + filename_gen + "/" + cc_out));

  return new LibraryBuildResult; 
}
LibraryBuildResult* FileContext::Eval(string_view name, rule_spec::LibraryDecl* decl) {
  auto options = IndexOptionSet(filename, name, decl->options, {"deps", "srcs", "hdrs"});
  auto srcs = StringArgList(options["srcs"]);
  StringArgList(options["hdrs"]);
  std::vector<string_view> srcs_copy;
  for (auto& src : srcs) {
    srcs_copy.push_back(src);
  }
  auto deps = ProcessLibraryBuildResultList(options["deps"]);
  deps.push_back(parent->default_flags);
  return SimpleCompileCXXFile(deps, filename, srcs_copy); 
}

unit FileContext::Eval(string_view rule_name, rule_spec::SoLinkDecl* decl) {
  LinkCommand out;
  out.output_name = strdup(".build/" + std::string(rule_name));
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"deps"});
  out.deps = ProcessLibraryBuildResultList(options["deps"]);
  out.deps.push_back(parent->so_flags);
  BuildLinkCommand(&out);
  return {};
}
unit FileContext::Eval(string_view rule_name, rule_spec::LinkDecl* decl) {
  LinkCommand out;
  out.output_name = strdup(".build/" + std::string(rule_name));
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"deps"});
  out.deps = ProcessLibraryBuildResultList(options["deps"]);
  BuildLinkCommand(&out);
  return {};
}

std::vector<LibraryBuildResult*> FileContext::ProcessLibraryBuildResultList(rule_spec::Option* option) {
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

void FileContext::Eval(string_view name, rule_spec::Decl* decl) {
  fprintf(stderr, "problem!\n");
  exit(-1);
}

FileContext* GlobalContext::GetFile(string_view base) {
  std::string key(base);
  auto it = cache.find(key);
  if (it != cache.end()) return &it->second;
  auto* result = &cache[key];
  result->parent = this;
  result->filename = key;
  for (auto* decl : ReadRuleFile(base)->decls) {
    result->data[GetName(decl)] = decl;
  }
  return result;
}

}  // namespace rules
