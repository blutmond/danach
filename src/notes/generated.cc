#include "notes/generated.h"


template <>
ConstStringDrawState* dyn_cast<ConstStringDrawState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::ConstStringDrawState ? static_cast<ConstStringDrawState*>(t) : nullptr;
}

template <>
StructDecl* dyn_cast<StructDecl, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::StructDecl ? static_cast<StructDecl*>(t) : nullptr;
}

template <>
PointerDecl* dyn_cast<PointerDecl, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::PointerDecl ? static_cast<PointerDecl*>(t) : nullptr;
}

template <>
ReferenceDecl* dyn_cast<ReferenceDecl, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::ReferenceDecl ? static_cast<ReferenceDecl*>(t) : nullptr;
}

template <>
Void* dyn_cast<Void, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::Void ? static_cast<Void*>(t) : nullptr;
}

template <>
VectorSpecialization* dyn_cast<VectorSpecialization, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::VectorSpecialization ? static_cast<VectorSpecialization*>(t) : nullptr;
}

template <>
TypeGroupDecl* dyn_cast<TypeGroupDecl, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::TypeGroupDecl ? static_cast<TypeGroupDecl*>(t) : nullptr;
}

template <>
BuiltinDecl* dyn_cast<BuiltinDecl, TypeRef>(TypeRef* t) {
  return t->getKind() == TypeRef::Kind::BuiltinDecl ? static_cast<BuiltinDecl*>(t) : nullptr;
}

template <>
struct_metatype* dyn_cast<struct_metatype, metatype>(metatype* t) {
  return t->getKind() == metatype::Kind::struct_metatype ? static_cast<struct_metatype*>(t) : nullptr;
}

template <>
pointer_metatype* dyn_cast<pointer_metatype, metatype>(metatype* t) {
  return t->getKind() == metatype::Kind::pointer_metatype ? static_cast<pointer_metatype*>(t) : nullptr;
}

template <>
vector_metatype* dyn_cast<vector_metatype, metatype>(metatype* t) {
  return t->getKind() == metatype::Kind::vector_metatype ? static_cast<vector_metatype*>(t) : nullptr;
}

template <>
type_group_metatype* dyn_cast<type_group_metatype, metatype>(metatype* t) {
  return t->getKind() == metatype::Kind::type_group_metatype ? static_cast<type_group_metatype*>(t) : nullptr;
}

template <>
builtin_metatype* dyn_cast<builtin_metatype, metatype>(metatype* t) {
  return t->getKind() == metatype::Kind::builtin_metatype ? static_cast<builtin_metatype*>(t) : nullptr;
}

template <>
StringEditState* dyn_cast<StringEditState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::StringEditState ? static_cast<StringEditState*>(t) : nullptr;
}

template <>
TextButtonTypeSelect* dyn_cast<TextButtonTypeSelect, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::TextButtonTypeSelect ? static_cast<TextButtonTypeSelect*>(t) : nullptr;
}

template <>
MultiLineEditState* dyn_cast<MultiLineEditState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::MultiLineEditState ? static_cast<MultiLineEditState*>(t) : nullptr;
}

template <>
CommaForState* dyn_cast<CommaForState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::CommaForState ? static_cast<CommaForState*>(t) : nullptr;
}

template <>
ForDrawState* dyn_cast<ForDrawState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::ForDrawState ? static_cast<ForDrawState*>(t) : nullptr;
}

template <>
TypeDeclToolEditState* dyn_cast<TypeDeclToolEditState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::TypeDeclToolEditState ? static_cast<TypeDeclToolEditState*>(t) : nullptr;
}

template <>
StringDrawState* dyn_cast<StringDrawState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::StringDrawState ? static_cast<StringDrawState*>(t) : nullptr;
}

template <>
DrawFunctorList* dyn_cast<DrawFunctorList, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::DrawFunctorList ? static_cast<DrawFunctorList*>(t) : nullptr;
}

template <>
DrawIfNotNullState* dyn_cast<DrawIfNotNullState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::DrawIfNotNullState ? static_cast<DrawIfNotNullState*>(t) : nullptr;
}

