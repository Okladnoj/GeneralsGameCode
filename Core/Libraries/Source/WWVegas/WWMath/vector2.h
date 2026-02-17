#pragma once

#include "always.h"
#include "wwmath.h"
#include <assert.h>

class Vector2 {
public:
  union {
    float X;
    float U;
  };
  union {
    float Y;
    float V;
  };

  WWINLINE Vector2(void) {}
  WWINLINE Vector2(const Vector2 &v) : X(v.X), Y(v.Y) {}
  WWINLINE Vector2(float x, float y) : X(x), Y(y) {}
  WWINLINE Vector2(const float v[2]) : X(v[0]), Y(v[1]) {}

  WWINLINE Vector2 &operator=(const Vector2 &v) {
    X = v.X;
    Y = v.Y;
    return *this;
  }
  WWINLINE void Set(float x, float y) {
    X = x;
    Y = y;
  }
  WWINLINE void Set(const Vector2 &v) {
    X = v.X;
    Y = v.Y;
  }
  WWINLINE void Rotate(float a) {
    float s = sinf(a);
    float c = cosf(a);
    float tx = X;
    float ty = Y;
    X = c * tx - s * ty;
    Y = s * tx + c * ty;
  }

  WWINLINE float &operator[](int i) { return (&X)[i]; }
  WWINLINE const float &operator[](int i) const { return (&X)[i]; }

  WWINLINE void Normalize(void) {
    float l2 = Length2();
    if (l2 != 0.0f) {
      float ool = WWMath::Inv_Sqrt(l2);
      X *= ool;
      Y *= ool;
    }
  }

  WWINLINE float Length(void) const { return WWMath::Sqrt(X * X + Y * Y); }
  WWINLINE float Length2(void) const { return X * X + Y * Y; }

  WWINLINE Vector2 operator-() const { return Vector2(-X, -Y); }
  WWINLINE Vector2 operator+() const { return *this; }

  WWINLINE Vector2 &operator+=(const Vector2 &v) {
    X += v.X;
    Y += v.Y;
    return *this;
  }
  WWINLINE Vector2 &operator-=(const Vector2 &v) {
    X -= v.X;
    Y -= v.Y;
    return *this;
  }
  WWINLINE Vector2 &operator*=(float k) {
    X *= k;
    Y *= k;
    return *this;
  }
  WWINLINE Vector2 &operator/=(float k) {
    float ook = 1.0f / k;
    X *= ook;
    Y *= ook;
    return *this;
  }

  friend WWINLINE Vector2 operator*(const Vector2 &a, float k) {
    return Vector2(a.X * k, a.Y * k);
  }
  friend WWINLINE Vector2 operator*(float k, const Vector2 &a) {
    return Vector2(a.X * k, a.Y * k);
  }
  friend WWINLINE Vector2 operator/(const Vector2 &a, float k) {
    float ook = 1.0f / k;
    return Vector2(a.X * ook, a.Y * ook);
  }
  friend WWINLINE Vector2 operator+(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.X + b.X, a.Y + b.Y);
  }
  friend WWINLINE Vector2 operator-(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.X - b.X, a.Y - b.Y);
  }

  friend bool operator==(const Vector2 &a, const Vector2 &b) {
    return (a.X == b.X && a.Y == b.Y);
  }
  friend bool operator!=(const Vector2 &a, const Vector2 &b) {
    return (a.X != b.X || a.Y != b.Y);
  }

  friend WWINLINE float operator*(const Vector2 &a, const Vector2 &b) {
    return a.X * b.X + a.Y * b.Y;
  }
  static WWINLINE float Dot_Product(const Vector2 &a, const Vector2 &b) {
    return a.X * b.X + a.Y * b.Y;
  }
  static WWINLINE float Perp_Dot_Product(const Vector2 &a, const Vector2 &b) {
    return a.X * b.Y - a.Y * b.X;
  }

  static WWINLINE Vector2 Lerp(const Vector2 &a, const Vector2 &b,
                               float alpha) {
    return Vector2((a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha));
  }
  static WWINLINE void Lerp(const Vector2 &a, const Vector2 &b, float alpha,
                            Vector2 *set_result) {
    assert(set_result != nullptr);
    set_result->X = (a.X + (b.X - a.X) * alpha);
    set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
  }
};
