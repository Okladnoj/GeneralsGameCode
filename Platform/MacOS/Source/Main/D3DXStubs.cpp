#include <Utility/win_compat.h>

// Redeclare types to avoid including the header which makes functions inline
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
} D3DMATRIX;

typedef struct D3DXVECTOR3 {
  float x, y, z;
} D3DXVECTOR3;

typedef struct D3DXVECTOR4 {
  float x, y, z, w;
} D3DXVECTOR4;

typedef D3DMATRIX D3DXMATRIX;
typedef D3DXMATRIX *LPD3DXMATRIX;
typedef D3DXVECTOR4 *LPD3DXVECTOR4;
typedef const D3DXVECTOR4 *LPCD3DXVECTOR4;
typedef const D3DXVECTOR3 *LPCD3DXVECTOR3;
typedef const D3DXMATRIX *LPCD3DXMATRIX;

extern "C" {

D3DXMATRIX *WINAPI D3DXMatrixIdentity(D3DXMATRIX *pOut) {
  if (!pOut) return pOut;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      pOut->m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
  }
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1,
                                     const D3DXMATRIX *pM2) {
  if (!pOut || !pM1 || !pM2) return pOut;
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

D3DXMATRIX *WINAPI D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy,
                                    float sz) {
  D3DXMatrixIdentity(pOut);
  pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y,
                                        float z) {
  D3DXMatrixIdentity(pOut);
  pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixTranspose(D3DXMATRIX *pOut, const D3DXMATRIX *pM) {
  if (!pOut || !pM) return pOut;
  D3DXMATRIX res;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      res.m[j][i] = pM->m[i][j];
    }
  }
  *pOut = res;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationX(D3DXMATRIX *pOut, float Angle) {
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[1][1] = c; pOut->m[1][2] = s;
  pOut->m[2][1] = -s; pOut->m[2][2] = c;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationY(D3DXMATRIX *pOut, float Angle) {
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[0][0] = c; pOut->m[0][2] = -s;
  pOut->m[2][0] = s; pOut->m[2][2] = c;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixRotationZ(D3DXMATRIX *pOut, float Angle) {
  D3DXMatrixIdentity(pOut);
  float c = cosf(Angle);
  float s = sinf(Angle);
  pOut->m[0][0] = c; pOut->m[0][1] = s;
  pOut->m[1][0] = -s; pOut->m[1][1] = c;
  return pOut;
}

D3DXMATRIX *WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, float *pDet,
                                   const D3DXMATRIX *pM) {
  if (pDet) *pDet = 1.0f;
  return D3DXMatrixIdentity(pOut);
}

D3DXVECTOR4 *WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV,
                                     const D3DXMATRIX *pM) {
  if (!pOut || !pV || !pM) return pOut;
  D3DXVECTOR4 res;
  res.x = pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pV->w * pM->_41;
  res.y = pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pV->w * pM->_42;
  res.z = pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pV->w * pM->_43;
  res.w = pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pV->w * pM->_44;
  *pOut = res;
  return pOut;
}

D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV,
                                     const D3DXMATRIX *pM) {
  if (!pOut || !pV || !pM) return pOut;
  D3DXVECTOR4 v4;
  v4.x = pV->x; v4.y = pV->y; v4.z = pV->z; v4.w = 1.0f;
  return D3DXVec4Transform(pOut, &v4, pM);
}

// Stubs for others
typedef void *LPDIRECT3DDEVICE8;
typedef void *LPDIRECT3DTEXTURE8;
typedef void *LPDIRECT3DCUBETEXTURE8;
typedef void *LPDIRECT3DVOLUMETEXTURE8;
typedef void *LPDIRECT3DBASETEXTURE8;
typedef void *LPDIRECT3DSURFACE8;
typedef void *LPD3DXBUFFER;
typedef uint32_t D3DFORMAT;
typedef uint32_t D3DPOOL;
typedef uint32_t D3DCOLOR;

typedef struct D3DXIMAGE_INFO {
  uint32_t Width;
  uint32_t Height;
  uint32_t Depth;
  uint32_t MipLevels;
  D3DFORMAT Format;
} D3DXIMAGE_INFO;

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, uint32_t SrcDataLen,
                                  uint32_t Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppShader,
                                  LPD3DXBUFFER *ppErrorMsgs) {
  return 0;
}
HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE8 pDevice, uint32_t Width,
                                uint32_t Height, uint32_t MipLevels,
                                uint32_t Usage, D3DFORMAT Format, D3DPOOL Pool,
                                LPDIRECT3DTEXTURE8 *ppTexture) {
  return 0;
}
HRESULT WINAPI D3DXCreateCubeTexture(LPDIRECT3DDEVICE8 pDevice, uint32_t Size,
                                    uint32_t MipLevels, uint32_t Usage,
                                    D3DFORMAT Format, D3DPOOL Pool,
                                    LPDIRECT3DCUBETEXTURE8 *ppCubeTexture) {
  return 0;
}
HRESULT WINAPI D3DXCreateVolumeTexture(LPDIRECT3DDEVICE8 pDevice, uint32_t Width,
                                      uint32_t Height, uint32_t Depth,
                                      uint32_t MipLevels, uint32_t Usage,
                                      D3DFORMAT Format, D3DPOOL Pool,
                                      LPDIRECT3DVOLUMETEXTURE8 *ppVolumeTexture) {
  return 0;
}
HRESULT WINAPI D3DXFilterTexture(LPDIRECT3DBASETEXTURE8 pBaseTexture,
                                const void *pPalette, uint32_t SrcLevel,
                                uint32_t Filter) {
  return 0;
}
HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8 pDestSurface,
                                         const void *pDestPalette,
                                         const void *pDestRect,
                                         LPDIRECT3DSURFACE8 pSrcSurface,
                                         const void *pSrcPalette,
                                         const void *pSrcRect, uint32_t Filter,
                                         D3DCOLOR ColorKey) {
  return 0;
}
const char *WINAPI D3DXGetErrorStringA(HRESULT hr) { return "D3DX Error"; }
uint32_t WINAPI D3DXGetFVFVertexSize(uint32_t FVF) { return 32; }
HRESULT WINAPI D3DXCreateTextureFromFileExA(
    LPDIRECT3DDEVICE8 pDevice, const char *pSrcFile, uint32_t Width,
    uint32_t Height, uint32_t MipLevels, uint32_t Usage, D3DFORMAT Format,
    D3DPOOL Pool, uint32_t Filter, uint32_t MipFilter, D3DCOLOR ColorKey,
    D3DXIMAGE_INFO *pSrcInfo, void *pPalette, LPDIRECT3DTEXTURE8 *ppTexture) {
  return 0;
}

}
