#pragma once

#include "notes/type-info.h"
#include "notes/type-support.h"
#include "rules/string-utils.h"
#include <vector>
#include <string>

class FunctorBase {
 public:
  virtual ~FunctorBase() {}
  FunctorBase(const char* name, size_t fn_id) : name(name), fn_id(fn_id) {}
  const char* name;
  size_t fn_id;
};

template <typename T>
class BuiltinFunctor : public FunctorBase {
 public:
  using FunctorBase::FunctorBase;
};

template <typename T>
class BuiltinFunctorConcrete : public BuiltinFunctor<T> {
 public:
  BuiltinFunctorConcrete(const char* name, size_t fn_id, T* fn_ptr) : BuiltinFunctor<T>(name, fn_id), fn_ptr(fn_ptr) {}
  T* fn_ptr;
};

template <typename R, typename ...Args>
class BuiltinFunctor<R(Args...)> : public FunctorBase {
 public:
  using FunctorBase::FunctorBase;
  virtual R apply(Args... args) = 0; 
};

template <typename R, typename ...Args>
class BuiltinFunctorConcrete<R(Args...)> : public BuiltinFunctor<R(Args...)> {
 public:
  using T = R(Args...);
  BuiltinFunctorConcrete(const char* name, size_t fn_id, T* fn_ptr) : BuiltinFunctor<T>(name, fn_id), fn_ptr(fn_ptr) {}
  T* fn_ptr;
  R apply(Args... args) override {
    return (this->fn_ptr)(std::forward<Args>(args)...);
  }
};

template <typename T>
FunctorBase* wrapFn(const char* name, size_t fn_id, T* fn_ptr) {
  return new BuiltinFunctorConcrete<T>(name, fn_id, fn_ptr);
}

const std::vector<FunctorBase*>& builtin_fns();

FunctorBase* find_fn(const char* name);

template <typename T>
struct sfn {
  BuiltinFunctor<T>* fn;
  sfn(BuiltinFunctor<T>* fn) : fn(fn) {}
};

template <typename R, typename ...Args>
struct sfn<R(Args...)> {
  using T = R(Args...);
  BuiltinFunctor<T>* fn;
  sfn(BuiltinFunctor<T>* fn) : fn(fn) {}
  sfn(T* t) : fn(new BuiltinFunctorConcrete("unknown", 22222, t)) {}
  R operator()(Args... args) const {
    return fn->apply(std::forward<Args>(args)...);
  }
};

template <>
struct sfn<std::string&(any_ref)> {
  using T = std::string&(any_ref);
  BuiltinFunctor<T>* fn;
  sfn(BuiltinFunctor<T>* fn) : fn(fn) {}
  std::string& operator()(any_ref obj) const {
    return fn->apply(obj);
  }
};

template <typename T>
sfn<T> find_fn(const char* name) {
  auto* fn = dynamic_cast<BuiltinFunctor<T>*>(find_fn(name));
  return {fn};
}

