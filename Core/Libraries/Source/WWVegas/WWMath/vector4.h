#pragma once

#include "always.h"
#include "wwmath.h"

class Vector4 {
public:
  float X, Y, Z, W;

  WWINLINE Vector4(void) {}
  WWINLINE Vector4(const Vector4 &v) : X(v.X), Y(v.Y), Z(v.Z), W(v.W) {}
  WWINLINE Vector4(float x, float y, float z, float w)
      : X(x), Y(y), Z(z), W(w) {}

  WWINLINE Vector4 &operator=(const Vector4 &v) {
    X = v.X;
    Y = v.Y;
    Z = v.Z;
    W = v.W;
    return *this;
  }
  WWINLINE void Set(float x, float y, float z, float w) {
    X = x;
    Y = y;
    Z = z;
    W = w;
  }

  WWINLINE float &operator[](int i) { return (&X)[i]; }
  WWINLINE const float &operator[](int i) const { return (&X)[i]; }

  WWINLINE Vector4 operator-() const { return Vector4(-X, -Y, -Z, -W); }
  WWINLINE Vector4 operator+() const { return *this; }

  WWINLINE Vector4 &operator+=(const Vector4 &v) {
    X += v.X;
    Y += v.Y;
    Z += v.Z;
    W += v.W;
    return *this;
  }
  WWINLINE Vector4 &operator-=(const Vector4 &v) {
    X -= v.X;
    Y -= v.Y;
    Z -= v.Z;
    W -= v.W;
    return *this;
  }
  WWINLINE Vector4 &operator*=(float k) {
    X *= k;
    Y *= k;
    Z *= k;
    W *= k;
    return *this;
  }

  friend WWINLINE Vector4 operator*(const Vector4 &a, float k) {
    return Vector4(a.X * k, a.Y * k, a.Z * k, a.W * k);
  }
  friend WWINLINE Vector4 operator*(float k, const Vector4 &a) {
    return Vector4(a.X * k, a.Y * k, a.Z * k, a.W * k);
  }
  friend WWINLINE Vector4 operator+(const Vector4 &a, const Vector4 &b) {
    return Vector4(a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W);
  }
  friend WWINLINE Vector4 operator-(const Vector4 &a, const Vector4 &b) {
    return Vector4(a.X - b.X, a.Y - b.Y, a.Z - b.Z, a.W - b.W);
  }

  friend WWINLINE bool operator==(const Vector4 &a, const Vector4 &b) {
    return (a.X == b.X && a.Y == b.Y && a.Z == b.Z && a.W == b.W);
  }
  friend WWINLINE bool operator!=(const Vector4 &a, const Vector4 &b) {
    return (a.X != b.X || a.Y != b.Y || a.Z != b.Z || a.W != b.W);
  }

  friend WWINLINE float operator*(const Vector4 &a, const Vector4 &b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
  }
  static WWINLINE float Dot_Product(const Vector4 &a, const Vector4 &b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
  }

  static WWINLINE Vector4 Lerp(const Vector4 &a, const Vector4 &b,
                               float alpha) {
    return Vector4((a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha),
                   (a.Z + (b.Z - a.Z) * alpha), (a.W + (b.W - a.W) * alpha));
  }
  static WWINLINE void Lerp(const Vector4 &a, const Vector4 &b, float alpha,
                            Vector4 *set_result) {
    assert(set_result != nullptr);
    set_result->X = (a.X + (b.X - a.X) * alpha);
    set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
    set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
    set_result->W = (a.W + (b.W - a.W) * alpha);
  }
};
