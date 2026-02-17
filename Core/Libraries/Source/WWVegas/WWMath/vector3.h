#pragma once

#include "always.h"
#include "wwmath.h"
#include <assert.h>
#include <math.h>

class Vector3 {
public:
  float X, Y, Z;

  WWINLINE Vector3(void) {}
  WWINLINE Vector3(const Vector3 &v) : X(v.X), Y(v.Y), Z(v.Z) {}
  WWINLINE Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
  WWINLINE Vector3(const float vector[3])
      : X(vector[0]), Y(vector[1]), Z(vector[2]) {}

  WWINLINE Vector3 &operator=(const Vector3 &v) {
    X = v.X;
    Y = v.Y;
    Z = v.Z;
    return *this;
  }
  WWINLINE void Set(float x, float y, float z) {
    X = x;
    Y = y;
    Z = z;
  }
  WWINLINE void Set(const Vector3 &that) {
    X = that.X;
    Y = that.Y;
    Z = that.Z;
  }

  WWINLINE float &operator[](int i) { return (&X)[i]; }
  WWINLINE const float &operator[](int i) const { return (&X)[i]; }

  WWINLINE void Normalize(void) {
    float l2 = Length2();
    if (l2 != 0.0f) {
      float ool = WWMath::Inv_Sqrt(l2);
      X *= ool;
      Y *= ool;
      Z *= ool;
    }
  }

  WWINLINE float Length(void) const { return WWMath::Sqrt(Length2()); }
  WWINLINE float Length2(void) const { return X * X + Y * Y + Z * Z; }

  WWINLINE float Quick_Length(void) const {
    float max = WWMath::Fabs(X);
    float mid = WWMath::Fabs(Y);
    float min = WWMath::Fabs(Z);
    float tmp;

    if (max < mid) {
      tmp = max;
      max = mid;
      mid = tmp;
    }
    if (max < min) {
      tmp = max;
      max = min;
      min = tmp;
    }
    if (mid < min) {
      tmp = mid;
      mid = min;
      min = tmp;
    }

    return max + (11.0f / 32.0f) * mid + (1.0f / 4.0f) * min;
  }

  WWINLINE void Scale(const Vector3 &scale) {
    X *= scale.X;
    Y *= scale.Y;
    Z *= scale.Z;
  }

  WWINLINE void Rotate_X(float s, float c) {
    float ty = Y;
    float tz = Z;
    Y = c * ty - s * tz;
    Z = s * ty + c * tz;
  }
  WWINLINE void Rotate_X(float a) { Rotate_X(sinf(a), cosf(a)); }

  WWINLINE void Rotate_Y(float s, float c) {
    float tx = X;
    float tz = Z;
    X = c * tx + s * tz;
    Z = -s * tx + c * tz;
  }
  WWINLINE void Rotate_Y(float a) { Rotate_Y(sinf(a), cosf(a)); }

  WWINLINE void Rotate_Z(float s, float c) {
    float tx = X;
    float ty = Y;
    X = c * tx - s * ty;
    Y = s * tx + c * ty;
  }
  WWINLINE void Rotate_Z(float a) { Rotate_Z(sinf(a), cosf(a)); }

  WWINLINE Vector3 operator-() const { return Vector3(-X, -Y, -Z); }
  WWINLINE Vector3 operator+() const { return *this; }

  WWINLINE Vector3 &operator+=(const Vector3 &v) {
    X += v.X;
    Y += v.Y;
    Z += v.Z;
    return *this;
  }
  WWINLINE Vector3 &operator-=(const Vector3 &v) {
    X -= v.X;
    Y -= v.Y;
    Z -= v.Z;
    return *this;
  }
  WWINLINE Vector3 &operator*=(float k) {
    X *= k;
    Y *= k;
    Z *= k;
    return *this;
  }
  WWINLINE Vector3 &operator/=(float k) {
    float ook = 1.0f / k;
    X *= ook;
    Y *= ook;
    Z *= ook;
    return *this;
  }

  friend WWINLINE Vector3 operator*(const Vector3 &a, float k) {
    return Vector3(a.X * k, a.Y * k, a.Z * k);
  }
  friend WWINLINE Vector3 operator*(float k, const Vector3 &a) {
    return Vector3(a.X * k, a.Y * k, a.Z * k);
  }
  friend WWINLINE Vector3 operator/(const Vector3 &a, float k) {
    float ook = 1.0f / k;
    return Vector3(a.X * ook, a.Y * ook, a.Z * ook);
  }
  friend WWINLINE Vector3 operator+(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
  }
  friend WWINLINE Vector3 operator-(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
  }

  friend bool operator==(const Vector3 &a, const Vector3 &b) {
    return (a.X == b.X && a.Y == b.Y && a.Z == b.Z);
  }
  friend bool operator!=(const Vector3 &a, const Vector3 &b) {
    return (a.X != b.X || a.Y != b.Y || a.Z != b.Z);
  }
  friend WWINLINE bool Equal_Within_Epsilon(const Vector3 &a, const Vector3 &b,
                                            float epsilon) {
    return (WWMath::Fabs(a.X - b.X) < epsilon &&
            WWMath::Fabs(a.Y - b.Y) < epsilon &&
            WWMath::Fabs(a.Z - b.Z) < epsilon);
  }

