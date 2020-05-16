#pragma once

#include <unordered_map>
#include "gen/rules/rule-spec.h"
#include "rules/string-utils.h"
#include "rules/compiler.h"
#include "gui/emit-buffer.h"

struct unit {};

namespace rules {

struct GlobalContext;
struct VirtualFileCollection;

struct FileContext {
  GlobalContext* parent;
  VirtualFileCollection* mid_parent;

  std::string filename;

  int64_t filename_key;
  std::unordered_map<std::string, rule_spec::Decl*> data; 
  std::unordered_map<rule_spec::Decl*, LibraryBuildResult*> rule_cache;
  std::unordered_map<rule_spec::Decl*, unit> link_cache;

  rule_spec::Decl* GetDecl(string_view rule);

  const std::string &GetBufferContents(rule_spec::Option* option);

  LibraryBuildResult* ProcessLibraryBuildResult(rule_spec::Expr* expr);

  std::vector<LibraryBuildResult*> ProcessLibraryBuildResultList(rule_spec::Option* option);

  void Eval(string_view name, rule_spec::Decl* decl);

  std::string GetGeneratedFilename();
  FileContext* Eval(string_view name, rule_spec::ImportDecl* decl);
  FileContext* Eval(string_view name, rule_spec::ImportBufferDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::LibraryDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::OldParserDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::OldLoweringSpecDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::BufferParserDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::BufferLoweringSpecDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::LetDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::WidgetSpecDecl* decl);
  unit Eval(string_view name, rule_spec::SoLinkDecl* decl);
  unit Eval(string_view name, rule_spec::LinkDecl* decl);

  std::vector<std::string> BufferContentsArgList(rule_spec::Option* option,
                                                 const std::string& gen_dir);

  template <typename ResultT>
      ResultT EvalRule(string_view rule, rule_spec::Decl* decl) {
#define VISIT_TYPE(tname) \
      case rule_spec::Decl::Kind::tname: { \
        auto* decl_ = reinterpret_cast<rule_spec::tname ## Decl*>(decl); \
        if constexpr(std::is_same<decltype(Eval(rule, decl_)), ResultT>::value) { \
          return Eval(rule, decl_); \
        } else { \
          std::cerr << #tname << " " << rule << " does not eval as: " \
          << typeid(ResultT).name() << "\n"; \
          exit(-1); \
        } \
      }
        switch (decl->getKind()) {
          VISIT_TYPE(OldParser)
              VISIT_TYPE(OldLoweringSpec)
              VISIT_TYPE(Library)
              VISIT_TYPE(PassesTemplate)
              VISIT_TYPE(Passes)
              VISIT_TYPE(Import)
              VISIT_TYPE(ImportBuffer)
              VISIT_TYPE(WidgetSpec)
              VISIT_TYPE(BufferParser)
              VISIT_TYPE(BufferLoweringSpec)
              VISIT_TYPE(Link)
              VISIT_TYPE(Let)
              VISIT_TYPE(SoLink)
        }
#undef VISIT_TYPE
      }

  template <typename ResultT>
  bool CanEval(rule_spec::Decl* decl) {
#define VISIT_TYPE(tname) \
      case rule_spec::Decl::Kind::tname: \
        return std::is_same<decltype(Eval("", \
        reinterpret_cast<rule_spec::tname ## Decl*>(decl) \
          )), ResultT>::value;
        switch (decl->getKind()) {
          VISIT_TYPE(OldParser)
              VISIT_TYPE(OldLoweringSpec)
              VISIT_TYPE(Library)
              VISIT_TYPE(PassesTemplate)
              VISIT_TYPE(Passes)
              VISIT_TYPE(Import)
              VISIT_TYPE(ImportBuffer)
              VISIT_TYPE(WidgetSpec)
              VISIT_TYPE(BufferParser)
              VISIT_TYPE(BufferLoweringSpec)
              VISIT_TYPE(Link)
              VISIT_TYPE(Let)
              VISIT_TYPE(SoLink)
        }
#undef VISIT_TYPE
  }

  template <typename ResultT>
      ResultT EvalRule(string_view rule, std::unordered_map<rule_spec::Decl*, ResultT>& cache) {
        auto* decl = GetDecl(rule);
        auto it = cache.find(decl);
        if (it != cache.end()) return it->second;
        return cache[decl] = EvalRule<ResultT>(rule, decl);
      }

  LibraryBuildResult* GetLibRule(string_view rule) {
    return EvalRule<LibraryBuildResult*>(rule, rule_cache);
  }
  void DoLink(string_view rule) {
    EvalRule<unit>(rule, link_cache);
  }
  void LinkOrTrigger(string_view rule) {
    auto* decl = GetDecl(rule);
    if (CanEval<unit>(decl)) {
      DoLink(rule);
      return;
    }
    GetLibRule(rule);
  }
  ASTContext& ast_ctx() const;
};

struct VirtualFileCollection {
  std::vector<CollapsedBuffer> buffer;
  std::unordered_map<int64_t, FileContext> cache;

  FileContext* GetFile(int64_t key); 

  GlobalContext* parent;
};

struct GlobalContext {
  ASTContext ctx;
  std::unordered_map<std::string, FileContext> cache;
  std::unordered_map<std::string, VirtualFileCollection> buffer_cache;

  LibraryBuildResult* default_flags = MakeDefaultFlags(ctx);
  LibraryBuildResult* default_buffer_flags = MakeDefaultBufferFlags(ctx);
  LibraryBuildResult* so_flags = MakeSoFlags(ctx);
  LibraryBuildResult* gtk_flags = MakeGtkFlags(ctx);
  LibraryBuildResult* dl_flags = MakeDLFlags(ctx);

  LibraryBuildResult* GetRule(string_view base, string_view rule) { 
    return GetFile(base)->GetLibRule(rule);
  }

  FileContext* GetFile(string_view base);
};

}  // namespace
