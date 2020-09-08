#include "notes/generated.h"

#include <unordered_map>

any_ref array_index(any_ref arr, size_t index) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  return any_ref(ptr->get_element(arr.raw_pointer(), index), ptr->element);
}

size_t array_size(any_ref arr) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  return ptr->get_size(arr.raw_pointer());
}

any_ref vector_append_back(any_ref arr) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  size_t idx = ptr->get_size(arr.raw_pointer());
  ptr->emplace_back(arr.raw_pointer());
  return any_ref(ptr->get_element(arr.raw_pointer(), idx), ptr->element);
}

void do_pointer_assign(any_ref ptr, any_ref value) {
  auto* ptrt = dyn_cast<pointer_metatype>(&ptr.type());
  if (ptrt == nullptr) throw TypeMismatchException("expected pointer");
  assert_compatible(ptrt->pointee, &value.type());
  *reinterpret_cast<void**>(ptr.raw_pointer()) = value.raw_pointer();
}

any_ref dereference(any_ref ptr) {
  auto* ptrt = dyn_cast<pointer_metatype>(&ptr.type());
  if (ptrt == nullptr) throw TypeMismatchException("expected pointer");
  void* tmp = *reinterpret_cast<void**>(ptr.raw_pointer());
  auto* group_type = dyn_cast<type_group_metatype>(ptrt->pointee);
  if (group_type && tmp) {
    return any_ref(tmp, group_type->load_type(group_type->get_kind(tmp)));
  } else {
    return any_ref(*reinterpret_cast<void**>(ptr.raw_pointer()), ptrt->pointee);
  }
}

std::string& lookup_builtin_name(void* builtin) {
  static const auto index = [] {
    std::unordered_map<void*, std::string*> result;
    for (auto& record : get_builtins()) result[*reinterpret_cast<void**>(record.data.raw_pointer())] = &record.name;
    return result;
  }();
  auto it = index.find(builtin);
  if (it == index.end()) {
    std::cerr << "problem finding builtin\n";
    exit(-1);
  }
  return *it->second;
}
any_ref lookup_builtin(const std::string name) {
  static const auto index = [] {
    std::unordered_map<std::string, any_ref> result;
    for (auto& record : get_builtins()) result.emplace(record.name, record.data);
    return result;
  }();
  auto it = index.find(name);
  if (it == index.end()) {
    std::cerr << "problem finding builtin: " << name << "\n";
    exit(-1);
  }
  return it->second;
}

any_ref any_ref::operator[](var_field_info& var) {
  assert_compatible(type_, var.base);
  return any_ref(var.fetch_var(value_), var.type);
}

void emit_metatype(std::ostream& stream, struct_metatype& t) { stream << t.name; }
void emit_metatype(std::ostream& stream, pointer_metatype& t) { stream << "ptr<"; emit_metatype(stream, *t.pointee); stream << ">"; }
void emit_metatype(std::ostream& stream, vector_metatype& t) { stream << "vector<"; emit_metatype(stream, *t.element); stream << ">"; }
void emit_metatype(std::ostream& stream, type_group_metatype& t) { stream << t.name; }
void emit_metatype(std::ostream& stream, builtin_metatype& t) { stream << t.name; }

void emit_metatype(std::ostream& stream, metatype& t) {
  switch (t.getKind()) {
  case metatype::Kind::struct_metatype: return emit_metatype(stream, reinterpret_cast<struct_metatype&>(t));
  case metatype::Kind::pointer_metatype: return emit_metatype(stream, reinterpret_cast<pointer_metatype&>(t));
  case metatype::Kind::vector_metatype: return emit_metatype(stream, reinterpret_cast<vector_metatype&>(t));
  case metatype::Kind::type_group_metatype: return emit_metatype(stream, reinterpret_cast<type_group_metatype&>(t));
  case metatype::Kind::builtin_metatype: return emit_metatype(stream, reinterpret_cast<builtin_metatype&>(t));
  }
}

void assert_compatible(metatype* expected, metatype* actual) {
  if (is_compatible(expected, actual)) return;
  EmitStream error_stream;
  error_stream.stream() << "expected: ";
  emit_metatype(error_stream.stream(), *expected);
  error_stream.stream() << " got: ";
  emit_metatype(error_stream.stream(), *actual);
  throw TypeMismatchException(error_stream.get());
}
bool is_compatible(metatype* expected, metatype* actual) {
  if (expected == actual) return true;
  if (auto* actual_struct = dyn_cast<struct_metatype>(actual)) {
    if (actual_struct->group == expected) return true;
  }
  return false;
}

metatype* make_pointer_metatype(metatype* pointee) {
  // Add cache here as well in order to support building runtime types.
  auto* result = new pointer_metatype;
  result->pointee = pointee;
  return result;
}

var_field_info& find_var(metatype* base, string_view name) {
  if (base->getKind() != metatype::Kind::struct_metatype) {
    std::cerr << "problem: " << name << "\n";
    exit(-1);
  }
  auto* t = reinterpret_cast<struct_metatype*>(base);
  for (auto& var : t->vars) {
    if (var.name == name) {
      return var;
    }
  }
  std::cerr << "problem: " << name << "\n";
  exit(-1);

}

const std::unordered_map<std::string, metatype*>& getIndexedTypes() {
  static auto indexed_metatypes = []() {
    std::unordered_map<std::string, metatype*> indexed;
    for (auto* type : register_type_info::all_metatypes()) {
      EmitStream ss;
      emit_metatype(ss.stream(), *type);
      indexed[ss.get()] = type;
    }
    return indexed;
  }();
  return indexed_metatypes;
}

metatype* load_metatype_by_name(string_view type_name) {
  auto& types = getIndexedTypes();
  auto it = types.find(std::string(type_name));
  if (it != types.end()) return it->second;
  std::cerr << "No such type: " << type_name << "\n";
  exit(-1);
}

any_ref allocate_type_by_name(string_view type_name) {
  auto* type = load_metatype_by_name(type_name);
  if (auto* sdecl = dyn_cast<struct_metatype>(type)) return any_ref(sdecl->allocate(), sdecl);
  if (type_name == "vector<any_ref>") {
    return any_ref(new std::vector<any_ref>());
  }

  std::cerr << "Cannot allocate: " << type_name << "\n";
  exit(-1);
}
