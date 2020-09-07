#pragma once

#include <vector>
#include <string>
#include "rules/template-support.h"
#include "notes/type-support.h"
#include "notes/type-info.h"
#include "notes/gui-support.h"
#include "gui/buffer-view.h"

struct ConstStringDrawState;
struct StructDecl_Var;
struct StructDecl;
struct PointerDecl;
struct ReferenceDecl;
struct Void;
struct VectorSpecialization;
struct TypeGroupDecl;
struct BuiltinDecl;
struct struct_metatype;
struct var_field_info;
struct pointer_metatype;
struct vector_metatype;
struct type_group_metatype;
struct builtin_metatype;
struct StringEditState;
struct FunctionDeclArg;
struct FunctionDecl;
struct SwitchFunctionDecl;
struct TextButtonTypeSelect;
struct MultiLineEditState;
struct CommaForState;
struct ForDrawState;
struct TypeDeclToolEditState;
struct StringDrawState;
struct DrawFunctorList;
struct DrawIfNotNullState;
struct DrawIfIsAState;
struct PointerBuilderState;
struct VectorBuilderState;
struct IndentChangeState;
struct ColorChangeState;
struct NewLineDrawState;
struct TextButton;
struct NamedDrawFunctor;
struct SwitchDrawFunctor;
struct UnhandledTypeDrawFunctor;
struct TypeRef{
  enum class Kind {
    StructDecl,
    PointerDecl,
    ReferenceDecl,
    Void,
    VectorSpecialization,
    TypeGroupDecl,
    BuiltinDecl,
  };
  explicit TypeRef(Kind kind) : kind_(kind) {}
Kind getKind() { return kind_; }
private:
Kind kind_;
};
struct metatype{
  enum class Kind {
    struct_metatype,
    pointer_metatype,
    vector_metatype,
    type_group_metatype,
    builtin_metatype,
  };
  explicit metatype(Kind kind) : kind_(kind) {}
Kind getKind() { return kind_; }
private:
Kind kind_;
};
struct UnaryDrawFunctor{
  enum class Kind {
    ConstStringDrawState,
    StringEditState,
    TextButtonTypeSelect,
    MultiLineEditState,
    CommaForState,
    ForDrawState,
    TypeDeclToolEditState,
    StringDrawState,
    DrawFunctorList,
    DrawIfNotNullState,
    DrawIfIsAState,
    PointerBuilderState,
    VectorBuilderState,
    IndentChangeState,
    ColorChangeState,
    NewLineDrawState,
    TextButton,
    NamedDrawFunctor,
    SwitchDrawFunctor,
    UnhandledTypeDrawFunctor,
  };
  explicit UnaryDrawFunctor(Kind kind) : kind_(kind) {}
Kind getKind() { return kind_; }
private:
Kind kind_;
};
struct ConstStringDrawState : public UnaryDrawFunctor {
  ConstStringDrawState() : UnaryDrawFunctor(Kind::ConstStringDrawState) {}
  std::string value;
};
struct StructDecl_Var {
  std::string name;
  TypeRef* type = nullptr;
};
struct StructDecl : public TypeRef {
  StructDecl() : TypeRef(Kind::StructDecl) {}
  std::string name;
  TypeRef* group = nullptr;
  std::vector<StructDecl_Var> vars;
};
struct PointerDecl : public TypeRef {
  PointerDecl() : TypeRef(Kind::PointerDecl) {}
  TypeRef* pointee = nullptr;
};
struct ReferenceDecl : public TypeRef {
  ReferenceDecl() : TypeRef(Kind::ReferenceDecl) {}
  TypeRef* wrapping = nullptr;
};
struct Void : public TypeRef {
  Void() : TypeRef(Kind::Void) {}
};
struct VectorSpecialization : public TypeRef {
  VectorSpecialization() : TypeRef(Kind::VectorSpecialization) {}
  TypeRef* element = nullptr;
};
struct TypeGroupDecl : public TypeRef {
  TypeGroupDecl() : TypeRef(Kind::TypeGroupDecl) {}
  std::string name;
};
struct BuiltinDecl : public TypeRef {
  BuiltinDecl() : TypeRef(Kind::BuiltinDecl) {}
  std::string name;
};
struct struct_metatype : public metatype {
  struct_metatype() : metatype(Kind::struct_metatype) {}
  std::string name;
  metatype* group = nullptr;
  int kind;
  std::vector<var_field_info> vars;
  raw_fn_ptr<void*()> allocate;
};
struct var_field_info {
  struct_metatype* base = nullptr;
  std::string name;
  metatype* type = nullptr;
  raw_fn_ptr<void*(void*)> fetch_var;
};
struct pointer_metatype : public metatype {
  pointer_metatype() : metatype(Kind::pointer_metatype) {}
  metatype* pointee = nullptr;
};
struct vector_metatype : public metatype {
  vector_metatype() : metatype(Kind::vector_metatype) {}
  metatype* element = nullptr;
  raw_fn_ptr<size_t(void*)> get_size;
  raw_fn_ptr<void*(void*, size_t)> get_element;
  raw_fn_ptr<void(void*)> emplace_back;
};
struct type_group_metatype : public metatype {
  type_group_metatype() : metatype(Kind::type_group_metatype) {}
  std::string name;
  std::vector<metatype*> children;
  raw_fn_ptr<size_t(void*)> get_kind;
  raw_fn_ptr<metatype*(size_t)> load_type;
};
struct builtin_metatype : public metatype {
  builtin_metatype() : metatype(Kind::builtin_metatype) {}
  std::string name;
  const std::type_info* typeinfo = nullptr;
};
struct StringEditState : public UnaryDrawFunctor {
  StringEditState() : UnaryDrawFunctor(Kind::StringEditState) {}
  var_field_info* var = nullptr;
  std::string alt_text;
};
struct FunctionDeclArg {
  std::string name;
  TypeRef* type = nullptr;
};
struct FunctionDecl {
  std::string name;
  TypeRef* result = nullptr;
  std::vector<FunctionDeclArg> args;
  Buffer body;
};
struct SwitchFunctionDecl {
  std::string name;
  TypeRef* result = nullptr;
  std::vector<FunctionDeclArg> args;
  Buffer body;
};
struct TextButtonTypeSelect : public UnaryDrawFunctor {
  TextButtonTypeSelect() : UnaryDrawFunctor(Kind::TextButtonTypeSelect) {}
  std::string name;
};
struct MultiLineEditState : public UnaryDrawFunctor {
  MultiLineEditState() : UnaryDrawFunctor(Kind::MultiLineEditState) {}
  var_field_info* read_buffer = nullptr;
};
struct CommaForState : public UnaryDrawFunctor {
  CommaForState() : UnaryDrawFunctor(Kind::CommaForState) {}
  var_field_info* var = nullptr;
  UnaryDrawFunctor* child = nullptr;
  UnaryDrawFunctor* comma = nullptr;
};
struct ForDrawState : public UnaryDrawFunctor {
  ForDrawState() : UnaryDrawFunctor(Kind::ForDrawState) {}
  var_field_info* var = nullptr;
  UnaryDrawFunctor* child = nullptr;
};
struct TypeDeclToolEditState : public UnaryDrawFunctor {
  TypeDeclToolEditState() : UnaryDrawFunctor(Kind::TypeDeclToolEditState) {}
  var_field_info* var = nullptr;
  std::string alt_text;
};
struct StringDrawState : public UnaryDrawFunctor {
  StringDrawState() : UnaryDrawFunctor(Kind::StringDrawState) {}
  var_field_info* var = nullptr;
};
struct DrawFunctorList : public UnaryDrawFunctor {
  DrawFunctorList() : UnaryDrawFunctor(Kind::DrawFunctorList) {}
  std::vector<UnaryDrawFunctor*> children;
};
struct DrawIfNotNullState : public UnaryDrawFunctor {
  DrawIfNotNullState() : UnaryDrawFunctor(Kind::DrawIfNotNullState) {}
  UnaryDrawFunctor* child = nullptr;
};
struct DrawIfIsAState : public UnaryDrawFunctor {
  DrawIfIsAState() : UnaryDrawFunctor(Kind::DrawIfIsAState) {}
  metatype* type = nullptr;
  UnaryDrawFunctor* child = nullptr;
};
struct PointerBuilderState : public UnaryDrawFunctor {
  PointerBuilderState() : UnaryDrawFunctor(Kind::PointerBuilderState) {}
};
struct VectorBuilderState : public UnaryDrawFunctor {
  VectorBuilderState() : UnaryDrawFunctor(Kind::VectorBuilderState) {}
};
struct IndentChangeState : public UnaryDrawFunctor {
  IndentChangeState() : UnaryDrawFunctor(Kind::IndentChangeState) {}
  UnaryDrawFunctor* child = nullptr;
  size_t indent;
};
struct ColorChangeState : public UnaryDrawFunctor {
  ColorChangeState() : UnaryDrawFunctor(Kind::ColorChangeState) {}
  UnaryDrawFunctor* child = nullptr;
  gui::ColorRGB color;
};
struct NewLineDrawState : public UnaryDrawFunctor {
  NewLineDrawState() : UnaryDrawFunctor(Kind::NewLineDrawState) {}
};
struct TextButton : public UnaryDrawFunctor {
  TextButton() : UnaryDrawFunctor(Kind::TextButton) {}
  std::string button_text;
  raw_fn_ptr<void(any_ref)> exec_fn;
};
struct NamedDrawFunctor : public UnaryDrawFunctor {
  NamedDrawFunctor() : UnaryDrawFunctor(Kind::NamedDrawFunctor) {}
  std::string name;
  UnaryDrawFunctor* child = nullptr;
};
struct SwitchDrawFunctor : public UnaryDrawFunctor {
  SwitchDrawFunctor() : UnaryDrawFunctor(Kind::SwitchDrawFunctor) {}
  std::vector<UnaryDrawFunctor*> table;
  UnaryDrawFunctor* other = nullptr;
};
struct UnhandledTypeDrawFunctor : public UnaryDrawFunctor {
  UnhandledTypeDrawFunctor() : UnaryDrawFunctor(Kind::UnhandledTypeDrawFunctor) {}
};
void layout(ConstStringDrawState& self, tptr obj, LayoutLineState& state, CursorState& cstate);
void layout(UnaryDrawFunctor& self, tptr obj, LayoutLineState& state, CursorState& cstate);
