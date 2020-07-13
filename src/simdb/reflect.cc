#include "simdb/reflect.h"
#include <typeinfo>
#include <stdio.h>
#include <cxxabi.h>
#include <string>
#include <type_traits>
#include <memory>
#include <vector>

std::string demangle(const char* name) {
  int status = -4; // some arbitrary value to eliminate the compiler warning

  // enable c++11 by passing the flag -std=c++11 to g++
  std::unique_ptr<char, void(*)(void*)> res {
    abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
  return (status==0) ? res.get() : name ;
}

any_ptr apply(any_ptr fn, const std::vector<any_ptr>& args) {
  auto* t = dynamic_cast<ApplyFnWrapperBase*>(fn.get());
  if (!t) throw InvalidApplyException();
  return t->apply(args);
}

int basic_fn(int a, float b) {
  printf("a + b = %d + %d = %d\n", a, int(b), a + (int)b);
  return a + (int)b;
}

void print_thing(int value) {
  printf("silly = %d\n", value);
}

void reflect_test() {
  auto tmp1 = wrap<int>(3);
  auto tmp2 = wrap<float>(4);
  auto fn = wrap(basic_fn);
  auto fn2 = wrap(print_thing);
  printf("silly: %s\n", demangle(fn->type_name()).c_str());
  auto result = apply(fn, {tmp1, tmp2});
  apply(fn, {result, tmp2});
  apply(fn2, {result});
}
