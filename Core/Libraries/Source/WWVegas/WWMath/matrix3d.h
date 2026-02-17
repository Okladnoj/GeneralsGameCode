#ifndef MATRIX3D_H
#define MATRIX3D_H

#include "always.h"
#include "vector3.h"
#include "vector4.h"
#include <assert.h>
#include <math.h>

#include "matrix3.h"
class Quaternion;

class Matrix3D {
public:
  Matrix3D() {}
  explicit Matrix3D(bool init) {
    if (init)
      Make_Identity();
  }

  explicit Matrix3D(float m[12]) {
    Row[0].Set(m[0], m[1], m[2], m[3]);
    Row[1].Set(m[4], m[5], m[6], m[7]);
    Row[2].Set(m[8], m[9], m[10], m[11]);
  }

  explicit Matrix3D(float m11, float m12, float m13, float m14, float m21,
                    float m22, float m23, float m24, float m31, float m32,
                    float m33, float m34) {
    Row[0].Set(m11, m12, m13, m14);
    Row[1].Set(m21, m22, m23, m24);
    Row[2].Set(m31, m32, m33, m34);
  }

  explicit Matrix3D(const Vector3 &x, const Vector3 &y, const Vector3 &z,
                    const Vector3 &pos) {
    Set(x, y, z, pos);
  }

  explicit Matrix3D(const Vector3 &axis, float angle) { Set(axis, angle); }
  explicit Matrix3D(const Vector3 &axis, float sine, float cosine) {
    Set(axis, sine, cosine);
  }

  explicit Matrix3D(const Vector3 &position) { Set(position); }
  explicit Matrix3D(const Matrix3x3 &basis, const Vector3 &translation);
  Matrix3D(const Matrix3D &m) {
    Row[0] = m.Row[0];
    Row[1] = m.Row[1];
    Row[2] = m.Row[2];
  }

  Matrix3D &operator=(const Matrix3D &m) {
    Row[0] = m.Row[0];
    Row[1] = m.Row[1];
    Row[2] = m.Row[2];
    return *this;
  }

  void Set(float m[12]) {
    Row[0].Set(m[0], m[1], m[2], m[3]);
    Row[1].Set(m[4], m[5], m[6], m[7]);
    Row[2].Set(m[8], m[9], m[10], m[11]);
  }

  void Set(float m00, float m01, float m02, float m03, float m10, float m11,
           float m12, float m13, float m20, float m21, float m22, float m23) {
    Row[0].Set(m00, m01, m02, m03);
    Row[1].Set(m10, m11, m12, m13);
    Row[2].Set(m20, m21, m22, m23);
  }

  void Set(const Vector3 &x, const Vector3 &y, const Vector3 &z,
           const Vector3 &pos) {
    Row[0].Set(x[0], y[0], z[0], pos[0]);
    Row[1].Set(x[1], y[1], z[1], pos[1]);
    Row[2].Set(x[2], y[2], z[2], pos[2]);
  }

  void Set(const Vector3 &axis, float angle) {
    Set(axis, sinf(angle), cosf(angle));
  }
  void Set(const Vector3 &axis, float s, float c) {
    float t = 1.0f - c;
    Row[0].Set(axis[0] * axis[0] * t + c, axis[0] * axis[1] * t - axis[2] * s,
               axis[0] * axis[2] * t + axis[1] * s, 0.0f);
    Row[1].Set(axis[0] * axis[1] * t + axis[2] * s, axis[1] * axis[1] * t + c,
               axis[1] * axis[2] * t - axis[0] * s, 0.0f);
    Row[2].Set(axis[0] * axis[2] * t - axis[1] * s,
               axis[1] * axis[2] * t + axis[0] * s, axis[2] * axis[2] * t + c,
               0.0f);
  }

  void Set(const Vector3 &position) {
    Make_Identity();
    Set_Translation(position);
  }

  void Set(const Matrix3x3 &basis, const Vector3 &translation);

  void Make_Identity(void) {
    Row[0].Set(1, 0, 0, 0);
    Row[1].Set(0, 1, 0, 0);
    Row[2].Set(0, 0, 1, 0);
  }

  Vector4 &operator[](int i) { return Row[i]; }
  const Vector4 &operator[](int i) const { return Row[i]; }

