#pragma once

namespace gui {

struct Point {
  double x;
  double y;
  Point(double x, double y) : x(x), y(y) {}
  Point() {}
  Point operator-(const Point& other) const {
    return Point{x - other.x, y - other.y};
  }

  Point operator+(const Point& other) const {
    return Point{x + other.x, y + other.y};
  }

  Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  bool operator==(const Point& other) const {
    return other.x == x && other.y == y;
  }

  double self_dot() const {
    return x * x + y * y;
  }
};

inline Point operator*(double a, const Point& b) {
  return {b.x * a, a * b.y};
}

inline Point operator*(const Point& b, double a) {
  return {b.x * a, a * b.y};
}

inline Point operator/(const Point& b, double a) {
  return {b.x / a, b.y / a};
}

}  // namespace gui
