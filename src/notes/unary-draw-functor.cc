#include "notes/generated.h"
#include "notes/serialize.h"

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

class TypeDeclTool : public Tool {
 public:
  TypeRef*& type;
  TypeDeclTool(TypeRef*& type) : type(type) {}

  void FoundTypeRef(TypeRef* ref) override {
    type = ref;
  }
};

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
