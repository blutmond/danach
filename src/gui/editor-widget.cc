#include "gui/editor-widget.h"
#include <exception>

struct NoMoreKeyvalException : public std::exception {
	const char * what () const throw () {
    return "No more keyval. (This is not an error)";
  }
};

string_view NextToken(string_view& data) {
  auto tmp = data;
  size_t n = 0;
  while (!data.empty() && data[0] != '\n' && data[0] != ' ') {
    data.remove_prefix(1);
    ++n;
  }
  while (!data.empty() && (data[0] == '\n' || data[0] == ' ')) {
    data.remove_prefix(1);
  }
  return tmp.substr(0, n);
}

void pango_cairo_gtk_set_copy_string(string_view sp) {
  auto* clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
  gtk_clipboard_set_text(clipboard, (const gchar *)sp.data(), sp.size());
}

void SaveFile(const std::string& filename, const std::vector<IdBuffer>& buffers) {
  EmitStream out2;
  SaveFile(out2.stream(), buffers);
  out2.write(filename);
}

void SillyStruct2::Whatever(const char* filename) {
  auto tmp = ParseMultiBuffer(LoadFile(filename));

  for (auto& id_b : tmp) {
    buffers.emplace_back(std::make_unique<Buffer>(std::move(id_b.buffer)));
  }

  if (buffers.empty()) {
    AddBuffer();
    auto temp = std::make_unique<ChunkViewTemplate>();
    temp->title = "Unknown title";
    temp->buffers = &buffers;
    temp->config_chunk_id = AddBuffer();
    views.push_back(std::move(temp));
    return;
  }

  auto data_str = Collapse(*buffers[0]);
  string_view data{data_str};

  while (!data.empty()) {
    if (NextToken(data) != "file") {
      std::cerr << "Bad manifest: ---\n" << data_str << "\n---\n";
      __builtin_trap();
    }
    int id = std::stoi(std::string(NextToken(data)));
    size_t n = data.find('\n');
    auto temp = std::make_unique<ChunkViewTemplate>();
    temp->title = std::string(data.substr(0, n));
    data.remove_prefix(temp->title.size());
    temp->buffers = &buffers;
    temp->config_chunk_id = id;
    views.push_back(std::move(temp));
    if (n != string_view::npos) data.remove_prefix(1);
  }
  for (auto& view : views) {
    if (auto* t = dynamic_cast<ChunkViewTemplate*>(view.get())) {
      t->Refresh(*t->GetChunk(t->config_chunk_id));
    }
  }
}

void SillyStruct2::Thing(const char* filename) {
  auto tmp = ParseMultiBuffer(LoadFile(filename));

  for (auto& id_b : tmp) {
    buffers.emplace_back(std::make_unique<Buffer>(std::move(id_b.buffer)));
  }

  // RefreshViews();
}

Buffer* SillyStruct2::GetChunk(int id) {
  while (id >= buffers.size()) {
    buffers.emplace_back(std::make_unique<Buffer>());
  }
  assert(id >= 0 && id < buffers.size());
  return buffers[id].get();
}

/*
void SillyStruct2::RefreshViews() {
  views.clear();
  ASTContext ctx;
  auto view_manifest_str = Collapse(*buffers[1]);
  view_manifest::Tokenizer tokens(ctx, view_manifest_str.c_str());
  auto* m = view_manifest::parser::DoParse(tokens);

  auto get_chunk = [&](view_manifest::ChunkSrc* src) -> Buffer* {
    return GetChunk(stoi(std::string(src->id.str)));
  };
  for (view_manifest::EmitFileDecl* decl : m->decls) {
    using namespace view_manifest;
    auto view = std::make_unique<ChunkView>();
    view->title = std::string(decl->name.str);
    size_t i = 0;
    for (auto* action_ : decl->actions) {
      if (i != 0) { view->gaps.push_back(20); }
      switch (action_->getKind()) {
      case Action::Kind::Raw: {
        auto* action = reinterpret_cast<RawAction*>(action_);
        view->buffers.push_back(get_chunk(action->id));
        view->decorations.push_back({});
        break;
      } case Action::Kind::DefineFunc: {
        auto* action = reinterpret_cast<DefineFuncAction*>(action_);
        view->buffers.push_back(get_chunk(action->sig_id));
        view->buffers.push_back(get_chunk(action->body_id));
        view->decorations.push_back({});
        view->decorations.push_back({false});
        view->gaps.push_back(0);
      }
      }
      ++i;
    }
    views.push_back(std::move(view));
  }
}
*/

