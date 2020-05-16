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
};

struct Rectangle {
  Point st;
  Shape shape;
};

inline bool TestRectangleInside(double& x, double& y, gui::Rectangle rect) {
  gui::Point pt = gui::Point{x, y} - rect.st;
  if (pt.x > 0 && pt.y > 0 && pt.x < double(rect.shape.w) && pt.y < double(rect.shape.h)) {
    x -= rect.st.x;
    y -= rect.st.y;
    return true;
  }
  return false;
}

inline bool TestRectangleClick(GdkEventButton* button, gui::Rectangle rect) {
  return TestRectangleInside(button->x, button->y, rect);
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

  bool needs_redraw = true;
  void redraw();
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
  ~RestoreEventXY() {
    *sx = x;
    *sy = y;
  }
  double* sx;
  double* sy;
  double x;
  double y;
};
