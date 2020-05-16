#pragma once

#include "gui/widget-helper.h"
#include "gui/font-face.h"
#include "gui/buffer.h"
#include "rules/template-support.h"
#include "parser/parser-support.h"
#include "gui/buffer-edit.h"
#include "gui/buffer-view.h"
#include "gen/gui/view_manifest.h"

#include <assert.h>
#include <functional>

struct ChunkViewTemplate : public ChunkViewState {

  struct BufferLayoutState {
    explicit BufferLayoutState(ChunkViewTemplate& t) : t(t) {
      width = GetLineNumberRenderingWidth(std::max<size_t>(100, t.compute_num_lines()), font);
      st = gui::Point {2 + width, 2 + t.scroll};
    }
    ChunkViewTemplate& t;
    gui::FontLayoutFace* font = gui::DefaultFont();
    double width;
    gui::Point st;
    size_t buffer_id = 0;

    bool FindChunkViewPos(gui::Point pt, size_t& buffer_id_, BufferPos& cursor, size_t chunk_id, bool toplevel = true) {
      const auto& buffer = *t.GetChunk(chunk_id);
      st.x = 2 + width;
      if (!toplevel) {
        st.x += 20;
      }
      if (FindPos(st, pt, font, cursor, buffer)) { buffer_id_ = buffer_id; return true; }
      buffer_id += 1;
      return false;
    }
  };

  struct BufferDrawingState : public BufferLayoutState {
    BufferDrawingState(gui::DrawCtx& cr, size_t window_height, ChunkViewTemplate& t) : BufferLayoutState(t), cr(cr), window_height(window_height) {}
    size_t line_no = 1;
    gui::DrawCtx& cr;
    size_t window_height;
    void DrawBufferLines(size_t chunk_id) {
      const auto& buffer = *t.GetChunk(chunk_id);
      cr.set_source_rgb(1.0, 0.0, 0.0);
      DoDrawLines(cr, {2, st.y}, width, line_no, buffer.lines.size(), window_height, font);
    }
    void DrawTopLevelLine(size_t chunk_id) {
      const auto& buffer = *t.GetChunk(chunk_id);
      cr.set_source_rgb(1.0, 1.0, 0.0);
      cr.move_to({2 + width + 13, st.y + 6});
      cr.line_to({2 + width + 13, st.y + font->height() * buffer.lines.size()});
      cr.stroke();
    }
    void DrawJustBuffer(size_t chunk_id, size_t indent = 0) {
      const auto& buffer = *t.GetChunk(chunk_id);
      cr.set_source_rgb(1.0, 1.0, 0.0);
      st.x = 2 + width + indent;
      ::DrawBuffer(cr, st, buffer, buffer_id == t.buffer_id_ ? &t.cursor : nullptr);
      st.y += font->height() * buffer.lines.size();
      line_no += buffer.lines.size();
      buffer_id += 1;
    }
    void DrawNoEditBuffer(size_t chunk_id, size_t indent = 0) {
      const auto& buffer = *t.GetChunk(chunk_id);
      cr.set_source_rgb(0.88, 0.88, 0.88);
      st.x = 2 + width + indent;
      ::DrawBuffer(cr, st, buffer, nullptr);
      st.y += font->height() * buffer.lines.size();
    }
    void DrawBuffer(size_t chunk_id, bool toplevel = true) {
      DrawBufferLines(chunk_id);
      if (!toplevel) DrawTopLevelLine(chunk_id);
      DrawJustBuffer(chunk_id, toplevel ? 0 : 20);
    }

    void Annotate(const std::string& str) {
      st.x = 2;
      cr.set_source_rgb(0.5, 1.0, 0.5);
      std::vector<cairo_glyph_t> glyphs;
      font->LayoutWrap(st, glyphs, str);
      font->Flush(cr, glyphs);
      st.x = 2 + width;
      st.y += font->height();
    }
  };

  class Decl {
   public:
    virtual ~Decl() {}

    virtual size_t GetBufferIndex(size_t id) = 0;
    virtual size_t count() = 0;
    virtual size_t num_lines(ChunkViewTemplate& templ) = 0;

