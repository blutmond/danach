#pragma once

#include "gui/point.h"
#include <memory>
#include <cairo/cairo.h>

namespace gui {

struct ColorRGB {
  double r;
  double g;
  double b;
};

struct ColorRGBA {
  double r;
  double g;
  double b;
  double a;
};

class Matrix {
 public:
  gui::Point operator*(const gui::Point& pt) const {
    return {
      matrix_.xx * pt.x + matrix_.xy * pt.y + matrix_.x0,
          matrix_.yx * pt.x + matrix_.yy * pt.y + matrix_.y0
    };
  }

  static Matrix Identity() {
    Matrix matrix;
    cairo_matrix_init_identity(&matrix.matrix_);
    return matrix;
  }
 private:
  friend class DrawCtx;
  _cairo_matrix matrix_;
};

#define Unpack_Point(v) (v).x, (v).y
#define Unpack_ColorRGB(v) (v).r, (v).g, (v).b
#define Unpack_ColorRGBA(v) (v).r, (v).g, (v).b, (v).a
#define Unpack_double(v) v
#define Unpack_int(v) v
#define Unpack_PointRef(v) &(v)->x, &(v)->y
#define Unpack_cairo_line_join_t(v) v
#define Unpack_SurfacePtr(v) v
#define DefFn(name) auto name() {\
  return NAME_PREFIX(name)(this_param); }
