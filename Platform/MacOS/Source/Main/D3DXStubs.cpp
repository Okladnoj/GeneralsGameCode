/**
 * D3DXStubs.cpp — D3DX8 helper function implementations for macOS
 *
 * Contains:
 *  - D3DX* functions (texture creation, FVF parsing, etc.)
 *  - Legacy MacOSD3DTexture/MacOSD3DSurface wrappers (will be replaced in Stage
 * 5)
 *  - CreateMacOSD3D8() / CreateMacOSD3DDevice8() entry points
 */
#include "IRenderDevice.h"
#include "d3d8.h"
#include "d3dx8.h"
#include <Utility/win_compat.h>
#include <algorithm>
#include <cstdio>

// Forward declarations — factory functions defined in MetalInterface8.mm /
// MetalDevice8.mm We cannot #include Metal headers here since D3DXStubs.cpp is
// C++, not ObjC++
extern "C" IDirect3D8 *CreateMetalInterface8();
extern "C" IDirect3DDevice8 *CreateMetalDevice8();

// ═══════════════════════════════════════════════════════════════
//  Helper: D3DFORMAT → WW3DFormat
// ═══════════════════════════════════════════════════════════════
static WW3DFormat D3DFormat_To_WW3DFormat(D3DFORMAT format) {
  switch (format) {
  case D3DFMT_A8R8G8B8:
    return WW3D_FORMAT_A8R8G8B8;
  case D3DFMT_X8R8G8B8:
    return WW3D_FORMAT_X8R8G8B8;
  case D3DFMT_R5G6B5:
    return WW3D_FORMAT_R5G6B5;
  case D3DFMT_A1R5G5B5:
    return WW3D_FORMAT_A1R5G5B5;
  case D3DFMT_A4R4G4B4:
    return WW3D_FORMAT_A4R4G4B4;
  case D3DFMT_DXT1:
    return WW3D_FORMAT_DXT1;
  case D3DFMT_DXT3:
    return WW3D_FORMAT_DXT3;
  case D3DFMT_DXT5:
    return WW3D_FORMAT_DXT5;
  default:
    return WW3D_FORMAT_UNKNOWN;
  }
}

// ═══════════════════════════════════════════════════════════════
//  Legacy COM base (used by Surface/Texture wrappers below)
// ═══════════════════════════════════════════════════════════════
class MacOSD3DMockBase : public IUnknown {
public:
  MacOSD3DMockBase() : m_RefCount(1) {}
  virtual ~MacOSD3DMockBase() {}

  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    if (ppvObj)
      *ppvObj = nullptr;
    return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() { return ++m_RefCount; }
  STDMETHOD_(ULONG, Release)() {
    ULONG res = --m_RefCount;
    if (res == 0) {
      delete this;
      return 0;
    }
    return res;
  }

protected:
  ULONG m_RefCount;
};

// ═══════════════════════════════════════════════════════════════
//  MacOSD3DSurface — wraps ITexture level as IDirect3DSurface8
//  (Legacy, will be replaced by MetalSurface8 in Stage 5)
// ═══════════════════════════════════════════════════════════════
class MacOSD3DSurface : public MacOSD3DMockBase, public IDirect3DSurface8 {
public:
  MacOSD3DSurface(ITexture *tex, int level) : m_Texture(tex), m_Level(level) {}

  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }
  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }
  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) { return E_NOTIMPL; }
  STDMETHOD(SetPrivateData)(REFGUID g, const void *d, DWORD s, DWORD f) {
    return E_NOTIMPL;
  }
  STDMETHOD(GetPrivateData)(REFGUID g, void *d, DWORD *s) { return E_NOTIMPL; }
  STDMETHOD(FreePrivateData)(REFGUID g) { return E_NOTIMPL; }
  STDMETHOD(GetContainer)(REFIID r, void **c) { return E_NOTIMPL; }
  STDMETHOD(GetDesc)(D3DSURFACE_DESC *pDesc) {
    LevelInfo info;
    if (m_Texture && m_Texture->GetLevelDesc(m_Level, &info)) {
      pDesc->Width = info.Width;
      pDesc->Height = info.Height;
      pDesc->Format = D3DFMT_A8R8G8B8;
      return D3D_OK;
    }
    return E_FAIL;
  }
  STDMETHOD(LockRect)(D3DLOCKED_RECT *pLR, const RECT *pRect, DWORD Flags) {
    LockedRect lr;
    if (m_Texture && m_Texture->LockRect(m_Level, &lr, nullptr, Flags)) {
      pLR->pBits = lr.pBits;
      pLR->Pitch = lr.Pitch;
      return D3D_OK;
    }
    return E_FAIL;
  }
  STDMETHOD(UnlockRect)() {
    if (m_Texture)
      m_Texture->UnlockRect(m_Level);
    return D3D_OK;
  }

