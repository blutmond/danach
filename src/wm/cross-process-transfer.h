#pragma once

#include <unistd.h>
#include <memory>
#include <vector>

template <typename T>
size_t GetId();

#define ADD_TRANSFER_TYPE(Name, id) \
  template <> \
  size_t GetId<Name>() { return id; } \
  static TypeNameRegister register_ ## Name{#Name, id};

void* cast_to(size_t id, size_t type_id, void* payload);
void add_subclass_info(size_t child_id, size_t base_id, void* (*casting)(void* child));
void add_type_name(const char* name, size_t id);
const char* GetTypeName(size_t id);

struct TypeNameRegister {
  TypeNameRegister(const char* name, size_t id) { add_type_name(name, id); }
};

struct OpaqueObjectRef {
  size_t type_id;
  void* payload;
  template <typename T>
  std::unique_ptr<T> LoadAs() const {
    return std::unique_ptr<T>(reinterpret_cast<T*>(cast_to(GetId<T>(), type_id, payload)));
  }
};

using FactorLoaderType = OpaqueObjectRef(*)(void*);

struct OpaqueTransferRef {
  template <typename T>
  std::unique_ptr<T> Load() const { return OpaqueLoad().LoadAs<T>(); }
  OpaqueObjectRef OpaqueLoad() const;
  size_t buffer_type_id;
  void* payload;
};

template <typename T>
struct TransferRef : public OpaqueTransferRef {
  std::unique_ptr<T> Load() { return OpaqueLoad().template LoadAs<T>(); }
};

template <typename Base, typename Child>
struct RegisterSubclass {
  RegisterSubclass() {
    add_subclass_info(GetId<Child>(), GetId<Base>(), +[](void* child) -> void* {
      return static_cast<Base*>(reinterpret_cast<Child*>(child));
    });
  }
};

template <typename T>
size_t GetContextId();

struct RegisterLoader {
  RegisterLoader(size_t id, FactorLoaderType loader);
};

#define ADD_SUBCLASS(BASE, CHILD) static RegisterSubclass<BASE, CHILD> register_ ## CHILD ## _to_ ## BASE;

#define ADD_BASIC_DECODER(ContextType, ResultType, loader_id) \
  template <> \
  size_t GetContextId<ContextType>() { return loader_id; } \
  static RegisterLoader register_ ## ContextType{loader_id, (+[](void* body) { \
    return OpaqueObjectRef{GetId<ResultType>(), new ResultType(*reinterpret_cast<ContextType*>(body))}; \
  })};

class BufferContext {
 public:
  template <typename T>
  OpaqueTransferRef encode(const T& v) {
    auto* res = new T(v);
    to_delete_.push_back({res, +[](void* p) { delete reinterpret_cast<T*>(p); }});
    return {GetContextId<T>(), res};
  }

 private:
  struct Deleter {
    void* ptr;
    void (*deleter)(void*);
  };
  std::vector<Deleter> to_delete_;
};

/*
template <typename T>
void RegisterLoader(size_t loader_id, OpaqueObjectRef(handler*)(void*)) {
  // ...
}
*/
