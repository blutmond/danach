#include "wm/cross-process-transfer.h"

#include <unordered_map>
#include <vector>

struct CastingFunction {
  size_t base_id;
  void* (*casting)(void* child);
};

struct TypeRecord {
  const char* name;
  std::vector<CastingFunction> parents;
};


static std::unordered_map<size_t, TypeRecord>& type_record() {
  static std::unordered_map<size_t, TypeRecord> tmp;
  return tmp;
}
static std::unordered_map<size_t, FactorLoaderType>& ctors() {
  static std::unordered_map<size_t, FactorLoaderType> tmp;
  return tmp;
}

RegisterLoader::RegisterLoader(size_t id, FactorLoaderType loader) {
  ctors()[id] = loader;
}

void add_type_name(const char* name, size_t id) {
  type_record()[id].name = name;
}

void add_subclass_info(size_t child_id, size_t base_id, void* (*casting)(void* child)) {
  CastingFunction fn {base_id, casting};
  type_record()[child_id].parents.push_back(fn);
}

const char* GetTypeName(size_t id) {
  auto it = type_record().find(id);
  if (it == type_record().end()) return "Unknown-Type";
  return it->second.name;
}

void* cast_to(size_t goal_id, size_t type_id, void* payload) {
  if (payload == nullptr) return nullptr;
  if (goal_id == type_id) return payload;
  auto it = type_record().find(type_id);
  for (auto& parent : it->second.parents) {
    if (parent.base_id == goal_id) return parent.casting(payload);
  }

  fprintf(stderr, "Bad cast: %s vs %s\n", GetTypeName(goal_id), GetTypeName(type_id)); 
  __builtin_trap();
}

OpaqueObjectRef OpaqueTransferRef::OpaqueLoad() const {
  if (payload == nullptr) return {0, nullptr};
  auto it = ctors().find(buffer_type_id);
  if (it == ctors().end()) {
    fprintf(stderr, "Bad builder: %zu.\n", buffer_type_id);
    __builtin_trap();
  }
  return it->second(payload);
}
