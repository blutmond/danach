#include "gui/widget-helper.h"

void BasicWindowState::InitBasicState() {
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  drawing_area = gtk_drawing_area_new();
  gtk_widget_set_size_request(drawing_area, window_width_, window_height_);

  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), window_width_, window_height_);

  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  gtk_widget_show_all(window);

  gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK);
}

void BasicWindowState::SigConnect(GtkWidget* object, const char* name, GCallback callback, void* data) {
  gulong event = g_signal_connect(object, name, callback, data);
  events.push_back({object, event});
}

void BasicWindowState::DeregisterEvents() {
  for (const auto& event : events) {
    g_signal_handler_disconnect(event.first, event.second);
  }
}

void BasicWindowState::redraw() {
  if (!needs_redraw) {
    needs_redraw = true;
    gtk_widget_queue_draw(window);
  }
}

namespace gui {
void DoHSplit(Rectangle input, Rectangle& out1, Rectangle& out2, int split_point) {
  split_point = std::min(std::max(0, split_point), input.shape.w);
  out1 = input;
  out1.shape.w = split_point;
  out2 = input;
  out2.shape.w -= split_point;
  out2.st.x += double(split_point);
}

void DoVSplit(Rectangle input, Rectangle& out1, Rectangle& out2, int split_point) {
  if (split_point < 0) { split_point = input.shape.h + split_point; }
  split_point = std::min(std::max(0, split_point), input.shape.h);
  out1 = input;
  out1.shape.h = split_point;
  out2 = input;
  out2.shape.h -= split_point;
  out2.st.y += double(split_point);
}
} // namespace gui