void SillyStruct2::DrawOther(gui::DrawCtx& cr, gui::Shape shape) {
  cr.set_source_rgb(0.0, 0.0, 0.0);
  cr.paint();

  cr.set_source_rgb(1.0, 0.0, 0.0);
  auto* font = gui::DefaultFont();

  {
    std::vector<cairo_glyph_t> glyphs;
    gui::Point opt {2, 2};
    
    cr.set_source_rgb(0.5, 1.0, 1.0);

    for (size_t i = 0; i < views.size(); ++i) {
      gui::Point st = opt;
      glyphs.clear();
      if (i == view_id_) {
        font->LayoutWrap(st, glyphs, "x ", 12, shape.w - 2);
      } else {
        font->LayoutWrap(st, glyphs, "- ", 12, shape.w - 2);
      }
      font->LayoutWrap(st, glyphs, views[i]->title, 12, shape.w - 2);
      font->Flush(cr, glyphs);
      opt.y = st.y + font->height() + 10;
    }
  }
}
void SillyStruct2::Draw(gui::DrawCtx& cr, gui::Shape shape) {
  cr.set_source_rgb(1.0, 0.0, 0.0);
  views[view_id_]->Draw(cr, shape.h, shape.w);
}

void SillyStruct2::DrawStatus(gui::DrawCtx& cr, gui::Shape shape) {
  auto* font = gui::DefaultFont();
  // Status line:
  {
    cr.set_source_rgb(0.1, 0.1, 0.1);

    double h = font->height() + 4;
    auto st_pt = gui::Point{0, shape.h - h};
    cr.rectangle(st_pt,
                 gui::Point{static_cast<double>(shape.w), h});

    cr.fill();

    std::vector<cairo_glyph_t> glyphs_;

    st_pt.y += 2;
    if (mode == Mode::INSERT) {
      cr.set_source_rgb(1.0, 0.0, 0.0);
      font->LayoutWrap(st_pt, glyphs_, "-- INSERT --");
      font->Flush(cr, glyphs_);
    } else if (mode == Mode::COLON) {
      font->LayoutWrap(st_pt, glyphs_, ":");
      font->LayoutWrap(st_pt, glyphs_, colon_text);
      cr.set_source_rgb(1.0, 1.0, 0.0);
      font->Flush(cr, glyphs_);
    }
  }
}

void SillyStruct2SideView::Draw(gui::DrawCtx& cr, gui::Shape shape) {
  obj.DrawOther(cr, shape);
}
bool SillyStruct2SideView::ButtonPress(GdkEventButton* button) {
  return obj.ButtonPressSidebar(button);
}

bool SillyStruct2::ButtonPressSidebar(GdkEventButton* button) {
  gui::Point pt{button->x, button->y};
  auto* font = gui::DefaultFont();

  std::vector<cairo_glyph_t> glyphs;
  gui::Point opt {2, 2};

  for (size_t i = 0; i < views.size(); ++i) {
    gui::Point st = opt;
    glyphs.clear();
    font->LayoutWrap(st, glyphs, "- ", 12, 300.0 - 2);
    font->LayoutWrap(st, glyphs, views[i]->title, 12, 300.0 - 2);
    opt.y = st.y + font->height() + 10;
    if (pt.y < opt.y) {
      fprintf(stderr, "Going to view %zu\n", i);
      view_id_ = i;
      views[view_id_]->SanitizeCursor();
      return true;
    }
  }
  return false;
}

