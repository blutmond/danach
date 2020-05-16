#include "rules/rule-invoker.h"

#include <assert.h>
#include <unordered_map>
#include "rules/string-utils.h"
#include "parser/parser-support.h"
#include "parser/lowering_spec_lowering.h"
#include "parser/parser_lowering.h"
#include "gui/widget-spec-lowering.h"
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

int64_t GetIntegerOption(rule_spec::Option* option) {
  using namespace rule_spec;
  assert(option && option->value->getKind() == Expr::Kind::IntegerLiteral && "Must be an integer literal...");
  return std::stoi(std::string(reinterpret_cast<IntegerLiteralExpr*>(option->value)->value.str));
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
    DEFINE_NAME(ImportBuffer, name, std::string)
    DEFINE_NAME(WidgetSpec, name, std::string)
    DEFINE_NAME(BufferParser, name, std::string)
    DEFINE_NAME(Let, name, std::string)
    DEFINE_NAME(BufferLoweringSpec, name, std::string)
    DEFINE_NAME(Link, fname, Unescaped)
    DEFINE_NAME(SoLink, fname, Unescaped)
  }
}

rule_spec::Module* ReadRuleFile(ASTContext& ast_context, string_view path) {
  rule_spec::Tokenizer tokens(ast_context, ast_context.RegisterFileContents({LoadFile(std::string(path) + "/BUILD")}));
  return rule_spec::parser::DoParse(tokens);
}

LibraryBuildResult *SimpleCompileCXXFile(ASTContext& ctx, const std::vector<LibraryBuildResult*>& deps,
                                         string_view folder, const std::vector<string_view>& srcs) {
  return SimpleCompileCXXFile(ctx, deps, folder, ".build/objects/" + std::string(folder), srcs);
}

void EmitParserGenRaw(const std::string& parser, const std::string& tokenizer,
                      std::ostream& h_stream,
                      std::ostream& cc_stream) {
  ASTContext ast_ctx;
  production_spec::Tokenizer tokens(ast_ctx, parser.c_str());
  auto* m = production_spec::parser::DoParse(tokens);

  parser_spec::Tokenizer tokens_tok(ast_ctx, tokenizer.c_str());
  auto* m2 = parser_spec::parser::DoParse(tokens_tok);

  auto* ctx = parser::DoAnalysis(ast_ctx, m, m2);
  
  // Actual Emit...
  parser::EmitParser(cc_stream, m, m2, ctx, /* is_header= */ false);
  parser::EmitParser(h_stream, m, m2, ctx, /* is_header= */ true);

  delete ctx;
}

void EmitParserGen(const char* parser, const char* tokenizer,
                   std::ostream& h_stream,
                   std::ostream& cc_stream) {
  EmitParserGenRaw(LoadFile(parser), LoadFile(tokenizer), h_stream, cc_stream);
}

void SimpleOldLoweringSpecGenRaw(const std::string& contents, const std::string& outname) {
  ASTContext ast_ctx;
  lowering_spec::Tokenizer tokens(ast_ctx, contents.c_str());
  auto* m = lowering_spec::parser::DoParse(tokens);

  EmitStream stream;
  lowering_spec::Emit(&ast_ctx, stream.stream(), m);
  stream.write(outname);
}

