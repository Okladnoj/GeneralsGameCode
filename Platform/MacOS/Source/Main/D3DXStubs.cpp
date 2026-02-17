#include "IRenderDevice.h"
#include "d3d8.h"
#include "d3dx8.h"
#include <Utility/win_compat.h>
#include <algorithm>
#include <stdio.h>

// Helper to convert D3D formats to WW3D formats
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

// Mock implementation of Direct3D 8 interfaces for macOS
class MacOSD3DMockBase : public IUnknown {
public:
  MacOSD3DMockBase() : m_RefCount(1) {}
  virtual ~MacOSD3DMockBase() {}

  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
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

class MacOSD3D8 : public MacOSD3DMockBase, public IDirect3D8 {

public:
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {

    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }

  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }

  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  STDMETHOD(RegisterSoftwareDevice)(void *pInitializeFunction) override {

    return E_NOTIMPL;
  }

  STDMETHOD_(UINT, GetAdapterCount)() { return 1; }

  STDMETHOD(GetAdapterIdentifier)(

      UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8 *pIdentifier) override {

    memset(pIdentifier, 0, sizeof(*pIdentifier));

    strncpy(pIdentifier->Description, "Apple Metal Renderer", 511);

    return 0;
  }

  STDMETHOD_(UINT, GetAdapterModeCount)(UINT Adapter) { return 1; }

  STDMETHOD(EnumAdapterModes)(UINT Adapter, UINT Mode,

                              D3DDISPLAYMODE *pMode) {

    pMode->Width = 800;

    pMode->Height = 600;

    pMode->RefreshRate = 60;

    pMode->Format = D3DFMT_A8R8G8B8;

    return 0;
  }

  STDMETHOD(GetAdapterDisplayMode)(UINT Adapter,

                                   D3DDISPLAYMODE *pMode) {

    pMode->Width = 800;

    pMode->Height = 600;

    pMode->RefreshRate = 60;

    pMode->Format = D3DFMT_A8R8G8B8;

    return 0;
  }

  STDMETHOD(CheckDeviceType)(UINT Adapter, DWORD CheckType,
                             D3DFORMAT DisplayFormat,
                             D3DFORMAT BackBufferFormat,
                             BOOL Windowed) override {

    return 0;
  }

  STDMETHOD(CheckDeviceFormat)(UINT Adapter, DWORD DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, DWORD RType, D3DFORMAT CheckFormat) override {

    return 0;
  }

  STDMETHOD(CheckDeviceMultiSampleType)(

      UINT Adapter, DWORD DeviceType, D3DFORMAT SurfaceFormat,

      BOOL Windowed, DWORD MultiSampleType) {

    return 0;
  }

  STDMETHOD(CheckDepthStencilMatch)(UINT Adapter, DWORD DeviceType,

                                    D3DFORMAT AdapterFormat,

                                    D3DFORMAT RenderTargetFormat,

                                    D3DFORMAT DepthStencilFormat) {

    return 0;
  }

  STDMETHOD(GetDeviceCaps)(UINT Adapter, DWORD DeviceType,
                           D3DCAPS8 *c) override {

    memset(c, 0, sizeof(*c));

    c->VertexShaderVersion = 0x0101;

    c->PixelShaderVersion = 0x0101;

    return 0;
  }

  STDMETHOD_(HMONITOR, GetAdapterMonitor)(UINT Adapter) { return nullptr; }

  STDMETHOD(CreateDevice)(

      UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow,

      DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters,

      IDirect3DDevice8 **ppReturnedDeviceInterface) {

    return E_NOTIMPL;
  }
};

class MacOSD3DSurface;
class MacOSD3DSurface : public MacOSD3DMockBase, public IDirect3DSurface8 {
public:
  MacOSD3DSurface(ITexture *tex, int level) : m_Texture(tex), m_Level(level) {}