  Vector3 Get_X_Vector(void) const {
    return Vector3(Row[0][0], Row[1][0], Row[2][0]);
  }
  void Get_X_Vector(Vector3 *v) const {
    v->X = Row[0][0];
    v->Y = Row[1][0];
    v->Z = Row[2][0];
  }
  Vector3 Get_Y_Vector(void) const {
    return Vector3(Row[0][1], Row[1][1], Row[2][1]);
  }
  void Get_Y_Vector(Vector3 *v) const {
    v->X = Row[0][1];
    v->Y = Row[1][1];
    v->Z = Row[2][1];
  }
  Vector3 Get_Z_Vector(void) const {
    return Vector3(Row[0][2], Row[1][2], Row[2][2]);
  }
  void Get_Z_Vector(Vector3 *v) const {
    v->X = Row[0][2];
    v->Y = Row[1][2];
    v->Z = Row[2][2];
  }

  Vector3 Get_Translation(void) const {
    return Vector3(Row[0][3], Row[1][3], Row[2][3]);
  }
  void Get_Translation(Vector3 *t) const {
    t->X = Row[0][3];
    t->Y = Row[1][3];
    t->Z = Row[2][3];
  }
  void Set_Translation(const Vector3 &t) {
    Row[0][3] = t.X;
    Row[1][3] = t.Y;
    Row[2][3] = t.Z;
  }

  void Set_Rotation(const Matrix3x3 &m);
  void Set_Rotation(const Quaternion &q);

  void Translate(float x, float y, float z) {
    Row[0][3] += x;
    Row[1][3] += y;
    Row[2][3] += z;
  }
  void Translate(const Vector3 &t) { Translate(t.X, t.Y, t.Z); }

  float Get_X_Translation(void) const { return Row[0][3]; }
  float Get_Y_Translation(void) const { return Row[1][3]; }
  float Get_Z_Translation(void) const { return Row[2][3]; }

  void Translate_X(float x) { Row[0][3] += x; }
  void Translate_Y(float y) { Row[1][3] += y; }
  void Translate_Z(float z) { Row[2][3] += z; }

  void Adjust_X_Translation(float x) { Row[0][3] += x; }
  void Adjust_Y_Translation(float y) { Row[1][3] += y; }
  void Adjust_Z_Translation(float z) { Row[2][3] += z; }

  void Adjust_Translation(const Vector3 &t) {
    Row[0][3] += t.X;
    Row[1][3] += t.Y;
    Row[2][3] += t.Z;
  }

  void Scale(float s) {
    for (int i = 0; i < 3; i++) {
      Row[i][0] *= s;
      Row[i][1] *= s;
      Row[i][2] *= s;
    }
  }

  void Scale(const Vector3 &s) {
    for (int i = 0; i < 3; i++) {
      Row[i][0] *= s.X;
      Row[i][1] *= s.Y;
      Row[i][2] *= s.Z;
    }
  }

  void mulVector3(const Vector3 &in, Vector3 &out) const {
    out.X = Row[0][0] * in.X + Row[0][1] * in.Y + Row[0][2] * in.Z + Row[0][3];
    out.Y = Row[1][0] * in.X + Row[1][1] * in.Y + Row[1][2] * in.Z + Row[1][3];
    out.Z = Row[2][0] * in.X + Row[2][1] * in.Y + Row[2][2] * in.Z + Row[2][3];
  }

  void mulVector3(Vector3 &vec) const {
    Vector3 in = vec;
    mulVector3(in, vec);
  }

  void Rotate_X(float theta) { Rotate_X(sinf(theta), cosf(theta)); }
  void Rotate_X(float s, float c) {
    float t1, t2;
    t1 = Row[0][1];
    t2 = Row[0][2];
    Row[0][1] = c * t1 + s * t2;
    Row[0][2] = -s * t1 + c * t2;
    t1 = Row[1][1];
    t2 = Row[1][2];
    Row[1][1] = c * t1 + s * t2;
    Row[1][2] = -s * t1 + c * t2;
    t1 = Row[2][1];
    t2 = Row[2][2];
    Row[2][1] = c * t1 + s * t2;
    Row[2][2] = -s * t1 + c * t2;
  }

