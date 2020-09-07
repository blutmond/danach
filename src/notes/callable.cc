#include "notes/callable.h"
#include <unordered_map>

const std::unordered_map<std::string, FunctorBase*>& builtin_fn_map() {
  static auto out = ([] {
    std::unordered_map<std::string, FunctorBase*> out;
    for (auto* fn : builtin_fns()) {
      out[fn->name] = fn;
    }
    return out;
  })();
  return out;
}

FunctorBase* find_fn(const char* name) {
  auto& fn_map = builtin_fn_map();
  auto it = fn_map.find(name);
  if (it == fn_map.end()) {
    fprintf(stderr, "problem: %s\n", name);
    exit(-1);
  }
  return it->second;
}
