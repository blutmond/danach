#include "gui/so-handoff-lib.h"

#include <gtk/gtk.h>

struct Trampoline {
  Trampoline (*fn)(void*);
  void* arg;
};

struct BasicState {
  SoHandle prev_handle;
  SoHandle self_handle;
  SoHandle next_handle;
  size_t jump_id;
  ~BasicState() {
    printf("[so-reloader] exiting so file\n");
    self_handle.release();
  }
};

static BasicState state;

extern "C"
Trampoline get_main_loop() {
  return Trampoline {
    +[](void*) -> Trampoline {
      // Consider deleting on a bg thread.
      state.prev_handle = nullptr;

      gtk_main();
      if (state.next_handle.isValid()) {
        auto* set_self_so_file = state.next_handle.get_sym<void(SoHandle)>("set_self_so_file");
        auto* set_prev_so_file = state.next_handle.get_sym<void(SoHandle)>("set_prev_so_file");
        auto* get_main_loop = state.next_handle.get_sym<Trampoline()>("get_main_loop");
        state.next_handle.get_sym<void(size_t)>("set_jump_id")(state.jump_id + 1);

        // Final steps of handover...
        set_prev_so_file(std::move(state.self_handle));
        set_self_so_file(std::move(state.next_handle));
        return get_main_loop();
      }
      exit(EXIT_SUCCESS);
    },
    nullptr
  };
}

extern "C" void set_self_so_file(SoHandle handle) { state.self_handle = std::move(handle); }
extern "C" void set_prev_so_file(SoHandle handle) { state.prev_handle = std::move(handle); }
extern "C" void set_jump_id(size_t jump_id) { state.jump_id = jump_id; }

extern "C" Trampoline get_dlopen_trampoline(void* handle, int argc, char **argv) {
  gtk_init(&argc, &argv);
  state.self_handle = SoHandle(handle);
  state.jump_id = 0;

  state.self_handle.get_sym<void(int argc, char **argv)>("dl_plugin_entry")(argc, argv);
  return get_main_loop();
}

namespace main {
size_t GetJumpId() {
  return state.jump_id;
}
void SwapToNewSoFile(SoHandle handle) {
  state.next_handle = std::move(handle);
  gtk_main_quit();
}
}  // namespace main
