#pragma once
#include "d3d8.h"
#include "d3dx8core.h"
#include <string.h>

#define D3DX_PI ((float)3.141592654f)
#define D3DX_1BYPI ((float)0.318309886f)

typedef struct D3DXVECTOR3 {
#ifdef __cplusplus
  D3DXVECTOR3() {};
  D3DXVECTOR3(float f1, float f2, float f3) : x(f1), y(f2), z(f3) {};
  operator float *() { return &x; }
  operator const float *() const { return &x; }
#endif
  float x, y, z;
} D3DXVECTOR3, *LPD3DXVECTOR3;

typedef struct D3DXQUATERNION {
#ifdef __cplusplus
  D3DXQUATERNION() {};
  D3DXQUATERNION(float f1, float f2, float f3, float f4)
      : x(f1), y(f2), z(f3), w(f4) {};
  operator float *() { return &x; }
  operator const float *() const { return &x; }
#endif
  float x, y, z, w;
} D3DXQUATERNION, *LPD3DXQUATERNION;

typedef struct D3DXVECTOR4 {
#ifdef __cplusplus
  D3DXVECTOR4() {};
  D3DXVECTOR4(float f1, float f2, float f3, float f4)
      : x(f1), y(f2), z(f3), w(f4) {};
  operator float *() { return &x; }
  operator const float *() const { return &x; }
#endif
  float x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;

typedef struct D3DXMATRIX : public D3DMATRIX {
#ifdef __cplusplus
  D3DXMATRIX() {};
  D3DXMATRIX(const float *f) {
    _11 = f[0];
    _12 = f[1];
    _13 = f[2];
    _14 = f[3];
    _21 = f[4];
    _22 = f[5];
    _23 = f[6];
    _24 = f[7];
    _31 = f[8];
    _32 = f[9];
    _33 = f[10];
    _34 = f[11];
    _41 = f[12];
    _42 = f[13];
    _43 = f[14];
    _44 = f[15];
  };
  D3DXMATRIX(float f11, float f12, float f13, float f14, float f21, float f22,
             float f23, float f24, float f31, float f32, float f33, float f34,
             float f41, float f42, float f43, float f44) {
    _11 = f11;
    _12 = f12;
    _13 = f13;
    _14 = f14;
    _21 = f21;
    _22 = f22;
    _23 = f23;
    _24 = f24;
    _31 = f31;
    _32 = f32;
    _33 = f33;
    _34 = f34;
    _41 = f41;
    _42 = f42;
    _43 = f43;
    _44 = f44;
  };
  operator float *() { return &_11; }
  operator const float *() const { return &_11; }
#endif
} D3DXMATRIX, *LPD3DXMATRIX;

#ifdef __cplusplus
inline D3DXMATRIX operator*(const D3DXMATRIX &a, const D3DXMATRIX &b) {
  D3DXMATRIX out;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      out.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] +
                    a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
    }
  }
  return out;
}

inline D3DXMATRIX &operator*=(D3DXMATRIX &a, const D3DXMATRIX &b) {
  a = a * b;
  return a;
}

inline D3DXMATRIX *D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy,
                                     float sz) {
  memset(pOut, 0, sizeof(D3DXMATRIX));
  pOut->_11 = sx;
  pOut->_22 = sy;
  pOut->_33 = sz;
  pOut->_44 = 1.0f;
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y,
                                         float z) {
  memset(pOut, 0, sizeof(D3DXMATRIX));
  pOut->_11 = 1.0f;
  pOut->_22 = 1.0f;
  pOut->_33 = 1.0f;
  pOut->_44 = 1.0f;
  pOut->_41 = x;
  pOut->_42 = y;
  pOut->_43 = z;
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDeterminant,
                                     const D3DXMATRIX *pM) {
  if (pOut)
    *pOut = *pM;
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1,
                                      const D3DXMATRIX *pM2) {
  *pOut = (*pM1) * (*pM2);
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM) {
  D3DXMATRIX tmp;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      tmp.m[i][j] = pM->m[j][i];
    }
  }
  *pOut = tmp;
  return pOut;
}

inline D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *pOut, float Angle) {
  float s = sinf(Angle);
  float c = cosf(Angle);
  memset(pOut, 0, sizeof(D3DXMATRIX));
  pOut->_11 = c;
  pOut->_12 = s;
  pOut->_21 = -s;
  pOut->_22 = c;
  pOut->_33 = 1.0f;
  pOut->_44 = 1.0f;
  return pOut;
}

#ifdef __cplusplus
extern "C" {
#endif
D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV,
                                      const D3DXMATRIX *pM);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
inline D3DXVECTOR4 *D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV,
                                      const D3DXMATRIX *pM) {
  float x = pV->x, y = pV->y, z = pV->z, w = pV->w;
  pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31 + w * pM->_41;
  pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32 + w * pM->_42;
  pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33 + w * pM->_43;
  pOut->w = x * pM->_14 + y * pM->_24 + z * pM->_34 + w * pM->_44;
  return pOut;
}

inline D3DXVECTOR4 *D3DXVec3Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV,
                                      const D3DXMATRIX *pM) {
  float x = pV->x, y = pV->y, z = pV->z;
  pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41;
  pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42;
  pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43;
  pOut->w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;
  return pOut;
}

inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *pOut,
                                           const D3DXVECTOR3 *pV,
                                           const D3DXMATRIX *pM) {
  float x = pV->x, y = pV->y, z = pV->z;
  float w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;
  pOut->x = (x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41) / w;
  pOut->y = (x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42) / w;
  pOut->z = (x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43) / w;
  return pOut;
}

inline float D3DXVec4Dot(const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2) {
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

inline float D3DXVec3Dot(const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2) {
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

inline float D3DXVec3Length(const D3DXVECTOR3 *pV) {
  return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

inline float D3DXVec3LengthSq(const D3DXVECTOR3 *pV) {
  return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}
#endif
#endif
