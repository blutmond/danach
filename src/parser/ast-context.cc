#include "parser/ast-context.h"
#include <string.h>

const char* ASTContext::RegisterFileContents(std::string contents) {
  found_buffers_.push_back(std::move(contents));
  return found_buffers_.back().c_str();
}

static void deleteStr(void* str) {
  free(str);
}
const char* ASTContext::strdup(string_view str) {
  auto* res = ::strdup(std::string(str).c_str());
  to_delete_.push_back({res, deleteStr});
  return res;
}
const char* ASTContext::strdup(const std::string& str) {
  auto* res = ::strdup(std::string(str).c_str());
  to_delete_.push_back({res, deleteStr});
  return res;
}