    virtual void Draw(BufferDrawingState& state) = 0;
    virtual bool FindChunkViewPos(BufferLayoutState& state, gui::Point pt, size_t& buffer_id_, BufferPos& cursor) = 0;
    virtual void Emit(ChunkViewTemplate& templ, std::ostream& h_stream, std::ostream& cc_stream) = 0;
    virtual void EmitMetadata(ChunkViewTemplate& templ, std::ostream& stream) = 0;
  };

  class FileConfigDecl : public Decl {
   public:
    explicit FileConfigDecl(int chunk_id) : chunk_id(chunk_id) {}
    size_t GetBufferIndex(size_t id) override { return chunk_id; }
    size_t count() override { return 0; }
    size_t num_lines(ChunkViewTemplate& templ) override { return 0; } // templ.GetChunk(chunk_id)->lines.size(); }
    
    void Draw(BufferDrawingState& state) override {
      state.DrawNoEditBuffer(chunk_id);
    }
    bool FindChunkViewPos(BufferLayoutState& state, gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override {
      state.st.y += state.font->height() * (state.t.GetChunk(chunk_id)->lines.size());
      return false;
    }
    void Emit(ChunkViewTemplate& templ, std::ostream& h_stream, std::ostream& cc_stream) override {
    }

    void EmitMetadata(ChunkViewTemplate& templ, std::ostream& stream) override {
      stream << "file_config " << chunk_id << "\n";
    }

    int chunk_id;
  };
  int config_chunk_id;

  class RawDecl : public Decl {
   public:
    explicit RawDecl(int chunk_id, bool flag = true) : chunk_id(chunk_id), hraw_type(flag) {}
    size_t GetBufferIndex(size_t id) override { return chunk_id; }
    size_t count() override { return 1; }
    size_t num_lines(ChunkViewTemplate& templ) override { return templ.GetChunk(chunk_id)->lines.size(); }
    
    void Draw(BufferDrawingState& state) override {
      state.Annotate(hraw_type ? "h-raw:" : "cc-raw:");
      state.DrawBuffer(chunk_id);
    }
    bool FindChunkViewPos(BufferLayoutState& state, gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override {
      state.st.y += state.font->height(); 
      return state.FindChunkViewPos(pt, buffer_id_, cursor, chunk_id);
    }
    void Emit(ChunkViewTemplate& templ, std::ostream& h_stream, std::ostream& cc_stream) override {
      templ.Emit(chunk_id, (hraw_type ? h_stream : cc_stream));
    }
    void EmitMetadata(ChunkViewTemplate& templ, std::ostream& stream) override {
      stream << "raw " << (hraw_type ? "true" : "false") << " " << chunk_id << "\n";
    }

    int chunk_id;
    bool hraw_type = false;
  };

  class DepDecl : public Decl {
   public:
    explicit DepDecl(int chunk_id, bool hdr_type = true) : hdr_type(hdr_type), chunk_id(chunk_id) {}
    size_t count() override { return 1; }
    size_t GetBufferIndex(size_t id) override { return chunk_id; }
    size_t num_lines(ChunkViewTemplate& templ) override { return templ.GetChunk(chunk_id)->lines.size(); }
    void Draw(BufferDrawingState& state) override {
      state.Annotate("deps:");
      state.DrawBuffer(chunk_id);
    }
    bool FindChunkViewPos(BufferLayoutState& state, gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override {
      state.st.y += state.font->height(); 
      return state.FindChunkViewPos(pt, buffer_id_, cursor, chunk_id);
    }
    void Emit(ChunkViewTemplate& templ, std::ostream& h_stream, std::ostream& cc_stream) override {
//      templ.Emit(chunk_id, (hdr_type ? h_stream : cc_stream));
    }
    void EmitMetadata(ChunkViewTemplate& templ, std::ostream& stream) override {
      stream << "dep " << (hdr_type ? "true" : "false") << " " << chunk_id << "\n";
    }
    bool hdr_type = false;
    int chunk_id;
  };

