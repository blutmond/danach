#include "gui/widget-spec-lowering.h"
#include "gen/widgets/widgets-spec.h"

namespace widget_spec {

struct EventSpec {
  const char* widget_name;
  const char* event_name;
  const char* event_type = nullptr;
  const char* event_obj = nullptr;
};

void EmitEventHandlerHeader(std::ostream& stream,
                      const EventSpec& spec) {
  stream << "  SigConnect(" << spec.widget_name << ", \"" << spec.event_name << R"(", G_CALLBACK((
          +[](GtkWidget*, )";
  if (spec.event_type) {
    stream << spec.event_type << " " << spec.event_obj << ", ";
  }
  stream << R"(Window* state) -> gboolean {
)";
}

constexpr const char* handler_footer = R"(
    return TRUE;
  })), this);
)";

void DoRectLayout(std::ostream& cc_stream, size_t N) {
  for (size_t i = 0; i < N; ++i) {
    cc_stream << "    gui::Rectangle rect" << i << ";\n";
  }
  cc_stream << "    SillyLayoutExample_Layout(state->example, {static_cast<int>(state->window_width_), static_cast<int>(state->window_height_)}";
  for (size_t i = 0; i < N; ++i) {
    cc_stream << ", rect" << i;
  }
  cc_stream << ");\n";
}

void EmitWidgetSpec(ASTContext& ast_ctx, const std::string& spec,
                      std::ostream& h_stream,
                      std::ostream& cc_stream) {
  widget_spec::Tokenizer tokens(ast_ctx, spec.c_str());
  // auto* module = widget_spec::parser::DoParse(tokens);

  cc_stream << R"(
#include "gui/widget-helper.h"
#include "gui/editor-widget.h"

class Window : public BasicWindowState {
 public:
  Window() {
    InitBasicState();
    InitEvents();
  }

  void InitEvents() {
)";

  EmitEventHandlerHeader(cc_stream, {"window", "destroy"});
  cc_stream << R"(    gtk_main_quit();
   delete state;)" << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "draw", "cairo_t*", "cr_ptr"});
  cc_stream << R"(
    auto& cr = *gui::DrawCtx::wrap(cr_ptr);
)";
  const char* fnames[] = {"s2v", "s2", "s2v_status"};
  size_t N = sizeof(fnames) / sizeof(fnames[0]);
  DoRectLayout(cc_stream, N);
  for (size_t i = 0; i < N; ++i) {
    cc_stream << "    SaveClipTranslate(cr, rect" << i << " );\n";
    cc_stream << "    state->example." << (fnames[i]) << ".Draw(cr, rect" << i << ".shape);\n";
    cc_stream << "    cr.restore();\n";
  }
  cc_stream << "    state->needs_redraw = false;\n" << handler_footer;
  
  EmitEventHandlerHeader(cc_stream, {"window", "button-press-event", "GdkEventButton*", "button"});
  cc_stream << R"(    bool is_event = (button->time != state->press_time);
    state->press_time = button->time;
    if (is_event) {
      RestoreEventXY button_restore(button);
)";
  DoRectLayout(cc_stream, N);
  cc_stream << "    if (false) {\n";
  for (size_t i = 0; i < N; ++i) {
    cc_stream << "      } else if (gui::TestRectangleClick(button, rect" << i << ")) {\n";
    cc_stream << "        if (state->example." << fnames[i] << ".ButtonPress(button)) {\n";
    cc_stream << "          state->redraw();\n";
    cc_stream << "        }\n";
  }
  cc_stream << "    }\n";
  cc_stream << "  }" << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "configure-event", "GdkEventConfigure*", "config"});
  cc_stream << R"(
    if (state->window_width_ != config->width || state->window_height_ != config->height) {
      state->window_width_ = config->width;
      state->window_height_ = config->height;
      state->redraw();
    }
    return FALSE;
)";
  cc_stream << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "key-press-event", "GdkEventKey*", "event"});
  cc_stream << R"(
    if (state->HandleSpecialEvents(event)) {
      state->redraw();
    } else if (state->example.s2.KeyPress(event)) {
      state->redraw();
    }
)";
  cc_stream << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "scroll-event", "GdkEventScroll*", "event"});
  cc_stream << R"(
    if (state->example.s2.ScrollEvent(event)) {
      state->redraw();
    }
)";
  cc_stream << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "scroll-event", "GdkEventScroll*", "event"});
  cc_stream << R"(
    if (state->example.s2.ScrollEvent(event)) {
      state->redraw();
    }
)";
  cc_stream << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "motion-notify-event", "GdkEventMotion*", "event"});
  DoRectLayout(cc_stream, N);
  cc_stream << "    RestoreEventXY button_restore(event); \n";
  cc_stream << R"(
    if (TestRectangleInside(event->x, event->y, rect1)) {
      if (state->example.s2.MotionEvent(event)) {
        state->redraw();
      } 
    }
)";
  cc_stream << handler_footer;

  EmitEventHandlerHeader(cc_stream, {"window", "button-release-event", "GdkEventButton*", "event"});
  DoRectLayout(cc_stream, N);
  cc_stream << "    RestoreEventXY button_restore(event); \n";
  cc_stream << R"(
    if (TestRectangleInside(event->x, event->y, rect1)) {
      if (state->example.s2.ButtonRelease(event)) {
        state->redraw();
      } 
    }
)";
  cc_stream << handler_footer;

cc_stream << R"(
  }

  uint32_t press_time = -1;
  SillyLayoutExample example;
};

)";

  cc_stream << R"(
int main(int argc, char** argv) {
  gtk_init(&argc, &argv);

  new Window();

  gtk_main();
}
)";
}

}  // namespace