template <>
DrawIfIsAState* dyn_cast<DrawIfIsAState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::DrawIfIsAState ? static_cast<DrawIfIsAState*>(t) : nullptr;
}

template <>
PointerBuilderState* dyn_cast<PointerBuilderState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::PointerBuilderState ? static_cast<PointerBuilderState*>(t) : nullptr;
}

template <>
VectorBuilderState* dyn_cast<VectorBuilderState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::VectorBuilderState ? static_cast<VectorBuilderState*>(t) : nullptr;
}

template <>
IndentChangeState* dyn_cast<IndentChangeState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::IndentChangeState ? static_cast<IndentChangeState*>(t) : nullptr;
}

template <>
ColorChangeState* dyn_cast<ColorChangeState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::ColorChangeState ? static_cast<ColorChangeState*>(t) : nullptr;
}

template <>
NewLineDrawState* dyn_cast<NewLineDrawState, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::NewLineDrawState ? static_cast<NewLineDrawState*>(t) : nullptr;
}

template <>
TextButton* dyn_cast<TextButton, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::TextButton ? static_cast<TextButton*>(t) : nullptr;
}

template <>
NamedDrawFunctor* dyn_cast<NamedDrawFunctor, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::NamedDrawFunctor ? static_cast<NamedDrawFunctor*>(t) : nullptr;
}

template <>
SwitchDrawFunctor* dyn_cast<SwitchDrawFunctor, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::SwitchDrawFunctor ? static_cast<SwitchDrawFunctor*>(t) : nullptr;
}

