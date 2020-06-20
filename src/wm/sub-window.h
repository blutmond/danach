#pragma once

#include "gui/widget-helper.h"
#include "wm/cross-process-transfer.h"

class Dragger {
 public:
  virtual ~Dragger() {}
  virtual void Drag(gui::Point pt2) = 0;
};

class WindowState;

class SubWindow {
 public:
  WindowState* wm = nullptr;
  void AddWindow(std::unique_ptr<SubWindow> window);
  void redraw();
  void close();
  void RefreshBinary();
  virtual ~SubWindow() {}

  virtual void Draw(gui::DrawCtx& cr) {
    cr.set_source_rgb(0.0, 0.0, 0.0);
    cr.paint();
  }

  virtual void KeyPress(GdkEventKey* event) {}

  virtual void ScrollEvent(GdkEventScroll* event) {}

  virtual void ButtonPress(GdkEventButton* button) {}

  gui::Rectangle rect() const;

  gui::Rectangle decorated_rect;

  virtual OpaqueTransferRef encode(BufferContext& context) const { return {0, nullptr}; }
};

gui::Rectangle DefaultRectangle();

std::unique_ptr<SubWindow> MakeCommandWindow();