bool SillyStruct2::ButtonPress(GdkEventButton* button) {
  gui::Point pt{button->x, button->y};

  if (button->button == 2) {
    auto* clipboard = gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY);
    auto* data = gtk_clipboard_wait_for_text(clipboard);

    auto& view = *views[view_id_];

    if (view.num_buffers() == 0) return true;
    auto& cursor = view.cursor;
    auto& buffer = *view.GetBuffer(view.buffer_id_);
    for (char c : string_view(data)) {
      Insert(buffer, cursor, c);
    }

    g_free(data);
    return true;
  }
  auto& view = *views[view_id_];
  return view.FindChunkViewPos(pt, view.buffer_id_, view.cursor);
}

bool SillyStruct2::ScrollEvent(GdkEventScroll* event) {
  auto& scroll = views[view_id_]->scroll;
  auto* font = gui::DefaultFont();
  if (event->direction == GDK_SCROLL_UP) {
    scroll += font->height() * 3;
  } else if (event->direction == GDK_SCROLL_DOWN) {
    scroll -= font->height() * 3;
  }
  if (scroll > 0) { scroll = 0; }
  return true;
}

struct KeyFeed {
  uint32_t Next() {
    if (i < command_history.size()) return command_history[i++];
    throw NoMoreKeyvalException();
  }
  std::vector<uint32_t>& command_history;
  size_t i = 0;
};

bool SillyStruct2::KeyPressEscape(KeyFeed& feed) {
  auto keyval = feed.Next();
  auto& view = *views[view_id_];
  if (keyval == ':') {
    mode = Mode::COLON;
    colon_text = "";
    col_col = 0;
  }
  if (view.num_buffers() == 0) return true;
  auto& buffer = *view.GetBuffer(view.buffer_id_);
  auto* temp = dynamic_cast<ChunkViewTemplate*>(&view);
  auto& cursor = view.cursor;
  if (DoEscapeKeyPress(view, keyval)) {
  } else if (keyval == 'i') {
    mode = Mode::INSERT;
  } else if (keyval == 'o') {
    mode = Mode::INSERT;
    cursor.col = buffer.lines[cursor.row].size();
    Insert(buffer, cursor, '\n');
  } else if (keyval == 'd') {
    keyval = feed.Next();
    if (keyval == 'd') {
      fprintf(stderr, "Something\n");
    } else if (keyval == GDK_KEY_apostrophe) {
      keyval = feed.Next();
      if (keyval == GDK_KEY_a) {
        fprintf(stderr, "Delete to mark 'a\n");
      } else {
        fprintf(stderr, "unknown keyval (d'): %d, %s\n", keyval, gdk_keyval_name(keyval));
      }
    } else if (temp && keyval == 'c') {
      copy_decl = temp->Delete(temp->GetBufferId(view.buffer_id_));
    } else {
      fprintf(stderr, "unknown keyval (d): %d, %s\n", keyval, gdk_keyval_name(keyval));
    }
  } else if (copy_decl && keyval == 'p') {
    size_t chunk_pos = temp->GetBufferId(view.buffer_id_);
    temp->decls.insert(temp->decls.begin() + chunk_pos + 1, std::move(copy_decl));
  } else if (copy_decl && keyval == 'P') {
    size_t n = copy_decl->count();
    size_t chunk_pos = temp->GetBufferId(view.buffer_id_);
    temp->decls.insert(temp->decls.begin() + chunk_pos, std::move(copy_decl));
    view.buffer_id_ += n;
  } else {
    fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
    return false;
  }
  return true;
}

int GetId(string_view& data) {
  return std::stoi(std::string(NextToken(data)));
}