  // IUnknown delegate
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }
  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }
  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  // IDirect3DSurface8
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) { return E_NOTIMPL; }
  STDMETHOD(SetPrivateData)(REFGUID refguid, const void *pData,
                            DWORD SizeOfData, DWORD Flags) {
    return E_NOTIMPL;
  }
  STDMETHOD(GetPrivateData)(REFGUID refguid, void *pData, DWORD *pSizeOfData) {
    return E_NOTIMPL;
  }
  STDMETHOD(FreePrivateData)(REFGUID refguid) { return E_NOTIMPL; }
  STDMETHOD(GetContainer)(REFIID riid, void **ppContainer) { return E_NOTIMPL; }
  STDMETHOD(GetDesc)(D3DSURFACE_DESC *pDesc) {
    LevelInfo info;
    if (m_Texture && m_Texture->GetLevelDesc(m_Level, &info)) {
      pDesc->Width = info.Width;
      pDesc->Height = info.Height;
      pDesc->Format = (D3DFORMAT)21; // D3DFMT_A8R8G8B8
      return 0;
    }
    return -1;
  }
  STDMETHOD(LockRect)(D3DLOCKED_RECT *pLockedRect, const RECT *pRect,
                      DWORD Flags) {
    LockedRect lr;
    if (m_Texture && m_Texture->LockRect(m_Level, &lr, nullptr, Flags)) {
      pLockedRect->pBits = lr.pBits;
      pLockedRect->Pitch = lr.Pitch;
      return 0;
    }
    return -1;
  }
  STDMETHOD(UnlockRect)() {
    if (m_Texture)
      m_Texture->UnlockRect(m_Level);
    return 0;
  }

private:
  ITexture *m_Texture;
  int m_Level;
};

class MacOSD3DTexture;
class MacOSD3DTexture : public MacOSD3DMockBase, public IDirect3DTexture8 {
public:
  MacOSD3DTexture(ITexture *tex) : m_Texture(tex) {
    printf("DEBUG: Created MacOSD3DTexture %p for ITexture %p\n", this,
           m_Texture);
  }
  virtual ~MacOSD3DTexture() {
    // Note: ITexture is managed by IRenderDevice normally, but here we might
    // own it Depending on how CreateTexture works.
  }

  // IUnknown delegate
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }
  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }
  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  // IDirect3DResource8
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) { return E_NOTIMPL; }
  STDMETHOD(SetPrivateData)(REFGUID refguid, const void *pData,
                            DWORD SizeOfData, DWORD Flags) {
    return E_NOTIMPL;
  }
  STDMETHOD(GetPrivateData)(REFGUID refguid, void *pData, DWORD *pSizeOfData) {
    return E_NOTIMPL;
  }
  STDMETHOD(FreePrivateData)(REFGUID refguid) { return E_NOTIMPL; }
  STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew) { return 0; }
  STDMETHOD_(DWORD, GetPriority)() { return 0; }
  STDMETHOD_(void, PreLoad)() {}
  STDMETHOD_(D3DRESOURCETYPE, GetType)() { return D3DRTYPE_TEXTURE; }

  // IDirect3DBaseTexture8
  STDMETHOD_(DWORD, SetLOD)(DWORD LODNew) { return 0; }
  STDMETHOD_(DWORD, GetLOD)() { return 0; }
  STDMETHOD_(DWORD, GetLevelCount)() { return 1; }

  // IDirect3DTexture8
  STDMETHOD(GetLevelDesc)(UINT Level, D3DSURFACE_DESC *pDesc) {
    LevelInfo info;
    if (m_Texture && m_Texture->GetLevelDesc(Level, &info)) {
      pDesc->Width = info.Width;
      pDesc->Height = info.Height;
      pDesc->Format = (D3DFORMAT)21; // D3DFMT_A8R8G8B8
      return 0;
    }
    return -1;
  }
  STDMETHOD(GetSurfaceLevel)(UINT Level, IDirect3DSurface8 **ppSurfaceLevel) {
    if (!ppSurfaceLevel)
      return E_POINTER;
    *ppSurfaceLevel = new MacOSD3DSurface(m_Texture, Level);
    return 0;
  }
  STDMETHOD(LockRect)(UINT Level, D3DLOCKED_RECT *pLockedRect,
                      const RECT *pRect, DWORD Flags) {
    LockedRect lr;
    if (m_Texture && m_Texture->LockRect(Level, &lr, nullptr, Flags)) {
      pLockedRect->pBits = lr.pBits;
      pLockedRect->Pitch = lr.Pitch;
      return 0;
    }
    return -1;
  }
  STDMETHOD(UnlockRect)(UINT Level) {
    if (m_Texture)
      m_Texture->UnlockRect(Level);
    return 0;
  }
  STDMETHOD(AddDirtyRect)(const RECT *pDirtyRect) { return 0; }