  class FuncDecl : public Decl {
   public:
    explicit FuncDecl(int sig_id, int body_id) : sig_id(sig_id), body_id(body_id) {}
    size_t count() override { return 2; }
    size_t GetBufferIndex(size_t id) override { return id == 0 ? sig_id : body_id; }
    size_t num_lines(ChunkViewTemplate& templ) override { return templ.GetChunk(sig_id)->lines.size() + templ.GetChunk(body_id)->lines.size(); }
    void Draw(BufferDrawingState& state) override {
      state.Annotate("func-decl:");
      state.DrawBuffer(sig_id);
      state.DrawBuffer(body_id, false);
    }
    bool FindChunkViewPos(BufferLayoutState& state, gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override {
      state.st.y += state.font->height(); 
      return state.FindChunkViewPos(pt, buffer_id_, cursor, sig_id)
          || state.FindChunkViewPos(pt, buffer_id_, cursor, body_id, false);
    }
    void Emit(ChunkViewTemplate& templ, std::ostream& h_stream, std::ostream& cc_stream) override {
      templ.Emit(sig_id, h_stream);
      h_stream << ";\n";
      templ.Emit(sig_id, cc_stream);
      cc_stream << "{\n";
      templ.Emit(body_id, cc_stream);
      cc_stream << "}\n";
    }
    void EmitMetadata(ChunkViewTemplate& templ, std::ostream& stream) override {
      stream << "func " << sig_id << " " << body_id << "\n";
    }
    int sig_id;
    int body_id;
  };

  size_t GetBufferId(size_t buffer_id) const {
    size_t i = 0;
    size_t base_id = 0;
    for (; i < decls.size(); ++i) {
      base_id += decls[i]->count();
      if (base_id > buffer_id) break; 
    }
    return i;
  }

  Buffer* GetBuffer(size_t buffer_id) override {
    for (auto& decl : decls) {
      size_t count = decl->count();
      if (buffer_id < count) {
        return (*buffers)[decl->GetBufferIndex(buffer_id)].get();
      }
      buffer_id -= count;
    }
    fprintf(stderr, "Out of range...\n");
    __builtin_trap();
  }

  size_t num_buffers() override {
    size_t out = 0;
    for (auto& decl : decls) { out += decl->count(); }
    return out;
  }
  size_t compute_num_lines() {
    size_t num_lines = 0;
    for (auto& decl : decls) num_lines += decl->num_lines(*this);
    return num_lines;
  }
  Buffer* GetChunk(size_t chunk_id) {
    return (*buffers)[chunk_id].get();
  }
  void Draw(gui::DrawCtx& cr, size_t window_height, size_t window_width) override {
    BufferDrawingState state(cr, window_height, *this);
    for (size_t i = 0; i < decls.size(); ++i) {
      if (i > 0) state.st.y += 20;
      decls[i]->Draw(state);
    }

    cr.set_source_rgb(1.0, 0.0, 0.0);
    DoDrawTwidle(cr, {2, state.st.y}, window_height, state.font);
  }

  void Emit(size_t chunk_id, std::ostream& stream) {
    stream << Collapse(*GetChunk(chunk_id));
  }

  bool FindChunkViewPos(gui::Point pt, size_t& buffer_id_, BufferPos& cursor) override {
    BufferLayoutState state(*this);
    // auto* font = gui::DefaultFont();
    double width = state.width;

    gui::Point st {2 + width, 2 + scroll};

    for (size_t i = 0; i < decls.size(); ++i) {
      if (i > 0) state.st.y += 20;
      if (decls[i]->FindChunkViewPos(state, pt, buffer_id_, cursor)) { return true; }
    }
    return false;
  }

  void Refresh(const Buffer& buffer) { Refresh(Collapse(buffer)); }
  void Refresh(string_view data);
  void EmitManifestChunk(Buffer& out_buffer) {
    std::string data;
    std::stringstream ss{data};
    for (auto& decl : decls) {
      decl->EmitMetadata(*this, ss);
    }
    ss.flush();
    out_buffer.lines.clear();
    Init(out_buffer, ss.str());
  }
  std::unique_ptr<Decl> Delete(size_t id) {
    auto it = decls.begin() + id;
    auto tmp = std::move(*it);
    decls.erase(it);
    cursor = {0, 0};
    buffer_id_ = 0;
    return tmp;
  }
  std::vector<std::unique_ptr<Decl>> decls;
  std::vector<std::unique_ptr<Buffer>>* buffers = nullptr;
};

struct KeyFeed;

struct SillyStruct2 {
  enum class Mode {
    INSERT,
    ESCAPE,
    COLON,
  };
  Mode mode = Mode::INSERT;

