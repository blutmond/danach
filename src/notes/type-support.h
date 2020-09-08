#pragma once

#include <vector>
#include "rules/string-utils.h"
#include <iostream>

template <typename Child, typename Base>
Child* dyn_cast(Base* base);

template <typename Child, typename Base>
Child* dyn_cast_or_null(Base* base) {
  return base == nullptr ? nullptr : dyn_cast<Child>(base);
}

struct metatype;
struct register_type_info {
  static std::vector<metatype*>& all_metatypes() {
    static std::vector<metatype*> result;
    return result;
  }
  register_type_info(metatype* type) { all_metatypes().push_back(type); }
};

template <typename T>
struct metatype_type_info {
  static register_type_info do_register;
  static metatype* get();
};

template <typename T>
metatype* get_metatype();

metatype* make_pointer_metatype(metatype* pointee);
template <typename T>
struct metatype_type_info<T*> {
  static register_type_info do_register;
  static metatype* get() {
    static auto* result = make_pointer_metatype(get_metatype<T>());
    return result;
  }
};

template <typename T>
register_type_info metatype_type_info<T>::do_register{get_metatype<T>()};
template <typename T>
register_type_info metatype_type_info<T*>::do_register{get_metatype<T*>()};

template <typename T>
metatype* get_metatype() {
  (void)metatype_type_info<T>::do_register;
  return metatype_type_info<T>::get();
}

template <typename T>
using raw_fn_ptr = T*;

template <typename R, typename T>
raw_fn_ptr<R> make_raw_fn(T* t) {
  return reinterpret_cast<R*>(t);
}

template <typename T>
raw_fn_ptr<void*(void*)> eraseFn(T* t) {
  return make_raw_fn<void*(void*)>(t);
}

void assert_compatible(metatype* requested, metatype* actual);
bool is_compatible(metatype* expected, metatype* actual);

struct var_field_info;
struct any_ref {
  template <typename T>
  any_ref(T* value) : value_(value), type_(get_metatype<T>()) {}

  any_ref(void* value, metatype* type) : value_(value), type_(type) {}

  any_ref() : value_(nullptr), type_(nullptr) {}

  template <typename T>
  bool is_a() const { return is_a(get_metatype<T>()); }

  bool is_a(metatype* type) const { return is_compatible(type, type_); }

  template <typename T>
  T& get() const {
    assert_compatible(get_metatype<T>(), type_);
    return *reinterpret_cast<T*>(value_);
  }

  template <typename T>
  T* get_or_null() {
    if (!is_a<T>()) return nullptr;
    return reinterpret_cast<T*>(value_);
  }
  
  template <typename T>
  T* get_or_null() const {
    if (!is_a<T>()) return nullptr;
    return reinterpret_cast<T*>(value_);
  }

  any_ref operator[](var_field_info& var);

  void* raw_pointer() const { return value_; }

  metatype& type() const { return *type_; }
 private:
  void* value_;
  metatype* type_;
};
using tptr = any_ref;

any_ref dereference(any_ref ptr);
void do_pointer_assign(any_ref ptr, any_ref value);
any_ref vector_append_back(any_ref arr);
size_t array_size(any_ref arr);
any_ref array_index(any_ref arr, size_t index);

struct builtin_record {
  std::string name;
  any_ref data;
};
std::vector<builtin_record>& get_builtins();
std::string& lookup_builtin_name(void* builtin);
any_ref lookup_builtin(const std::string name);

void emit_metatype(std::ostream& stream, metatype& t);
var_field_info& find_var(metatype* base, string_view name);

template <typename Base>
var_field_info& find_var(string_view name) {
  return find_var(get_metatype<Base>(), name);
}
any_ref allocate_type_by_name(string_view type_name);
metatype* load_metatype_by_name(string_view type_name);
