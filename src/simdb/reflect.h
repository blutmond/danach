#pragma once

#include <memory>
#include <typeinfo>
#include <vector>

void reflect_test();

std::string demangle(const char* name);

class ObjectWrapper {
 public:
  virtual ~ObjectWrapper() {}
  template <typename T>
  T* unwrap();

  template <typename T>
  T* unwrap_or_nullptr();
  virtual const char* type_name() const = 0;
};

template <typename T>
class TypedObjectRefSource : public ObjectWrapper {
 public:
  virtual T* typed_unwrap() = 0;
};

using any_ptr = std::shared_ptr<ObjectWrapper>;

template <typename T>
struct TypedObjectWrapper : public TypedObjectRefSource<T> {
  template <typename T2>
  explicit TypedObjectWrapper(T2&& t) : value(std::forward<T2>(t)) {}
  T value;
  T* typed_unwrap() override { return &value; }

  const char* type_name() const override { return typeid(T).name(); }
};

struct TypeMismatchException : public std::exception {
  const char * what() const throw () {
    return "Type mismatch.";
  }
};

template <typename T>
T* ObjectWrapper::unwrap() {
  auto* tmp = dynamic_cast<TypedObjectRefSource<T>*>(this);
  if (!tmp) throw TypeMismatchException();
  return tmp->typed_unwrap();
}

template <typename T>
T* ObjectWrapper::unwrap_or_nullptr() {
  auto* tmp = dynamic_cast<TypedObjectRefSource<T>*>(this);
  if (!tmp) return nullptr;
  return tmp->typed_unwrap();
}

template <typename T>
any_ptr wrap(T&& t) {
  return std::make_shared<TypedObjectWrapper<typename std::decay<T>::type>>(std::forward<T>(t));
}

template <typename T>
class ReferenceSource : public TypedObjectRefSource<T> {
 public:
  explicit ReferenceSource(T* value) : value(value) {}
  T* value;
  T* typed_unwrap() override { return value; }
  const char* type_name() const override { return typeid(this).name(); }
};

template <typename T>
any_ptr ref_wrap(T& t) {
  return std::make_shared<ReferenceSource<typename std::decay<T>::type>>(&t);
}

struct InvalidApplyException : public std::exception {
  const char * what() const throw () {
    return "Invalid apply.";
  }
};

class ApplyFnWrapperBase : public ObjectWrapper {
 public:
  virtual any_ptr apply(const std::vector<any_ptr>& args) = 0;
  virtual size_t size() const = 0;
};

template <typename R, typename ...Args>
class TypedObjectWrapper<R(*)(Args...)> : public ApplyFnWrapperBase {
 public:
  using ValueType = R(*)(Args...);
  ValueType value;
  explicit TypedObjectWrapper(ValueType value) : value(value) {}

  template <std::size_t... I>
  any_ptr apply_internal(const std::vector<any_ptr>& args, std::index_sequence<I...>) const {
    return wrap((*value)(*args[I]->unwrap<typename std::decay<Args>::type>()...));
  }
  
  any_ptr apply(const std::vector<any_ptr>& args) override {
    if (args.size() != sizeof...(Args)) throw InvalidApplyException();
    return apply_internal(args, std::make_index_sequence<sizeof...(Args)>{});
  }
  const char* type_name() const override { return typeid(ValueType).name(); }
  size_t size() const override { return sizeof...(Args); }
};

template <typename ...Args>
class TypedObjectWrapper<void(*)(Args...)> : public ApplyFnWrapperBase {
 public:
  using ValueType = void(*)(Args...);
  ValueType value;
  explicit TypedObjectWrapper(ValueType value) : value(value) {}

  template <std::size_t... I>
  any_ptr apply_internal(const std::vector<any_ptr>& args, std::index_sequence<I...>) const {
    (*value)(*args[I]->unwrap<typename std::decay<Args>::type>()...);
    return nullptr;
  }
  
  any_ptr apply(const std::vector<any_ptr>& args) override {
    if (args.size() != sizeof...(Args)) throw InvalidApplyException();
    return apply_internal(args, std::make_index_sequence<sizeof...(Args)>{});
  }
  const char* type_name() const override { return typeid(this).name(); }
  size_t size() const override { return sizeof...(Args); }
};

any_ptr apply(any_ptr fn, const std::vector<any_ptr>& args);
