/* macOS shim: d3dx8core.h — D3DX core utilities */
#pragma once

#include "d3d8_stub.h"
#include "d3dx8math.h"

/*──────────────────── ID3DXFont ────────────────────*/
/* ID3DXFont not used in game runtime on macOS — stub */
struct ID3DXFont {
  virtual ULONG WINAPI AddRef() { return 1; }
  virtual ULONG WINAPI Release() { return 0; }
  virtual HRESULT WINAPI QueryInterface(REFIID, void **) {
    return E_NOINTERFACE;
  }
  virtual HRESULT WINAPI Begin() { return D3D_OK; }
  virtual HRESULT WINAPI End() { return D3D_OK; }
  virtual INT WINAPI DrawTextA(const char *, INT, RECT *, DWORD, D3DCOLOR) {
    return 0;
  }
  virtual ~ID3DXFont() {}
};
typedef ID3DXFont *LPD3DXFONT;

/*──────────────────── D3DX Function Declarations ────────────────────*/
/* Implemented in Platform/MacOS/Source/Main/D3DXStubs.mm */

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI D3DXCreateFont(IDirect3DDevice8 *pDevice, HFONT hFont,
                              ID3DXFont **ppFont);

HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DTexture8 **ppTexture);

HRESULT WINAPI D3DXCreateTextureFromFileExA(
    IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width, UINT Height,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter,
    DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo, void *pPalette,
    IDirect3DTexture8 **ppTexture);

HRESULT WINAPI D3DXCreateCubeTexture(IDirect3DDevice8 *pDevice, UINT Size,
                                     UINT MipLevels, DWORD Usage,
                                     D3DFORMAT Format, D3DPOOL Pool,
                                     IDirect3DCubeTexture8 **ppCubeTexture);

HRESULT WINAPI D3DXCreateVolumeTexture(
    IDirect3DDevice8 *pDevice, UINT Width, UINT Height, UINT Depth,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    IDirect3DVolumeTexture8 **ppVolumeTexture);

HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *pDestSurface,
                                          const void *pDestPalette,
                                          const RECT *pDestRect,
                                          IDirect3DSurface8 *pSrcSurface,
                                          const void *pSrcPalette,
                                          const RECT *pSrcRect, DWORD Filter,
                                          D3DCOLOR ColorKey);

HRESULT WINAPI D3DXLoadSurfaceFromMemory(
    IDirect3DSurface8 *pDestSurface, const void *pDestPalette,
    const RECT *pDestRect, const void *pSrcMemory, D3DFORMAT SrcFormat,
    UINT SrcPitch, const void *pSrcPalette2, const RECT *pSrcRect, DWORD Filter,
    D3DCOLOR ColorKey);

HRESULT WINAPI D3DXSaveTextureToFileA(const char *pDestFile,
                                      D3DXIMAGE_FILEFORMAT DestFormat,
                                      IDirect3DBaseTexture8 *pSrcTexture,
                                      const void *pSrcPalette);

HRESULT WINAPI D3DXGetImageInfoFromFileA(const char *pSrcFile, void *pSrcInfo);

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors);

#ifdef __cplusplus
}
#endif

/* Inline utilities that don't need separate .mm implementation */
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char *pBuffer, UINT BufferLen) {
  (void)hr;
  if (pBuffer && BufferLen > 0)
    snprintf(pBuffer, BufferLen, "D3D error 0x%08lx (stub)", (unsigned long)hr);
  return S_OK;
}
#define D3DXGetErrorString D3DXGetErrorStringA

/* D3DXFilterTexture — generate mip levels for a texture.
   Uses simple 2x2 box filter to downsample from level 0 down to smallest mip.
   Critical for terrain/tree textures which only load level 0 data. */