void ChunkViewTemplate::Refresh(string_view data) {
  decls.clear();
  while (true) {
    auto pdata = NextToken(data);
    if (pdata.empty()) break;
    if (pdata == "func") {
      int bid1 = GetId(data);
      int bid2 = GetId(data);
      decls.emplace_back(new ChunkViewTemplate::FuncDecl(bid1, bid2));
    } else if (pdata == "dep") {
      bool hdr_type = NextToken(data) == "true";
      int bid = GetId(data);
      decls.emplace_back(new ChunkViewTemplate::DepDecl(bid, hdr_type));
    } else if (pdata == "raw") {
      bool flag = NextToken(data) == "true";
      int bid = GetId(data);
      decls.emplace_back(new ChunkViewTemplate::RawDecl(bid, flag));
    } else if (pdata == "file_config") {
      int bid = GetId(data);
      decls.emplace_back(new ChunkViewTemplate::FileConfigDecl(bid));
    } else {
      std::cerr << "Unknown: " << pdata << std::endl;
      exit(-1);
    }
  }
}

void SillyStruct2::RefreshManifest() {
  auto& out_buffer = *GetChunk(0);
  std::string data;
  std::stringstream ss{data};
  for (auto& view : views) {
    ss << "file " << view->config_chunk_id << " " << view->title <<  "\n"; 
  }
  ss.flush();
  out_buffer.lines.clear();
  Init(out_buffer, ss.str());
}

bool SillyStruct2::HandleChunkViewTemplate(ChunkViewTemplate& temp, const std::string& colon_text) {
  auto& buffer_id = views[view_id_]->buffer_id_;
  if (colon_text == "si func") {
    int bid1 = AddBuffer();
    int bid2 = AddBuffer();
    auto it = temp.decls.begin() + temp.GetBufferId(buffer_id);
    temp.decls.insert(it, std::unique_ptr<ChunkViewTemplate::Decl>(new ChunkViewTemplate::FuncDecl(bid1, bid2)));
  } else if (colon_text == "si dep") {
    int bid1 = AddBuffer();
    auto it = temp.decls.begin() + temp.GetBufferId(buffer_id);
    temp.decls.insert(it, std::unique_ptr<ChunkViewTemplate::Decl>(new ChunkViewTemplate::DepDecl(bid1)));
  } else if (colon_text == "si raw") {
    int bid1 = AddBuffer();
    auto it = temp.decls.begin() + temp.GetBufferId(buffer_id);
    temp.decls.insert(it, std::unique_ptr<ChunkViewTemplate::Decl>(new ChunkViewTemplate::RawDecl(bid1)));
  } else if (colon_text == "si craw") {
    int bid1 = AddBuffer();
    auto it = temp.decls.begin() + temp.GetBufferId(buffer_id);
    temp.decls.insert(it, std::unique_ptr<ChunkViewTemplate::Decl>(new ChunkViewTemplate::RawDecl(bid1, false)));
  } else if (colon_text == "si file") {
    auto temp = std::make_unique<ChunkViewTemplate>();
    temp->title = "Some title2";
    temp->buffers = &buffers;
    temp->config_chunk_id = AddBuffer();
    views.push_back(std::move(temp));
  } else if (colon_text == "si silly") {
    DoBuild(); 
  } else if (colon_text == "si emit") {
    EmitStream h_stream;
    EmitStream cc_stream;
    for (auto& decl : temp.decls) {
      decl->Emit(temp, h_stream.stream(), cc_stream.stream());
    }
    h_stream.write("/tmp/silly.h");
    cc_stream.write("/tmp/silly.cc");
  } else if (colon_text == "si refresh") {
    temp.EmitManifestChunk(*temp.GetChunk(temp.config_chunk_id));
    RefreshManifest();
  } else if (colon_text == "rv") {
  } else {
    return false;
  }
  return true;
}

