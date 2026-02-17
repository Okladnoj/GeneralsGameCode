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

HRESULT WINAPI D3DXFilterTexture(struct IDirect3DBaseTexture8 *pBaseTexture,
                                 const void *pPalette, UINT SrcLevel,
                                 DWORD Filter);

HRESULT WINAPI D3DXLoadSurfaceFromSurface(
    struct IDirect3DSurface8 *pDestSurface, const void *pDestPalette,
    const RECT *pDestRect, struct IDirect3DSurface8 *pSrcSurface,
    const void *pSrcPalette, const RECT *pSrcRect, DWORD Filter,
    DWORD ColorKey);

#define D3DX_DEFAULT (UINT) - 1

HRESULT WINAPI D3DXCreateTexture(struct IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 struct IDirect3DTexture8 **ppTexture);

HRESULT WINAPI D3DXCreateTextureFromFileExA(
    struct IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width,
    UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo,
    const void *pPalette, struct IDirect3DTexture8 **ppTexture);

#define D3DXCreateTextureFromFileEx D3DXCreateTextureFromFileExA

HRESULT WINAPI D3DXCreateCubeTexture(
    struct IDirect3DDevice8 *pDevice, UINT Size, UINT MipLevels, DWORD Usage,
    D3DFORMAT Format, D3DPOOL Pool,
    struct IDirect3DCubeTexture8 **ppCubeTexture);

HRESULT WINAPI D3DXCreateVolumeTexture(
    struct IDirect3DDevice8 *pDevice, UINT Width, UINT Height, UINT Depth,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    struct IDirect3DVolumeTexture8 **ppVolumeTexture);

#ifdef __cplusplus
}
#endif
