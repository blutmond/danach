#include "gui/so-handle.h"
#include "gui/widget-helper.h"
#include "gui/so-handoff-lib.h"
#include "gui/buffer-view.h"
#include "parser/ast-context.h"
#include "rules/template-support.h"
#include "notes/generated.h"
#include "notes/callable.h"
#include "notes/type-info.h"
#include "notes/serialize.h"
#include "notes/gui-support.h"

#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <stdlib.h>

void draw_entry_with_alt(DrawLineState& state, std::string& str, CursorState& cstate,
                         string_view alt_text) {
  state.flush();
  auto* cursor = dynamic_cast<Cursor*>(cstate.active.get());
  if (cursor && &cursor->ref == &str) {
    state.layout(str);
    auto pos = cursor->pos;
    if (pos >= str.size()) state.layout(" ");
    FlushLineWithCursor(state.cr, state.pt, state.font, state.glyphs, pos);
    state.glyphs.clear();
  } else if (!str.empty()) {
    state.layout(str);
    state.flush();
  } else {
    state.cr.save();
    state.cr.set_source_rgb(0.8, 0.8, 0.8);
    state.layout(alt_text);
    state.flush();
    state.cr.restore();
  }
}

bool TestInside(gui::Point st, LayoutLineState& state, GdkEventButton* button) {
  return gui::TestInside({st, {int(state.pt.x - st.x), int(state.font->height())}}, {button->x, button->y});
}

bool TestInside(LayoutLineState& state, string_view text, GdkEventButton* button) {
  auto st = state.pt;
  state.layout(text);
  bool result = TestInside(st, state, button);
  state.flush();
  return result;
}

size_t GetClickPos(LayoutLineState& state, double x, size_t max_count) {
  size_t i = 0;
  size_t count = std::min(state.glyphs.size(), max_count + 1) - 1;
  for (; i < count; ++i) {
    if (x < state.glyphs[i + 1].x) break;
  }
  return i;
}

void button_press_with_alt(LayoutLineState& state, std::string& str,
                           GdkEventButton* button, CursorState& cstate,
                           string_view alt_text) {
  state.flush();
  auto st = state.pt;
  auto* cursor = dynamic_cast<Cursor*>(cstate.active.get());
  if (cursor && &cursor->ref == &str) {
    state.layout(str);
    auto pos = cursor->pos;
    if (pos >= str.size()) state.layout(" ");
  } else if (!str.empty()) {
    state.layout(str);
  } else {
    state.layout(alt_text);
  }
  if (TestInside(st, state, button)) {
    size_t i = GetClickPos(state, button->x, str.size());
    cstate.active = std::make_unique<Cursor>(Cursor{str, i});
  }
  state.flush();
}


any_ref array_index(any_ref arr, size_t index) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  return any_ref(ptr->get_element(arr.raw_pointer(), index), ptr->element);
}

size_t array_size(any_ref arr) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  return ptr->get_size(arr.raw_pointer());
}

any_ref vector_append_back(any_ref arr) {
  auto* ptr = dyn_cast<vector_metatype>(&arr.type());
  if (ptr == nullptr) throw TypeMismatchException("expected array");
  size_t idx = ptr->get_size(arr.raw_pointer());
  ptr->emplace_back(arr.raw_pointer());
  return any_ref(ptr->get_element(arr.raw_pointer(), idx), ptr->element);
}

void do_pointer_assign(any_ref ptr, any_ref value) {
  auto* ptrt = dyn_cast<pointer_metatype>(&ptr.type());
  if (ptrt == nullptr) throw TypeMismatchException("expected pointer");
  assert_compatible(ptrt->pointee, &value.type());
  *reinterpret_cast<void**>(ptr.raw_pointer()) = value.raw_pointer();
}

any_ref dereference(any_ref ptr) {
  auto* ptrt = dyn_cast<pointer_metatype>(&ptr.type());
  if (ptrt == nullptr) throw TypeMismatchException("expected pointer");
  void* tmp = *reinterpret_cast<void**>(ptr.raw_pointer());
  auto* group_type = dyn_cast<type_group_metatype>(ptrt->pointee);
  if (group_type && tmp) {
    return any_ref(tmp, group_type->load_type(group_type->get_kind(tmp)));
  } else {
    return any_ref(*reinterpret_cast<void**>(ptr.raw_pointer()), ptrt->pointee);
  }
}

struct builtin_record {
  std::string name;
  any_ref data;
};
std::vector<builtin_record>& get_builtins();

std::string& lookup_builtin_name(void* builtin) {
  static const auto index = [] {
    std::unordered_map<void*, std::string*> result;
    for (auto& record : get_builtins()) result[*reinterpret_cast<void**>(record.data.raw_pointer())] = &record.name;
    return result;
  }();
  auto it = index.find(builtin);
  if (it == index.end()) {
    std::cerr << "problem finding builtin\n";
    exit(-1);
  }
  return *it->second;
}
any_ref lookup_builtin(const std::string name) {
  static const auto index = [] {
    std::unordered_map<std::string, any_ref> result;
    for (auto& record : get_builtins()) result.emplace(record.name, record.data);
    return result;
  }();
  auto it = index.find(name);
  if (it == index.end()) {
    std::cerr << "problem finding builtin: " << name << "\n";
    exit(-1);
  }
  return it->second;
}

class TypeDeclTool : public Tool {
 public:
  TypeRef*& type;
  TypeDeclTool(TypeRef*& type) : type(type) {}

  void FoundTypeRef(TypeRef* ref) override {
    type = ref;
  }
};

TypeRef* make_ref(TypeRef* t) {
  auto* result = new ReferenceDecl;
  result->wrapping = t;
  return result;
}

TypeRef* make_ptr(TypeRef* t) {
  auto* result = new PointerDecl;
  result->pointee = t;
  return result;
}

void debug_draw(ConstStringDrawState& self, LayoutLineState& state) {
  state.layout("const \"");
  state.layout(self.value);
  state.layout("\"");
  state.flush();
  state.nl();
}

void draw(UnaryDrawFunctor& self, tptr obj, DrawLineState& state, CursorState& cstate) {
  switch (self.getKind()) {
  default: {
    layout(self, obj, state, cstate);
  } 
  }
}

void type_ref_layout(TypeRef* type, LayoutLineState& state);
void type_ref_layout(StructDecl* type, LayoutLineState& state) { state.layout(type->name); }
void type_ref_layout(BuiltinDecl* type, LayoutLineState& state) { state.layout(type->name); }
void type_ref_layout(TypeGroupDecl* type, LayoutLineState& state) { state.layout(type->name); }
void type_ref_layout(ReferenceDecl* type, LayoutLineState& state) { state.layout("ref<"); type_ref_layout(type->wrapping, state); state.layout(">"); }
void type_ref_layout(VectorSpecialization* type, LayoutLineState& state) { state.layout("vector<"); type_ref_layout(type->element, state); state.layout(">"); }
void type_ref_layout(PointerDecl* type, LayoutLineState& state)  { state.layout("ptr<"); type_ref_layout(type->pointee, state); state.layout(">"); }
void type_ref_layout(Void* type, LayoutLineState& state) { state.layout("void"); }

void type_ref_layout(TypeRef* type, LayoutLineState& state) {
  switch (type->getKind()) {
  case TypeRef::Kind::StructDecl: return type_ref_layout(reinterpret_cast<StructDecl*>(type), state);
  case TypeRef::Kind::BuiltinDecl: return type_ref_layout(reinterpret_cast<BuiltinDecl*>(type), state);
  case TypeRef::Kind::TypeGroupDecl: return type_ref_layout(reinterpret_cast<TypeGroupDecl*>(type), state);
  case TypeRef::Kind::ReferenceDecl: return type_ref_layout(reinterpret_cast<ReferenceDecl*>(type), state);
  case TypeRef::Kind::VectorSpecialization: return type_ref_layout(reinterpret_cast<VectorSpecialization*>(type), state);
  case TypeRef::Kind::PointerDecl: return type_ref_layout(reinterpret_cast<PointerDecl*>(type), state);
  case TypeRef::Kind::Void: return type_ref_layout(reinterpret_cast<Void*>(type), state);
  }
}

void draw_metatype_ref(metatype* t, DrawLineState& state, CursorState& cstate);
void draw_metatype_ref(struct_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout(t->name);
}
void draw_metatype_ref(pointer_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("ptr<");
  draw_metatype_ref(t->pointee, state, cstate); 
  state.layout(">");
}
void draw_metatype_ref(vector_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("vector<");
  draw_metatype_ref(t->element, state, cstate); 
  state.layout(">");
}
void draw_metatype_ref(type_group_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout(t->name);
}
void draw_metatype_ref(builtin_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout(t->name);
}

void draw_metatype_ref(metatype* t, DrawLineState& state, CursorState& cstate) {
  switch (t->getKind()) {
  case metatype::Kind::struct_metatype: return draw_metatype_ref(reinterpret_cast<struct_metatype*>(t), state, cstate);
  case metatype::Kind::pointer_metatype: return draw_metatype_ref(reinterpret_cast<pointer_metatype*>(t), state, cstate);
  case metatype::Kind::vector_metatype: return draw_metatype_ref(reinterpret_cast<vector_metatype*>(t), state, cstate);
  case metatype::Kind::type_group_metatype: return draw_metatype_ref(reinterpret_cast<type_group_metatype*>(t), state, cstate);
  case metatype::Kind::builtin_metatype: return draw_metatype_ref(reinterpret_cast<builtin_metatype*>(t), state, cstate);
  }
}

void emit_metatype(std::ostream& stream, metatype& t);

void emit_metatype(std::ostream& stream, struct_metatype& t) { stream << t.name; }
void emit_metatype(std::ostream& stream, pointer_metatype& t) { stream << "ptr<"; emit_metatype(stream, *t.pointee); stream << ">"; }
void emit_metatype(std::ostream& stream, vector_metatype& t) { stream << "vector<"; emit_metatype(stream, *t.element); stream << ">"; }
void emit_metatype(std::ostream& stream, type_group_metatype& t) { stream << t.name; }
void emit_metatype(std::ostream& stream, builtin_metatype& t) { stream << t.name; }

void emit_metatype(std::ostream& stream, metatype& t) {
  switch (t.getKind()) {
  case metatype::Kind::struct_metatype: return emit_metatype(stream, reinterpret_cast<struct_metatype&>(t));
  case metatype::Kind::pointer_metatype: return emit_metatype(stream, reinterpret_cast<pointer_metatype&>(t));
  case metatype::Kind::vector_metatype: return emit_metatype(stream, reinterpret_cast<vector_metatype&>(t));
  case metatype::Kind::type_group_metatype: return emit_metatype(stream, reinterpret_cast<type_group_metatype&>(t));
  case metatype::Kind::builtin_metatype: return emit_metatype(stream, reinterpret_cast<builtin_metatype&>(t));
  }
}

void assert_compatible(metatype* expected, metatype* actual) {
  if (is_compatible(expected, actual)) return;
  EmitStream error_stream;
  error_stream.stream() << "expected: ";
  emit_metatype(error_stream.stream(), *expected);
  error_stream.stream() << " got: ";
  emit_metatype(error_stream.stream(), *actual);
  throw TypeMismatchException(error_stream.get());
}
bool is_compatible(metatype* expected, metatype* actual) {
  if (expected == actual) return true;
  if (auto* actual_struct = dyn_cast<struct_metatype>(actual)) {
    if (actual_struct->group == expected) return true;
  }
  return false;
}

any_ref any_ref::operator[](var_field_info& var) {
  assert_compatible(type_, var.base);
  return any_ref(var.fetch_var(value_), var.type);
}

void draw_view_decl(UnaryDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate);
void button_press(UnaryDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate);

void draw_view_decl(StringEditState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  draw_entry_with_alt(state, obj[*self->var].get<std::string>(), cstate, self->alt_text);
}
void button_press(StringEditState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  button_press_with_alt(state, obj[*self->var].get<std::string>(), button, cstate, self->alt_text);
}

void draw_view_decl(ConstStringDrawState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.layout(self->value);
  state.flush();
}
void button_press(ConstStringDrawState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  state.layout(self->value);
  state.flush();
}

