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

void draw_view_decl(UnaryDrawFunctor* self, any_ref obj, DrawLineState& state, CursorState& cstate);
void button_press(UnaryDrawFunctor* self, any_ref obj, GdkEventButton* button, LayoutLineState& state, CursorState& cstate);

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

namespace dt {

class Base {
 public:
  virtual ~Base() {}
};

class Switch : public Base {
 public:
  type_group_metatype* group = nullptr;
  std::unordered_map<struct_metatype*, Base*> items;
};

class StructDraw : public Base {
 public:
  struct FieldDraw {
    var_field_info* field;
    Base* item_draw = nullptr;
  };
  std::vector<FieldDraw> fields;
};

class PointerDraw : public Base {
 public:
  Base* ptr = nullptr;
};

class VectorDraw : public Base {
 public:
  Base* item = nullptr;
};

class RectDrawable {
 public:
  virtual ~RectDrawable() {}

  virtual void draw(gui::DrawCtx& cr, gui::Shape shape) = 0;
  virtual void button_press(gui::Shape shape, GdkEventButton* button) = 0; 
};

}  // namespace dt

class WindowState : public BasicWindowState {
 public:
  VItemList items;
  VItemList fns;
  VItemList views;
  // VItemList actions; // ??
  double rect2_y_scroll = 0;
  double rect3_y_scroll = 0;
  Builder builder;
  CursorState cstate;

  WindowState() {
    builder.append_to = &items;

    auto* tmp = new NamedDrawFunctor;
    tmp->name = "mutate_draw_functor";
    auto* tmp2 = new SwitchDrawFunctor;
    tmp->child = tmp2;
    tmp2->other = new UnhandledTypeDrawFunctor;
    views.item_draw = tmp;
    views.items.push_back(views.item_draw);
    { 
      auto tmp_str = LoadFile(".gui/notes-data");
      auto result = ParseTextFormat(tmp_str);
      items.items = result[0].get<std::vector<any_ref>>();
      fns.items = result[1].get<std::vector<any_ref>>();
      builder.draw_fn = &result[2].get<UnaryDrawFunctor>();
      fns.item_draw = &result[3].get<UnaryDrawFunctor>();
      items.item_draw = &result[4].get<UnaryDrawFunctor>();
    }
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
