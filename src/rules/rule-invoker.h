#pragma once

#include <unordered_map>
#include "gen/rules/rule-spec.h"
#include "rules/string-utils.h"
#include "rules/compiler.h"

struct unit {};

namespace rules {

struct GlobalContext;

struct FileContext {
  GlobalContext* parent;
  std::string filename;
  std::unordered_map<std::string, rule_spec::Decl*> data; 
  std::unordered_map<rule_spec::Decl*, LibraryBuildResult*> rule_cache;
  std::unordered_map<rule_spec::Decl*, unit> link_cache;

  rule_spec::Decl* GetDecl(string_view rule);

  LibraryBuildResult* ProcessLibraryBuildResult(rule_spec::Expr* expr);

  std::vector<LibraryBuildResult*> ProcessLibraryBuildResultList(rule_spec::Option* option);

  void Eval(string_view name, rule_spec::Decl* decl);

  std::string GetGeneratedFilename();
  FileContext* Eval(string_view name, rule_spec::ImportDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::LibraryDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::OldParserDecl* decl);
  LibraryBuildResult* Eval(string_view name, rule_spec::OldLoweringSpecDecl* decl);
  unit Eval(string_view name, rule_spec::SoLinkDecl* decl);
  unit Eval(string_view name, rule_spec::LinkDecl* decl);

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
              VISIT_TYPE(Link)
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
              VISIT_TYPE(Link)
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
};

struct GlobalContext {
  std::unordered_map<std::string, FileContext> cache;
  LibraryBuildResult* default_flags = MakeDefaultFlags();
  LibraryBuildResult* so_flags = MakeSoFlags();
  LibraryBuildResult* gtk_flags = MakeGtkFlags();
  LibraryBuildResult* dl_flags = MakeDLFlags();

  LibraryBuildResult* GetRule(string_view base, string_view rule) { 
    return GetFile(base)->GetLibRule(rule);
  }

  FileContext* GetFile(string_view base);
};

}  // namespace
