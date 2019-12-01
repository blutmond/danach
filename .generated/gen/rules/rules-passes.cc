namespace rules {

RuleFile* RuleModuleContext::GetFile(string_view key) {
  auto key_copy = std::string(key);
  auto it = GetFile_storage.find(key_copy);
  if (it != GetFile_storage.end()) return it->second;
  auto* result = LoadRuleFile(this, key);
  GetFile_storage[key_copy] = result;
  return result;
}

  LibraryBuildResult* RuleModuleContext::gtk_flags() {
    if (_cache_gtk_flags) {
      return _cache_gtk_flags;
    }
    return _cache_gtk_flags = MakeGtkFlags(this);
  }
RuleFile* LoadRuleFile(RuleModuleContext* ctx, string_view path) {
  auto* result = new RuleFile;
  result->parent = ctx;
  result->filename = std::string(path);
  result->module = ReadRuleFile(path);
  result->DoIndex();
  return result;
}

void RuleFile::DoIndex() {
  for (auto* decl_ : module->decls) {
    switch (decl_->getKind()) {
    case rule_spec::Decl::Kind::Import: {
      auto* decl = reinterpret_cast<rule_spec::ImportDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::OldParser: {
      auto* decl = reinterpret_cast<rule_spec::OldParserDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::OldLoweringSpec: {
      auto* decl = reinterpret_cast<rule_spec::OldLoweringSpecDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::Library: {
      auto* decl = reinterpret_cast<rule_spec::LibraryDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::Passes: {
      auto* decl = reinterpret_cast<rule_spec::PassesDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::PassesTemplate: {
      auto* decl = reinterpret_cast<rule_spec::PassesTemplateDecl*>(decl_);
      auto key = std::string(decl->name.str);
      if (libs.find(key) != libs.end()) {
        fprintf(stderr, "Duplicate rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      libs[key] = decl;
      break;
    }
    case rule_spec::Decl::Kind::Link: {
      auto* decl = reinterpret_cast<rule_spec::LinkDecl*>(decl_);
      auto key = Unescaped(decl->fname.str);
      if (links.find(key) != links.end()) {
        fprintf(stderr, "Duplicate link rule: %s\n", key.c_str());
        exit(EXIT_FAILURE);
      }
      links[key] = decl;
      break;
    }
    }
  }
}

void RuleFile::LinkOrTrigger(string_view rule_name) {
  if (links.find(std::string(rule_name)) != links.end()) {
    return Link(rule_name);
  }
  GetAndRunRule(rule_name);
}

LibraryBuildResult* RuleFile::GetAndRunRule(string_view key) {
  auto& state = GetAndRunRule_storage[std::string(key)];
  if (!state.finished) {
    assert(!state.started);
    state.started = true;
    state.result = DoGetAndRunRule(this, key);
    state.finished = true;
  }
  return state.result;
}

void RuleFile::Link(string_view key) {
  auto& state = Link_storage[std::string(key)];
  if (!state.finished) {
    assert(!state.started);
    state.started = true;
    DoLink(this, key);
    state.finished = true;
  }
}

rule_spec::LinkDecl* RuleFile::GetLinkDecl(string_view path) {
  auto it = links.find(std::string(path));
  if (it == links.end()) {
    std::cerr << "No such GetLinkDecl: " << path << "\n";
    exit(EXIT_FAILURE);
  }
  return it->second;
}

rule_spec::Decl* RuleFile::GetRuleDecl(string_view path) {
  auto it = libs.find(std::string(path));
  if (it == libs.end()) {
    std::cerr << "No such GetRuleDecl: " << path << "\n";
    exit(EXIT_FAILURE);
  }
  return it->second;
}
}  // namespace rules
