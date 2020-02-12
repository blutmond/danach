#pragma once

#include <vector>
#include <utility>
#include <string>
#include <deque>
#include "rules/string-utils.h"

class ASTContext {
 public:
  ~ASTContext() { for (Deleter d : to_delete_) d.deleter(d.ptr); }

  template<typename T, typename... Args>
  T* New(Args&&... args) {
    auto* res = new T(std::forward<Args>(args)...);
    to_delete_.push_back({res, +[](void* p) { delete reinterpret_cast<T*>(p); }});
    return res;
  }

  const char* RegisterFileContents(std::string contents);

  const char* strdup(string_view str);
  const char* strdup(const std::string& str);
 private:
  struct Deleter {
    void* ptr;
    void (*deleter)(void*);
  };
  std::deque<std::string> found_buffers_;
  std::vector<Deleter> to_delete_;
};

