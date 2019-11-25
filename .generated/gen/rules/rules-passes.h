#pragma once
namespace rules {

class RuleModuleContext;
class RuleFile;

class RuleModuleContext {
 public:
  RuleFile* GetFile(string_view key);
  LibraryBuildResult* default_flags = MakeDefaultFlags();
 private:
  std::unordered_map<std::string, RuleFile*> GetFile_storage;
};
  RuleFile* LoadRuleFile(RuleModuleContext* ctx, string_view path);

class RuleFile {
 public:
  rule_spec::Module* module;
  RuleModuleContext* parent;
  std::string filename;
  void DoIndex();
  std::unordered_map<std::string, rule_spec::LinkDecl*> links;
  std::unordered_map<std::string, rule_spec::Decl*> libs;
  void LinkOrTrigger(string_view rule_name);
  LibraryBuildResult* GetAndRunRule(string_view key);
  void Link(string_view key);
  rule_spec::LinkDecl* GetLinkDecl(string_view path);
  rule_spec::Decl* GetRuleDecl(string_view path);
 private:
  struct GetAndRunRule_CacheState {
    bool finished = false;
    bool started = false;
    LibraryBuildResult* result = nullptr;
  };
  std::unordered_map<std::string, GetAndRunRule_CacheState> GetAndRunRule_storage;
  struct Link_CacheState {
    bool finished = false;
    bool started = false;
  };
  std::unordered_map<std::string, Link_CacheState> Link_storage;
};
}  // namespace rules