void draw_view_decl(TextButtonTypeSelect* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.layout(self->name);
  state.flush();
}
void button_press(TextButtonTypeSelect* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  if (TestInside(state, self->name, button) && cstate.active) {
    cstate.active->FoundPointerType(obj);
  }
}

void draw_view_decl(MultiLineEditState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  auto& buffer = obj[*self->read_buffer].get<Buffer>();
  DrawBuffer(state.cr, state.pt, buffer, cstate.active ? cstate.active->GetCursor(buffer) : nullptr);
  state.pt.y += state.font->height() * buffer.lines.size();
}
void button_press(MultiLineEditState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto& buffer = obj[*self->read_buffer].get<Buffer>();
  BufferPos cursor;
  if (button->y >= state.pt.y && FindPos(state.pt, {button->x, button->y}, state.font, cursor, buffer)) {
    cstate.active = std::make_unique<BufferCursor>(buffer, cursor);
  }
}

void draw_view_decl(CommaForState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  any_ref arr = obj[*self->var];
  size_t count = array_size(arr);
  for (size_t i = 0; i < count; ++i) {
    if (i != 0) draw_view_decl(self->comma, obj, state, cstate);
    draw_view_decl(self->child, array_index(arr, i), state, cstate);
  }
}
void button_press(CommaForState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  any_ref arr = obj[*self->var];
  size_t count = array_size(arr);
  for (size_t i = 0; i < count; ++i) {
    if (i != 0) button_press(self->comma, obj, button, state, cstate);
    button_press(self->child, array_index(arr, i), button, state, cstate);
  }
}

void draw_view_decl(ForDrawState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  any_ref arr = obj[*self->var];
  size_t count = array_size(arr);
  for (size_t i = 0; i < count; ++i) {
    draw_view_decl(self->child, array_index(arr, i), state, cstate);
  }
}
void button_press(ForDrawState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  any_ref arr = obj[*self->var];
  size_t count = array_size(arr);
  for (size_t i = 0; i < count; ++i) {
    button_press(self->child, array_index(arr, i), button, state, cstate);
  }
}

void draw_view_decl(TypeDeclToolEditState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  auto& value = obj[*self->var].get<TypeRef*>();
  if (value) type_ref_layout(value, state);
  else state.layout(self->alt_text);
  state.flush();
}
void button_press(TypeDeclToolEditState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto st = state.pt;
  auto& value = obj[*self->var].get<TypeRef*>();
  if (value) type_ref_layout(value, state);
  else state.layout(self->alt_text);
  if (TestInside(st, state, button)) {
    cstate.active = std::make_unique<TypeDeclTool>(value);
  }
  state.flush();
}

void draw_view_decl(StringDrawState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.layout(obj[*self->var].get<std::string>());
  state.flush();
}
void button_press(StringDrawState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  state.layout(obj[*self->var].get<std::string>());
  state.flush();
}

void draw_view_decl(DrawFunctorList* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  for (auto* child : self->children) draw_view_decl(child, obj, state, cstate);
}
void button_press(DrawFunctorList* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  for (auto* child : self->children) button_press(child, obj, button, state, cstate);
}

void draw_view_decl(DrawIfNotNullState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  if (obj.raw_pointer() != nullptr) draw_view_decl(self->child, obj, state, cstate);
}
void button_press(DrawIfNotNullState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  if (obj.raw_pointer() != nullptr) button_press(self->child, obj, button, state, cstate);
}

void draw_view_decl(DrawIfIsAState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  if (&obj.type() == self->type) draw_view_decl(self->child, obj, state, cstate);
}
void button_press(DrawIfIsAState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  if (&obj.type() == self->type) button_press(self->child, obj, button, state, cstate);
}

class PointerTypeDeclTool : public Tool {
 public:
  std::unique_ptr<GenericCursor> other_tool;
  PointerTypeDeclTool(std::unique_ptr<GenericCursor> other_tool) : other_tool(std::move(other_tool)) {}

  void FoundTypeRef(TypeRef* ref) override {
    other_tool->FoundTypeRef(make_ptr(ref));
  }
};
void draw_view_decl(PointerBuilderState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.layout("pointer builder");
  state.flush();
}
void button_press(PointerBuilderState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto st = state.pt;
  state.layout("pointer builder");
  if (TestInside(st, state, button)) {
    if (cstate.active) {
      cstate.active = std::make_unique<PointerTypeDeclTool>(std::move(cstate.active));
    }
  }
  state.flush();
}

class VectorTypeDeclTool : public Tool {
 public:
  std::unique_ptr<GenericCursor> other_tool;
  VectorTypeDeclTool(std::unique_ptr<GenericCursor> other_tool) : other_tool(std::move(other_tool)) {}

  void FoundTypeRef(TypeRef* ref) override {
    auto* res = new VectorSpecialization;
    res->element = ref;
    other_tool->FoundTypeRef(res);
  }
};
void draw_view_decl(VectorBuilderState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.layout("vector builder");
  state.flush();
}
void button_press(VectorBuilderState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto st = state.pt;
  state.layout("vector builder");
  if (TestInside(st, state, button)) {
    if (cstate.active) {
      cstate.active = std::make_unique<VectorTypeDeclTool>(std::move(cstate.active));
    }
  }
  state.flush();
}

void draw_view_decl(IndentChangeState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  auto reset = state.reset_point;
  state.reset_point += self->indent;
  if (state.just_reset) state.pt.x = state.reset_point;
  draw_view_decl(self->child, obj, state, cstate);
  state.reset_point = reset;
  if (state.just_reset) state.pt.x = reset;
}
void button_press(IndentChangeState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto reset = state.reset_point;
  state.reset_point += self->indent;
  if (state.just_reset) state.pt.x = reset;
  button_press(self->child, obj, button, state, cstate);
  state.reset_point = reset;
  if (state.just_reset) state.pt.x = reset;
}

void draw_view_decl(ColorChangeState* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.cr.save();
  state.cr.set_source_rgb(self->color);
  draw_view_decl(self->child, obj, state, cstate);
  state.cr.restore();
}
void button_press(ColorChangeState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  button_press(self->child, obj, button, state, cstate);
}

void draw_view_decl(NewLineDrawState* self, any_ref obj, DrawLineState& state, CursorState& cstate) { state.nl(); }
void button_press(NewLineDrawState* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) { state.nl(); }

void draw_view_decl(TextButton* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  state.cr.save();
  state.cr.set_source_rgb(0.8, 0.8, 0.8);
  state.layout(self->button_text);
  state.flush();
  state.nl();
  state.cr.restore();
}
void button_press(TextButton* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  if (TestInside(state, self->button_text, button)) self->exec_fn(obj);
  state.nl();
}

void draw_view_decl(NamedDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  draw_view_decl(self->child, obj, state, cstate);
}
void button_press(NamedDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  button_press(self->child, obj, button, state, cstate);
}

void draw_view_decl(SwitchDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  auto* type = dyn_cast<struct_metatype>(&obj.type());
  if (type) {
    size_t kind = type->kind;
    if (kind < self->table.size() && self->table[kind])
      return draw_view_decl(self->table[kind], obj, state, cstate);
  }
  if (self->other) draw_view_decl(self->other, obj, state, cstate);
}
void button_press(SwitchDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  auto* type = dyn_cast<struct_metatype>(&obj.type());
  if (type) {
    size_t kind = type->kind;
    if (kind < self->table.size() && self->table[kind])
      return button_press(self->table[kind], obj, button, state, cstate);
  }
  if (self->other) return button_press(self->other, obj, button, state, cstate);
}

void draw_view_decl(UnhandledTypeDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  EmitStream h;
  state.layout("Unknown type: ");
  emit_metatype(h.stream(), obj.type());
  state.layout(h.get());
  state.flush();
  state.nl();
}
void button_press(UnhandledTypeDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  EmitStream h;
  state.layout("Unknown type: ");
  emit_metatype(h.stream(), obj.type());
  state.layout(h.get());
  state.flush();
  state.nl();
}
/*
void draw_view_decl(* self, any_ref obj, DrawLineState& state, CursorState& cstate) {}
void button_press(* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {}
*/