  friend WWINLINE float operator*(const Vector3 &a, const Vector3 &b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
  }
  static WWINLINE float Dot_Product(const Vector3 &a, const Vector3 &b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
  }

#ifdef ALLOW_TEMPORARIES
  static WWINLINE Vector3 Cross_Product(const Vector3 &a, const Vector3 &b) {
    return Vector3((a.Y * b.Z - a.Z * b.Y), (a.Z * b.X - a.X * b.Z),
                   (a.X * b.Y - a.Y * b.X));
  }
#endif

  static WWINLINE void Cross_Product(const Vector3 &a, const Vector3 &b,
                                     Vector3 *res) {
    res->X = (a.Y * b.Z - a.Z * b.Y);
    res->Y = (a.Z * b.X - a.X * b.Z);
    res->Z = (a.X * b.Y - a.Y * b.X);
  }

  static WWINLINE void
  Normalized_Cross_Product(const Vector3 &a, const Vector3 &b, Vector3 *res) {
    Cross_Product(a, b, res);
    res->Normalize();
  }

  static WWINLINE float Cross_Product_X(const Vector3 &a, const Vector3 &b) {
    return a.Y * b.Z - a.Z * b.Y;
  }
  static WWINLINE float Cross_Product_Y(const Vector3 &a, const Vector3 &b) {
    return a.Z * b.X - a.X * b.Z;
  }
  static WWINLINE float Cross_Product_Z(const Vector3 &a, const Vector3 &b) {
    return a.X * b.Y - a.Y * b.X;
  }

  static WWINLINE void Add(const Vector3 &a, const Vector3 &b, Vector3 *c) {
    c->X = a.X + b.X;
    c->Y = a.Y + b.Y;
    c->Z = a.Z + b.Z;
  }
  static WWINLINE void Subtract(const Vector3 &a, const Vector3 &b,
                                Vector3 *c) {
    c->X = a.X - b.X;
    c->Y = a.Y - b.Y;
    c->Z = a.Z - b.Z;
  }

  WWINLINE void Update_Min(const Vector3 &a) {
    if (a.X < X)
      X = a.X;
    if (a.Y < Y)
      Y = a.Y;
    if (a.Z < Z)
      Z = a.Z;
  }
  WWINLINE void Update_Max(const Vector3 &a) {
    if (a.X > X)
      X = a.X;
    if (a.Y > Y)
      Y = a.Y;
    if (a.Z > Z)
      Z = a.Z;
  }
  WWINLINE void Cap_Absolute_To(const Vector3 &a) {
    if (X > 0) {
      if (a.X < X)
        X = a.X;
    } else {
      if (-a.X > X)
        X = -a.X;
    }

    if (Y > 0) {
      if (a.Y < Y)
        Y = a.Y;
    } else {
      if (-a.Y > Y)
        Y = -a.Y;
    }

    if (Z > 0) {
      if (a.Z < Z)
        Z = a.Z;
    } else {
      if (-a.Z > Z)
        Z = -a.Z;
    }
  }

  static WWINLINE float Find_X_At_Y(float y, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.X + ((y - p1.Y) * ((p2.X - p1.X) / (p2.Y - p1.Y))));
  }
  static WWINLINE float Find_X_At_Z(float z, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.X + ((z - p1.Z) * ((p2.X - p1.X) / (p2.Z - p1.Z))));
  }
  static WWINLINE float Find_Y_At_X(float x, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.Y + ((x - p1.X) * ((p2.Y - p1.Y) / (p2.X - p1.X))));
  }
  static WWINLINE float Find_Y_At_Z(float z, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.Y + ((z - p1.Z) * ((p2.Y - p1.Y) / (p2.Z - p1.Z))));
  }
  static WWINLINE float Find_Z_At_X(float x, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.Z + ((x - p1.X) * ((p2.Z - p1.Z) / (p2.X - p1.X))));
  }
  static WWINLINE float Find_Z_At_Y(float y, const Vector3 &p1,
                                    const Vector3 &p2) {
    return (p1.Z + ((y - p1.Y) * ((p2.Z - p1.Z) / (p2.Y - p1.Y))));
  }

  WWINLINE bool Is_Valid(void) const {
    return WWMath::Is_Valid_Float(X) && WWMath::Is_Valid_Float(Y) &&
           WWMath::Is_Valid_Float(Z);
  }

  static WWINLINE float Distance(const Vector3 &p1, const Vector3 &p2) {
    Vector3 d;
    Subtract(p1, p2, &d);
    return d.Length();
  }

  static WWINLINE Vector3 Lerp(const Vector3 &a, const Vector3 &b,
                               float alpha) {
    return Vector3((a.X + (b.X - a.X) * alpha), (a.Y + (b.Y - a.Y) * alpha),
                   (a.Z + (b.Z - a.Z) * alpha));
  }
  static WWINLINE void Lerp(const Vector3 &a, const Vector3 &b, float alpha,
                            Vector3 *set_result) {
    assert(set_result != nullptr);
    set_result->X = (a.X + (b.X - a.X) * alpha);
    set_result->Y = (a.Y + (b.Y - a.Y) * alpha);
    set_result->Z = (a.Z + (b.Z - a.Z) * alpha);
  }

  WWINLINE unsigned long Convert_To_ARGB(float alpha) const {
    unsigned char a = (unsigned char)(alpha * 255.0f);
    unsigned char r = (unsigned char)(X * 255.0f);
    unsigned char g = (unsigned char)(Y * 255.0f);
    unsigned char b = (unsigned char)(Z * 255.0f);
    return ((unsigned long)a << 24) | ((unsigned long)r << 16) |
           ((unsigned long)g << 8) | (unsigned long)b;
  }
};