private:
  ITexture *m_Texture;
  int m_Level;
};

// ═══════════════════════════════════════════════════════════════
//  MacOSD3DTexture — wraps ITexture as IDirect3DTexture8
//  (Legacy, will be replaced by MetalTexture8 in Stage 5)
// ═══════════════════════════════════════════════════════════════
class MacOSD3DTexture : public MacOSD3DMockBase, public IDirect3DTexture8 {
public:
  MacOSD3DTexture(ITexture *tex) : m_Texture(tex) {}
  virtual ~MacOSD3DTexture() {}

  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }
  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }
  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  // IDirect3DResource8
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) { return E_NOTIMPL; }
  STDMETHOD(SetPrivateData)(REFGUID g, const void *d, DWORD s, DWORD f) {
    return E_NOTIMPL;
  }
  STDMETHOD(GetPrivateData)(REFGUID g, void *d, DWORD *s) { return E_NOTIMPL; }
  STDMETHOD(FreePrivateData)(REFGUID g) { return E_NOTIMPL; }
  STDMETHOD_(DWORD, SetPriority)(DWORD p) { return 0; }
  STDMETHOD_(DWORD, GetPriority)() { return 0; }
  STDMETHOD_(void, PreLoad)() {}
  STDMETHOD_(D3DRESOURCETYPE, GetType)() { return D3DRTYPE_TEXTURE; }

  // IDirect3DBaseTexture8
  STDMETHOD_(DWORD, SetLOD)(DWORD l) { return 0; }
  STDMETHOD_(DWORD, GetLOD)() { return 0; }
  STDMETHOD_(DWORD, GetLevelCount)() { return 1; }

  // IDirect3DTexture8
  STDMETHOD(GetLevelDesc)(UINT Level, D3DSURFACE_DESC *pDesc) {
    LevelInfo info;
    if (m_Texture && m_Texture->GetLevelDesc(Level, &info)) {
      pDesc->Width = info.Width;
      pDesc->Height = info.Height;
      pDesc->Format = D3DFMT_A8R8G8B8;
      return D3D_OK;
    }
    return E_FAIL;
  }
  STDMETHOD(GetSurfaceLevel)(UINT Level, IDirect3DSurface8 **ppSurf) {
    if (!ppSurf)
      return E_POINTER;
    *ppSurf = new MacOSD3DSurface(m_Texture, Level);
    return D3D_OK;
  }
  STDMETHOD(LockRect)(UINT Level, D3DLOCKED_RECT *pLR, const RECT *pRect,
                      DWORD Flags) {
    LockedRect lr;
    if (m_Texture && m_Texture->LockRect(Level, &lr, nullptr, Flags)) {
      pLR->pBits = lr.pBits;
      pLR->Pitch = lr.Pitch;
      return D3D_OK;
    }
    return E_FAIL;
  }
  STDMETHOD(UnlockRect)(UINT Level) {
    if (m_Texture)
      m_Texture->UnlockRect(Level);
    return D3D_OK;
  }
  STDMETHOD(AddDirtyRect)(const RECT *r) { return D3D_OK; }

private:
  ITexture *m_Texture;
};

// ═══════════════════════════════════════════════════════════════
//  Factory functions for MetalDevice8 to create legacy wrappers
// ═══════════════════════════════════════════════════════════════

IDirect3DTexture8 *CreateMacOSD3DTextureWrapper(ITexture *tex) {
  return new MacOSD3DTexture(tex);
}

IDirect3DSurface8 *CreateMacOSD3DSurfaceWrapper(ITexture *tex, int level) {
  return new MacOSD3DSurface(tex, level);
}

// ═══════════════════════════════════════════════════════════════
//  D3DX Functions
// ═══════════════════════════════════════════════════════════════

extern "C" {

HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  ITexture *tex = GetRenderDevice()->CreateTexture(
      Width, Height, MipLevels, Usage, D3DFormat_To_WW3DFormat(Format));
  if (tex) {
    *ppTexture = new MacOSD3DTexture(tex);
    return D3D_OK;
  }
  return E_FAIL;
}

HRESULT WINAPI D3DXCreateTextureFromFileExA(
    IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width, UINT Height,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter,
    DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo, const void *pPalette,
    IDirect3DTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  ITexture *tex = GetRenderDevice()->CreateTextureFromFile(pSrcFile);
  if (tex) {
    *ppTexture = new MacOSD3DTexture(tex);
    return D3D_OK;
  }
  return E_FAIL;
}

HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture8 *pBaseTexture,
                                 const void *pPalette, UINT SrcLevel,
                                 DWORD Filter) {
  return D3D_OK;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *pDestSurface,
                                          const void *pDestPalette,
                                          const RECT *pDestRect,
                                          IDirect3DSurface8 *pSrcSurface,
                                          const void *pSrcPalette,
                                          const RECT *pSrcRect, DWORD Filter,
                                          DWORD ColorKey) {
  D3DLOCKED_RECT srcLR, destLR;
  if (pSrcSurface->LockRect(&srcLR, pSrcRect, 0) == 0) {
    if (pDestSurface->LockRect(&destLR, pDestRect, 0) == 0) {
      D3DSURFACE_DESC desc;
      pDestSurface->GetDesc(&desc);
      for (unsigned int y = 0; y < desc.Height; ++y) {
        memcpy((char *)destLR.pBits + y * destLR.Pitch,
               (char *)srcLR.pBits + y * srcLR.Pitch, desc.Width * 4);
      }
      pDestSurface->UnlockRect();
    }
    pSrcSurface->UnlockRect();
  }
  return D3D_OK;
}

HRESULT WINAPI D3DXCreateCubeTexture(IDirect3DDevice8 *pDevice, UINT Size,
                                     UINT MipLevels, DWORD Usage,
                                     D3DFORMAT Format, D3DPOOL Pool,
                                     IDirect3DCubeTexture8 **ppCubeTexture) {
  return E_NOTIMPL;
}

HRESULT WINAPI D3DXCreateVolumeTexture(
    IDirect3DDevice8 *pDevice, UINT Width, UINT Height, UINT Depth,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    IDirect3DVolumeTexture8 **ppVolumeTexture) {
  return E_NOTIMPL;
}

HRESULT WINAPI D3DXGetErrorStringA(HRESULT hr, char *pBuffer, UINT BufferSize) {
  if (pBuffer && BufferSize > 0) {
    snprintf(pBuffer, BufferSize, "D3D Error 0x%08X", (unsigned int)hr);
  }
  return D3D_OK;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF) {
  UINT size = 0;

  // Position
  DWORD posType = FVF & 0x00E; // mask for XYZ/XYZRHW/XYZBn
  if (FVF & D3DFVF_XYZRHW) {
    size += 16; // float4: x, y, z, rhw
  } else if (posType >= D3DFVF_XYZB1 && posType <= D3DFVF_XYZB5) {
    size += 12; // float3: x, y, z
    // Blend weights
    int numWeights = (posType - D3DFVF_XYZ) / 2;
    size += numWeights * 4;
    if (FVF & D3DFVF_LASTBETA_UBYTE4) {
      // Last weight is stored as UBYTE4 (no extra size since it replaces a
      // float)
    }
  } else if (FVF & D3DFVF_XYZ) {
    size += 12; // float3: x, y, z
  }

  if (FVF & D3DFVF_NORMAL)
    size += 12; // float3
  if (FVF & D3DFVF_PSIZE)
    size += 4; // float
  if (FVF & D3DFVF_DIFFUSE)
    size += 4; // D3DCOLOR
  if (FVF & D3DFVF_SPECULAR)
    size += 4; // D3DCOLOR

  // Texture coordinates
  UINT nTex = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (UINT i = 0; i < nTex; i++) {
    // Check texture coordinate format (2 bits per tex coord)
    UINT texFmt = (FVF >> (i * 2 + 16)) & 0x3;
    switch (texFmt) {
    case D3DFVF_TEXTUREFORMAT1:
      size += 4;
      break; // float1
    case D3DFVF_TEXTUREFORMAT2:
      size += 8;
      break; // float2 (default)
    case D3DFVF_TEXTUREFORMAT3:
      size += 12;
      break; // float3
    case D3DFVF_TEXTUREFORMAT4:
      size += 16;
      break; // float4
    }
  }

  return size;
}

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors) {
  return E_NOTIMPL;
}

} // extern "C"

// ═══════════════════════════════════════════════════════════════
//  Entry Points — called from dx8wrapper.cpp
// ═══════════════════════════════════════════════════════════════

extern "C" IDirect3D8 *CreateMacOSD3D8() { return CreateMetalInterface8(); }

extern "C" IDirect3DDevice8 *CreateMacOSD3DDevice8() {
  return CreateMetalDevice8();
}
