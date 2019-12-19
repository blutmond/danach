#pragma once

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory>

class SoHandle {
 public:
  struct Free { void operator()(void* handle); };
  SoHandle() {}
  SoHandle(const char* filename, int flags);
  SoHandle(void* handle) : handle(handle) {}

  bool isValid() { return handle.get(); }

  void* release() { return handle.release(); }

  template <typename ffi_function_type>
  ffi_function_type* get_sym(const char* symname) {
    return (ffi_function_type*)(get_sym_raw(symname));
  }
  void* get_sym_raw(const char* symname);
 private:
  std::unique_ptr<void, Free> handle;
};