private:
  ITexture *m_Texture;
};

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
    return 0;
  }
  return -1;
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
    return 0;
  }
  return -1;
}

HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture8 *pBaseTexture,
                                 const void *pPalette, UINT SrcLevel,
                                 DWORD Filter) {
  return 0;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *pDestSurface,
                                          const void *pDestPalette,
                                          const RECT *pDestRect,
                                          IDirect3DSurface8 *pSrcSurface,
                                          const void *pSrcPalette,
                                          const RECT *pSrcRect, DWORD Filter,
                                          DWORD ColorKey) {
  // This is used by MissingTexture::_Init to copy levels.
  // For now, let's just assume it's okay or maybe we should actually copy.
  D3DLOCKED_RECT srcLR, destLR;
  if (pSrcSurface->LockRect(&srcLR, pSrcRect, 0) == 0) {
    if (pDestSurface->LockRect(&destLR, pDestRect, 0) == 0) {
      // Very simple copy assuming same size and format (A8R8G8B8)
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
  return 0;
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
  return 0;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF) {
  UINT size = 0;
  if (FVF & D3DFVF_XYZRHW)
    size += 16;
  else if (FVF & D3DFVF_XYZ)
    size += 12;

  if (FVF & D3DFVF_XYZB1)
    size += 4;
  if (FVF & D3DFVF_XYZB2)
    size += 8;
  if (FVF & D3DFVF_XYZB3)
    size += 12;
  if (FVF & D3DFVF_XYZB4)
    size += 16;
  if (FVF & D3DFVF_XYZB5)
    size += 20;

  if (FVF & D3DFVF_NORMAL)
    size += 12;
  if (FVF & D3DFVF_PSIZE)
    size += 4;
  if (FVF & D3DFVF_DIFFUSE)
    size += 4;
  if (FVF & D3DFVF_SPECULAR)
    size += 4;

  UINT nTex = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (UINT i = 0; i < nTex; i++) {
    // Default size is 2 floats = 8 bytes
    size += 8;
  }
  return size;
}

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors) {
  return E_NOTIMPL;
}
}

class MacOSD3DDevice8 : public MacOSD3DMockBase, public IDirect3DDevice8 {
public:
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override {
    return MacOSD3DMockBase::QueryInterface(riid, ppvObj);
  }
  STDMETHOD_(ULONG, AddRef)() { return MacOSD3DMockBase::AddRef(); }
  STDMETHOD_(ULONG, Release)() { return MacOSD3DMockBase::Release(); }