inline HRESULT D3DXFilterTexture(IDirect3DBaseTexture8 *pTexture,
                                  const void *pPalette, DWORD SrcLevel,
                                  DWORD Filter) {
  if (!pTexture) return E_INVALIDARG;
  
  // All game callers pass IDirect3DTexture8* — safe to cast directly
  IDirect3DTexture8 *tex2D = static_cast<IDirect3DTexture8*>(pTexture);
  
  DWORD levelCount = tex2D->GetLevelCount();
  if (levelCount <= 1) {
    return S_OK;
  }
  
  // Get level 0 description to check format
  D3DSURFACE_DESC desc0;
  if (tex2D->GetLevelDesc(0, &desc0) != S_OK) {
    return S_OK;
  }
  
  // Only handle uncompressed BGRA8 formats (terrain/tree textures)
  bool isBGRA = (desc0.Format == D3DFMT_A8R8G8B8 || desc0.Format == D3DFMT_X8R8G8B8);
  if (!isBGRA) {
    return S_OK; // Compressed textures already have all mip levels from DDS
  }
  
  // For each mip level pair, read from source level and write box-filtered to dest level
  for (DWORD lvl = 0; lvl + 1 < levelCount; lvl++) {
    D3DSURFACE_DESC srcDesc, dstDesc;
    tex2D->GetLevelDesc(lvl, &srcDesc);
    tex2D->GetLevelDesc(lvl + 1, &dstDesc);
    
    D3DLOCKED_RECT srcLocked, dstLocked;
    if (tex2D->LockRect(lvl, &srcLocked, nullptr, D3DLOCK_READONLY) != S_OK) break;
    if (tex2D->LockRect(lvl + 1, &dstLocked, nullptr, 0) != S_OK) {
      tex2D->UnlockRect(lvl);
      break;
    }
    
    // Box filter: average each 2x2 block of source pixels into 1 dest pixel
    const uint8_t *src = (const uint8_t *)srcLocked.pBits;
    uint8_t *dst = (uint8_t *)dstLocked.pBits;
    
    for (UINT y = 0; y < dstDesc.Height; y++) {
      for (UINT x = 0; x < dstDesc.Width; x++) {
        UINT sx = x * 2;
        UINT sy = y * 2;
        // Clamp to source bounds
        UINT sx1 = (sx + 1 < srcDesc.Width) ? sx + 1 : sx;
        UINT sy1 = (sy + 1 < srcDesc.Height) ? sy + 1 : sy;
        
        const uint8_t *p00 = src + sy  * srcLocked.Pitch + sx  * 4;
        const uint8_t *p10 = src + sy  * srcLocked.Pitch + sx1 * 4;
        const uint8_t *p01 = src + sy1 * srcLocked.Pitch + sx  * 4;
        const uint8_t *p11 = src + sy1 * srcLocked.Pitch + sx1 * 4;
        
        uint8_t *pd = dst + y * dstLocked.Pitch + x * 4;
        pd[0] = (uint8_t)((p00[0] + p10[0] + p01[0] + p11[0] + 2) / 4); // B
        pd[1] = (uint8_t)((p00[1] + p10[1] + p01[1] + p11[1] + 2) / 4); // G
        pd[2] = (uint8_t)((p00[2] + p10[2] + p01[2] + p11[2] + 2) / 4); // R
        pd[3] = (uint8_t)((p00[3] + p10[3] + p01[3] + p11[3] + 2) / 4); // A
      }
    }
    
    tex2D->UnlockRect(lvl + 1); // triggers replaceRegion upload
    tex2D->UnlockRect(lvl);
  }
  
  static int s_filterCount = 0;
  if (++s_filterCount <= 10) {
    printf("[D3DXFilterTexture] #%d: %ux%u fmt=%u levels=%u\n",
           s_filterCount, desc0.Width, desc0.Height, (unsigned)desc0.Format, levelCount);
    fflush(stdout);
  }
  
  return S_OK;
}

/* Compute vertex size from FVF flags (used by dx8fvf.cpp) */
inline UINT D3DXGetFVFVertexSize(DWORD FVF) {
  UINT size = 0;
  if (FVF & D3DFVF_XYZ)
    size += 3 * sizeof(float);
  if (FVF & D3DFVF_XYZRHW)
    size += 4 * sizeof(float);
  if (FVF & D3DFVF_NORMAL)
    size += 3 * sizeof(float);
  if (FVF & D3DFVF_DIFFUSE)
    size += sizeof(DWORD);
  if (FVF & D3DFVF_SPECULAR)
    size += sizeof(DWORD);
  if (FVF & D3DFVF_PSIZE)
    size += sizeof(float);
  UINT texCount = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  size += texCount * 2 * sizeof(float); // assume 2D tex coords
  return size;
}

/*──────────────────── D3DX Filter flags ────────────────────*/
#ifndef D3DX_FILTER_NONE
#define D3DX_FILTER_NONE 0x00000001
#define D3DX_FILTER_POINT 0x00000002
#define D3DX_FILTER_LINEAR 0x00000003
#define D3DX_FILTER_TRIANGLE 0x00000004
#define D3DX_FILTER_BOX 0x00000005
#define D3DX_FILTER_MIRROR 0x00010000
#define D3DX_FILTER_DITHER 0x00080000
#define D3DX_DEFAULT 0xFFFFFFFF
#endif

/*──────────────────── D3DXIMAGE_INFO ────────────────────*/
#ifndef _D3DXIMAGE_INFO_DEFINED
#define _D3DXIMAGE_INFO_DEFINED
typedef struct _D3DXIMAGE_INFO {
  UINT Width;
  UINT Height;
  UINT Depth;
  UINT MipLevels;
  D3DFORMAT Format;
  D3DRESOURCETYPE ResourceType;
  D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;
#endif
