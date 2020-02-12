#include "gui/emit-buffer.h"

#include "rules/template-support.h"
#include "rules/string-utils.h"
#include "gen/gui/emit_manifest.h"
#include <assert.h>

std::vector<CollapsedBuffer> Collapse(const std::vector<ParsedIdBuffer>& src) {
  std::vector<CollapsedBuffer> out;
  for (const auto& buff : src) out.push_back({buff.id, Collapse(buff.buffer)});
  return out;
}
void EmitFromMultiBuffer(const std::vector<CollapsedBuffer>& buffers) {
  using namespace emit_manifest;
  auto& tmp = buffers;
  ASTContext context;
  emit_manifest::Tokenizer tokens(context, tmp[0].text.c_str());
  auto get_chunk = [&](ChunkSrc* src) -> const std::string& {
    int id = stoi(std::string(src->id.str));
    assert(id >= 0 && id < tmp.size());
    return tmp[id].text;
  };
  Module* m = emit_manifest::parser::DoParse(tokens);
  for (EmitFileDecl* emit_file : m->decls) {
    EmitStream stream;
    for (Action* action_ : emit_file->actions) {
      switch (action_->getKind()) {
      case Action::Kind::PragmaOnce: {
        stream.stream() << "#pragma once\n\n";
        break;
      } case Action::Kind::Import: {
        auto* action = reinterpret_cast<ImportAction*>(action_);
        stream.stream() << "#include " << action->filename.str << "\n";
        break;
      } case Action::Kind::HdrChunk: {
        auto* action = reinterpret_cast<HdrChunkAction*>(action_);
        stream.stream() << get_chunk(action->id) << "\n";
        break;
      } case Action::Kind::FwdDeclareFunc: {
        auto* action = reinterpret_cast<FwdDeclareFuncAction*>(action_);
        stream.stream() << get_chunk(action->id) << ";\n";
        break;
      } case Action::Kind::DefineFunc: {
        auto* action = reinterpret_cast<DefineFuncAction*>(action_);
        stream.stream() << get_chunk(action->sig_id) << " {\n";
        stream.stream() << get_chunk(action->body_id) << "\n}\n";
        break;
      }
      }
    }
    stream.write(std::string(".generated/") + Unescaped(emit_file->filename.str));
  }
}