  void Rotate_Y(float theta) { Rotate_Y(sinf(theta), cosf(theta)); }
  void Rotate_Y(float s, float c) {
    float t0, t2;
    t0 = Row[0][0];
    t2 = Row[0][2];
    Row[0][0] = c * t0 - s * t2;
    Row[0][2] = s * t0 + c * t2;
    t0 = Row[1][0];
    t2 = Row[1][2];
    Row[1][0] = c * t0 - s * t2;
    Row[1][2] = s * t0 + c * t2;
    t0 = Row[2][0];
    t2 = Row[2][2];
    Row[2][0] = c * t0 - s * t2;
    Row[2][2] = s * t0 + c * t2;
  }

  void Rotate_Z(float theta) { Rotate_Z(sinf(theta), cosf(theta)); }
  void Rotate_Z(float s, float c) {
    float t0, t1;
    t0 = Row[0][0];
    t1 = Row[0][1];
    Row[0][0] = c * t0 + s * t1;
    Row[0][1] = -s * t0 + c * t1;
    t0 = Row[1][0];
    t1 = Row[1][1];
    Row[1][0] = c * t0 + s * t1;
    Row[1][1] = -s * t0 + c * t1;
    t0 = Row[2][0];
    t1 = Row[2][1];
    Row[2][0] = c * t0 + s * t1;
    Row[2][1] = -s * t0 + c * t1;
  }

  float Get_X_Rotation(void) const;
  float Get_Y_Rotation(void) const;
  float Get_Z_Rotation(void) const;

  Vector3 operator*(const Vector3 &in) const {
    Vector3 out;
    mulVector3(in, out);
    return out;
  }

  static void Transform_Vector(const Matrix3D &tm, const Vector3 &in,
                               Vector3 *out) {
    tm.mulVector3(in, *out);
  }

  Vector3 Rotate_Vector(const Vector3 &vect) const;
  static void Rotate_Vector(const Matrix3D &tm, const Vector3 &in,
                            Vector3 *out) {
    *out = tm.Rotate_Vector(in);
  }

  Vector3 Inverse_Rotate_Vector(const Vector3 &vect) const;
  static void Inverse_Rotate_Vector(const Matrix3D &tm, const Vector3 &in,
                                    Vector3 *out) {
    *out = tm.Inverse_Rotate_Vector(in);
  }

  Vector3 Inverse_Transform_Vector(const Vector3 &in) const {
    Vector3 v(in.X - Row[0][3], in.Y - Row[1][3], in.Z - Row[2][3]);
    return Inverse_Rotate_Vector(v);
  }
  static void Inverse_Transform_Vector(const Matrix3D &tm, const Vector3 &in,
                                       Vector3 *out) {
    *out = tm.Inverse_Transform_Vector(in);
  }

  bool operator==(const Matrix3D &m) const {
    return Row[0] == m.Row[0] && Row[1] == m.Row[1] && Row[2] == m.Row[2];
  }
  bool operator!=(const Matrix3D &m) const { return !(*this == m); }

  void Transform_Center_Extent_AABox(const Vector3 &in_center,
                                     const Vector3 &in_extent,
                                     Vector3 *out_center,
                                     Vector3 *out_extent) const;

  void Transform_Min_Max_AABox(const Vector3 &in_min, const Vector3 &in_max,
                               Vector3 *out_min, Vector3 *out_max) const;

  int Is_Orthogonal(void) const;
  void Re_Orthogonalize(void);

  static void Multiply(const Matrix3D &A, const Matrix3D &B, Matrix3D *set_res);