bool SillyStruct2::KeyPress(GdkEventKey* event) {
  auto keyval = event->keyval;
  if (keyval == GDK_KEY_F5) {
    fprintf(stderr, "Doing recompile\n");
    int status = system(".build/rules src/gui ide-dynamic.so");
    if (status == 0) {
      // Invoke compile here...
      // Should probably delay this until the end of the event so it can happen
      // at any point.

      /*
      fprintf(stderr, "[so-reloader] ?? doing load number: %zu\n", main::GetJumpId());
      const char* so_name = (main::GetJumpId() % 2) == 0 ? "/tmp/gui-so-red.so" : "/tmp/gui-so-black.so";
      const char* base_so_name = ".build/ide-dynamic.so";
      fprintf(stderr, "[so-reloader] Loading from: %s\n", so_name);

      link(base_so_name, so_name);
      SoHandle handle(so_name, RTLD_NOW | RTLD_LOCAL);
      unlink(so_name);

      handle.get_sym<void(GtkWidget*,GtkWidget*,size_t,size_t, const char*)>("gtk_transfer_window")(window, drawing_area,
                                                                                                    window_width_, window_height_, filename_.c_str());
      main::SwapToNewSoFile(std::move(handle));
      delete this;
      */
    } else {
      fprintf(stderr, "Could not compile....\n");
    }
    return true;
  }

  if (mode == Mode::INSERT) {
    if (DoKeyPress(*views[view_id_], keyval)) {
    } else if (keyval == GDK_KEY_Escape) {
      mode = Mode::ESCAPE;
    } else {
      fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return false;
    }
    return true;
  } else if (mode == Mode::ESCAPE) {
    command_history.push_back(keyval);
    try {
      KeyFeed feed{command_history};
      bool result = KeyPressEscape(feed);
      command_history.clear();
      return result;
    } catch(NoMoreKeyvalException& e) {}
    return true;
  } else if (mode == Mode::COLON) {
    auto* temp = dynamic_cast<ChunkViewTemplate*>(views[view_id_].get());
    if (keyval >= ' ' && keyval <= '~') {
      colon_text.insert(colon_text.begin() + col_col, keyval);
      col_col += 1;
    } else if (keyval == GDK_KEY_BackSpace && col_col > 0) {
      col_col -= 1;
      colon_text.erase(colon_text.begin() + col_col);
    } else if (keyval == GDK_KEY_Left && col_col > 0) {
      col_col -= 1;
    } else if (keyval == GDK_KEY_Right && col_col < colon_text.size()) {
      col_col += 1;
    } else if (keyval == GDK_KEY_Return) {
      if (colon_text == "w") {
        for (auto& view : views) {
          if (auto* t = dynamic_cast<ChunkViewTemplate*>(view.get())) {
            t->EmitManifestChunk(*t->GetChunk(t->config_chunk_id));
          }
        }
        RefreshManifest();
        std::vector<IdBuffer> buffers_out;
        for (size_t i = 0; i < buffers.size(); ++i) {
          buffers_out.push_back({i, buffers[i].get()});
        }
        SaveFile(filename_, buffers_out);
      } else if (colon_text == "t") {
        fprintf(stderr, "--------Eval--------\n");
        std::vector<IdBuffer> buffers_out;
        for (size_t i = 0; i < buffers.size(); ++i) {
          buffers_out.push_back({i, buffers[i].get()});
        }
        SaveFile(filename_, buffers_out);

        if (system(".build/run-buffer-tests") == 0) {
          fprintf(stderr, "... Test Success ...\n");
        } else {
          fprintf(stderr, "... Test Failure ...\n");
        }
      } else if (temp && HandleChunkViewTemplate(*temp, colon_text)) {
      } else {
        fprintf(stderr, "Unknown command: %s\n", colon_text.c_str());
      }
      mode = Mode::ESCAPE;
    } else if (keyval == GDK_KEY_Escape) {
      mode = Mode::ESCAPE;
    } else {
      fprintf(stderr, "unknown keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
      return false;
    }
    return true;
  } else {
    fprintf(stderr, "unknown mode and keyval: %d, %s\n", keyval, gdk_keyval_name(keyval));
    return false;
  }
}

bool SillyStruct2::MotionEvent(GdkEventMotion* event) {
//  fprintf(stderr, "whatever: %g %g %d\n", event->x, event->y, (int)(event->state & GDK_BUTTON1_MASK));
  return true;
}

bool SillyStruct2::ButtonRelease(GdkEventButton* event) {
//  fprintf(stderr, "~ whatever: %g %g\n", event->x, event->y);
  return true;
}