  STDMETHOD(TestCooperativeLevel)() override { return 0; }
  STDMETHOD_(UINT, GetAvailableTextureMem)() { return 512 * 1024 * 1024; }
  STDMETHOD(ResourceManagerDiscardBytes)(DWORD Bytes) override { return 0; }
  STDMETHOD(GetDirect3D)(IDirect3D8 **ppD3D8) {
    *ppD3D8 = nullptr;
    return 0;
  }
  STDMETHOD(GetAdapterIdentifier)(UINT a, DWORD f,
                                  D3DADAPTER_IDENTIFIER8 *i) override {
    return 0;
  }
  STDMETHOD(GetDeviceCaps)(D3DCAPS8 *c) override {
    memset(c, 0, sizeof(*c));
    c->VertexShaderVersion = 0x0101;
    c->PixelShaderVersion = 0x0101;
    return 0;
  }
  STDMETHOD(GetDisplayMode)(D3DDISPLAYMODE *pMode) override {
    pMode->Width = 800;
    pMode->Height = 600;
    pMode->RefreshRate = 60;
    pMode->Format = D3DFMT_A8R8G8B8;
    return 0;
  }
  // Removed method with broken type
  STDMETHOD(SetCursorProperties)(UINT XHotSpot, UINT YHotSpot,
                                 IDirect3DSurface8 *pCursorBitmap) {
    return 0;
  }
  STDMETHOD_(void, SetCursorPosition)(UINT XScreenSpace, UINT YScreenSpace,
                                      DWORD Flags) {}
  STDMETHOD_(BOOL, ShowCursor)(BOOL bShow) { return TRUE; }
  STDMETHOD(CreateAdditionalSwapChain)(
      D3DPRESENT_PARAMETERS *pPresentationParameters,
      IDirect3DSwapChain8 **pSwapChain) override {
    return E_NOTIMPL;
  }
  STDMETHOD(Reset)(D3DPRESENT_PARAMETERS *p) override { return 0; }
  STDMETHOD(Present)(const void *s, const void *d, HWND w,
                     const void *r) override {
    return 0;
  }
  STDMETHOD(GetBackBuffer)(UINT i, D3DBACKBUFFER_TYPE t,
                           IDirect3DSurface8 **b) override {
    return E_NOTIMPL;
  }
  // Removed method with broken type
  STDMETHOD(SetGammaRamp)(DWORD Flags, const D3DGAMMARAMP *pRamp) override {
    return 0;
  }
  STDMETHOD(GetGammaRamp)(D3DGAMMARAMP *pRamp) override { return 0; }
  STDMETHOD(CreateTexture)(UINT w, UINT h, UINT l, DWORD u, D3DFORMAT f,
                           D3DPOOL p, IDirect3DTexture8 **t) override {
    ITexture *tex = GetRenderDevice()->CreateTexture(
        w, h, l, u, D3DFormat_To_WW3DFormat(f));
    if (tex) {
      *t = (IDirect3DTexture8 *)new MacOSD3DTexture(tex);
      return 0;
    }
    return -1;
  }

  STDMETHOD(CreateVolumeTexture)(UINT w, UINT h, UINT d, UINT l, DWORD u,
                                 D3DFORMAT f, D3DPOOL p,
                                 IDirect3DVolumeTexture8 **t) override {
    return E_NOTIMPL;
  }
  STDMETHOD(CreateImageSurface)(UINT w, UINT h, D3DFORMAT f,
                                IDirect3DSurface8 **s) override {
    ITexture *tex = GetRenderDevice()->CreateTexture(
        w, h, 1, 0, D3DFormat_To_WW3DFormat(f));
    if (tex) {
      *s = new MacOSD3DSurface(tex, 0);
      return 0;
    }
    return -1;
  }
  STDMETHOD(CreateCubeTexture)(UINT s, UINT l, DWORD u, D3DFORMAT f, D3DPOOL p,
                               IDirect3DCubeTexture8 **t) override {
    return E_NOTIMPL;
  }
  STDMETHOD(CreateVertexBuffer)(UINT l, DWORD u, DWORD f, D3DPOOL p,
                                IDirect3DVertexBuffer8 **v) override {
    return E_NOTIMPL;
  }
  STDMETHOD(CreateIndexBuffer)(UINT l, DWORD u, D3DFORMAT f, D3DPOOL p,
                               IDirect3DIndexBuffer8 **i) override {
    return E_NOTIMPL;
  }

  STDMETHOD(GetRenderTarget)(IDirect3DSurface8 **s) override {
    return E_NOTIMPL;
  }
  STDMETHOD(SetRenderTarget)(IDirect3DSurface8 *s,
                             IDirect3DSurface8 *d) override {
    return 0;
  }
  STDMETHOD(GetDepthStencilSurface)(IDirect3DSurface8 **s) override {
    return E_NOTIMPL;
  }
  STDMETHOD(SetDepthStencilSurface)(IDirect3DSurface8 *s) override { return 0; }
  STDMETHOD(CopyRects)(IDirect3DSurface8 *s, const void *r, UINT c,
                       IDirect3DSurface8 *d, const void *p) override {
    return 0;
  }

  STDMETHOD(UpdateTexture)(IDirect3DBaseTexture8 *s,
                           IDirect3DBaseTexture8 *d) override {
    return 0;
  }
  STDMETHOD(GetFrontBuffer)(IDirect3DSurface8 *d) override { return 0; }