#define DefFn1(name, arg_t) auto name(arg_t arg1) {\
  return NAME_PREFIX(name)(this_param, Unpack_##arg_t(arg1)); }
#define DefFn2(name, arg_t1, arg_t2) auto name(arg_t1 arg1, arg_t2 arg2) {\
  return NAME_PREFIX(name)(this_param, Unpack_##arg_t1(arg1), Unpack_##arg_t2(arg2)); }
#define DefFn3(name, arg_t1, arg_t2, arg_t3) auto name(arg_t1 arg1, arg_t2 arg2, arg_t3 arg3) {\
  return NAME_PREFIX(name)(this_param, Unpack_##arg_t1(arg1), Unpack_##arg_t2(arg2), Unpack_##arg_t3(arg3)); }
#define DefFn4(name, arg_t1, arg_t2, arg_t3, arg_t4) auto name(arg_t1 arg1, arg_t2 arg2, arg_t3 arg3, arg_t4 arg4) {\
  return NAME_PREFIX(name)(this_param, Unpack_##arg_t1(arg1), Unpack_##arg_t2(arg2), Unpack_##arg_t3(arg3), Unpack_##arg_t4(arg4)); }

class MeshPattern {
 public:
  explicit MeshPattern() {
    pattern_.reset(cairo_pattern_create_mesh());
  }

#define NAME_PREFIX(name) cairo_mesh_pattern_ ## name
#define this_param pattern_.get()

  DefFn1(move_to, Point);
  DefFn1(line_to, Point);
  DefFn(begin_patch);
  DefFn(end_patch);
  DefFn2(set_corner_color_rgb, int, ColorRGB);

#undef NAME_PREFIX
#undef this_param

 private:
  friend class DrawCtx;
  cairo_pattern_t* pattern() const { return pattern_.get(); }
  struct Dtor {
    void operator()(cairo_pattern_t* v) {
      cairo_pattern_destroy(v);
    }
  };
  std::unique_ptr<cairo_pattern_t, Dtor> pattern_;
};

class DrawCtx {
  using PointRef = Point*;
  using SurfacePtr = cairo_surface_t*;
 public:

#define NAME_PREFIX(name) cairo_ ## name
#define this_param cr()

  DefFn1(get_current_point, PointRef);
  DefFn1(move_to, Point);
  DefFn1(line_to, Point);
  DefFn1(rel_move_to, Point);
  DefFn1(rel_line_to, Point);
  DefFn4(arc, Point, double, double, double);
  DefFn4(arc_negative, Point, double, double, double);
  DefFn2(rectangle, Point, Point);
  DefFn1(paint_with_alpha, double);
  DefFn(paint);
  DefFn(stroke);
  DefFn(stroke_preserve);
  DefFn(fill);
  DefFn(clip);
  DefFn(get_target);
  DefFn(fill_preserve);
  DefFn3(curve_to, Point, Point, Point);
  DefFn1(in_stroke, Point);
  DefFn3(set_source_rgb, double, double, double);
  DefFn1(set_source_rgb, ColorRGB);
  DefFn1(set_source_rgba, ColorRGBA);
  DefFn2(set_source_surface, SurfacePtr, Point);
  DefFn(save);
  DefFn(restore);
  DefFn(close_path);
  DefFn1(set_line_width, double);
  DefFn(get_line_width);
  DefFn1(scale, Point);
  DefFn2(scale, double, double);
  DefFn1(translate, Point);
  DefFn2(translate, double, double);
  DefFn1(set_line_join, cairo_line_join_t);

  void set_matrix(const Matrix& matrix) { cairo_set_matrix(cr(), &matrix.matrix_); }
  Matrix get_matrix() {
    Matrix matrix;
    cairo_get_matrix(cr(), &matrix.matrix_);
    return matrix;
  }

  void set_source(const MeshPattern& mesh_pattern) {
    cairo_set_source(cr(), mesh_pattern.pattern());
  }

#undef NAME_PREFIX
#undef this_param
  cairo_t* cr() { return reinterpret_cast<cairo_t*>(this); }
  static DrawCtx* wrap(cairo_t* cr) { return reinterpret_cast<DrawCtx*>(cr); }
 private:
  // Should never be constructed directly.
  DrawCtx() {}
  DrawCtx(const DrawCtx& o) = delete;
  DrawCtx(DrawCtx&& o) = delete;
};

#undef DefFn
#undef DefFn1
#undef DefFn2
#undef DefFn3
#undef DefFn4
#undef Unpack_Point
#undef Unpack_double
#undef Unpack_int
#undef Unpack_PointRef
#undef Unpack_ColorRGB
#undef Unpack_ColorRGBA
#undef Unpack_cairo_line_join_t

class ScopedRawCtx {
 public:
  explicit ScopedRawCtx(cairo_surface_t* ctx) : ctx_(ctx) {
    cr_ = cairo_create(ctx_);
  }
  ScopedRawCtx() {}
  ScopedRawCtx(const ScopedRawCtx& o) = delete;
  ScopedRawCtx& operator=(const ScopedRawCtx& o) = delete;
  ScopedRawCtx& operator=(ScopedRawCtx&& o) {
    if (this == &o) return *this;
    cr_ = o.cr_;
    ctx_ = o.ctx_;
    o.cr_ = nullptr;
    o.ctx_ = nullptr;
    return *this;
  }
  ~ScopedRawCtx() {
    if (cr_) {
      cairo_surface_destroy(ctx_);
      cairo_destroy(cr_);
    }
  }
  cairo_surface_t* surface() { return ctx_; }
  DrawCtx* ctx() {
    return reinterpret_cast<gui::DrawCtx*>(cr_);
  }
 private:
  cairo_t* cr_ = nullptr;
  cairo_surface_t* ctx_ = nullptr;
};

class Save {
 public:
  explicit Save(DrawCtx& ctx) : ctx_(ctx) {
    ctx_.save();
  }
  ~Save() {
    ctx_.restore();
  }
 private:
  DrawCtx& ctx_;
};

class ScopedIdentity {

 public:
  explicit ScopedIdentity(DrawCtx& ctx) : save_(ctx) {
    auto matrix = Matrix::Identity();
    ctx.set_matrix(matrix);
   // cairo_set_matrix(ctx.cr(), &matrix);
  }
 private:
  Save save_;
};

}  // namespace gui
