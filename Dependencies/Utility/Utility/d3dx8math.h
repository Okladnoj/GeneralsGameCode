#pragma once

#ifndef _D3DX8MATH_H_
#define _D3DX8MATH_H_

#include "win_compat.h"
#include <math.h>

#define D3DX_PI ((float)3.141592654f)
#define D3DX_1BYPI ((float)0.318309886f)

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
  union {
    struct {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };
} D3DMATRIX, *LPD3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

typedef struct D3DXVECTOR3 {
  float x, y, z;
#ifdef __cplusplus
  D3DXVECTOR3() {}
  D3DXVECTOR3(const float *f) : x(f[0]), y(f[1]), z(f[2]) {}
  D3DXVECTOR3(float x, float y, float z) : x(x), y(y), z(z) {}
  operator float *() { return (float *)&x; }
  operator const float *() const { return (const float *)&x; }
#endif
} D3DXVECTOR3, *LPD3DXVECTOR3;

typedef struct D3DXVECTOR4 {
  float x, y, z, w;
#ifdef __cplusplus
  D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
  D3DXVECTOR4(const float *f) : x(f[0]), y(f[1]), z(f[2]), w(f[3]) {}
  D3DXVECTOR4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  operator float *() { return (float *)&x; }
  operator const float *() const { return (const float *)&x; }
#endif
} D3DXVECTOR4, *LPD3DXVECTOR4;

#ifdef __cplusplus
struct D3DXMATRIX : public _D3DMATRIX {
  D3DXMATRIX() {}
  D3DXMATRIX(const float *f) {
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        m[i][j] = f[i * 4 + j];
  }
  D3DXMATRIX(float _11, float _12, float _13, float _14, float _21, float _22,
             float _23, float _24, float _31, float _32, float _33, float _34,
             float _41, float _42, float _43, float _44) {
    this->_11 = _11;
    this->_12 = _12;
    this->_13 = _13;
    this->_14 = _14;
    this->_21 = _21;
    this->_22 = _22;
    this->_23 = _23;
    this->_24 = _24;
    this->_31 = _31;
    this->_32 = _32;
    this->_33 = _33;
    this->_34 = _34;
    this->_41 = _41;
    this->_42 = _42;
    this->_43 = _43;
    this->_44 = _44;
  }
  operator float *() { return (float *)&_11; }
  operator const float *() const { return (const float *)&_11; }
};
#else
typedef struct _D3DMATRIX D3DXMATRIX;
#endif

typedef struct D3DXMATRIX *LPD3DXMATRIX;

#ifdef __cplusplus
extern "C" {
#endif

inline D3DXMATRIX *WINAPI D3DXMatrixIdentity(D3DXMATRIX *pOut) {
  if (!pOut)
    return pOut;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      pOut->m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
  }
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut,
                                             const D3DXMATRIX *pM1,
                                             const D3DXMATRIX *pM2) {
  if (!pOut || !pM1 || !pM2)
    return pOut;
  D3DXMATRIX res;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res.m[i][j] = pM1->m[i][0] * pM2->m[0][j] + pM1->m[i][1] * pM2->m[1][j] +
                    pM1->m[i][2] * pM2->m[2][j] + pM1->m[i][3] * pM2->m[3][j];
    }
  }
  *pOut = res;
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixRotationX(D3DXMATRIX *pOut, float Angle) {
  if (!pOut)
    return pOut;
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[1][1] = c;
  pOut->m[1][2] = s;
  pOut->m[2][1] = -s;
  pOut->m[2][2] = c;
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixRotationY(D3DXMATRIX *pOut, float Angle) {
  if (!pOut)
    return pOut;
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[0][0] = c;
  pOut->m[0][2] = -s;
  pOut->m[2][0] = s;
  pOut->m[2][2] = c;
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixRotationZ(D3DXMATRIX *pOut, float Angle) {
  if (!pOut)
    return pOut;
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[0][0] = c;
  pOut->m[0][1] = s;
  pOut->m[1][0] = -s;
  pOut->m[1][1] = c;
  return pOut;
}

inline D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut,
                                             const D3DXVECTOR4 *pV,
                                             const D3DXMATRIX *pM) {
  if (!pOut || !pV || !pM)
    return pOut;
  D3DXVECTOR4 res;
  res.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pV->w * pM->_41;
  res.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pV->w * pM->_42;
  res.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pV->w * pM->_43;
  res.w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pV->w * pM->_44;
  *pOut = res;
  return pOut;
}

inline D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *pOut,
                                             const D3DXVECTOR3 *pV,
                                             const D3DXMATRIX *pM) {
  if (!pOut || !pV || !pM)
    return pOut;
  D3DXVECTOR4 v4(pV->x, pV->y, pV->z, 1.0f);
  return D3DXVec4Transform(pOut, &v4, pM);
}

inline float WINAPI D3DXVec4Dot(const D3DXVECTOR4 *pV1,
                                const D3DXVECTOR4 *pV2) {
  if (!pV1 || !pV2)
    return 0.0f;
  return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

inline D3DXMATRIX *WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, float x,
                                                float y, float z) {
  if (!pOut)
    return pOut;
  D3DXMatrixIdentity(pOut);
  pOut->_41 = x;
  pOut->_42 = y;
  pOut->_43 = z;
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixScaling(D3DXMATRIX *pOut, float sx,
                                            float sy, float sz) {
  if (!pOut)
    return pOut;
  D3DXMatrixIdentity(pOut);
  pOut->_11 = sx;
  pOut->_22 = sy;
  pOut->_33 = sz;
  return pOut;
}

inline D3DXMATRIX *WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut,
                                              const D3DXMATRIX *pM) {
  if (!pOut || !pM)
    return pOut;
  D3DXMATRIX res;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res.m[j][i] = pM->m[i][j];
    }
  }
  *pOut = res;
  return pOut;
}

// Dummy inverse - just identity for now to satisfy compiler
inline D3DXMATRIX *WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDet,
                                            const D3DXMATRIX *pM) {
  if (!pOut)
    return pOut;
  if (pDet)
    *pDet = 1.0f;
  D3DXMatrixIdentity(pOut);
  return pOut;
}

// ID3DXBuffer stubs removed - use real d3dx8.h

#ifdef __cplusplus
}

inline D3DXMATRIX operator*(const D3DXMATRIX &a, const D3DXMATRIX &b) {
  D3DXMATRIX res;
  D3DXMatrixMultiply(&res, &a, &b);
  return res;
}
#endif

#endif // _D3DX8MATH_H_
