#include "notes/serialize.h"

#include <unordered_set>

template <>
metatype* metatype_type_info<raw_fn_ptr<void(any_ref)>>::get();

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

    h << "template <>\n";
    h << "any_ref::any_ref(" << group->name << "* value);\n";

    cc << "template <>\n";
    cc << "any_ref::any_ref(" << group->name << "* value) : value_(value), type_(get_metatype<" << group->name << ">()) {\n";
    cc << "  type_ = reinterpret_cast<type_group_metatype*>(type_)->load_type(static_cast<size_t>(value->getKind()));\n";
    cc << "}\n";
  }

  for (auto* t_ : all_ordered) {
    switch (t_->getKind()) {
    case TypeRef::Kind::BuiltinDecl: { auto* t = reinterpret_cast<BuiltinDecl*>(t_);
      h << "template <>\n";
      h << "metatype* metatype_type_info<" << t->name << ">::get();\n";
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

void DoTextFormatEmit(std::ostream& os, const std::vector<any_ref>& refs) {
  TextFormatEmissionState state;
  for (auto ref : refs) state.assign_root_id(ref);
  for (auto ref : refs) state.explore(ref);
  state.DoEmit(os);
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
