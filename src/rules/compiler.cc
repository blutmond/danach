#include <string.h>

#include "rules/compiler.h"
#include "rules/process.h"
#include "rules/template-support.h"

#include <unordered_set>
#include <algorithm>

struct TraceCache {
  std::vector<std::vector<const char*>> cmds;
};

static TraceCache trace;

void RunTrace(std::vector<const char*> argv) {
  trace.cmds.push_back(argv);
  Run(std::move(argv));
}

void EmitCompilerTrace(const char* filename) {
  EmitStream stream;
  stream.stream() << "#!/bin/bash\n";
  for (auto& cmd : trace.cmds) {
    int i = 0;
    for (const char* c : cmd) {
      if (i != 0) stream.stream() << " ";
      ++i;
      stream.stream() << c;
    }
    stream.stream() << "\n";
  }
  trace.cmds.clear();
  stream.write(filename);
}

struct MkdirCache {
  std::unordered_set<std::string> mkdirs;
  void Ensure(const char* fname) {
    auto key = std::string(fname);
    if (mkdirs.find(key) == mkdirs.end()) {
      mkdirs.insert(key);
      RunTrace({"/bin/mkdir", "-p", fname});
    }
  }
};
static MkdirCache mkdir;

void DoMkdir(const char* filename) {
  mkdir.Ensure(filename);
}

string_view RemoveExt(string_view filename) {
  return filename.substr(0, filename.find_last_of("."));
}

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

bool use_asan = false;
LibraryBuildResult *SimpleCompileCXXFile(ASTContext& ast_ctx, const std::vector<LibraryBuildResult*>& deps,
                                         string_view src_folder, string_view o_folder, 
                                         const std::vector<string_view>& srcs) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  for (string_view cxx_name : srcs) {
    std::vector<const char*> eval = {"/usr/bin/ccache", "/usr/bin/clang-6.0", "-Wall", "-std=c++17"};
    if (use_asan)
      for (const char* flag : {"-fsanitize=address", "-fno-omit-frame-pointer", "-g3"}) eval.push_back(flag);
    CollectCXXFlags(postorder(deps), &eval);
    std::string base = std::string(RemoveExt(cxx_name));
    DoMkdir(ast_ctx.strdup(o_folder));
    const char* object_filename = ast_ctx.strdup((std::string(o_folder) + "/" + base + ".o"));
    eval.push_back("-MF");
    eval.push_back(ast_ctx.strdup((std::string(o_folder) + "/" + base + ".d")));
    eval.push_back("-MD");
    eval.push_back("-c");
    eval.push_back(ast_ctx.strdup(std::string(src_folder) + "/" + std::string(cxx_name)));
    eval.push_back("-o");
    eval.push_back(object_filename);
    RunTrace(std::move(eval));
    res->object_files.push_back(object_filename);
  }
  res->deps = deps;
  return res;
}

void BuildLinkCommand(LinkCommand* cmd) {
  std::vector<const char*> eval = {"/usr/bin/clang-6.0", "-Wall"};
  if (use_asan)
    for (const char* flag : {"-fsanitize=address", "-fno-omit-frame-pointer", "-g3"}) eval.push_back(flag);
  auto toposort_deps = toposort(cmd->deps);
  CollectObjects(toposort_deps, &eval);
  CollectLinkFlags(toposort_deps, &eval);
  eval.push_back("-o");
  eval.push_back(cmd->output_name);
  RunTrace(eval);
}

LibraryBuildResult *MakeDefaultFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  res->link_flags = {"-lstdc++"};
  res->cxx_flags = {"-fpic", "-I", ".generated/", "-I", "src", "-I", ".build/"};
  return res;
}

LibraryBuildResult *MakeBaseFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  res->link_flags = {"-lstdc++"};
  res->cxx_flags = {"-fpic", "-I", "."};
  return res;
}

LibraryBuildResult *MakeDefaultBufferFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  res->cxx_flags = {"-I", ".generated/src"};
  return res;
}

LibraryBuildResult *MakeSoFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  res->link_flags = {"-shared", "-Wl,-z,defs", "-Wl,-rpath='$ORIGIN'"};
  res->cxx_flags = {};
  return res;
}

LibraryBuildResult *MakeGtkFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
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

LibraryBuildResult *MakeDLFlags(ASTContext& ast_ctx) {
  auto* res = ast_ctx.New<LibraryBuildResult>();
  res->link_flags = {"-ldl"};
  return res;
}
