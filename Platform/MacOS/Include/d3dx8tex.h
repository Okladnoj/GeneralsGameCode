#pragma once
#include "d3d8.h"

#define D3DX_FILTER_NONE 0x00000001
#define D3DX_FILTER_POINT 0x00000002
#define D3DX_FILTER_LINEAR 0x00000003
#define D3DX_FILTER_TRIANGLE 0x00000004
#define D3DX_FILTER_BOX 0x00000005

#ifdef __cplusplus
extern "C" {
#endif

inline HRESULT D3DXFilterTexture(struct IDirect3DBaseTexture8 *pBaseTexture,
                                 const void *pPalette, UINT SrcLevel,
                                 DWORD Filter) {
  return 0; // D3D_OK
}

inline HRESULT
D3DXLoadSurfaceFromSurface(struct IDirect3DSurface8 *pDestSurface,
                           const void *pDestPalette, const RECT *pDestRect,
                           struct IDirect3DSurface8 *pSrcSurface,
                           const void *pSrcPalette, const RECT *pSrcRect,
                           DWORD Filter, DWORD ColorKey) {
  return 0; // D3D_OK
}

#define D3DX_DEFAULT (UINT) - 1

inline HRESULT D3DXCreateTexture(struct IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 struct IDirect3DTexture8 **ppTexture) {
  return 0; // D3D_OK
}

inline HRESULT D3DXCreateTextureFromFileExA(
    struct IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width,
    UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo,
    const void *pPalette, struct IDirect3DTexture8 **ppTexture) {
  return 0; // D3D_OK
}

#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA

inline HRESULT
D3DXCreateCubeTexture(struct IDirect3DDevice8 *pDevice, UINT Size,
                      UINT MipLevels, DWORD Usage, D3DFORMAT Format,
                      D3DPOOL Pool,
                      struct IDirect3DCubeTexture8 **ppCubeTexture) {
  return 0; // D3D_OK
}

inline HRESULT
D3DXCreateVolumeTexture(struct IDirect3DDevice8 *pDevice, UINT Width,
                        UINT Height, UINT Depth, UINT MipLevels, DWORD Usage,
                        D3DFORMAT Format, D3DPOOL Pool,
                        struct IDirect3DVolumeTexture8 **ppVolumeTexture) {
  return 0; // D3D_OK
}

#ifdef __cplusplus
}
#endif
