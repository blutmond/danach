#include "gui/editor-widget.h"
#include "rules/compiler.h"
#include "gen/gui/chunk-manifest.h"

#include <unordered_map>

std::string GetBaseName(size_t id, bool is_hdr) {
  EmitStream ss;
  ss.stream() << "silly-" << id << (is_hdr ? ".h" : ".cc");
  return ss.get();
}
std::string GetFullName(size_t id, bool is_hdr) {
  EmitStream ss;
  ss.stream() << ".gui-build/gen/silly-" << id << (is_hdr ? ".h" : ".cc");
  return ss.get();
}

struct FileManifest {
  ChunkViewTemplate* templ;
  bool started = false;
  LibraryBuildResult* result = nullptr;
  std::vector<std::string> ccdeps;
  std::vector<std::string> hdeps;
};

struct BuildAnalysisContext {
  ASTContext ctx;
  LibraryBuildResult* d_flags = MakeBaseFlags(ctx);
  std::unordered_map<std::string, size_t> named_files;
  std::unordered_map<size_t, FileManifest> views_by_id;
  std::unordered_map<std::string, size_t> link_commands;

  void ResolveDeps(const std::vector<std::string>& deps, std::ostream& stream,
                   std::vector<LibraryBuildResult*>& deps_out) {
    for (auto& dep : deps) {
      auto it = named_files.find(dep);
      if (it == named_files.end()) {
        std::cerr << "Problem finding: " << dep << "\n";
        __builtin_trap();
      }
      size_t id = it->second;
      stream << "#include \".gui-build/gen/silly-" << id << ".h\"\n";
      deps_out.push_back(ResolveLibrary(id));
    }
  }
  LibraryBuildResult* ResolveLibrary(size_t id) {
    auto it = views_by_id.find(id);
    if (it == views_by_id.end()) {
      std::cerr << "Problem finding: " << id << "\n";
      __builtin_trap();
    }
    FileManifest& manifest = it->second;
    if (manifest.result) return manifest.result;
    if (manifest.started) {
      std::cerr << "Found a loop in the deps: " << id << "\n";
      __builtin_trap();
    }
    manifest.started = true;
    std::vector<LibraryBuildResult*> deps = {d_flags};
    EmitStream h_stream;
    EmitStream cc_stream;
    h_stream.stream() << "#pragma once\n";
    cc_stream.stream() << "#include \".gui-build/gen/silly-" << id << ".h\"\n";
    ResolveDeps(manifest.hdeps, h_stream.stream(), deps);
    ResolveDeps(manifest.ccdeps, cc_stream.stream(), deps);
    for (auto& decl : manifest.templ->decls) {
      decl->Emit(*manifest.templ, h_stream.stream(), cc_stream.stream());
    }
    cc_stream.write(GetFullName(id, false));
    h_stream.write(GetFullName(id, true));

    auto* lib = SimpleCompileCXXFile(ctx, deps, ".gui-build/gen", ".gui-build/objects",
                                     {ctx.strdup(GetBaseName(id, false))});
    manifest.result = lib;
    return lib;
  }
  void DoLink(const std::string& key) {
    auto it = link_commands.find(key);
    if (it == link_commands.end()) {
      std::cerr << "Could not find: " << key << "\n";
      __builtin_trap();
    }
    LinkCommand lcmd;
    lcmd.output_name = ctx.strdup(".gui-build/" + key);
    lcmd.deps = {ResolveLibrary(it->second)};
    BuildLinkCommand(&lcmd);
  }
};

void SillyStruct2::DoBuild() {
  DoMkdir(".gui-build/gen");
  BuildAnalysisContext ctx;
  for (auto& view : views) {
    FileManifest& manifest = ctx.views_by_id[view->config_chunk_id];
    manifest.templ = view.get();
    for (auto& decl : manifest.templ->decls) {
      if (auto* ddecl = dynamic_cast<ChunkViewTemplate::DepDecl*>(decl.get())) {
        auto data = Collapse(*manifest.templ->GetChunk(ddecl->chunk_id));
        ASTContext ast_ctx;
        chunk_manifest::Tokenizer tokens(ast_ctx, data.c_str());
        auto* module = chunk_manifest::parser::DoParse(tokens);
        size_t view_id = view->config_chunk_id;
        for (auto* _decl : module->decls) {
          using namespace chunk_manifest;
          switch (_decl->getKind()) {
          case Decl::Kind::Binary: {
            auto* decl = reinterpret_cast<BinaryDecl*>(_decl);
            ctx.link_commands[Unescaped(decl->fname.str)] = view_id;
            break;
          }
          case Decl::Kind::Deps: {
            auto* decl = reinterpret_cast<DepsDecl*>(_decl);
            for (auto* _dep : decl->deps) {
              switch (_dep->getKind()) {
              case Dep::Kind::H:
                ctx.views_by_id[view_id].hdeps.push_back(std::string(reinterpret_cast<HDep*>(_dep)->name.str));
                break;
              case Dep::Kind::CC:
                ctx.views_by_id[view_id].hdeps.push_back(std::string(reinterpret_cast<CCDep*>(_dep)->name.str));
                break;
              }
            }
            break;
          }
          case Decl::Kind::Name: {
            ctx.named_files[std::string(reinterpret_cast<NameDecl*>(_decl)->name.str)] = view_id;
            break;
          }
          }
        }
      }
    }
  }
  // TODO: parse this stuff...
  ctx.DoLink("example");

  fprintf(stderr, "\e[32mSuccess!\e[m\n");
}
