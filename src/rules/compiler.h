#pragma once

#include "rules/string-utils.h"
#include "parser/ast-context.h"
#include <vector>

void DoMkdir(const char* filename);

const char* strdup(const std::string& str);

const char* strdup(string_view str);

string_view RemoveExt(string_view filename);

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

std::vector<LibraryBuildResult*> toposort(const std::vector<LibraryBuildResult*>& deps);
std::vector<LibraryBuildResult*> postorder(const std::vector<LibraryBuildResult*>& deps);

void CollectObjects(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out);
void CollectLinkFlags(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out);
void CollectCXXFlags(const std::vector<LibraryBuildResult*>& deps, std::vector<const char*>* out);

LibraryBuildResult *SimpleCompileCXXFile(ASTContext& ast_ctx, const std::vector<LibraryBuildResult*>& deps,
                                         string_view src_folder, string_view o_folder, 
                                         const std::vector<string_view>& srcs);

void BuildLinkCommand(LinkCommand* cmd);

LibraryBuildResult *MakeBaseFlags(ASTContext& ast_ctx);
LibraryBuildResult *MakeDefaultFlags(ASTContext& ast_ctx);
LibraryBuildResult *MakeDefaultBufferFlags(ASTContext& ast_ctx);
LibraryBuildResult *MakeSoFlags(ASTContext& ast_ctx);
LibraryBuildResult *MakeGtkFlags(ASTContext& ast_ctx);
LibraryBuildResult *MakeDLFlags(ASTContext& ast_ctx);

void RunTrace(std::vector<const char*> argv);

void EmitCompilerTrace(const char* filename);