void SimpleOldLoweringSpecGen(const char* lowering_spec, const std::string& outname) {
  SimpleOldLoweringSpecGenRaw(LoadFile(lowering_spec), outname);
}

}
namespace rules {

// Only needed cause we can't pass code directly into the c++ compiler...
std::vector<std::string> FileContext::BufferContentsArgList(rule_spec::Option* option,
                                                            const std::string& gen_dir) {
  using namespace rule_spec;
  std::vector<std::string> out;
  if (!option) return out;
  assert(option->value->getKind() == Expr::Kind::ArrayLiteral && "Must be array literal...");
  auto* value = reinterpret_cast<ArrayLiteralExpr*>(option->value);
  for (auto* child : value->values) {
    assert(child->getKind() == Expr::Kind::FileEmit && "Must be a file emit literal...");
    auto* to_emit = reinterpret_cast<FileEmitExpr*>(child);
    out.push_back(Unescaped(to_emit->fname.str));

    auto get_chunk = [&](ChunkSrc* src) -> const std::string& {
      int id = stoi(std::string(src->id.str));
      assert(id >= 0 && id < mid_parent->buffer.size());
      assert(mid_parent && "Must have mid parent");
      return mid_parent->buffer.at(id).text;
    };
    EmitStream stream;

    for (Action* action_ : to_emit->actions) {
      switch (action_->getKind()) {
      case Action::Kind::PragmaOnce: {
        stream.stream() << "#pragma once\n\n";
        break;
      } case Action::Kind::Import: {
        auto* action = reinterpret_cast<ImportAction*>(action_);
        stream.stream() << "#include " << action->filename.str << "\n";
        break;
      } case Action::Kind::HdrChunk: {
        auto* action = reinterpret_cast<HdrChunkAction*>(action_);
        stream.stream() << get_chunk(action->id) << "\n";
        break;
      } case Action::Kind::FwdDeclareFunc: {
        auto* action = reinterpret_cast<FwdDeclareFuncAction*>(action_);
        stream.stream() << get_chunk(action->id) << ";\n";
        break;
      } case Action::Kind::DefineFunc: {
        auto* action = reinterpret_cast<DefineFuncAction*>(action_);
        stream.stream() << get_chunk(action->sig_id) << " {\n";
        stream.stream() << get_chunk(action->body_id) << "\n}\n";
        break;
      }
      }
    }

    stream.write(gen_dir + "/" + Unescaped(to_emit->fname.str));
  }
  return out;
}

rule_spec::Decl* FileContext::GetDecl(string_view rule) {
  auto it = data.find(std::string(rule));
  if (it == data.end()) {
    fprintf(stderr, "Could not find rule: \"%s\" in \"%s\"\n", std::string(rule).c_str(),
            filename.c_str());
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

FileContext* FileContext::Eval(string_view name, rule_spec::ImportBufferDecl* decl) {
  auto filename = Unescaped(decl->filename.str);
  
  auto it = parent->buffer_cache.find(filename);
  if (it == parent->buffer_cache.end()) {
    auto data = Collapse(ParseMultiBuffer(LoadFile(Unescaped(decl->filename.str))));

    // EmitFromMultiBuffer(data);

    auto& my_result = parent->buffer_cache[filename];
    my_result.buffer = std::move(data);
    my_result.parent = parent;
    it = parent->buffer_cache.find(filename);
  }

  return it->second.GetFile(std::stoi(std::string(decl->id.str)));
}

std::string FileContext::GetGeneratedFilename() {
  if (string_view(filename).substr(0, 4) == "src/") {
    return "gen/" + filename.substr(4);
  } else {
    return filename;
  }
}

const std::string &FileContext::GetBufferContents(rule_spec::Option* option) {
  int64_t v = GetIntegerOption(option);
  assert(mid_parent && "Must have mid parent");
  return mid_parent->buffer.at(v).text;
}

LibraryBuildResult* FileContext::Eval(string_view name, rule_spec::LetDecl* decl) {
  return ProcessLibraryBuildResult(decl->value);
}

LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::BufferParserDecl* decl) {
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"parser", "tokens", "cc_out", "h_out", "gen_dir"});
  std::string parser = GetBufferContents(options["parser"]);
  std::string tokenizer = GetBufferContents(options["tokens"]);
  std::string cc_out = GetStringOption(options["cc_out"]);
  std::string h_out = GetStringOption(options["h_out"]);
  std::string gen_dir = GetStringOption(options["gen_dir"]);

  DoMkdir(ast_ctx().strdup(gen_dir));
  EmitStream h_stream;
  EmitStream cc_stream;
  EmitParserGenRaw(parser, tokenizer, h_stream.stream(), cc_stream.stream());
  cc_stream.write(gen_dir + "/" + cc_out);
  h_stream.write(gen_dir + "/" + h_out);

  return SimpleCompileCXXFile(ast_ctx(), {parent->default_flags, parent->GetRule("src/parser", "tokenizer_helper")},
                              gen_dir, {cc_out}); 
}

LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::OldParserDecl* decl) {
  auto filename_gen = GetGeneratedFilename();
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"parser", "tokens", "cc_out", "h_out"});

  auto parser = GetStringOption(options["parser"]);
  auto tokenizer = GetStringOption(options["tokens"]);
  auto cc_out = GetStringOption(options["cc_out"]);
  std::string h_out = GetStringOption(options["h_out"]);

  DoMkdir(ast_ctx().strdup(".generated/" + filename_gen));
  EmitStream h_stream;
  EmitStream cc_stream;
  EmitParserGen(ast_ctx().strdup(filename + "/" + parser),
                ast_ctx().strdup(filename + "/" + tokenizer), h_stream.stream(), cc_stream.stream());
  cc_stream.write(".generated/" + filename_gen + "/" + cc_out);
  h_stream.write(".generated/" + filename_gen + "/" + h_out);
  return SimpleCompileCXXFile(ast_ctx(), {parent->default_flags, parent->GetRule("src/parser", "tokenizer_helper")},
                              ".generated/" + filename_gen, {cc_out}); 

}

LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::WidgetSpecDecl* decl) {
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"spec", "gen_dir", "cc_out", "h_out", "deps"});

  auto src = GetBufferContents(options["spec"]);
  auto gen_dir = GetStringOption(options["gen_dir"]);
  auto cc_out = GetStringOption(options["cc_out"]);
  auto h_out = GetStringOption(options["h_out"]);
  auto deps = ProcessLibraryBuildResultList(options["deps"]);
  deps.push_back(parent->default_flags);
  deps.push_back(parent->GetRule("src/gui", "widget_helper"));

  DoMkdir(ast_ctx().strdup(gen_dir));

  EmitStream h_stream;
  EmitStream cc_stream;
  widget_spec::EmitWidgetSpec(ast_ctx(), src, h_stream.stream(), cc_stream.stream());
  cc_stream.write(gen_dir + "/" + cc_out);
  h_stream.write(gen_dir + "/" + h_out);

  return SimpleCompileCXXFile(ast_ctx(), deps, gen_dir, {cc_out}); 
}
LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::BufferLoweringSpecDecl* decl) {
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"src", "gen_dir", "cc_out"});

  auto src = GetBufferContents(options["src"]);
  auto gen_dir = GetStringOption(options["gen_dir"]);
  auto cc_out = GetStringOption(options["cc_out"]);

  DoMkdir(ast_ctx().strdup(gen_dir));
  SimpleOldLoweringSpecGenRaw(src, gen_dir + "/" + cc_out);

  return ast_ctx().New<LibraryBuildResult>(); 
}
LibraryBuildResult* FileContext::Eval(string_view rule_name, rule_spec::OldLoweringSpecDecl* decl) {
  auto filename_gen = GetGeneratedFilename();
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"src", "cc_out"});

  auto src = GetStringOption(options["src"]);
  auto cc_out = GetStringOption(options["cc_out"]);

  DoMkdir(ast_ctx().strdup(".generated/" + filename_gen));
  SimpleOldLoweringSpecGen(ast_ctx().strdup(filename + "/" + src), ".generated/" + filename_gen + "/" + cc_out);

  return ast_ctx().New<LibraryBuildResult>(); 
}
LibraryBuildResult* FileContext::Eval(string_view name, rule_spec::LibraryDecl* decl) {
  auto options = IndexOptionSet(filename, name, decl->options, {"deps", "srcs", "hdrs", "gen_dir"});
  if (options["gen_dir"]) {
    auto gen_dir = GetStringOption(options["gen_dir"]);

    DoMkdir(ast_ctx().strdup(gen_dir));

    auto srcs = BufferContentsArgList(options["srcs"], gen_dir);
    BufferContentsArgList(options["hdrs"], gen_dir);

    std::vector<string_view> srcs_copy;
    for (auto& src : srcs) {
      srcs_copy.push_back(src);
    }
    auto deps = ProcessLibraryBuildResultList(options["deps"]);
    deps.push_back(parent->default_flags);
    deps.push_back(parent->default_buffer_flags);
    return SimpleCompileCXXFile(ast_ctx(), deps, gen_dir, srcs_copy); 
  }
  auto srcs = StringArgList(options["srcs"]);
  StringArgList(options["hdrs"]);
  std::vector<string_view> srcs_copy;
  for (auto& src : srcs) {
    srcs_copy.push_back(src);
  }
  auto deps = ProcessLibraryBuildResultList(options["deps"]);
  deps.push_back(parent->default_flags);
  return SimpleCompileCXXFile(ast_ctx(), deps, filename, srcs_copy); 
}

unit FileContext::Eval(string_view rule_name, rule_spec::SoLinkDecl* decl) {
  LinkCommand out;
  out.output_name = ast_ctx().strdup(".build/" + std::string(rule_name));
  auto options = IndexOptionSet(filename, rule_name, decl->options, {"deps"});
  out.deps = ProcessLibraryBuildResultList(options["deps"]);
  out.deps.push_back(parent->so_flags);
  BuildLinkCommand(&out);
  return {};
}
unit FileContext::Eval(string_view rule_name, rule_spec::LinkDecl* decl) {
  LinkCommand out;
  out.output_name = ast_ctx().strdup(".build/" + std::string(rule_name));
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
  for (auto* decl : ReadRuleFile(ctx, base)->decls) {
    result->data[GetName(decl)] = decl;
  }
  return result;
}

FileContext* VirtualFileCollection::GetFile(int64_t key) {
  auto it = cache.find(key);
  if (it != cache.end()) return &it->second;
  auto* result = &cache[key];
  result->mid_parent = this;
  result->parent = parent;
  result->filename = ".generated/ide-gen";
  result->filename_key = key;

  rule_spec::Tokenizer tokens(parent->ctx, parent->ctx.RegisterFileContents(buffer[key].text));
  for (auto* decl : rule_spec::parser::DoParse(tokens)->decls) {
    result->data[GetName(decl)] = decl;
  }
  return result;
}

ASTContext& FileContext::ast_ctx() const {
  return parent->ctx;
}
}  // namespace rules
