#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include "notes/generated.h"

void cpp_write_type(std::ostream& os, TypeRef* type);
void cpp_write_fn_sig(std::ostream& os, std::string& name, TypeRef* result, std::vector<FunctionDeclArg>& args);

void EmitStructMetadata(std::ostream& h, std::ostream& cc,
                        const std::unordered_map<TypeGroupDecl*, std::vector<StructDecl*>>& groups,
                        const std::vector<StructDecl*>& structs);
void DoEmitAllTypes(std::ostream& h, std::ostream& cc, std::vector<tptr>& items, std::vector<tptr>& fns);
void EmitCppBuilder(std::vector<TypeRef*> items, const std::vector<tptr>& fns, std::ostream& os);

void emit_metatype(std::ostream& stream, metatype& t);
void DoTextFormatEmit(std::ostream& os, const std::vector<any_ref>& refs);

var_field_info& load_field_by_name(string_view field_name);

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
ParsedTextFormat ParseTextFormat(string_view data);