  std::string colon_text;
  size_t col_col = 0;

  std::vector<uint32_t> command_history;
  std::vector<std::unique_ptr<Buffer>> buffers;
  size_t view_id_ = 0;
  std::vector<std::unique_ptr<ChunkViewTemplate>> views;
  size_t last_view_id_ = 1;
  std::unique_ptr<ChunkViewTemplate::Decl> copy_decl;

  Buffer* GetChunk(int id);
  int AddBuffer() {
    int id = int(buffers.size());
    buffers.emplace_back(std::make_unique<Buffer>());
    return id;
  }
  void Thing(const char* filename);
  void Whatever(const char* filename);
//  void RefreshViews();
  void DoBuild();
  void RefreshManifest();
  void RefreshTemplateViews();
  bool HandleChunkViewTemplate(ChunkViewTemplate& temp, const std::string& colon_text);

  std::string filename_ = ".gui/self-edit";
  SillyStruct2() { Whatever(filename_.c_str()); } // Thing(filename_.c_str()); }

  void Draw(gui::DrawCtx& cr, gui::Shape shape);
  void DrawOther(gui::DrawCtx& cr, gui::Shape shape);
  void DrawStatus(gui::DrawCtx& cr, gui::Shape shape);
  void DrawLineNo(gui::DrawCtx& cr, gui::Shape shape);
  bool MotionEvent(GdkEventMotion* event);

  bool ButtonPressSidebar(GdkEventButton* button);
  bool ButtonPress(GdkEventButton* button);
  bool ButtonRelease(GdkEventButton* button);

  bool ScrollEvent(GdkEventScroll* event);
  bool KeyPress(GdkEventKey* event);

  bool KeyPressEscape(KeyFeed& feed);
};

struct SillyStruct2SideView {
  void Draw(gui::DrawCtx& cr, gui::Shape shape);
  bool ButtonPress(GdkEventButton* button);
  SillyStruct2& obj;
};

struct SillyStruct2LineNumView {
  void Draw(gui::DrawCtx& cr, gui::Shape shape) {
    obj.DrawLineNo(cr, shape);
  }
  bool ButtonPress(GdkEventButton* button) { return false; }
  SillyStruct2& obj;
};

struct SillyStruct2StatusView {
  void Draw(gui::DrawCtx& cr, gui::Shape shape) {
    obj.DrawStatus(cr, shape);
  }
  bool ButtonPress(GdkEventButton* button) { return false; }
  SillyStruct2& obj;
};

struct SillyStruct2Menu {
  void Draw(gui::DrawCtx& cr, gui::Shape shape) {
    obj.DrawStatus(cr, shape);
  }
  bool ButtonPress(GdkEventButton* button) { return false; }
  SillyStruct2& obj;
};

struct SillyLayoutExample {
  SillyStruct2 s2;
  SillyStruct2SideView s2v = {s2};
  SillyStruct2LineNumView s2v_lineno = {s2};
  SillyStruct2StatusView s2v_status = {s2};
};

inline void SillyLayoutExample_Layout(const SillyLayoutExample& s, gui::Shape shape,
                                      gui::Rectangle& rect1,
                                      gui::Rectangle& rect2,
                                      gui::Rectangle& rect3 //,
                                      //gui::Rectangle& rect4,
                                      //gui::Rectangle& rect5
                                      ) {
  gui::DoHSplit(ConvertRectangle(shape), rect1, rect2, 300);
  gui::DoVSplit(rect2, rect2, rect3, -int(gui::DefaultFont()->height() + 4));
//  gui::DoVSplit(rect1, rect4, rect1, int(gui::DefaultFont()->height() * 4 + 8));
//  gui::DoVSplit(rect4, rect5, rect4, int(gui::DefaultFont()->height() * 2 + 4));
}