  void mul(const Matrix3D &a, const Matrix3D &b) {
    float a00 = a.Row[0][0], a01 = a.Row[0][1], a02 = a.Row[0][2],
          a03 = a.Row[0][3];
    Row[0][0] = a00 * b.Row[0][0] + a01 * b.Row[1][0] + a02 * b.Row[2][0];
    Row[0][1] = a00 * b.Row[0][1] + a01 * b.Row[1][1] + a02 * b.Row[2][1];
    Row[0][2] = a00 * b.Row[0][2] + a01 * b.Row[1][2] + a02 * b.Row[2][2];
    Row[0][3] = a00 * b.Row[0][3] + a01 * b.Row[1][3] + a02 * b.Row[2][3] + a03;

    float a10 = a.Row[1][0], a11 = a.Row[1][1], a12 = a.Row[1][2],
          a13 = a.Row[1][3];
    Row[1][0] = a10 * b.Row[0][0] + a11 * b.Row[1][0] + a12 * b.Row[2][0];
    Row[1][1] = a10 * b.Row[0][1] + a11 * b.Row[1][1] + a12 * b.Row[2][1];
    Row[1][2] = a10 * b.Row[0][2] + a11 * b.Row[1][2] + a12 * b.Row[2][2];
    Row[1][3] = a10 * b.Row[0][3] + a11 * b.Row[1][3] + a12 * b.Row[2][3] + a13;

    float a20 = a.Row[2][0], a21 = a.Row[2][1], a22 = a.Row[2][2],
          a23 = a.Row[2][3];
    Row[2][0] = a20 * b.Row[0][0] + a21 * b.Row[1][0] + a22 * b.Row[2][0];
    Row[2][1] = a20 * b.Row[0][1] + a21 * b.Row[1][1] + a22 * b.Row[2][1];
    Row[2][2] = a20 * b.Row[0][2] + a21 * b.Row[1][2] + a22 * b.Row[2][2];
    Row[2][3] = a20 * b.Row[0][3] + a21 * b.Row[1][3] + a22 * b.Row[2][3] + a23;
  }

  friend Matrix3D operator*(const Matrix3D &a, const Matrix3D &b) {
    Matrix3D res;
    res.mul(a, b);
    return res;
  }

  void Set_X_Translation(float x) { Row[0][3] = x; }
  void Set_Y_Translation(float y) { Row[1][3] = y; }
  void Set_Z_Translation(float z) { Row[2][3] = z; }

  void preMul(const Matrix3D &m) {
    Matrix3D res;
    Multiply(m, *this, &res);
    *this = res;
  }

  void Pre_Rotate_X(float angle) {
    Matrix3D m(true);
    m.Rotate_X(angle);
    preMul(m);
  }
  void Pre_Rotate_Y(float angle) {
    Matrix3D m(true);
    m.Rotate_Y(angle);
    preMul(m);
  }
  void Pre_Rotate_Z(float angle) {
    Matrix3D m(true);
    m.Rotate_Z(angle);
    preMul(m);
  }

  void In_Place_Pre_Rotate_X(float angle) { Pre_Rotate_X(angle); }
  void In_Place_Pre_Rotate_Y(float angle) { Pre_Rotate_Y(angle); }
  void In_Place_Pre_Rotate_Z(float angle) { Pre_Rotate_Z(angle); }

  void Look_At(const Vector3 &p, const Vector3 &t, float roll);
  void Obj_Look_At(const Vector3 &p, const Vector3 &t, float roll);

  static Matrix3D *Get_Inverse(Matrix3D *out, float *detOut, const Matrix3D *m);
  void Get_Inverse(Matrix3D &inv) const;
  Matrix3D Get_Inverse(void) const;

  void Get_Orthogonal_Inverse(Matrix3D &inv) const;
  Matrix3D Get_Orthogonal_Inverse(void) const;

  static void Lerp(const Matrix3D &A, const Matrix3D &B, float factor,
                   Matrix3D &result);

  static bool Solve_Linear_System(Matrix3D &system);

  void buildTransformMatrix(const Vector3 &pos, const Vector3 &dir);

  void Set(const Quaternion &q, const Vector3 &p);
  void Set(const Quaternion &q);

  void Copy_3x3_Matrix(float matrix[3][3]);

  void postMul(const Matrix3D &m) { *this = (*this) * m; }

  void mulVector3Array(Vector3 *pts, int count) const {
    for (int i = 0; i < count; i++) {
      (*this).mulVector3(pts[i]);
    }
  }

  void mulVector3Array(const Vector3 *src, Vector3 *dst, int count) const {
    for (int i = 0; i < count; i++) {
      mulVector3(src[i], dst[i]);
    }
  }

  // Static helpers
  static const Matrix3D Identity;
  static const Matrix3D RotateX90;
  static const Matrix3D RotateX180;
  static const Matrix3D RotateX270;
  static const Matrix3D RotateY90;
  static const Matrix3D RotateY180;
  static const Matrix3D RotateY270;
  static const Matrix3D RotateZ90;
  static const Matrix3D RotateZ180;
  static const Matrix3D RotateZ270;

protected:
  Vector4 Row[3];
};

struct _D3DMATRIX;
typedef struct _D3DMATRIX D3DMATRIX;
extern D3DMATRIX To_D3DMATRIX(const Matrix3D &m);

#endif
