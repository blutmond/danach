#pragma once

#include <typeinfo>
#include <string>

struct TypeMismatchException : public std::exception {
  std::string message;
  TypeMismatchException(const std::type_info& expected_id,
                        const std::type_info& actual_id) {
    message = std::string("expected: ") + std::string(expected_id.name())
        + " got: " + std::string(actual_id.name());
  }
  TypeMismatchException(std::string message) : message(std::move(message)) {}
  const char* what() const throw () {
    return message.c_str();
  }
};

/*
struct tptr {
 public:
  template <typename T>
  tptr(T* data) : data(data), type_id(&typeid(T)) {}
  template <typename T>
  T& get() {
    if (!is_a<T>()) throw TypeMismatchException(typeid(T), *type_id);
    return *reinterpret_cast<T*>(data);
  }
  template <typename T>
  T* get_or_null() {
    if (!is_a<T>()) return nullptr;
    return reinterpret_cast<T*>(data);
  }
  template <typename T>
  const T& get() const {
    if (!is_a<T>()) throw TypeMismatchException(typeid(T), *type_id);
    return *reinterpret_cast<T*>(data);
  }
  template <typename T>
  const T* get_or_null() const {
    if (!is_a<T>()) return nullptr;
    return reinterpret_cast<T*>(data);
  }
  template <typename T>
  bool is_a() const { return typeid(T).hash_code() == type_id->hash_code(); }
 private:
  void* data;
  const std::type_info* type_id;
};
*/
