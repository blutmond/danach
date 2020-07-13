#include "gui/so-handle.h"
#include "gui/widget-helper.h"
#include "gui/so-handoff-lib.h"
#include "parser/ast-context.h"
#include "simdb/reflect.h"
#include "simdb/base-widgets.h"

#include <iostream>
#include <stdlib.h>

class WindowState : public BasicWindowState {
 public:
  WindowState() {
    InitBasicState();
    InitEvents();
  }
  Palette palette;

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

    state->palette.Draw(cr);
/*
    for (size_t i = state->windows.size(); i > 0;) { 
      --i;
      auto& window = state->windows[i];
      auto rect = window->decorated_rect;
      DrawDecorations(cr, rect);
      SaveClipTranslate(cr, InsideDecoration(rect));
      window->Draw(cr);
      cr.restore();
    }
    if (state->tab_overlay_visible) {
      gui::Rectangle rect{{0,0}, {800, 800}};
      SaveClipTranslate(cr, rect);
      cr.set_source_rgb(0.0, 0.2, 0.2);
      cr.paint();
      // state->DrawTabOverlay(cr);
      cr.restore();
    }
*/
    state->needs_redraw = false;

    return TRUE;
  })), this);
  SigConnect(window, "button-press-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* button, WindowState* state) -> gboolean {
    bool is_event = (button->time != state->press_time);
    state->press_time = button->time;
    if (is_event) {
      RestoreEventXY button_restore(button);
      gui::Point pt{button->x, button->y};

/*
      size_t num_windows = state->windows.size();
      for (size_t i = 0; i < num_windows; ++i) {
        auto* window = state->windows[i].get();
        gui::Rectangle rect = window->decorated_rect;
        if (gui::TestInside(rect, pt)) {
          size_t dir;
          if (GetResizeHover(rect, pt, dir)) {
            state->dragger.reset(new ResizeDragger(window, dir, pt));
            break;
          } else if (pt.y < rect.st.y + 25) {
            state->dragger.reset(new WindowDragger(window, pt));
            state->set_cursor(state->cursors.grabbing.get());
            break;
          }
          if (i != 0) {
            state->MoveToTop(i);
            state->redraw();
          }
          auto tmp = InsideDecoration(rect).st;
          button->x -= tmp.x;
          button->y -= tmp.y;
          
          window->ButtonPress(button);
          break;
        }
      }
*/
    }
    return TRUE;
  })), this);
  SigConnect(window, "button-release-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventButton* event, WindowState* state) -> gboolean {
  //  state->dragger = nullptr;
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
    } else {
      state->palette.KeyPress(event);
      state->redraw();
    }
    return TRUE;
  })), this);
  SigConnect(window, "scroll-event", G_CALLBACK((
          +[](GtkWidget*, GdkEventScroll* event, WindowState* state) -> gboolean {

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
 // reflect_test();
 // exit(0);
  if (argc > 1) {
    fprintf(stderr, "calling(%s): %s\n", argv[0], argv[1]);
    new WindowState(); // argv[1]);
  } else {
    new WindowState(); // ".gui/data");
  }
}