  STDMETHOD(BeginScene)() override { return 0; }
  STDMETHOD(EndScene)() override { return 0; }
  STDMETHOD(Clear)(DWORD c, const void *r, DWORD f, D3DCOLOR cl, float z,
                   DWORD s) override {
    return 0;
  }
  STDMETHOD(SetTransform)(D3DTRANSFORMSTATETYPE t,
                          const D3DMATRIX *m) override {
    return 0;
  }
  STDMETHOD(GetTransform)(D3DTRANSFORMSTATETYPE t, D3DMATRIX *m) override {
    return 0;
  }
  STDMETHOD(MultiplyTransform)(D3DTRANSFORMSTATETYPE State,
                               CONST D3DMATRIX *pMatrix) {
    return 0;
  }
  STDMETHOD(SetViewport)(const D3DVIEWPORT8 *v) override { return 0; }
  STDMETHOD(GetViewport)(D3DVIEWPORT8 *pViewport) { return 0; }
  STDMETHOD(SetMaterial)(const D3DMATERIAL8 *m) override { return 0; }
  STDMETHOD(GetMaterial)(D3DMATERIAL8 *pMaterial) { return 0; }
  STDMETHOD(SetLight)(DWORD i, const D3DLIGHT8 *l) override { return 0; }
  STDMETHOD(GetLight)(DWORD Index, D3DLIGHT8 *pLight) { return 0; }
  STDMETHOD(LightEnable)(DWORD i, BOOL b) override { return 0; }
  STDMETHOD(GetLightEnable)(DWORD Index, BOOL *pEnable) { return 0; }
  STDMETHOD(SetClipPlane)(DWORD i, const float *p) override { return 0; }
  STDMETHOD(GetClipPlane)(DWORD Index, float *pPlane) { return 0; }
  STDMETHOD(SetRenderState)(D3DRENDERSTATETYPE s, DWORD v) override {
    return 0;
  }
  STDMETHOD(GetRenderState)(D3DRENDERSTATETYPE s, DWORD *v) override {
    return 0;
  }
  STDMETHOD(BeginStateBlock)() { return 0; }
  STDMETHOD(EndStateBlock)(DWORD *pToken) { return 0; }
  STDMETHOD(ApplyStateBlock)(DWORD Token) { return 0; }
  STDMETHOD(CaptureStateBlock)(DWORD Token) { return 0; }
  STDMETHOD(DeleteStateBlock)(DWORD Token) { return 0; }
  // Removed method with broken type
  // Removed method with broken type
  // Removed method with broken type
  STDMETHOD(GetTexture)(DWORD Stage, IDirect3DBaseTexture8 **ppTexture) {
    return E_NOTIMPL;
  }
  STDMETHOD(SetTexture)(DWORD s, IDirect3DBaseTexture8 *t) override {
    return 0;
  }
  STDMETHOD(GetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD *pValue) {
    return 0;
  }
  STDMETHOD(SetTextureStageState)(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,
                                  DWORD Value) override {
    return 0;
  }
  STDMETHOD(ValidateDevice)(DWORD *pPasses) override { return 0; }
  STDMETHOD(GetInfo)(DWORD DevInfoID, void *pDevInfoStruct,
                     DWORD DevInfoStructSize) {
    return 0;
  }
  STDMETHOD(SetPaletteEntries)(UINT PaletteNumber,
                               CONST PALETTEENTRY *pEntries) {
    return 0;
  }
  STDMETHOD(GetPaletteEntries)(UINT PaletteNumber, PALETTEENTRY *pEntries) {
    return 0;
  }
  STDMETHOD(SetCurrentTexturePalette)(UINT PaletteNumber) { return 0; }
  STDMETHOD(GetCurrentTexturePalette)(UINT *pPaletteNumber) { return 0; }
  STDMETHOD(DrawPrimitive)(DWORD t, UINT s, UINT p) override { return 0; }
  STDMETHOD(DrawIndexedPrimitive)(DWORD t, UINT m, UINT v, UINT s,
                                  UINT p) override {
    return 0;
  }
  STDMETHOD(DrawPrimitiveUP)(DWORD PrimitiveType, UINT PrimitiveCount,
                             const void *pVertexStreamZeroData,
                             UINT VertexStreamZeroStride) override {
    return 0;
  }
  STDMETHOD(DrawIndexedPrimitiveUP)(DWORD PrimitiveType, UINT MinVertexIndex,
                                    UINT NumVertexIndices, UINT PrimitiveCount,
                                    const void *pIndexData,
                                    D3DFORMAT IndexDataFormat,
                                    const void *pVertexStreamZeroData,
                                    UINT VertexStreamZeroStride) override {
    return 0;
  }
  STDMETHOD(ProcessVertices)(UINT SrcStartIndex, UINT DestIndex, UINT Count,
                             IDirect3DVertexBuffer8 *pDestBuffer, DWORD Flags) {
    return 0;
  }
  STDMETHOD(CreateVertexShader)(const DWORD *pDeclaration,
                                CONST DWORD *pFunction, DWORD *pHandle,
                                DWORD Usage) override {
    return 0;
  }
  STDMETHOD(SetVertexShader)(DWORD v) override { return 0; }
  STDMETHOD(GetVertexShader)(DWORD *pHandle) { return 0; }
  STDMETHOD(DeleteVertexShader)(DWORD v) override { return 0; }
  STDMETHOD(SetVertexShaderConstant)(DWORD Register, CONST void *pConstantData,
                                     DWORD ConstantCount) override {
    return 0;
  }
  STDMETHOD(GetVertexShaderConstant)(DWORD Register, void *pConstantData,
                                     DWORD ConstantCount) {
    return 0;
  }
  STDMETHOD(GetVertexShaderDeclaration)(DWORD Handle, void *pData,
                                        DWORD *pSizeOfData) {
    return 0;
  }
  STDMETHOD(GetVertexShaderFunction)(DWORD Handle, void *pData,
                                     DWORD *pSizeOfData) {
    return 0;
  }
  STDMETHOD(SetStreamSource)(UINT StreamNumber,
                             IDirect3DVertexBuffer8 *pStreamData,
                             UINT Stride) override {
    return 0;
  }
  STDMETHOD(GetStreamSource)(UINT StreamNumber,
                             IDirect3DVertexBuffer8 **ppStreamData,
                             UINT *pStride) {
    return E_NOTIMPL;
  }
  STDMETHOD(SetIndices)(IDirect3DIndexBuffer8 *i, UINT b) override { return 0; }
  STDMETHOD(GetIndices)(IDirect3DIndexBuffer8 **ppIndexData,
                        UINT *pBaseVertexIndex) {
    return E_NOTIMPL;
  }
  STDMETHOD(CreatePixelShader)(CONST DWORD *pFunction,
                               DWORD *pHandle) override {
    return 0;
  }
  STDMETHOD(SetPixelShader)(DWORD v) override { return 0; }
  STDMETHOD(GetPixelShader)(DWORD *pHandle) { return 0; }
  STDMETHOD(DeletePixelShader)(DWORD v) override { return 0; }
  STDMETHOD(SetPixelShaderConstant)(DWORD Register, CONST void *pConstantData,
                                    DWORD ConstantCount) override {
    return 0;
  }
  STDMETHOD(GetPixelShaderConstant)(DWORD Register, void *pConstantData,
                                    DWORD ConstantCount) {
    return 0;
  }
  STDMETHOD(GetPixelShaderFunction)(DWORD Handle, void *pData,
                                    DWORD *pSizeOfData) {
    return 0;
  }
  STDMETHOD(DrawRectPatch)(UINT h, const float *s, const void *p) { return 0; }
  STDMETHOD(DrawTriPatch)(UINT h, const float *s, const void *p) { return 0; }
  STDMETHOD(DeletePatch)(UINT Handle) { return 0; }
};

extern "C" IDirect3D8 *CreateMacOSD3D8() { return new MacOSD3D8(); }
extern "C" IDirect3DDevice8 *CreateMacOSD3DDevice8() {
  return new MacOSD3DDevice8();
}