template <>
UnhandledTypeDrawFunctor* dyn_cast<UnhandledTypeDrawFunctor, UnaryDrawFunctor>(UnaryDrawFunctor* t) {
  return t->getKind() == UnaryDrawFunctor::Kind::UnhandledTypeDrawFunctor ? static_cast<UnhandledTypeDrawFunctor*>(t) : nullptr;
}
template <>
metatype* metatype_type_info<ConstStringDrawState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "ConstStringDrawState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::ConstStringDrawState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "value", get_metatype<std::string>(), eraseFn(+[](ConstStringDrawState* v) { return &v->value; })},
  };
  result->allocate = +[]() -> void* { return new ConstStringDrawState; };
  return result;
}
template <>
metatype* metatype_type_info<StructDecl_Var>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "StructDecl_Var";
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](StructDecl_Var* v) { return &v->name; })},
    var_field_info{result, "type", get_metatype<TypeRef*>(), eraseFn(+[](StructDecl_Var* v) { return &v->type; })},
  };
  result->allocate = +[]() -> void* { return new StructDecl_Var; };
  return result;
}
template <>
metatype* metatype_type_info<StructDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "StructDecl";
  result->kind = static_cast<int>(TypeRef::Kind::StructDecl);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](StructDecl* v) { return &v->name; })},
    var_field_info{result, "group", get_metatype<TypeRef*>(), eraseFn(+[](StructDecl* v) { return &v->group; })},
    var_field_info{result, "vars", get_metatype<std::vector<StructDecl_Var>>(), eraseFn(+[](StructDecl* v) { return &v->vars; })},
  };
  result->allocate = +[]() -> void* { return new StructDecl; };
  return result;
}
template <>
metatype* metatype_type_info<PointerDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "PointerDecl";
  result->kind = static_cast<int>(TypeRef::Kind::PointerDecl);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "pointee", get_metatype<TypeRef*>(), eraseFn(+[](PointerDecl* v) { return &v->pointee; })},
  };
  result->allocate = +[]() -> void* { return new PointerDecl; };
  return result;
}
template <>
metatype* metatype_type_info<ReferenceDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "ReferenceDecl";
  result->kind = static_cast<int>(TypeRef::Kind::ReferenceDecl);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "wrapping", get_metatype<TypeRef*>(), eraseFn(+[](ReferenceDecl* v) { return &v->wrapping; })},
  };
  result->allocate = +[]() -> void* { return new ReferenceDecl; };
  return result;
}
template <>
metatype* metatype_type_info<Void>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "Void";
  result->kind = static_cast<int>(TypeRef::Kind::Void);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
  };
  result->allocate = +[]() -> void* { return new Void; };
  return result;
}
template <>
metatype* metatype_type_info<VectorSpecialization>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "VectorSpecialization";
  result->kind = static_cast<int>(TypeRef::Kind::VectorSpecialization);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "element", get_metatype<TypeRef*>(), eraseFn(+[](VectorSpecialization* v) { return &v->element; })},
  };
  result->allocate = +[]() -> void* { return new VectorSpecialization; };
  return result;
}
template <>
metatype* metatype_type_info<TypeGroupDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "TypeGroupDecl";
  result->kind = static_cast<int>(TypeRef::Kind::TypeGroupDecl);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](TypeGroupDecl* v) { return &v->name; })},
  };
  result->allocate = +[]() -> void* { return new TypeGroupDecl; };
  return result;
}
template <>
metatype* metatype_type_info<BuiltinDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "BuiltinDecl";
  result->kind = static_cast<int>(TypeRef::Kind::BuiltinDecl);
  result->group = get_metatype<TypeRef>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](BuiltinDecl* v) { return &v->name; })},
  };
  result->allocate = +[]() -> void* { return new BuiltinDecl; };
  return result;
}
template <>
metatype* metatype_type_info<struct_metatype>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "struct_metatype";
  result->kind = static_cast<int>(metatype::Kind::struct_metatype);
  result->group = get_metatype<metatype>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](struct_metatype* v) { return &v->name; })},
    var_field_info{result, "group", get_metatype<metatype*>(), eraseFn(+[](struct_metatype* v) { return &v->group; })},
    var_field_info{result, "kind", get_metatype<int>(), eraseFn(+[](struct_metatype* v) { return &v->kind; })},
    var_field_info{result, "vars", get_metatype<std::vector<var_field_info>>(), eraseFn(+[](struct_metatype* v) { return &v->vars; })},
    var_field_info{result, "allocate", get_metatype<raw_fn_ptr<void*()>>(), eraseFn(+[](struct_metatype* v) { return &v->allocate; })},
  };
  result->allocate = +[]() -> void* { return new struct_metatype; };
  return result;
}
template <>
metatype* metatype_type_info<var_field_info>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "var_field_info";
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "base", get_metatype<struct_metatype*>(), eraseFn(+[](var_field_info* v) { return &v->base; })},
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](var_field_info* v) { return &v->name; })},
    var_field_info{result, "type", get_metatype<metatype*>(), eraseFn(+[](var_field_info* v) { return &v->type; })},
    var_field_info{result, "fetch_var", get_metatype<raw_fn_ptr<void*(void*)>>(), eraseFn(+[](var_field_info* v) { return &v->fetch_var; })},
  };
  result->allocate = +[]() -> void* { return new var_field_info; };
  return result;
}
template <>
metatype* metatype_type_info<pointer_metatype>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "pointer_metatype";
  result->kind = static_cast<int>(metatype::Kind::pointer_metatype);
  result->group = get_metatype<metatype>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "pointee", get_metatype<metatype*>(), eraseFn(+[](pointer_metatype* v) { return &v->pointee; })},
  };
  result->allocate = +[]() -> void* { return new pointer_metatype; };
  return result;
}
template <>
metatype* metatype_type_info<vector_metatype>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "vector_metatype";
  result->kind = static_cast<int>(metatype::Kind::vector_metatype);
  result->group = get_metatype<metatype>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "element", get_metatype<metatype*>(), eraseFn(+[](vector_metatype* v) { return &v->element; })},
    var_field_info{result, "get_size", get_metatype<raw_fn_ptr<size_t(void*)>>(), eraseFn(+[](vector_metatype* v) { return &v->get_size; })},
    var_field_info{result, "get_element", get_metatype<raw_fn_ptr<void*(void*, size_t)>>(), eraseFn(+[](vector_metatype* v) { return &v->get_element; })},
    var_field_info{result, "emplace_back", get_metatype<raw_fn_ptr<void(void*)>>(), eraseFn(+[](vector_metatype* v) { return &v->emplace_back; })},
  };
  result->allocate = +[]() -> void* { return new vector_metatype; };
  return result;
}
template <>
metatype* metatype_type_info<type_group_metatype>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "type_group_metatype";
  result->kind = static_cast<int>(metatype::Kind::type_group_metatype);
  result->group = get_metatype<metatype>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](type_group_metatype* v) { return &v->name; })},
    var_field_info{result, "children", get_metatype<std::vector<metatype*>>(), eraseFn(+[](type_group_metatype* v) { return &v->children; })},
    var_field_info{result, "get_kind", get_metatype<raw_fn_ptr<size_t(void*)>>(), eraseFn(+[](type_group_metatype* v) { return &v->get_kind; })},
    var_field_info{result, "load_type", get_metatype<raw_fn_ptr<metatype*(size_t)>>(), eraseFn(+[](type_group_metatype* v) { return &v->load_type; })},
  };
  result->allocate = +[]() -> void* { return new type_group_metatype; };
  return result;
}
template <>
metatype* metatype_type_info<builtin_metatype>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "builtin_metatype";
  result->kind = static_cast<int>(metatype::Kind::builtin_metatype);
  result->group = get_metatype<metatype>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](builtin_metatype* v) { return &v->name; })},
    var_field_info{result, "typeinfo", get_metatype<const std::type_info*>(), eraseFn(+[](builtin_metatype* v) { return &v->typeinfo; })},
  };
  result->allocate = +[]() -> void* { return new builtin_metatype; };
  return result;
}
template <>
metatype* metatype_type_info<StringEditState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "StringEditState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::StringEditState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "var", get_metatype<var_field_info*>(), eraseFn(+[](StringEditState* v) { return &v->var; })},
    var_field_info{result, "alt_text", get_metatype<std::string>(), eraseFn(+[](StringEditState* v) { return &v->alt_text; })},
  };
  result->allocate = +[]() -> void* { return new StringEditState; };
  return result;
}
template <>
metatype* metatype_type_info<FunctionDeclArg>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "FunctionDeclArg";
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](FunctionDeclArg* v) { return &v->name; })},
    var_field_info{result, "type", get_metatype<TypeRef*>(), eraseFn(+[](FunctionDeclArg* v) { return &v->type; })},
  };
  result->allocate = +[]() -> void* { return new FunctionDeclArg; };
  return result;
}
template <>
metatype* metatype_type_info<FunctionDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "FunctionDecl";
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](FunctionDecl* v) { return &v->name; })},
    var_field_info{result, "result", get_metatype<TypeRef*>(), eraseFn(+[](FunctionDecl* v) { return &v->result; })},
    var_field_info{result, "args", get_metatype<std::vector<FunctionDeclArg>>(), eraseFn(+[](FunctionDecl* v) { return &v->args; })},
    var_field_info{result, "body", get_metatype<Buffer>(), eraseFn(+[](FunctionDecl* v) { return &v->body; })},
  };
  result->allocate = +[]() -> void* { return new FunctionDecl; };
  return result;
}
template <>
metatype* metatype_type_info<SwitchFunctionDecl>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "SwitchFunctionDecl";
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](SwitchFunctionDecl* v) { return &v->name; })},
    var_field_info{result, "result", get_metatype<TypeRef*>(), eraseFn(+[](SwitchFunctionDecl* v) { return &v->result; })},
    var_field_info{result, "args", get_metatype<std::vector<FunctionDeclArg>>(), eraseFn(+[](SwitchFunctionDecl* v) { return &v->args; })},
    var_field_info{result, "body", get_metatype<Buffer>(), eraseFn(+[](SwitchFunctionDecl* v) { return &v->body; })},
  };
  result->allocate = +[]() -> void* { return new SwitchFunctionDecl; };
  return result;
}
template <>
metatype* metatype_type_info<TextButtonTypeSelect>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "TextButtonTypeSelect";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::TextButtonTypeSelect);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](TextButtonTypeSelect* v) { return &v->name; })},
  };
  result->allocate = +[]() -> void* { return new TextButtonTypeSelect; };
  return result;
}
template <>
metatype* metatype_type_info<MultiLineEditState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "MultiLineEditState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::MultiLineEditState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "read_buffer", get_metatype<var_field_info*>(), eraseFn(+[](MultiLineEditState* v) { return &v->read_buffer; })},
  };
  result->allocate = +[]() -> void* { return new MultiLineEditState; };
  return result;
}
template <>
metatype* metatype_type_info<CommaForState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "CommaForState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::CommaForState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "var", get_metatype<var_field_info*>(), eraseFn(+[](CommaForState* v) { return &v->var; })},
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](CommaForState* v) { return &v->child; })},
    var_field_info{result, "comma", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](CommaForState* v) { return &v->comma; })},
  };
  result->allocate = +[]() -> void* { return new CommaForState; };
  return result;
}
template <>
metatype* metatype_type_info<ForDrawState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "ForDrawState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::ForDrawState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "var", get_metatype<var_field_info*>(), eraseFn(+[](ForDrawState* v) { return &v->var; })},
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](ForDrawState* v) { return &v->child; })},
  };
  result->allocate = +[]() -> void* { return new ForDrawState; };
  return result;
}
template <>
metatype* metatype_type_info<TypeDeclToolEditState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "TypeDeclToolEditState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::TypeDeclToolEditState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "var", get_metatype<var_field_info*>(), eraseFn(+[](TypeDeclToolEditState* v) { return &v->var; })},
    var_field_info{result, "alt_text", get_metatype<std::string>(), eraseFn(+[](TypeDeclToolEditState* v) { return &v->alt_text; })},
  };
  result->allocate = +[]() -> void* { return new TypeDeclToolEditState; };
  return result;
}
template <>
metatype* metatype_type_info<StringDrawState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "StringDrawState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::StringDrawState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "var", get_metatype<var_field_info*>(), eraseFn(+[](StringDrawState* v) { return &v->var; })},
  };
  result->allocate = +[]() -> void* { return new StringDrawState; };
  return result;
}
template <>
metatype* metatype_type_info<DrawFunctorList>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "DrawFunctorList";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::DrawFunctorList);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "children", get_metatype<std::vector<UnaryDrawFunctor*>>(), eraseFn(+[](DrawFunctorList* v) { return &v->children; })},
  };
  result->allocate = +[]() -> void* { return new DrawFunctorList; };
  return result;
}
template <>
metatype* metatype_type_info<DrawIfNotNullState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "DrawIfNotNullState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::DrawIfNotNullState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](DrawIfNotNullState* v) { return &v->child; })},
  };
  result->allocate = +[]() -> void* { return new DrawIfNotNullState; };
  return result;
}
template <>
metatype* metatype_type_info<DrawIfIsAState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "DrawIfIsAState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::DrawIfIsAState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "type", get_metatype<metatype*>(), eraseFn(+[](DrawIfIsAState* v) { return &v->type; })},
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](DrawIfIsAState* v) { return &v->child; })},
  };
  result->allocate = +[]() -> void* { return new DrawIfIsAState; };
  return result;
}
template <>
metatype* metatype_type_info<PointerBuilderState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "PointerBuilderState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::PointerBuilderState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
  };
  result->allocate = +[]() -> void* { return new PointerBuilderState; };
  return result;
}
template <>
metatype* metatype_type_info<VectorBuilderState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "VectorBuilderState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::VectorBuilderState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
  };
  result->allocate = +[]() -> void* { return new VectorBuilderState; };
  return result;
}
template <>
metatype* metatype_type_info<IndentChangeState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "IndentChangeState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::IndentChangeState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](IndentChangeState* v) { return &v->child; })},
    var_field_info{result, "indent", get_metatype<size_t>(), eraseFn(+[](IndentChangeState* v) { return &v->indent; })},
  };
  result->allocate = +[]() -> void* { return new IndentChangeState; };
  return result;
}
template <>
metatype* metatype_type_info<ColorChangeState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "ColorChangeState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::ColorChangeState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](ColorChangeState* v) { return &v->child; })},
    var_field_info{result, "color", get_metatype<gui::ColorRGB>(), eraseFn(+[](ColorChangeState* v) { return &v->color; })},
  };
  result->allocate = +[]() -> void* { return new ColorChangeState; };
  return result;
}
template <>
metatype* metatype_type_info<NewLineDrawState>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "NewLineDrawState";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::NewLineDrawState);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
  };
  result->allocate = +[]() -> void* { return new NewLineDrawState; };
  return result;
}
template <>
metatype* metatype_type_info<TextButton>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "TextButton";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::TextButton);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "button_text", get_metatype<std::string>(), eraseFn(+[](TextButton* v) { return &v->button_text; })},
    var_field_info{result, "exec_fn", get_metatype<raw_fn_ptr<void(any_ref)>>(), eraseFn(+[](TextButton* v) { return &v->exec_fn; })},
  };
  result->allocate = +[]() -> void* { return new TextButton; };
  return result;
}
template <>
metatype* metatype_type_info<NamedDrawFunctor>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "NamedDrawFunctor";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::NamedDrawFunctor);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "name", get_metatype<std::string>(), eraseFn(+[](NamedDrawFunctor* v) { return &v->name; })},
    var_field_info{result, "child", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](NamedDrawFunctor* v) { return &v->child; })},
  };
  result->allocate = +[]() -> void* { return new NamedDrawFunctor; };
  return result;
}
template <>
metatype* metatype_type_info<SwitchDrawFunctor>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "SwitchDrawFunctor";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::SwitchDrawFunctor);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "table", get_metatype<std::vector<UnaryDrawFunctor*>>(), eraseFn(+[](SwitchDrawFunctor* v) { return &v->table; })},
    var_field_info{result, "other", get_metatype<UnaryDrawFunctor*>(), eraseFn(+[](SwitchDrawFunctor* v) { return &v->other; })},
  };
  result->allocate = +[]() -> void* { return new SwitchDrawFunctor; };
  return result;
}
template <>
metatype* metatype_type_info<UnhandledTypeDrawFunctor>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "UnhandledTypeDrawFunctor";
  result->kind = static_cast<int>(UnaryDrawFunctor::Kind::UnhandledTypeDrawFunctor);
  result->group = get_metatype<UnaryDrawFunctor>();
  result->vars = std::vector<var_field_info>{
  };
  result->allocate = +[]() -> void* { return new UnhandledTypeDrawFunctor; };
  return result;
}
template <>
metatype* metatype_type_info<TypeRef>::get() {
  static type_group_metatype* result = nullptr;
  if (result) return result;
  result = new type_group_metatype;
  result->name = "TypeRef";
  result->get_kind = +[](void* v) { return static_cast<size_t>(reinterpret_cast<TypeRef*>(v)->getKind()); };
  result->load_type = +[](size_t raw_kind) -> metatype* { auto kind = static_cast<TypeRef::Kind>(raw_kind);
    switch (kind) {
      case TypeRef::Kind::StructDecl: return get_metatype<StructDecl>();
      case TypeRef::Kind::PointerDecl: return get_metatype<PointerDecl>();
      case TypeRef::Kind::ReferenceDecl: return get_metatype<ReferenceDecl>();
      case TypeRef::Kind::Void: return get_metatype<Void>();
      case TypeRef::Kind::VectorSpecialization: return get_metatype<VectorSpecialization>();
      case TypeRef::Kind::TypeGroupDecl: return get_metatype<TypeGroupDecl>();
      case TypeRef::Kind::BuiltinDecl: return get_metatype<BuiltinDecl>();
    }
  };
  return result;
}
template <>
any_ref::any_ref(TypeRef* value) : value_(value), type_(get_metatype<TypeRef>()) {
  type_ = reinterpret_cast<type_group_metatype*>(type_)->load_type(static_cast<size_t>(value->getKind()));
}
template <>
metatype* metatype_type_info<metatype>::get() {
  static type_group_metatype* result = nullptr;
  if (result) return result;
  result = new type_group_metatype;
  result->name = "metatype";
  result->get_kind = +[](void* v) { return static_cast<size_t>(reinterpret_cast<metatype*>(v)->getKind()); };
  result->load_type = +[](size_t raw_kind) -> metatype* { auto kind = static_cast<metatype::Kind>(raw_kind);
    switch (kind) {
      case metatype::Kind::struct_metatype: return get_metatype<struct_metatype>();
      case metatype::Kind::pointer_metatype: return get_metatype<pointer_metatype>();
      case metatype::Kind::vector_metatype: return get_metatype<vector_metatype>();
      case metatype::Kind::type_group_metatype: return get_metatype<type_group_metatype>();
      case metatype::Kind::builtin_metatype: return get_metatype<builtin_metatype>();
    }
  };
  return result;
}
template <>
any_ref::any_ref(metatype* value) : value_(value), type_(get_metatype<metatype>()) {
  type_ = reinterpret_cast<type_group_metatype*>(type_)->load_type(static_cast<size_t>(value->getKind()));
}
template <>
metatype* metatype_type_info<UnaryDrawFunctor>::get() {
  static type_group_metatype* result = nullptr;
  if (result) return result;
  result = new type_group_metatype;
  result->name = "UnaryDrawFunctor";
  result->get_kind = +[](void* v) { return static_cast<size_t>(reinterpret_cast<UnaryDrawFunctor*>(v)->getKind()); };
  result->load_type = +[](size_t raw_kind) -> metatype* { auto kind = static_cast<UnaryDrawFunctor::Kind>(raw_kind);
    switch (kind) {
      case UnaryDrawFunctor::Kind::ConstStringDrawState: return get_metatype<ConstStringDrawState>();
      case UnaryDrawFunctor::Kind::StringEditState: return get_metatype<StringEditState>();
      case UnaryDrawFunctor::Kind::TextButtonTypeSelect: return get_metatype<TextButtonTypeSelect>();
      case UnaryDrawFunctor::Kind::MultiLineEditState: return get_metatype<MultiLineEditState>();
      case UnaryDrawFunctor::Kind::CommaForState: return get_metatype<CommaForState>();
      case UnaryDrawFunctor::Kind::ForDrawState: return get_metatype<ForDrawState>();
      case UnaryDrawFunctor::Kind::TypeDeclToolEditState: return get_metatype<TypeDeclToolEditState>();
      case UnaryDrawFunctor::Kind::StringDrawState: return get_metatype<StringDrawState>();
      case UnaryDrawFunctor::Kind::DrawFunctorList: return get_metatype<DrawFunctorList>();
      case UnaryDrawFunctor::Kind::DrawIfNotNullState: return get_metatype<DrawIfNotNullState>();
      case UnaryDrawFunctor::Kind::DrawIfIsAState: return get_metatype<DrawIfIsAState>();
      case UnaryDrawFunctor::Kind::PointerBuilderState: return get_metatype<PointerBuilderState>();
      case UnaryDrawFunctor::Kind::VectorBuilderState: return get_metatype<VectorBuilderState>();
      case UnaryDrawFunctor::Kind::IndentChangeState: return get_metatype<IndentChangeState>();
      case UnaryDrawFunctor::Kind::ColorChangeState: return get_metatype<ColorChangeState>();
      case UnaryDrawFunctor::Kind::NewLineDrawState: return get_metatype<NewLineDrawState>();
      case UnaryDrawFunctor::Kind::TextButton: return get_metatype<TextButton>();
      case UnaryDrawFunctor::Kind::NamedDrawFunctor: return get_metatype<NamedDrawFunctor>();
      case UnaryDrawFunctor::Kind::SwitchDrawFunctor: return get_metatype<SwitchDrawFunctor>();
      case UnaryDrawFunctor::Kind::UnhandledTypeDrawFunctor: return get_metatype<UnhandledTypeDrawFunctor>();
    }
  };
  return result;
}
template <>
any_ref::any_ref(UnaryDrawFunctor* value) : value_(value), type_(get_metatype<UnaryDrawFunctor>()) {
  type_ = reinterpret_cast<type_group_metatype*>(type_)->load_type(static_cast<size_t>(value->getKind()));
}
template <>
metatype* metatype_type_info<std::string>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "std::string";
  result->typeinfo = &typeid(std::string);
  return result;
}
template <>
metatype* metatype_type_info<std::vector<StructDecl_Var>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<StructDecl_Var>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<StructDecl_Var>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<StructDecl_Var>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<StructDecl_Var>* v) { return (*v).emplace_back(); });
  return result;
}
template <>
metatype* metatype_type_info<int>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "int";
  result->typeinfo = &typeid(int);
  return result;
}
template <>
metatype* metatype_type_info<std::vector<var_field_info>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<var_field_info>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<var_field_info>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<var_field_info>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<var_field_info>* v) { return (*v).emplace_back(); });
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<void*(void*)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<void*(void*)>";
  result->typeinfo = &typeid(raw_fn_ptr<void*(void*)>);
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<void*()>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<void*()>";
  result->typeinfo = &typeid(raw_fn_ptr<void*()>);
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<size_t(void*)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<size_t(void*)>";
  result->typeinfo = &typeid(raw_fn_ptr<size_t(void*)>);
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<void*(void*, size_t)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<void*(void*, size_t)>";
  result->typeinfo = &typeid(raw_fn_ptr<void*(void*, size_t)>);
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<void(void*)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<void(void*)>";
  result->typeinfo = &typeid(raw_fn_ptr<void(void*)>);
  return result;
}
template <>
metatype* metatype_type_info<std::vector<metatype*>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<metatype*>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<metatype*>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<metatype*>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<metatype*>* v) { return (*v).emplace_back(); });
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<metatype*(size_t)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<metatype*(size_t)>";
  result->typeinfo = &typeid(raw_fn_ptr<metatype*(size_t)>);
  return result;
}
template <>
metatype* metatype_type_info<const std::type_info>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "const std::type_info";
  result->typeinfo = &typeid(const std::type_info);
  return result;
}
template <>
metatype* metatype_type_info<std::vector<FunctionDeclArg>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<FunctionDeclArg>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<FunctionDeclArg>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<FunctionDeclArg>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<FunctionDeclArg>* v) { return (*v).emplace_back(); });
  return result;
}
template <>
metatype* metatype_type_info<Buffer>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "Buffer";
  result->typeinfo = &typeid(Buffer);
  return result;
}
template <>
metatype* metatype_type_info<std::vector<UnaryDrawFunctor*>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<UnaryDrawFunctor*>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<UnaryDrawFunctor*>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<UnaryDrawFunctor*>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<UnaryDrawFunctor*>* v) { return (*v).emplace_back(); });
  return result;
}
template <>
metatype* metatype_type_info<size_t>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "size_t";
  result->typeinfo = &typeid(size_t);
  return result;
}
template <>
metatype* metatype_type_info<gui::ColorRGB>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "gui::ColorRGB";
  result->typeinfo = &typeid(gui::ColorRGB);
  return result;
}
template <>
metatype* metatype_type_info<raw_fn_ptr<void(any_ref)>>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "raw_fn_ptr<void(any_ref)>";
  result->typeinfo = &typeid(raw_fn_ptr<void(any_ref)>);
  return result;
}
void layout(ConstStringDrawState& self, tptr obj, LayoutLineState& state, CursorState& cstate) {
state.layout(self.value);
state.flush();
}
void layout(UnaryDrawFunctor& self, tptr obj, LayoutLineState& state, CursorState& cstate) {
  switch(self.getKind()) {
  case UnaryDrawFunctor::Kind::ConstStringDrawState: return layout(reinterpret_cast<ConstStringDrawState&>(self), obj, state, cstate);
  default: {

  }
  }
}
