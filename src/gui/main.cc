#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

struct Trampoline {
  Trampoline (*fn)(void*);
  void* arg;
};

int main(int argc, char **argv) {
  const char* filename = ".build/ide-dynamic.so";
  void* handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL);
  if (!handle) { fprintf(stderr, "dlopen(%s): %s\n", filename, dlerror()); exit(EXIT_FAILURE); }

  const char* sym_name = "get_dlopen_trampoline";
  void* sym = dlsym(handle, sym_name);
  if (!sym) { fprintf(stderr, "dlsym(%s): %s\n", sym_name, dlerror()); exit(EXIT_FAILURE);  }
  using get_dlopen_trampoline_t = Trampoline(void* handle, int argc, char **argv);

  Trampoline tramp = ((get_dlopen_trampoline_t*)sym)(handle, argc, argv);
  while (true) {
    tramp = tramp.fn(tramp.arg);
  }
}
