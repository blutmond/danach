#pragma once

#include "gui/cairo-bindings.h"
#include "gui/font-face.h"

#include <vector>
#include <utility>
#include <gtk/gtk.h>

namespace gui {

struct Shape {
  int w;
  int h;
  Point AsPoint() const { return {double(w), double(h)}; }
};

struct Rectangle {
  Point st;
  Shape shape;
};

inline bool TestInside(gui::Rectangle rect, gui::Point pt_orig) {
  gui::Point pt = pt_orig - rect.st;
  gui::Point shape = rect.shape.AsPoint();
  return (pt.x >= 0 && pt.y >= 0 && pt.x < shape.x && pt.y < shape.y);
}

inline bool TestRectangleInside(double& x, double& y, gui::Rectangle rect) {
  gui::Point pt = gui::Point{x, y} - rect.st;
  if (pt.x >= 0 && pt.y >= 0 && pt.x < double(rect.shape.w) && pt.y < double(rect.shape.h)) {
    x -= rect.st.x;
    y -= rect.st.y;
    return true;
  }
  return false;
}

inline bool TestRectangleClick(GdkEventButton* button, gui::Rectangle rect) {
  return TestRectangleInside(button->x, button->y, rect);
}
inline bool TestRectangleClick(GdkEventScroll* event, gui::Rectangle rect) {
  return TestRectangleInside(event->x, event->y, rect);
}

void DoHSplit(Rectangle input, Rectangle& out1, Rectangle& out2, int split_point);
void DoVSplit(Rectangle input, Rectangle& out1, Rectangle& out2, int split_point);

inline Rectangle ConvertRectangle(Shape shape) { return {Point{0, 0}, shape}; }

} // namespace gui

struct BasicWindowState {
  size_t window_width_ = 1024 * 3 / 2;
  size_t window_height_ = 680 * 3 / 2;

  GtkWidget* window;
  GtkWidget* drawing_area;

  void InitBasicState();
  void DeregisterEvents();

  std::vector<std::pair<GtkWidget*, gulong>> events;
  void SigConnect(GtkWidget* object, const char* name, GCallback callback, void* data);

  gui::Shape shape() { return {int(window_width_), int(window_height_)}; }
  
  bool is_fullscreen_ = false;
  bool needs_redraw = true;
  void redraw();
  void GrabSeat(const GdkEvent *event = nullptr);
  void UngrabSeat();
  GdkSeat *seat = nullptr;
  void toggle_fullscreen();

  bool HandleSpecialEvents(GdkEventKey* event);
};

inline void SaveClipTranslate(gui::DrawCtx& cr, gui::Point st, gui::Point width) {
  cr.save();
  cr.rectangle(st, width);
  cr.clip();
  cr.translate(st);
}

inline void SaveClipTranslate(gui::DrawCtx& cr, gui::Rectangle rect) {
  SaveClipTranslate(cr, rect.st, {double(rect.shape.w), double(rect.shape.h)});
}

struct RestoreEventXY {
  explicit RestoreEventXY(GdkEventButton* button) : sx(&button->x), sy(&button->y), x(button->x), y(button->y) {}
  explicit RestoreEventXY(GdkEventMotion* button) : sx(&button->x), sy(&button->y), x(button->x), y(button->y) {}
  explicit RestoreEventXY(GdkEventScroll* button) : sx(&button->x), sy(&button->y), x(button->x), y(button->y) {}
  ~RestoreEventXY() {
    *sx = x;
    *sy = y;
  }
  double* sx;
  double* sy;
  double x;
  double y;
};