void draw_view_decl(UnaryDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate) {
  switch (self->getKind()) {
  case UnaryDrawFunctor::Kind::StringEditState: return draw_view_decl(reinterpret_cast<StringEditState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::ConstStringDrawState: return draw_view_decl(reinterpret_cast<ConstStringDrawState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::TextButtonTypeSelect: return draw_view_decl(reinterpret_cast<TextButtonTypeSelect*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::MultiLineEditState: return draw_view_decl(reinterpret_cast<MultiLineEditState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::CommaForState: return draw_view_decl(reinterpret_cast<CommaForState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::ForDrawState: return draw_view_decl(reinterpret_cast<ForDrawState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::TypeDeclToolEditState: return draw_view_decl(reinterpret_cast<TypeDeclToolEditState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::StringDrawState: return draw_view_decl(reinterpret_cast<StringDrawState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::DrawFunctorList: return draw_view_decl(reinterpret_cast<DrawFunctorList*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::DrawIfNotNullState: return draw_view_decl(reinterpret_cast<DrawIfNotNullState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::DrawIfIsAState: return draw_view_decl(reinterpret_cast<DrawIfIsAState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::PointerBuilderState: return draw_view_decl(reinterpret_cast<PointerBuilderState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::VectorBuilderState: return draw_view_decl(reinterpret_cast<VectorBuilderState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::IndentChangeState: return draw_view_decl(reinterpret_cast<IndentChangeState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::ColorChangeState: return draw_view_decl(reinterpret_cast<ColorChangeState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::NewLineDrawState: return draw_view_decl(reinterpret_cast<NewLineDrawState*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::TextButton: return draw_view_decl(reinterpret_cast<TextButton*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::NamedDrawFunctor: return draw_view_decl(reinterpret_cast<NamedDrawFunctor*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::SwitchDrawFunctor: return draw_view_decl(reinterpret_cast<SwitchDrawFunctor*>(self), obj, state, cstate);
  case UnaryDrawFunctor::Kind::UnhandledTypeDrawFunctor: return draw_view_decl(reinterpret_cast<UnhandledTypeDrawFunctor*>(self), obj, state, cstate);
  }
}

void button_press(UnaryDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) {
  switch (self->getKind()) {
  case UnaryDrawFunctor::Kind::StringEditState: return button_press(reinterpret_cast<StringEditState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::ConstStringDrawState: return button_press(reinterpret_cast<ConstStringDrawState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::TextButtonTypeSelect: return button_press(reinterpret_cast<TextButtonTypeSelect*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::MultiLineEditState: return button_press(reinterpret_cast<MultiLineEditState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::CommaForState: return button_press(reinterpret_cast<CommaForState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::ForDrawState: return button_press(reinterpret_cast<ForDrawState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::TypeDeclToolEditState: return button_press(reinterpret_cast<TypeDeclToolEditState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::StringDrawState: return button_press(reinterpret_cast<StringDrawState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::DrawFunctorList: return button_press(reinterpret_cast<DrawFunctorList*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::DrawIfNotNullState: return button_press(reinterpret_cast<DrawIfNotNullState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::DrawIfIsAState: return button_press(reinterpret_cast<DrawIfIsAState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::PointerBuilderState: return button_press(reinterpret_cast<PointerBuilderState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::VectorBuilderState: return button_press(reinterpret_cast<VectorBuilderState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::IndentChangeState: return button_press(reinterpret_cast<IndentChangeState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::ColorChangeState: return button_press(reinterpret_cast<ColorChangeState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::NewLineDrawState: return button_press(reinterpret_cast<NewLineDrawState*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::TextButton: return button_press(reinterpret_cast<TextButton*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::NamedDrawFunctor: return button_press(reinterpret_cast<NamedDrawFunctor*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::SwitchDrawFunctor: return button_press(reinterpret_cast<SwitchDrawFunctor*>(self), obj, button, state, cstate);
  case UnaryDrawFunctor::Kind::UnhandledTypeDrawFunctor: return button_press(reinterpret_cast<UnhandledTypeDrawFunctor*>(self), obj, button, state, cstate);
  }
}

void draw_metatype_decl(struct_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("struct");
  state.layout(" ");
  state.layout(t->name);
  if (t->group) {
    state.layout(" : ");
    draw_metatype_ref(t->group, state, cstate);
  }
  state.flush();
  state.nl();
  state.pt.x += 20;
  ResetScope scope(state);
  for (auto& var: t->vars) {
    state.layout("var");
    state.layout(" ");
    state.layout(var.name);
    state.layout(" : ");
    draw_metatype_ref(var.type, state, cstate);
    state.nl();
    state.flush();
  }
}
void draw_metatype_decl(pointer_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("typealias ");
  draw_metatype_ref(t, state, cstate); 
  state.nl();
  state.flush();
}
void draw_metatype_decl(vector_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("typealias ");
  draw_metatype_ref(t, state, cstate); 
  state.nl();
  state.flush();
}
void draw_metatype_decl(type_group_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("type_group ");
  state.layout(t->name);
  state.nl();
  state.flush();
}
void draw_metatype_decl(builtin_metatype* t, DrawLineState& state, CursorState& cstate) {
  state.layout("builtin ");
  state.layout(t->name);
  state.nl();
  state.flush();
}

void draw_metatype_decl(metatype* t, DrawLineState& state, CursorState& cstate) {
  switch (t->getKind()) {
  case metatype::Kind::struct_metatype: return draw_metatype_decl(reinterpret_cast<struct_metatype*>(t), state, cstate);
  case metatype::Kind::pointer_metatype: return draw_metatype_decl(reinterpret_cast<pointer_metatype*>(t), state, cstate);
  case metatype::Kind::vector_metatype: return draw_metatype_decl(reinterpret_cast<vector_metatype*>(t), state, cstate);
  case metatype::Kind::type_group_metatype: return draw_metatype_decl(reinterpret_cast<type_group_metatype*>(t), state, cstate);
  case metatype::Kind::builtin_metatype: return draw_metatype_decl(reinterpret_cast<builtin_metatype*>(t), state, cstate);
  }
}

class ThingDraw {
 public:
  virtual ~ThingDraw() {}

  virtual void draw(tptr obj, DrawLineState& state,
                    CursorState& cstate) { layout(obj, state, cstate); }
  virtual void layout(tptr obj, LayoutLineState& state,
                    CursorState& cstate) {}
  virtual void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) { layout(obj, state, cstate); }

  virtual void debug_draw(LayoutLineState& state) = 0;

  virtual UnaryDrawFunctor* convert() = 0;
};

class ConstStringDraw : public ThingDraw {
 public:
  ConstStringDraw(std::string value) : value(std::move(value)) {}
  std::string value;
  void layout(tptr obj, LayoutLineState& state, CursorState& cstate) override {
    state.layout(value);
    state.flush();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("const \"");
    state.layout(value);
    state.layout("\"");
    state.flush();
    state.nl();
  }
  UnaryDrawFunctor* convert() override { auto* result = new ConstStringDrawState;
    result->value = value;
    return result;
  }
};

class DrawIfNotNull : public ThingDraw {
 public:
  ThingDraw* child;
  DrawIfNotNull(ThingDraw* child) : child(child) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    if (obj.raw_pointer() != nullptr) child->draw(obj, state, cstate);
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("if not null: ");
    state.flush();
    child->debug_draw(state);
  }
  UnaryDrawFunctor* convert() override { auto* result = new DrawIfNotNullState;
    result->child = child->convert();
    return result;
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                   CursorState& cstate) override { if (obj.raw_pointer() != nullptr) child->ButtonPress(obj, button, state, cstate); }
};

class DrawIfIsA : public ThingDraw {
 public:
  metatype* type;
  ThingDraw* child;
  DrawIfIsA(metatype* type, ThingDraw* child) : type(type), child(child) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    if (&obj.type() == type) child->draw(obj, state, cstate);
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("if is_a: ");
    state.flush();
    child->debug_draw(state);
  }
  UnaryDrawFunctor* convert() override { auto* result = new DrawIfIsAState;
    result->type = type;
    result->child = child->convert();
    return result;
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                   CursorState& cstate) override { if (&obj.type() == type) child->ButtonPress(obj, button, state, cstate); }
};

class NewLineDraw : public ThingDraw {
 public:
  NewLineDraw() {}
  void layout(tptr obj, LayoutLineState& state, CursorState& cstate) override {
    state.nl();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("nl;");
    state.flush();
    state.nl();
  }
  UnaryDrawFunctor* convert() override { return new NewLineDrawState; }
};

class ColorChange : public ThingDraw {
 public:
  ThingDraw* child;
  gui::ColorRGB color;
  ColorChange(ThingDraw* child, gui::ColorRGB color) : child(child), color(color) {}

  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.cr.save();
    state.cr.set_source_rgb(color);
    child->draw(obj, state, cstate);
    state.cr.restore();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override { child->ButtonPress(obj, button, state, cstate); }
  UnaryDrawFunctor* convert() override { auto* result = new ColorChangeState;
    result->child = child->convert();
    result->color = color;
    return result;
  }
};

class IndentChange : public ThingDraw {
 public:
  ThingDraw* child;
  size_t indent;
  IndentChange(ThingDraw* child, size_t indent) : child(child), indent(indent) {}

  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.pt.x += indent;
    ResetScope scope(state);
    state.cr.save();
    child->draw(obj, state, cstate);
    state.cr.restore();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    state.pt.x += indent;
    ResetScope scope(state);
    child->ButtonPress(obj, button, state, cstate);
  }
  void debug_draw(LayoutLineState& state) override {
    auto reset = state.reset_point;
    state.reset_point += 20;
    state.layout("indent: ");
    child->debug_draw(state);
    state.reset_point = reset;
    if (state.just_reset) state.pt.x = reset;
  }
  UnaryDrawFunctor* convert() override { auto* result = new IndentChangeState;
    result->child = child->convert();
    result->indent = indent;
    return result;
  }
};

class ThingDrawList : public ThingDraw {
 public:
  std::vector<ThingDraw*> children;
  ThingDrawList() {}
  ThingDrawList(std::vector<ThingDraw*> children) : children(std::move(children)) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    for (auto* child : children) {
      child->draw(obj, state, cstate);
    }
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    for (auto* child : children) {
      child->ButtonPress(obj, button, state, cstate);
    }
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("[");
    state.flush();
    state.nl();
    {
    state.pt.x += 20;
    ResetScope scope(state);
    for (auto* child : children) {
      child->debug_draw(state);
    }
    }
    state.layout("]");
    state.flush();
    state.nl();
  }
  UnaryDrawFunctor* convert() override { auto* result = new DrawFunctorList;
    for (auto* child : children) result->children.push_back(child->convert());
    return result;
  }
};

class StringEdit : public ThingDraw {
 public:
  var_field_info* lens;
  const char* alt_text;
  StringEdit(var_field_info* lens, const char* alt_text) : lens(lens), alt_text(alt_text) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    draw_entry_with_alt(state, obj[*lens].get<std::string>(), cstate, alt_text);
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    button_press_with_alt(state, obj[*lens].get<std::string>(), button, cstate, alt_text);
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("string edit: \"");
    state.layout(alt_text);
    state.layout("\"");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new StringEditState;
    result->var = lens;
    result->alt_text = alt_text;
    return result;
  }
};

class StringDraw : public ThingDraw {
 public:
  var_field_info* lens;
  StringDraw(var_field_info* lens) : lens(lens) {}
  void layout(tptr obj, LayoutLineState& state, CursorState& cstate) override {
    state.layout(obj[*lens].get<std::string>());
    state.flush();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("string draw");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new StringDrawState;
    result->var = lens;
    return result;
  }
};

// Convert to:
//  - lens to pointer type.
//  - alt_text.
//  - tool construction.
//  - dereference pointer.
//  - ThingDraw* child_draw.

class TypeDeclToolEdit : public ThingDraw {
 public:
  var_field_info* lens;
  const char* alt_text;
  TypeDeclToolEdit(var_field_info* lens, const char* alt_text = "insert type") : lens(lens), alt_text(alt_text) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    auto& value = obj[*lens].get<TypeRef*>();
    if (value) type_ref_layout(value, state);
    else state.layout(alt_text);
    state.flush();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    auto st = state.pt;
    auto& value = obj[*lens].get<TypeRef*>();
    if (value) type_ref_layout(value, state);
    else state.layout(alt_text);
    if (TestInside(st, state, button)) {
      cstate.active = std::make_unique<TypeDeclTool>(value);
    }
    state.flush();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("type decl tool edit: \"");
    state.layout(alt_text);
    state.layout("\"");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new TypeDeclToolEditState;
    result->var = lens;
    result->alt_text = alt_text;
    return result;
  }
};

class VectorBuilder : public ThingDraw {
 public:
  VectorBuilder() {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.layout("vector builder");
    state.flush();
  }

  class VectorTypeDeclTool : public Tool {
   public:
    std::unique_ptr<GenericCursor> other_tool;
    VectorTypeDeclTool(std::unique_ptr<GenericCursor> other_tool) : other_tool(std::move(other_tool)) {}

    void FoundTypeRef(TypeRef* ref) override {
      auto* res = new VectorSpecialization;
      res->element = ref;
      other_tool->FoundTypeRef(res);
    }
  };

  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    auto st = state.pt;
    state.layout("vector builder");
    if (TestInside(st, state, button)) {
      if (cstate.active) {
        cstate.active = std::make_unique<VectorTypeDeclTool>(std::move(cstate.active));
      }
    }
    state.flush();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("vector builder");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { return new VectorBuilderState; }
};

class PointerBuilder : public ThingDraw {
 public:
  PointerBuilder() {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.layout("pointer builder");
    state.flush();
  }

  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    auto st = state.pt;
    state.layout("pointer builder");
    if (TestInside(st, state, button)) {
      if (cstate.active) {
        cstate.active = std::make_unique<PointerTypeDeclTool>(std::move(cstate.active));
      }
    }
    state.flush();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("pointer builder");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { return new PointerBuilderState; }
};

class ForThingDraw : public ThingDraw {
 public:
  var_field_info* var;
  ThingDraw* child;
  explicit ForThingDraw(var_field_info* var, ThingDraw* child) : var(var), child(child) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    any_ref arr = obj[*var];
    size_t count = array_size(arr);
    for (size_t i = 0; i < count; ++i) {
      child->draw(array_index(arr, i), state, cstate);
    }
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    any_ref arr = obj[*var];
    size_t count = array_size(arr);
    for (size_t i = 0; i < count; ++i) {
      child->ButtonPress(array_index(arr, i), button, state, cstate);
    }
  }
  void debug_draw(LayoutLineState& state) override {
    auto reset = state.reset_point;
    state.reset_point += 20;
    state.layout("for: ");
    child->debug_draw(state);
    state.reset_point = reset;
    if (state.just_reset) state.pt.x = reset;
  }
  UnaryDrawFunctor* convert() override {
    auto* result = new ForDrawState;
    result->var = var;
    result->child = child->convert();
    return result;
  }
};

void draw_var_field_info_ref(LayoutLineState& state, var_field_info* var) {
  state.layout(var->base->name);
  state.layout("::");
  state.layout(var->name);
  state.flush();
}

class CommaForThingDraw : public ThingDraw {
 public:
  var_field_info* var;
  ThingDraw* child;
  ThingDraw* comma;
  explicit CommaForThingDraw(var_field_info* var, ThingDraw* child, ThingDraw* comma)
      : var(var), child(child), comma(comma) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    any_ref arr = obj[*var];
    size_t count = array_size(arr);
    for (size_t i = 0; i < count; ++i) {
      if (i != 0) comma->draw(obj, state, cstate);
      child->draw(array_index(arr, i), state, cstate);
    }
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    any_ref arr = obj[*var];
    size_t count = array_size(arr);
    for (size_t i = 0; i < count; ++i) {
      if (i != 0) comma->ButtonPress(obj, button, state, cstate);
      child->ButtonPress(array_index(arr, i), button, state, cstate);
    }
  }
  void debug_draw(LayoutLineState& state) override {
    auto reset = state.reset_point;
    state.reset_point += 20;
    state.layout("for(");
    draw_var_field_info_ref(state, var);
    state.layout("): ");
    child->debug_draw(state);
    state.nl();
    state.layout("comma: ");
    comma->debug_draw(state);
    state.reset_point = reset;
    if (state.just_reset) state.pt.x = reset;
  }
  UnaryDrawFunctor* convert() override { auto* result = new CommaForState;
    result->var = var;
    result->child = child->convert();
    result->comma = comma->convert();
    return result;
  }
};

class TextButtonDrawThing : public ThingDraw {
 public:
  const char* button_text;
  using ExecFn = void (*)(tptr obj);
  ExecFn exec_fn;
  TextButtonDrawThing(const char* button_text, ExecFn exec_fn) : button_text(button_text), exec_fn(exec_fn) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.cr.save();
    state.cr.set_source_rgb(0.8, 0.8, 0.8);
    state.layout(button_text);
    state.flush();
    state.nl();
    state.cr.restore();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    if (TestInside(state, button_text, button)) exec_fn(obj);
    state.nl();
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("button: \"");
    state.layout(button_text);
    state.layout("\"");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new TextButton;
    result->button_text = button_text;
    result->exec_fn = exec_fn;
    return result;
  }
};

bool FindPos(gui::Point& st, gui::Point pt, gui::FontLayoutFace* font, BufferPos& cursor, const Buffer& buffer);

class MultiLineEdit : public ThingDraw {
 public:
  var_field_info* read_buffer;
  MultiLineEdit(var_field_info* read_buffer) : read_buffer(read_buffer) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    auto& buffer = obj[*read_buffer].get<Buffer>();
    DrawBuffer(state.cr, state.pt, buffer, cstate.active ? cstate.active->GetCursor(buffer) : nullptr);
    state.pt.y += state.font->height() * buffer.lines.size();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate) override {
    auto& buffer = obj[*read_buffer].get<Buffer>();
    BufferPos cursor;
    if (button->y >= state.pt.y && FindPos(state.pt, {button->x, button->y}, state.font, cursor, buffer)) {
      cstate.active = std::make_unique<BufferCursor>(buffer, cursor);
    }
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("multiline edit");
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new MultiLineEditState;
    result->read_buffer = read_buffer;
    return result;
  }
};

class TextButtonTypeSelectDrawThing : public ThingDraw {
 public:
  std::string name;
  TextButtonTypeSelectDrawThing(std::string name) : name(std::move(name)) {}
  void draw(tptr obj, DrawLineState& state, CursorState& cstate) override {
    state.layout(name);
    state.flush();
  }
  void ButtonPress(tptr obj, GdkEventButton* button, LayoutLineState& state,
                    CursorState& cstate) override {
    if (TestInside(state, name, button) && cstate.active) {
      cstate.active->FoundPointerType(obj);
    }
  }
  void debug_draw(LayoutLineState& state) override {
    state.layout("type selector named: ");
    state.layout(name);
    state.nl();
    state.flush();
  }
  UnaryDrawFunctor* convert() override { auto* result = new TextButtonTypeSelect;
    result->name = name;
    return result;
  }
};

class VItemList {
 public:
  std::vector<tptr> items;
  UnaryDrawFunctor* item_draw = nullptr;
  void draw(DrawLineState& state, CursorState& cstate) {
    for (auto& item : items) {
      draw_view_decl(item_draw, item, state, cstate);
      state.pt.y += 10;
    }
  }
  void ButtonPress(LayoutLineState& state, GdkEventButton* button, CursorState& cstate) {
    for (auto& item : items) {
      button_press(item_draw, item, button, state, cstate);
      state.pt.y += 10;
    }
  }
};

class ConstructingFunctor {
 public:
  virtual ~ConstructingFunctor() {}

  virtual void Publish() = 0;

  enum Kind {
    kConstStringDraw,
  };
  virtual Kind getKind() = 0;
};

class AnyBuilder {
 public:
  ConstructingFunctor* obj;

  void publish() {
    obj->Publish();
  }
};

class Builder {
 public:
  UnaryDrawFunctor* draw_fn;
  VItemList* append_to;
  std::string type_name;
  TypeRef* group = nullptr;
  using Var = StructDecl_Var;
  std::vector<std::unique_ptr<Var>> vars;
  void draw(gui::DrawCtx& cr, gui::Shape shape, CursorState& cstate) {
    DrawLineState state(cr, shape);
    cr.set_source_rgb(1, 1, 0);
    draw_view_decl(draw_fn, this, state, cstate);
  }
  void ButtonPress(gui::Shape shape, GdkEventButton* button, CursorState& cstate) {
    LayoutLineState state(shape);
    button_press(draw_fn, this, button, state, cstate);
  }
  void publish() {
    auto* item = new StructDecl;
    item->group = group;
    item->name = std::move(type_name);
    for (auto& var : vars) {
      item->vars.push_back({std::move(var->name), var->type});
    }
    append_to->items.push_back(item);
    type_name.clear();
    vars.clear();
    group = nullptr;
  }
};

template <>
metatype* metatype_type_info<Builder>::get() {
  static struct_metatype* result = nullptr;
  if (result) return result;
  result = new struct_metatype;
  result->name = "Builder";
  result->kind = static_cast<int>(metatype::Kind::type_group_metatype);
  result->vars = std::vector<var_field_info>{
    var_field_info{result, "type_name", get_metatype<std::string>(), eraseFn(+[](Builder* v) { return &v->type_name; })},
    var_field_info{result, "group", get_metatype<TypeRef*>(), eraseFn(+[](Builder* v) { return &v->group; })},
    var_field_info{result, "vars", get_metatype<std::vector<std::unique_ptr<Builder::Var>>>(), eraseFn(+[](Builder* v) { return &v->vars; })},
  };
  return result;
}

template <>
metatype* metatype_type_info<std::vector<std::unique_ptr<Builder::Var>>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<Builder::Var>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<std::unique_ptr<Builder::Var>>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<std::unique_ptr<Builder::Var>>* v, size_t i) { return (*v)[i].get(); });
  return result;
}

template <>
metatype* metatype_type_info<any_ref>::get() {
  static builtin_metatype* result = nullptr;
  if (result) return result;
  result = new builtin_metatype;
  result->name = "any_ref";
  result->typeinfo = &typeid(any_ref);
  return result;
}

template <>
metatype* metatype_type_info<std::vector<any_ref>>::get() {
  static vector_metatype* result = nullptr;
  if (result) return result;
  result = new vector_metatype;
  result->element = get_metatype<any_ref>();
  result->get_size = make_raw_fn<size_t(void*)>(+[](std::vector<any_ref>* v) { return v->size(); });
  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](std::vector<any_ref>* v, size_t i) { return &(*v)[i]; });
  result->emplace_back = make_raw_fn<void(void*)>(+[](std::vector<any_ref>* v) { return (*v).emplace_back(); });
  return result;
}

void cpp_write_type(std::ostream& os, TypeRef* type);

void cpp_write_type(std::ostream& os, StructDecl* type) { os << type->name; }
void cpp_write_type(std::ostream& os, BuiltinDecl* type) { os << type->name; }
void cpp_write_type(std::ostream& os, TypeGroupDecl* type) { os << type->name; }
void cpp_write_type(std::ostream& os, ReferenceDecl* type) { cpp_write_type(os, type->wrapping); os << "&"; }
void cpp_write_type(std::ostream& os, VectorSpecialization* type) { os << "std::vector<"; cpp_write_type(os, type->element); os << ">"; }
void cpp_write_type(std::ostream& os, PointerDecl* type) { cpp_write_type(os, type->pointee); os << "*"; }
void cpp_write_type(std::ostream& os, Void* type) { os << "void"; }

void cpp_write_type(std::ostream& os, TypeRef* type) {
  switch (type->getKind()) {
  case TypeRef::Kind::StructDecl: return cpp_write_type(os, reinterpret_cast<StructDecl*>(type));
  case TypeRef::Kind::BuiltinDecl: return cpp_write_type(os, reinterpret_cast<BuiltinDecl*>(type));
  case TypeRef::Kind::TypeGroupDecl: return cpp_write_type(os, reinterpret_cast<TypeGroupDecl*>(type));
  case TypeRef::Kind::ReferenceDecl: return cpp_write_type(os, reinterpret_cast<ReferenceDecl*>(type));
  case TypeRef::Kind::VectorSpecialization: return cpp_write_type(os, reinterpret_cast<VectorSpecialization*>(type));
  case TypeRef::Kind::PointerDecl: return cpp_write_type(os, reinterpret_cast<PointerDecl*>(type));
  case TypeRef::Kind::Void: return cpp_write_type(os, reinterpret_cast<Void*>(type));
  }
};
void cpp_write_fn_sig(std::ostream& os, std::string& name, TypeRef* result, std::vector<FunctionDeclArg>& args) {
  cpp_write_type(os, result);
  os << " " << name << "(";
  size_t i = 0;
  for (auto& arg : args) {
    if (i != 0) os << ", ";
    cpp_write_type(os, arg.type);
    os << " " << arg.name;
    ++i;
  }
  os << ")";
}

struct DedupPointerAndVectorState {
  std::unordered_set<TypeRef*> visited;
  void visit(StructDecl* t) {
    if (visited.emplace(t).second) {
      for (auto& var : t->vars) {
        DoTypeDedup(var.type);
      }
    }
  }
  TypeRef* key_dedup(std::unordered_map<TypeRef*, TypeRef*>& dedup, TypeRef* base, TypeRef*& key) {
    DoTypeDedup(key);
    auto it = dedup.find(key);
    if (it != dedup.end()) return it->second;
    dedup[key] = base;
    return base;
  }
  std::unordered_map<TypeRef*, TypeRef*> pointer_dedup;
  TypeRef* visit(PointerDecl* ptr) { return key_dedup(pointer_dedup, ptr, ptr->pointee); }
  std::unordered_map<TypeRef*, TypeRef*> reference_dedup;
  TypeRef* visit(ReferenceDecl* ptr) { return key_dedup(reference_dedup, ptr, ptr->wrapping); }
  std::unordered_map<TypeRef*, TypeRef*> vector_dedup;
  TypeRef* visit(VectorSpecialization* ptr) { return key_dedup(vector_dedup, ptr, ptr->element); }
  void DoTypeDedup(TypeRef*& t) {
    switch (t->getKind()) {
    case TypeRef::Kind::StructDecl: visit(reinterpret_cast<StructDecl*>(t)); break;
    case TypeRef::Kind::PointerDecl: t = visit(reinterpret_cast<PointerDecl*>(t));
    case TypeRef::Kind::ReferenceDecl: t = visit(reinterpret_cast<ReferenceDecl*>(t)); break;
    case TypeRef::Kind::VectorSpecialization: t = visit(reinterpret_cast<VectorSpecialization*>(t)); break;
    default: break;
    }
  }
};

#include "notes/types_builder.inc.cc"

struct TextFormatEmissionState {
  constexpr static size_t kInvalidId = std::numeric_limits<size_t>::max();
  struct Record {
    size_t id = kInvalidId;
    size_t ref_count = 0;
  };
  std::unordered_map<void*, Record> ids;
  std::vector<any_ref> to_emit;

  void assign_root_id(any_ref ref) {
    if (ref.is_a<var_field_info>() || ref.is_a<metatype>()) {
      std::cerr << "Error, root level builtin type.\n";
      exit(-1);
    }
    auto& record = ids[ref.raw_pointer()];
    record.ref_count += 1;
    if (record.id == kInvalidId) {
      record.id = to_emit.size();
      to_emit.push_back(ref);
    }
  }
  void assign_id(any_ref ref) {
    if (ref.is_a<var_field_info>()) return;
    if (ref.is_a<metatype>()) return;
    /*
    auto it = ids.find(ref.raw_pointer());
    if (it != ids.end()) return;
    ids[ref.raw_pointer()].id = to_emit.size();
    to_emit.push_back(ref);
    explore(ref);
    */
    auto& record = ids[ref.raw_pointer()];
    record.ref_count += 1;
    if (record.ref_count > 1 && record.id == kInvalidId) {
      record.id = to_emit.size();
      to_emit.push_back(ref);
    }
    if (record.ref_count == 1) {
      explore(ref);
    }
  }
  void emit_id(std::ostream& os, any_ref ref) {
    if (ref.is_a<var_field_info>()) {
      auto& info = ref.get<var_field_info>();
      os << "#!" << info.base->name << "::" << info.name;
      return;
    }
    if (ref.is_a<metatype>()) {
      os << "#$";
      emit_metatype(os, ref.get<metatype>());
      return;
    }
    auto it = ids.find(ref.raw_pointer());
    if (it != ids.end()) {
      auto& record = it->second;
      if (record.id != kInvalidId) {
        os << "#" << record.id;
      } else {
        os << "@new ";
        emit_metatype(os, ref.type());
        os << ": ";
        emit(os, ref);
      }
    } else {
      os << "#error(";
      emit_metatype(os, ref.type());
      os << ")";
    }
  }
  void explore(struct_metatype& type, any_ref ref) {
    for (auto& var : type.vars) explore(ref[var]);
  }
  void explore(pointer_metatype& type, any_ref ref) {
    auto pointee = dereference(ref);
    if (pointee.raw_pointer() != nullptr) assign_id(pointee);
  }
  void explore(vector_metatype& type, any_ref ref) {
    auto size = array_size(ref);
    for (size_t i = 0; i < size; ++i) explore(array_index(ref, i));
  }
  void explore(type_group_metatype& type, any_ref ref) {
    std::cerr << "Problem with type: ";
    emit_metatype(std::cerr, type);
    std::cerr << "\n";
    exit(-1);
  }
  void explore(builtin_metatype& type, any_ref ref) {
    if (&type == get_metatype<any_ref>()) assign_id(ref.get<any_ref>());
    else if (&type == get_metatype<std::string>()) {
    } else if (&type == get_metatype<Buffer>()) {
    } else if (&type == get_metatype<size_t>()) {
    } else if (&type == get_metatype<raw_fn_ptr<void(any_ref)>>()) {
    } else {
      std::cerr << "Problem with type: ";
      emit_metatype(std::cerr, type);
      std::cerr << "\n";
    }
  }
  void explore(any_ref ref) {
    auto& type = ref.type();
    switch (type.getKind()) {
    case metatype::Kind::struct_metatype: return explore(reinterpret_cast<struct_metatype&>(type), ref);
    case metatype::Kind::pointer_metatype: return explore(reinterpret_cast<pointer_metatype&>(type), ref);
    case metatype::Kind::vector_metatype: return explore(reinterpret_cast<vector_metatype&>(type), ref);
    case metatype::Kind::type_group_metatype: return explore(reinterpret_cast<type_group_metatype&>(type), ref);
    case metatype::Kind::builtin_metatype: return explore(reinterpret_cast<builtin_metatype&>(type), ref);
    }
  }

  size_t indent = 0;
  void emitIndent(std::ostream& os) {
    for (size_t i = 0; i < indent; ++i) os << " ";
  }
  void emit(std::ostream& os, any_ref ref) {
    auto& type = ref.type();
    switch (type.getKind()) {
    case metatype::Kind::struct_metatype: emit(reinterpret_cast<struct_metatype&>(type), os, ref); break;
    case metatype::Kind::pointer_metatype: emit(reinterpret_cast<pointer_metatype&>(type), os, ref); break;
    case metatype::Kind::vector_metatype: emit(reinterpret_cast<vector_metatype&>(type), os, ref); break;
    case metatype::Kind::type_group_metatype: emit(reinterpret_cast<type_group_metatype&>(type), os, ref); break;
    case metatype::Kind::builtin_metatype: emit(reinterpret_cast<builtin_metatype&>(type), os, ref); break;
    }
  }
  void emit(struct_metatype& type, std::ostream& os, any_ref ref) {
    indent += 1;
    os << "{\n";
    for (auto& var : type.vars) {
      emitIndent(os);
      os << "- " << var.name << ": ";
      emit(os, ref[var]);
      os << "\n";
    }
    indent -= 1;
    emitIndent(os); os << "}";
  }
  void emit(pointer_metatype& type, std::ostream& os, any_ref ref) {
    auto pointee = dereference(ref);
    if (pointee.raw_pointer() != nullptr) emit_id(os, pointee);
    else os << "#nil";
  }
  void emit(vector_metatype& type, std::ostream& os, any_ref ref) {
    os << "[\n";
    indent += 1;
    auto size = array_size(ref);
    for (size_t i = 0; i < size; ++i) {
      emitIndent(os);
      emit(os, array_index(ref, i));
      os << "\n";
    }
    indent -= 1;
    emitIndent(os); os << "]";
  }
  void emit(type_group_metatype& type, std::ostream& os, any_ref ref) { os << "#error !!invalid group"; }
  void emit(builtin_metatype& type, std::ostream& os, any_ref ref) {
    if (&type == get_metatype<any_ref>()) emit_id(os, ref.get<any_ref>());
    else if (&type == get_metatype<std::string>()) {
      // TODO: Double check no newlines (editors cannot create, but should be safe).
      os << ref.get<std::string>();
    } else if (&type == get_metatype<Buffer>()) {
      auto& buf = ref.get<Buffer>();
      os << "<\n";
      indent += 1;
      for (auto& line : buf.lines) {
        emitIndent(os); os << line << "\n";
      }
      indent -= 1;
      emitIndent(os); os << ">";
    } else if (&type == get_metatype<size_t>()) {
      os << ref.get<size_t>();
    } else if (&type == get_metatype<raw_fn_ptr<void(any_ref)>>()) {
      os << lookup_builtin_name((void*)ref.get<raw_fn_ptr<void(any_ref)>>());
    } else {
      os << "#error(";
      emit_metatype(os, type);
      os << ")";
    }
  }
  void DoEmit(std::ostream& os) {
    for (size_t i = 0; i < to_emit.size(); ++i) {
      os << "#" << i << " ";
      any_ref ref = to_emit[i];
      emit_metatype(os, ref.type());
      os << ": ";
      emit(os, ref);
      os << "\n";
    }
  }
};

const std::unordered_map<std::string, metatype*>& getIndexedTypes() {
  static auto indexed_metatypes = []() {
    std::unordered_map<std::string, metatype*> indexed;
    for (auto* type : register_type_info::all_metatypes()) {
      EmitStream ss;
      emit_metatype(ss.stream(), *type);
      indexed[ss.get()] = type;
    }
    return indexed;
  }();
  return indexed_metatypes;
}

void DoTextFormatEmit(std::ostream& os, const std::vector<any_ref>& refs) {
  TextFormatEmissionState state;
  for (auto ref : refs) state.assign_root_id(ref);
  for (auto ref : refs) state.explore(ref);
  state.DoEmit(os);
}

metatype* load_metatype_by_name(string_view type_name) {
  auto& types = getIndexedTypes();
  auto it = types.find(std::string(type_name));
  if (it != types.end()) return it->second;
  std::cerr << "No such type: " << type_name << "\n";
  exit(-1);
}

any_ref allocate_type_by_name(string_view type_name) {
  auto* type = load_metatype_by_name(type_name);
  if (auto* sdecl = dyn_cast<struct_metatype>(type)) return any_ref(sdecl->allocate(), sdecl);
  if (type_name == "vector<any_ref>") {
    return any_ref(new std::vector<any_ref>());
  }

  std::cerr << "Cannot allocate: " << type_name << "\n";
  exit(-1);
}

var_field_info& load_field_by_name(string_view field_name) {
  auto idx = field_name.find(':');
  auto type_name = field_name.substr(0, idx);
  field_name.remove_prefix(idx + 2);
  if (auto* sdecl = dyn_cast<struct_metatype>(load_metatype_by_name(type_name))) {
    for (auto& var : sdecl->vars) {
      if (var.name == field_name) return var;
    }
  }
  std::cerr << "Cannot find: " << field_name << " in " << type_name << "\n";
  exit(-1);
}

struct ParsedTextFormat {
  any_ref getRef(size_t idx) {
    if (idx >= state.size()) {
      fprintf(stderr, "Out of bounds: %zu not in [0, %zu)\n", idx, state.size());
      exit(-1);
    }
    auto& record = state[idx];
    if (record.started) return record.result;
    record.result = allocate_type_by_name(record.type);
    record.started = true;
    return record.result;
  }
  void finalize(size_t idx) {
    if (state[idx].finalized) return;
    state[idx].finalized = true;
    ParseValueIntoRef(state[idx].result, state[idx].payload, 0);
  }
  static void consume_indent(string_view& data, size_t indent) {
    data.remove_prefix(indent);
  }
  static string_view read_line(string_view& data) {
    size_t off = data.find('\n');
    auto result = data.substr(0, off);
    data.remove_prefix(off);
    return result;
  }
  void ParseValueIntoRef(struct_metatype& type, any_ref ref, string_view& data, size_t indent) {
    data.remove_prefix(2); // "{\n"
    while (true) {
      consume_indent(data, indent);
      if (data[0] == '}') break;
      data.remove_prefix(3);
      auto idx = data.find(':');
      auto field_name = data.substr(0, idx);
      data.remove_prefix(idx + 2);
      size_t i = 0;
      for (;i < type.vars.size(); ++i) {
        if (type.vars[i].name == field_name) break;
      }
      if (i < type.vars.size()) {
        any_ref child = ref[type.vars[i]];
        ParseValueIntoRef(child, data, indent + 1);
        data.remove_prefix(1);
      } else {
        std::cerr << "xx Cannot find: " << field_name << "\n";
        std::cerr << "Rest: " << data << "\n";
        exit(-1);
      }
    }
    data.remove_prefix(1);
  }
  void ParseValueIntoRef(pointer_metatype& type, any_ref ref, string_view& data, size_t indent) {
    if (data.substr(0, 2) == "#n") {
      data.remove_prefix(4);
      return;
    }
    if (type.pointee == get_metatype<var_field_info>() && data.substr(0, 2) == "#!") {
      data.remove_prefix(2);
      auto field = read_line(data);
      ref.get<var_field_info*>() = &load_field_by_name(field);
      return;
    }
    if (type.pointee == get_metatype<metatype>() && data.substr(0, 2) == "#$") {
      data.remove_prefix(2);
      ref.get<metatype*>() = load_metatype_by_name(read_line(data));
      return;
    }
    do_pointer_assign(ref, load_id(data, indent));
  }
  void ParseValueIntoRef(type_group_metatype& type, any_ref ref, string_view& data, size_t indent) {
    std::cerr << "Cannot parse(";
    emit_metatype(std::cerr, ref.type());
    std::cerr << ", " << indent << "): \n";
    std::cerr << data;
    exit(-1);
  }
  void ParseValueIntoRef(builtin_metatype& type, any_ref ref, string_view& data, size_t indent) {
    if (&type == get_metatype<any_ref>()) ref.get<any_ref>() = load_id(data, indent);
    else if (&type == get_metatype<std::string>()) {
      ref.get<std::string>() = std::string(read_line(data));
    } else if (&type == get_metatype<Buffer>()) {
      data.remove_prefix(2);
      std::vector<std::string> lines;
      while (true) {
        consume_indent(data, indent);
        if (data[0] == ' ') {
          data.remove_prefix(1);
          lines.push_back(std::string(read_line(data)));
          data.remove_prefix(1);
        } else if (data[0] == '>') break;
      }
      ref.get<Buffer>().lines = std::move(lines);
      data.remove_prefix(1);
    } else if (&type == get_metatype<raw_fn_ptr<void(any_ref)>>()) {
      ref.get<raw_fn_ptr<void(any_ref)>>() = lookup_builtin(std::string(read_line(data))).get<raw_fn_ptr<void(any_ref)>>();
    } else if (&type == get_metatype<size_t>()) {
      ref.get<size_t>() = consume_integer(data); 
    } else {
      std::cerr << "Cannot parse builtin(";
      emit_metatype(std::cerr, ref.type());
      std::cerr << ", " << indent << "): \n";
      exit(-1);
    }
  }
  size_t consume_integer(string_view& data) {
    {
      char c = data[0];
      if (!(c >= '0' && c <= '9')) {
        std::cerr << "Cannot parse: " << data;
        exit(-1);
      }
    }
    size_t result = 0;
    while (true) {
      char c = data[0];
      if (!(c >= '0' && c <= '9')) break;
      result = result * 10 + (c - '0');
      data.remove_prefix(1);
    }
    return result;
  }
  void ParseValueIntoRef(vector_metatype& type, any_ref ref, string_view& data, size_t indent) {
    data.remove_prefix(2);
    while (true) {
      consume_indent(data, indent);
      if (data[0] == ']') break;
      data.remove_prefix(1);
      any_ref child = vector_append_back(ref);
      ParseValueIntoRef(child, data, indent + 1);
      data.remove_prefix(1);
    }
    data.remove_prefix(1);
  }
  void ParseValueIntoRef(any_ref ref, string_view& data, size_t indent) {
    auto& type = ref.type();
    switch (type.getKind()) {
      case metatype::Kind::struct_metatype: return ParseValueIntoRef(reinterpret_cast<struct_metatype&>(type), ref, data, indent);
      case metatype::Kind::pointer_metatype: return ParseValueIntoRef(reinterpret_cast<pointer_metatype&>(type), ref, data, indent);
      case metatype::Kind::vector_metatype: return ParseValueIntoRef(reinterpret_cast<vector_metatype&>(type), ref, data, indent);
      case metatype::Kind::type_group_metatype: return ParseValueIntoRef(reinterpret_cast<type_group_metatype&>(type), ref, data, indent);
      case metatype::Kind::builtin_metatype: return ParseValueIntoRef(reinterpret_cast<builtin_metatype&>(type), ref, data, indent);
    }
  }
  any_ref load_id(string_view& data, size_t indent) {
    if (data[0] == '#') {
      data.remove_prefix(1);
      size_t result = consume_integer(data);
      return (*this)[result];
    } else if (data[0] == '@') {
      data.remove_prefix(5); // "@new "
      auto result = allocate_type_by_name(read_type(data));
      ParseValueIntoRef(result, data, indent);
      return result;
    }
    std::cerr << "Problem!!: " << data << "\n";
    exit(-1);
  }
  any_ref operator[](size_t idx) {
    auto result = getRef(idx);
    finalize(idx);
    return result;
  }
  struct IdxParseState {
    string_view type;
    string_view payload;
    any_ref result;
    bool started = false;
    bool finalized = false;
  };
  string_view read_type(string_view& src) {
    auto idx = src.find(':');
    string_view result = src.substr(0, idx);
    src.remove_prefix(idx + 2);
    return result;
  }
  std::vector<IdxParseState> state;
};
ParsedTextFormat ParseTextFormat(string_view data) {
  ParsedTextFormat result;
  while (!data.empty()) {
    data.remove_prefix(data.find(' ') + 1);
    string_view type = result.read_type(data);
    size_t marker = data.find("\n#");
    string_view rest = (string_view::npos != marker) ? data.substr(0, marker + 1) : data;
    data.remove_prefix(rest.size());
    result.state.emplace_back();
    auto& record = result.state.back();
    record.type = type;
    record.payload = rest;
  }
  return result;
}

const std::vector<tptr>& compiled_types() {
  static bool _ = initializeAllBuiltins();
  (void)_;
  return _impl_static_compiled_types;
}

const std::vector<tptr>& compiled_functions() {
  static bool _ = initializeAllBuiltins();
  (void)_;
  return _impl_static_compiled_functions;
}

const std::vector<FunctorBase*>& builtin_fns() {
  static std::vector<FunctorBase*> out{
  };
  return out;
}

using StringFnType = BuiltinFunctor<std::string&(tptr obj)>;

StringFnType* find_string_fn(const char* name) {
  return find_fn<std::string&(tptr obj)>(name).fn;
}

struct EmissionState {
  std::unordered_map<TypeRef*, size_t> ids;
  size_t next_id = 0;
  std::ostream& os;
  EmissionState(std::ostream& os) : os(os) {}

  void emitName(size_t name) {
    os << "t" << name;
  }
  void emitTypeName(TypeRef* type) {
    if (type == nullptr) { os << "nullptr"; return; }
    switch (type->getKind()) {
    case TypeRef::Kind::StructDecl: os << "StructDecl"; break;
    case TypeRef::Kind::BuiltinDecl: os << "BuiltinDecl"; break;
    case TypeRef::Kind::TypeGroupDecl: os << "TypeGroupDecl"; break;
    case TypeRef::Kind::ReferenceDecl: os << "ReferenceDecl"; break;
    case TypeRef::Kind::VectorSpecialization: os << "VectorSpecialization"; break;
    case TypeRef::Kind::PointerDecl: os << "PointerDecl"; break;
    case TypeRef::Kind::Void: os << "Void"; break;
    }
  }
  void emitFields(StructDecl* type, size_t id) {
    os << "  t" << id << "->name = \"" << type->name << "\";\n";
    if (type->group) {
      size_t gid = getName(type->group);
      os << "  t" << id << "->group = t" << gid << ";\n";
    }
    for (auto& var : type->vars) {
      size_t vid = getName(var.type);
      os << "    t" << id << "->vars.push_back({\"" << var.name << "\", t" << vid << "});\n";
    }
  }
  void emitFields(BuiltinDecl* type, size_t id) {
    os << "  t" << id << "->name = \"" << type->name << "\";\n";
  }
  void emitFields(TypeGroupDecl* type, size_t id) {
    os << "  t" << id << "->name = \"" << type->name << "\";\n";
  }
  void emitFields(ReferenceDecl* type, size_t id) {
    size_t cid = getName(type->wrapping);
    os << "  t" << id << "->wrapping = t" << cid << ";\n";
  }
  void emitFields(VectorSpecialization* type, size_t id) {
    size_t cid = getName(type->element);
    os << "  t" << id << "->element = t" << cid << ";\n";
  }
  void emitFields(PointerDecl* type, size_t id) {
    size_t cid = getName(type->pointee);
    os << "  t" << id << "->pointee = t" << cid << ";\n";
  }
  void emitFields(Void* type, size_t id) {}
  TypeRef* void_type = nullptr;
  std::unordered_map<TypeRef*, TypeRef*> reference_dedup;
  std::unordered_map<TypeRef*, TypeRef*> vector_dedup;
  std::unordered_map<TypeRef*, TypeRef*> pointer_dedup;
  TypeRef* dedup(std::unordered_map<TypeRef*, TypeRef*>& map, TypeRef* base, TypeRef* index) {
    auto it = map.find(index);
    if (it != map.end()) return it->second;
    return map[index] = base;
  }

  size_t getName(TypeRef* type) {
    switch (type->getKind()) {
    case TypeRef::Kind::ReferenceDecl: type = dedup(reference_dedup, type, reinterpret_cast<ReferenceDecl*>(type)->wrapping); break;
    case TypeRef::Kind::VectorSpecialization: type = dedup(vector_dedup, type, reinterpret_cast<VectorSpecialization*>(type)->element); break;
    case TypeRef::Kind::PointerDecl: type = dedup(pointer_dedup, type, reinterpret_cast<PointerDecl*>(type)->pointee); break;
    case TypeRef::Kind::Void: type = void_type = (void_type == nullptr ? type : void_type); break;
    default: break;
    }
    auto it = ids.find(type);
    if (it != ids.end()) return it->second;
    size_t id = next_id++;
    os << "  auto* t" << id << " = new ";
    emitTypeName(type);
    os << ";\n";
    ids[type] = id;
    switch (type->getKind()) {
    case TypeRef::Kind::StructDecl: emitFields(reinterpret_cast<StructDecl*>(type), id); break;
    case TypeRef::Kind::BuiltinDecl: emitFields(reinterpret_cast<BuiltinDecl*>(type), id); break;
    case TypeRef::Kind::TypeGroupDecl: emitFields(reinterpret_cast<TypeGroupDecl*>(type), id); break;
    case TypeRef::Kind::ReferenceDecl: emitFields(reinterpret_cast<ReferenceDecl*>(type), id); break;
    case TypeRef::Kind::VectorSpecialization: emitFields(reinterpret_cast<VectorSpecialization*>(type), id); break;
    case TypeRef::Kind::PointerDecl: emitFields(reinterpret_cast<PointerDecl*>(type), id); break;
    case TypeRef::Kind::Void: emitFields(reinterpret_cast<Void*>(type), id); break;
    }
    return id;
  }
};
void EmitCppBuilder(std::vector<TypeRef*> items, const std::vector<tptr>& fns, std::ostream& os) {
  EmissionState state(os);
  os <<  "static std::vector<tptr> _impl_static_compiled_types;\n";
  os << "static std::vector<tptr> _impl_static_compiled_functions;\n\n";
  os << "bool initializeAllBuiltins() {\n";
  std::vector<size_t> names;
  for (auto* t : items) names.push_back(state.getName(t));
  size_t fn_id = 0;
  for (auto& fn : fns) {
    os << "  auto* f" << (fn_id) << " = new ";
    if (auto* fn_decl = fn.get_or_null<FunctionDecl>()) {
      os << "FunctionDecl;\n";
      os << "  f" << fn_id << "->name = \"" << fn_decl->name << "\";\n";
      size_t rid = state.getName(fn_decl->result);
      os << "  f" << fn_id << "->result = t" << rid << ";\n";
      for (auto& arg : fn_decl->args) {
        size_t aid = state.getName(arg.type);
        os << "  f" << fn_id << "->args.push_back({\"" << arg.name << "\", t" << aid << "});\n";
      }
      os << "  f" << fn_id << "->body = Buffer(R\"fnbody(" << Collapse(fn_decl->body) << ")fnbody\");\n";
    } else if (auto* fn_decl = fn.get_or_null<SwitchFunctionDecl>()) {
      os << "SwitchFunctionDecl;\n";
      os << "  f" << fn_id << "->name = \"" << fn_decl->name << "\";\n";
      size_t rid = state.getName(fn_decl->result);
      os << "  f" << fn_id << "->result = t" << rid << ";\n";
      for (auto& arg : fn_decl->args) {
        size_t aid = state.getName(arg.type);
        os << "  f" << fn_id << "->args.push_back({\"" << arg.name << "\", t" << aid << "});\n";
      }
      os << "  f" << fn_id << "->body = Buffer(R\"fnbody(" << Collapse(fn_decl->body) << ")fnbody\");\n";
    }
    ++fn_id;
  }
  os << "  _impl_static_compiled_functions = std::vector<tptr>{";
  for (size_t i = 0; i < fns.size(); ++i) os << ((i != 0) ? ", f" : "f") << i;
  os << "};\n";
  os << "  _impl_static_compiled_types = std::vector<tptr>{";
  size_t i = 0;
  for (auto tname : names) {
    if (i != 0) os << ", ";
    state.emitName(tname);
    ++i;
  }
  os << "};\n";
  os << "  return true;\n}\n";
}

void find_all_things(TypeRef* t_, std::unordered_set<TypeRef*>& visited, std::vector<TypeRef*>& ordered_out) {
  if (t_ == nullptr) return;
  auto it = visited.find(t_);
  if (it != visited.end()) return;
  visited.emplace(t_);
  ordered_out.push_back(t_);
  switch (t_->getKind()) {
  case TypeRef::Kind::StructDecl: { auto* t = reinterpret_cast<StructDecl*>(t_);
    find_all_things(t->group, visited, ordered_out);
    for (auto& var : t->vars) find_all_things(var.type, visited, ordered_out);
    break;
  }
  case TypeRef::Kind::PointerDecl:
    find_all_things(reinterpret_cast<PointerDecl*>(t_)->pointee, visited, ordered_out); break;
  case TypeRef::Kind::VectorSpecialization:
    find_all_things(reinterpret_cast<VectorSpecialization*>(t_)->element, visited, ordered_out); break;
  case TypeRef::Kind::ReferenceDecl:
    find_all_things(reinterpret_cast<ReferenceDecl*>(t_)->wrapping, visited, ordered_out); break;
  default: break;
  }
}

void EmitStructMetadata(std::ostream& h, std::ostream& cc, const std::unordered_map<TypeGroupDecl*, std::vector<StructDecl*>>& groups,
                        const std::vector<StructDecl*>& structs) {
  std::unordered_set<TypeRef*> visited;
  std::vector<TypeRef*> all_ordered;
  auto emit_ctor_decl = [&](const char* tname) {
    cc << "  static " << tname << "* result = nullptr;\n";
    cc << "  if (result) return result;\n";
    cc << "  result = new " << tname << ";\n";
  };

  for (auto* s : structs) {
    find_all_things(s, visited, all_ordered);
    cc << "template <>\n";
    cc << "metatype* metatype_type_info<" << s->name << ">::get() {\n";
    emit_ctor_decl("struct_metatype");
    cc << "  result->name = \"" << s->name << "\";\n";
    if (auto* group = dyn_cast_or_null<TypeGroupDecl>(s->group)) {
      cc << "  result->kind = static_cast<int>(" << group->name << "::Kind::" << s->name << ");\n";
      cc << "  result->group = get_metatype<" << group->name << ">();\n";
    }
    cc << "  result->vars = std::vector<var_field_info>{\n";
    for (auto& var : s->vars) {
      cc << "    var_field_info{result, \"" << var.name << "\", get_metatype<";
      cpp_write_type(cc, var.type);
      cc << ">(), eraseFn(+[](" << s->name << "* v) { return &v->" << var.name << "; })},\n";
    }
    cc << "  };\n";
    cc << "  result->allocate = +[]() -> void* { return new " << s->name << "; };\n";
    cc << "  return result;\n";
    cc << "}\n";
  }

  for (auto& group_list : groups) {
    auto* group = group_list.first;
    cc << "template <>\n";
    cc << "metatype* metatype_type_info<" << group->name << ">::get() {\n";
    emit_ctor_decl("type_group_metatype");
    cc << "  result->name = \"" << group->name << "\";\n";
    cc << "  result->get_kind = +[](void* v) { return static_cast<size_t>(reinterpret_cast<" << group->name << "*>(v)->getKind()); };\n";
    cc << "  result->load_type = +[](size_t raw_kind) -> metatype* { auto kind = static_cast<" << group->name << "::Kind>(raw_kind);\n";
    cc << "    switch (kind) {\n";
    for (auto* child : group_list.second) {
    cc << "      case " << group->name << "::Kind::" << child->name << ": return get_metatype<" << child->name << ">();\n";
    }
    cc << "    }\n";
    cc << "  };\n";
    cc << "  return result;\n";
    cc << "}\n";
  }

  for (auto* t_ : all_ordered) {
    switch (t_->getKind()) {
    case TypeRef::Kind::BuiltinDecl: { auto* t = reinterpret_cast<BuiltinDecl*>(t_);
      cc << "template <>\n";
      cc << "metatype* metatype_type_info<" << t->name << ">::get() {\n";
      emit_ctor_decl("builtin_metatype");
      cc << "  result->name = \"";
      cpp_write_type(cc, t);
      cc << "\";\n";
      cc << "  result->typeinfo = &typeid(";
      cpp_write_type(cc, t);
      cc << ");\n";
      cc << "  return result;\n";
      cc << "}\n";
      break;
    }
    case TypeRef::Kind::VectorSpecialization: { auto* t = reinterpret_cast<VectorSpecialization*>(t_);
      cc << "template <>\n";
      cc << "metatype* metatype_type_info<";
      cpp_write_type(cc, t);
      cc << ">::get() {\n";
      emit_ctor_decl("vector_metatype");
      cc << "  result->element = get_metatype<";
      cpp_write_type(cc, t->element);
      cc << ">();\n";
      cc << "  result->get_size = make_raw_fn<size_t(void*)>(+[](";
      cpp_write_type(cc, t);
      cc << "* v) { return v->size(); });\n";
      cc << "  result->get_element = make_raw_fn<void*(void*, size_t)>(+[](";
      cpp_write_type(cc, t);
      cc << "* v, size_t i) { return &(*v)[i]; });\n";
      cc << "  result->emplace_back = make_raw_fn<void(void*)>(+[](";
      cpp_write_type(cc, t);
      cc << "* v) { return (*v).emplace_back(); });\n";
      cc << "  return result;\n";
      cc << "}\n";
      break;
    }
    default: break;
    }
  }
}
void DoEmitAllTypes(std::ostream& h, std::ostream& cc, std::vector<tptr>& items, std::vector<tptr>& fns) {
  h << R"(#pragma once

#include <vector>
#include <string>
#include "rules/template-support.h"
#include "notes/type-support.h"
#include "notes/type-info.h"
#include "notes/gui-support.h"
#include "gui/buffer-view.h"

)";
  cc << R"(#include "notes/generated.h"

)";
  std::unordered_map<TypeGroupDecl*, std::vector<StructDecl*>> groups;
  std::vector<StructDecl*> structs;
  for (auto& s : items) {
    if (auto* struct_decl = s.get_or_null<StructDecl>()) {
      structs.push_back(struct_decl);
      if (auto* generic_group = struct_decl->group) {
        if (auto* group = dyn_cast_or_null<TypeGroupDecl>(generic_group)) {
          groups[group].push_back(struct_decl);
        }
      }
      h << "struct " << s.get<StructDecl>().name << ";\n"; 
    }
  }
  for (auto& group : groups) {
    auto& name = group.first->name;
    h << "struct " << name << "{\n";
    h << "  enum class Kind {\n";
    for (auto* item : group.second) {
      h << "    " << item->name << ",\n";
    }
    h << "  };\n";
    h << "  explicit " << name << R"((Kind kind) : kind_(kind) {}
Kind getKind() { return kind_; }
private:
Kind kind_;
};
)";
  }
  for (auto& s : items) {
    if (auto* sdecl = s.get_or_null<StructDecl>()) {
      h << "struct " << sdecl->name;
      if (auto* group = dyn_cast_or_null<TypeGroupDecl>(sdecl->group)) {
        h << " : public " << group->name << " {\n";
        cc << "\ntemplate <>\n";
        cc << sdecl->name << "* dyn_cast<" << sdecl->name << ", " << group->name << ">(";
        cc << group->name << "* t) {\n";
        cc << "  return t->getKind() == " << group->name << "::Kind::" << sdecl->name << " ? static_cast<" << sdecl->name << "*>(t) : nullptr;\n";
        cc << "}\n";
        h << "  " << sdecl->name << "() : " << group->name << "(Kind::" << sdecl->name << ") {}\n";
      } else {
        h << " {\n";
      }
      for (auto& var : sdecl->vars) {
        h << "  ";
        cpp_write_type(h, var.type);
        h << " " << var.name;
        if (dyn_cast<PointerDecl>(var.type)) h << " = nullptr";
        h << ";\n";
      }
      h << "};\n";
    }
  }
  EmitStructMetadata(h, cc, groups, structs);
  std::unordered_map<std::string, std::vector<FunctionDecl*>> overloads;
  for (auto& fn : fns) {
    if (auto* fn_decl = fn.get_or_null<FunctionDecl>()) {
      overloads[fn_decl->name].push_back(fn_decl);
    }
  }
  for (auto& fn : fns) {
    if (auto* fn_decl = fn.get_or_null<FunctionDecl>()) {
      cpp_write_fn_sig(h, fn_decl->name, fn_decl->result, fn_decl->args);
      h << ";\n";
      cpp_write_fn_sig(cc, fn_decl->name, fn_decl->result, fn_decl->args);
      cc << " {\n";
      for (auto& line : fn_decl->body.lines) {
        cc << line << "\n";
      }
      cc << "}\n";
    } else if (auto* fn_decl = fn.get_or_null<SwitchFunctionDecl>()) {
      cpp_write_fn_sig(h, fn_decl->name, fn_decl->result, fn_decl->args);
      h << ";\n";
      cpp_write_fn_sig(cc, fn_decl->name, fn_decl->result, fn_decl->args);
      cc << " {\n";
      auto* stype = fn_decl->args[0].type;
      if (auto* pstype = dyn_cast<ReferenceDecl>(stype)) {
        cc << "  switch(self.getKind()) {\n";
        stype = pstype->wrapping;
      } else if (auto* pstype = dyn_cast<PointerDecl>(stype)) {
        cc << "  switch(self->getKind()) {\n";
        stype = pstype->pointee;
      }
      auto* basic = dyn_cast<TypeGroupDecl>(stype);
      for (auto* overload : overloads[fn_decl->name]) {
        auto* otype = overload->args[0].type;
        TypeRef* otypeflat = nullptr;
        if (auto* potype = dyn_cast<ReferenceDecl>(otype)) otypeflat = potype->wrapping;
        else if (auto* potype = dyn_cast<PointerDecl>(otype)) otypeflat = potype->pointee;
        auto* child = dyn_cast<StructDecl>(otypeflat);
        cc << "  case " << basic->name << "::Kind::" << child->name << ": return " << fn_decl->name << "("; 
        size_t i = 0;
        for (auto& arg : fn_decl->args) {
          if (i != 0) cc << ", ";
          if (i == 0) {
            cc << "reinterpret_cast<";
            cpp_write_type(cc, otype);
            cc << ">(" << arg.name << ")";
          } else {
            cc << arg.name;
          }
          ++i;
        }
        cc << ");\n";
      }
      cc << "  default: {\n";
      for (auto& line : fn_decl->body.lines) cc << line << "\n";
      cc << "  }\n";
      cc << "  }\n";
      cc << "}\n";
    }
  }
}

template <>
any_ref::any_ref(UnaryDrawFunctor* value);

template <>
any_ref::any_ref(UnaryDrawFunctor* value) : value_(value), type_(get_metatype<UnaryDrawFunctor>()) {
  type_ = reinterpret_cast<type_group_metatype*>(type_)->load_type(static_cast<size_t>(value->getKind()));
}

metatype* make_pointer_metatype(metatype* pointee) {
  // Add cache here as well in order to support building runtime types.
  auto* result = new pointer_metatype;
  result->pointee = pointee;
  return result;
}

var_field_info& find_var(metatype* base, string_view name) {
  if (base->getKind() != metatype::Kind::struct_metatype) {
    std::cerr << "problem: " << name << "\n";
    exit(-1);
  }
  auto* t = reinterpret_cast<struct_metatype*>(base);
  for (auto& var : t->vars) {
    if (var.name == name) {
      return var;
    }
  }
  std::cerr << "problem: " << name << "\n";
  exit(-1);

}

template <typename Base>
var_field_info& find_var(string_view name) {
  return find_var(get_metatype<Base>(), name);
}

template <>
metatype* metatype_type_info<raw_fn_ptr<void(any_ref)>>::get();
std::vector<builtin_record>& get_builtins() {
  static raw_fn_ptr<void(any_ref)> add_builder_var = +[](tptr obj) { obj.get<Builder>().vars.push_back(std::make_unique<Builder::Var>(Builder::Var{})); };
  static raw_fn_ptr<void(any_ref)> publish_builder = +[](tptr obj) { obj.get<Builder>().publish(); };
  static std::vector<builtin_record> result{
    {"add_builder_var", any_ref(&add_builder_var)},
    {"publish_builder", any_ref(&publish_builder)},
  };
  return result;
}

class WindowState : public BasicWindowState {
 public:
  VItemList items;
  VItemList fns;
  VItemList views;
  // VItemList actions; ??
  double rect2_y_scroll = 0;
  double rect3_y_scroll = 0;
  Builder builder;
  CursorState cstate;

  WindowState() {
    /*
    auto* arg_list_draw = new ThingDrawList({
                new StringDraw(&find_var<FunctionDeclArg>("name")),
                new ConstStringDraw(" : "),
                new TypeDeclToolEdit(&find_var<FunctionDeclArg>("type")),
            });

    fns.item_draw = (new ThingDrawList({
      new DrawIfIsA(get_metatype<FunctionDecl>(),
      new ThingDrawList({
        new ConstStringDraw("func "),
        new StringDraw(&find_var<FunctionDecl>("name")),
        new ConstStringDraw("("),
        new CommaForThingDraw(&find_var(get_metatype<FunctionDecl>(), "args"),
            arg_list_draw, new ConstStringDraw(", ")),
        new ConstStringDraw(") -> "),
        new TypeDeclToolEdit(&find_var<FunctionDecl>("result")),
        new NewLineDraw(),
        new IndentChange(
          new MultiLineEdit(&find_var<FunctionDecl>("body")),
        20),
      })),
      new DrawIfIsA(get_metatype<SwitchFunctionDecl>(),
      new ThingDrawList({
        new ConstStringDraw("switch_func "),
        new StringDraw(&find_var<SwitchFunctionDecl>("name")),
        new ConstStringDraw("("),
        new CommaForThingDraw(&find_var(get_metatype<SwitchFunctionDecl>(), "args"),
            arg_list_draw, new ConstStringDraw(", ")),
        new ConstStringDraw(") -> "),
        new TypeDeclToolEdit(&find_var<SwitchFunctionDecl>("result")),
        new NewLineDraw(),
        new IndentChange(
          new MultiLineEdit(&find_var<SwitchFunctionDecl>("body")),
        20),
      })),
    }))->convert();

    items.item_draw = (new ThingDrawList({
      new DrawIfIsA(get_metatype<StructDecl>(),
        new ThingDrawList({
      new TextButtonTypeSelectDrawThing("struct"),
      new ConstStringDraw(" "),
      new StringDraw(&find_var<StructDecl>("name")),
      new DrawIfNotNull(
        new ThingDrawList({
          new ConstStringDraw(" : "),
          new TypeDeclToolEdit(&find_var<StructDecl>("group")),
        })
      ),
      new NewLineDraw(),
      new IndentChange(
      new ForThingDraw(&find_var<StructDecl>("vars"),
        new ThingDrawList({
            new ConstStringDraw("var "),
            new StringDraw(&find_var<StructDecl_Var>("name")),
            new ConstStringDraw(" : "),
            new TypeDeclToolEdit(&find_var<StructDecl_Var>("type")),
            new NewLineDraw
        })), 20),
      })),
      new DrawIfIsA(get_metatype<TypeGroupDecl>(),
          new ThingDrawList({
        new TextButtonTypeSelectDrawThing("type_group"),
        new ConstStringDraw(" "),
        new StringDraw(&find_var<TypeGroupDecl>("name")),
        new NewLineDraw(),
      })),
      new DrawIfIsA(get_metatype<BuiltinDecl>(),
          new ThingDrawList({
        new TextButtonTypeSelectDrawThing("builtin"),
        new ConstStringDraw(" "),
        new StringDraw(&find_var<BuiltinDecl>("name")),
        new NewLineDraw(),
      })),
      new DrawIfIsA(get_metatype<Void>(),
          new ThingDrawList({
        new TextButtonTypeSelectDrawThing("void"),
        new NewLineDraw(),
      })),
    }))->convert();

    items.items = compiled_types();
    fns.items = compiled_functions();

    auto* silly = new ThingDrawList({
      new VectorBuilder,
      new NewLineDraw,
      new NewLineDraw,
      new PointerBuilder,
      new NewLineDraw,
      new NewLineDraw,
      new ConstStringDraw("struct "),
      new StringEdit(&find_var<Builder>("type_name"), "enter type name"),
      new ConstStringDraw(" : "),
      new TypeDeclToolEdit(&find_var<Builder>("group")),
      new NewLineDraw,
      new IndentChange(
        new ThingDrawList({
          new ForThingDraw(&find_var<Builder>("vars"),
            new ThingDrawList({
                new ConstStringDraw("var "),
                new StringEdit(&find_var<Builder::Var>("name"), "enter var name"),
                new ConstStringDraw(" : "),
                new TypeDeclToolEdit(&find_var<Builder::Var>("type")),
                new NewLineDraw
            })),
          new TextButtonDrawThing("-- add layout --", lookup_builtin("add_builder_var").get<raw_fn_ptr<void(any_ref)>>()),
        }), 20),
      new TextButtonDrawThing("-- publish --", lookup_builtin("publish_builder").get<raw_fn_ptr<void(any_ref)>>())
    });

    builder.draw_fn = silly->convert();
    builder.append_to = &items;
    */

    auto* tmp = new NamedDrawFunctor;
    tmp->name = "mutate_draw_functor";
    auto* tmp2 = new SwitchDrawFunctor;
    tmp->child = tmp2;
    tmp2->other = new UnhandledTypeDrawFunctor;
    views.item_draw = tmp;
    views.items.push_back(views.item_draw);
    { 
      auto tmp_str = LoadFile(".gui/notes-data");
      // auto tmp_str = ss.get();
      auto result = ParseTextFormat(tmp_str);
      items.items = result[0].get<std::vector<any_ref>>();
      fns.items = result[1].get<std::vector<any_ref>>();
      builder.draw_fn = &result[2].get<UnaryDrawFunctor>();
      fns.item_draw = &result[3].get<UnaryDrawFunctor>();
      items.item_draw = &result[4].get<UnaryDrawFunctor>();
    }
//    views.item_draw = n
    InitBasicState();
    InitEvents();
  }

  void InitEvents() {
  SigConnect(window, "destroy", G_CALLBACK((
          +[](GtkWidget*, WindowState* state) -> gboolean {
    gtk_main_quit();
    delete state;
    return TRUE;
  })), this);
  SigConnect(window, "draw", G_CALLBACK((
          +[](GtkWidget*, cairo_t* cr_ptr, WindowState* state) -> gboolean {
    auto& cr = *gui::DrawCtx::wrap(cr_ptr);
    cr.set_source_rgb(0.0, 0.0, 0.2);
    cr.paint();
    cr.set_source_rgb(1, 1, 0);
    state->needs_redraw = false;
    gui::Rectangle rect1;
    gui::Rectangle rect2;
    gui::Rectangle rect3;
    gui::DoHSplit(gui::ConvertRectangle(state->shape()), rect1, rect2, 1000);
    gui::DoHSplit(rect1, rect1, rect3, 500);
    SaveClipTranslate(cr, rect2);
    {
      DrawLineState dstate(cr, rect2.shape);
      dstate.pt.y += state->rect2_y_scroll;
      state->items.draw(dstate, state->cstate);
      state->fns.draw(dstate, state->cstate);
    }
    cr.restore();

    SaveClipTranslate(cr, rect1);
    state->builder.draw(cr, rect1.shape, state->cstate);
    cr.restore();
    
    SaveClipTranslate(cr, rect3);
    {
      DrawLineState dstate(cr, rect3.shape);
      dstate.pt.y += state->rect3_y_scroll;
      state->views.draw(dstate, state->cstate);
      // draw_metatype_decl(get_metatype<struct_metatype>(), dstate, state->cstate);
      // state->builder.draw_fn->debug_draw(dstate);
      // state->items.item_draw->debug_draw(dstate);
      // state->fns.item_draw->debug_draw(dstate);
    }
    cr.restore();

    // auto& ctx = state->ctx;
    return TRUE;
  })), this);
  SigConnect(window, "button-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* button, WindowState* state) -> gboolean {
    bool is_event = (button->time != state->press_time);
    state->press_time = button->time;
    if (is_event) {
      RestoreEventXY button_restore(button);
      gui::Point pt{button->x, button->y};
      gui::Rectangle rect1;
      gui::Rectangle rect2;
      gui::Rectangle rect3;
      gui::DoHSplit(gui::ConvertRectangle(state->shape()), rect1, rect2, 1000);
      gui::DoHSplit(rect1, rect1, rect3, 500);
      if (TestRectangleClick(button, rect1)) {
        state->builder.ButtonPress(rect1.shape, button, state->cstate);
      } else if (TestRectangleClick(button, rect2)) {
        LayoutLineState lstate(rect2.shape);
        lstate.pt.y += state->rect2_y_scroll;
        state->items.ButtonPress(lstate, button, state->cstate);
        state->fns.ButtonPress(lstate, button, state->cstate);
      } else if (TestRectangleClick(button, rect3)) {
        LayoutLineState lstate(rect3.shape);
        lstate.pt.y += state->rect3_y_scroll;
        state->views.ButtonPress(lstate, button, state->cstate);
      }
      state->redraw();
    }
    return TRUE;
  })), this);
  SigConnect(window, "button-release-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* event, WindowState* state) -> gboolean {
    return TRUE;
  })), this);
  SigConnect(window, "configure-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventConfigure* config, WindowState* state) -> gboolean {

    if (state->window_width_ != config->width || state->window_height_ != config->height) {
      state->window_width_ = config->width;
      state->window_height_ = config->height;
      state->redraw();
    }
    return FALSE;

    return TRUE;
  })), this);
  SigConnect(window, "key-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventKey* event, WindowState* state) -> gboolean {

    if (state->HandleSpecialEvents(event)) { 

    } else if (event->keyval == GDK_KEY_F6) {
      // Serialize here... (At some point I'll have to serialize functions)...
      {
        std::vector<any_ref> tmp = {
            any_ref(&state->items.items),
            any_ref(&state->fns.items),
            any_ref(state->builder.draw_fn),
            any_ref(state->fns.item_draw),
            any_ref(state->items.item_draw),
            any_ref(&state->views.items),
            any_ref(state->views.item_draw),
        };
        EmitStream ss;
        DoTextFormatEmit(ss.stream(), tmp);
        ss.write(".gui/notes-data");
      }

      std::vector<TypeRef*> items;
      for (auto& s : state->items.items) {
        if (auto* decl0 = s.get_or_null<StructDecl>()) items.push_back(decl0);
        else if (auto* decl1 = s.get_or_null<TypeGroupDecl>()) items.push_back(decl1);
        else if (auto* decl2 = s.get_or_null<Void>()) items.push_back(decl2);
        else if (auto* decl3 = s.get_or_null<BuiltinDecl>()) items.push_back(decl3);
      }
        
      EmitStream types_builder;
      EmitCppBuilder(items, state->fns.items, types_builder.stream());
      types_builder.write("src/notes/types_builder.inc.cc");
    } else if (event->keyval == GDK_KEY_F5) {
      DedupPointerAndVectorState dstate;
      for (auto item : state->items.items) if (auto* sitem = item.get_or_null<StructDecl>()) dstate.visit(sitem);
      EmitStream cc;
      EmitStream h;
      DoEmitAllTypes(h.stream(), cc.stream(), state->items.items, state->fns.items);
      h.write("src/notes/generated.h");
      cc.write("src/notes/generated.cc");
    } else if (event->keyval == GDK_KEY_F4) {
      std::vector<any_ref> tmp = {any_ref(&state->items.items), any_ref(&state->fns.items)};
      // EmitStream tmp;
      DoTextFormatEmit(std::cout, tmp);
    } else if (event->keyval == GDK_KEY_F3) {
      fprintf(stderr, "Silly thing\n");
      // Rebuild gui things here.
    } else {
      if (state->cstate.active) {
        state->cstate.active->press(event->keyval);
      }
      state->redraw();
    }
    return TRUE;
  })), this);
  SigConnect(window, "scroll-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventScroll* event, WindowState* state) -> gboolean {
      RestoreEventXY event_restore(event);
      auto compute_scroll = [&](double& scroll) {
        auto* font = gui::DefaultFont();
        if (event->direction == GDK_SCROLL_UP) {
        scroll += font->height() * 3;
        } else if (event->direction == GDK_SCROLL_DOWN) {
        scroll -= font->height() * 3;
        }
        if (scroll > 0) scroll = 0;
      };

      gui::Rectangle rect1;
      gui::Rectangle rect2;
      gui::Rectangle rect3;
      gui::DoHSplit(gui::ConvertRectangle(state->shape()), rect1, rect2, 1000);
      gui::DoHSplit(rect1, rect1, rect3, 500);
      if (TestRectangleClick(event, rect1)) {
      } else if (TestRectangleClick(event, rect2)) {
        compute_scroll(state->rect2_y_scroll);
      } else if (TestRectangleClick(event, rect3)) {
        compute_scroll(state->rect3_y_scroll);
      }
    state->redraw();
    return TRUE;
  })), this);
  SigConnect(window, "motion-notify-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventMotion* event, WindowState* state) -> gboolean {
    gui::Point pt{event->x, event->y};

    return TRUE;
  })), this);

  }

  uint32_t press_time = -1;
};

extern "C" void dl_plugin_entry(int argc, char **argv) {
//  silly_test();
//  exit(0);
  if (argc > 1) {
    new WindowState(); // argv[1]);
  } else {
    new WindowState(); // ".gui/data");
  }
}
