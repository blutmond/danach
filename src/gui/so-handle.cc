#include "gui/so-handle.h"

void SoHandle::Free::operator()(void* handle) {
  if (!handle) return;
  if (dlclose(handle) != 0) {
    fprintf(stderr, "dlclose: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
}

void* SoHandle::get_sym_raw(const char* symname) {
  void* sym = dlsym(handle.get(), symname);
  if (!sym) { fprintf(stderr, "dlsym: %s\n", dlerror()); exit(EXIT_FAILURE);  }
  return sym; 
}

SoHandle::SoHandle(const char* filename, int flags) {
  handle = std::unique_ptr<void, Free>(dlopen(filename, flags));
  if (!handle) { fprintf(stderr, "dlopen(%s): %s\n", filename, dlerror()); exit(EXIT_FAILURE); }
}
