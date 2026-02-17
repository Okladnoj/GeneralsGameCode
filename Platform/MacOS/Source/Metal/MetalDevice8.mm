/**
 * MetalDevice8.mm — IDirect3DDevice8 implementation on Apple Metal
 *
 * Stage 0: Skeleton — all methods log and return D3D_OK.
 * BeginScene/EndScene/Present/Clear have real Metal frame lifecycle code.
 */
#ifdef __APPLE__

// Import ObjC/Metal frameworks FIRST, before win_compat.h
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// Now include our header (which includes d3d8.h / win_compat.h)
#include "IRenderDevice.h"
#include "MacOSBuffers.h"
#import "MetalDevice8.h"
#include <cstdio>
#include <cstring>

// Forward declaration (defined in D3DXStubs.cpp)
extern "C" UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF);

// ─────────────────────────────────────────────────────
//  Helpers: Cast opaque pointers to Metal types
// ─────────────────────────────────────────────────────
#define MTL_DEVICE ((__bridge id<MTLDevice>)m_Device)
#define MTL_QUEUE ((__bridge id<MTLCommandQueue>)m_CommandQueue)
#define MTL_LAYER ((__bridge CAMetalLayer *)m_MetalLayer)
#define MTL_CMD_BUF ((__bridge id<MTLCommandBuffer>)m_CurrentCommandBuffer)
#define MTL_DRAWABLE ((__bridge id<CAMetalDrawable>)m_CurrentDrawable)
#define MTL_ENCODER ((__bridge id<MTLRenderCommandEncoder>)m_CurrentEncoder)

#define SET_MTL(member, val)                                                   \
  do {                                                                         \
    m_##member = (__bridge_retained void *)(val);                              \
  } while (0)
#define CLEAR_MTL(member)                                                      \
  do {                                                                         \
    if (m_##member) {                                                          \
      CFRelease(m_##member);                                                   \
      m_##member = nullptr;                                                    \
    }                                                                          \
  } while (0)

// ─────────────────────────────────────────────────────
//  Construction / Destruction
// ─────────────────────────────────────────────────────

MetalDevice8::MetalDevice8()
    : m_RefCount(1), m_Device(nullptr), m_CommandQueue(nullptr),
      m_MetalLayer(nullptr), m_CurrentCommandBuffer(nullptr),
      m_CurrentDrawable(nullptr), m_CurrentEncoder(nullptr), m_InScene(false),
      m_StreamSource(nullptr), m_StreamStride(0), m_IndexBuffer(nullptr),
      m_BaseVertexIndex(0), m_VertexShader(0), m_PixelShader(0),
      m_HWND(nullptr), m_ScreenWidth(800), m_ScreenHeight(600) {
  memset(m_RenderStates, 0, sizeof(m_RenderStates));
  memset(m_TextureStageStates, 0, sizeof(m_TextureStageStates));
  memset(m_Textures, 0, sizeof(m_Textures));
  memset(m_Transforms, 0, sizeof(m_Transforms));
  memset(&m_Viewport, 0, sizeof(m_Viewport));
  memset(&m_Material, 0, sizeof(m_Material));
  memset(m_Lights, 0, sizeof(m_Lights));
  memset(m_LightEnabled, 0, sizeof(m_LightEnabled));

  auto setIdentity = [](D3DMATRIX &m) {
    memset(&m, 0, sizeof(m));
    m._11 = m._22 = m._33 = m._44 = 1.0f;
  };
  setIdentity(m_Transforms[D3DTS_VIEW]);
  setIdentity(m_Transforms[D3DTS_PROJECTION]);
  setIdentity(m_Transforms[D3DTS_WORLD]);
}

MetalDevice8::~MetalDevice8() {
  CLEAR_MTL(CurrentEncoder);
  CLEAR_MTL(CurrentCommandBuffer);
  CLEAR_MTL(CurrentDrawable);
  CLEAR_MTL(CommandQueue);
  CLEAR_MTL(Device);
  // MetalLayer is owned by the view, we don't release it
  m_MetalLayer = nullptr;
  fprintf(stderr, "[MetalDevice8] Destroyed\n");
}

bool MetalDevice8::InitMetal(void *windowHandle) {
  m_HWND = windowHandle;

  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  if (!device) {
    fprintf(stderr,
            "[MetalDevice8] ERROR: MTLCreateSystemDefaultDevice failed\n");
    return false;
  }
  SET_MTL(Device, device);

  id<MTLCommandQueue> queue = [device newCommandQueue];
  SET_MTL(CommandQueue, queue);

  CAMetalLayer *layer = [CAMetalLayer layer];
  layer.device = device;
  layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  layer.framebufferOnly = NO;
  m_MetalLayer = (__bridge_retained void *)layer;

  NSWindow *window = (__bridge NSWindow *)windowHandle;
  if (window) {
    window.contentView.layer = layer;
    window.contentView.wantsLayer = YES;
    CGSize viewSize = window.contentView.bounds.size;
    CGFloat scale = window.backingScaleFactor;
    layer.drawableSize =
        CGSizeMake(viewSize.width * scale, viewSize.height * scale);
    m_ScreenWidth = viewSize.width;
    m_ScreenHeight = viewSize.height;

    fprintf(stderr, "[MetalDevice8] Initialized: %gx%g (drawable: %gx%g)\n",
            m_ScreenWidth, m_ScreenHeight, layer.drawableSize.width,
            layer.drawableSize.height);
  }

  return true;
}

// ─────────────────────────────────────────────────────
//  IUnknown
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::QueryInterface(REFIID riid, void **ppvObj) {
  if (ppvObj)
    *ppvObj = nullptr;
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MetalDevice8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalDevice8::Release() {
  ULONG r = --m_RefCount;
  if (r == 0) {
    delete this;
    return 0;
  }
  return r;
}

// ─────────────────────────────────────────────────────
//  Device Status
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::TestCooperativeLevel() { return D3D_OK; }
STDMETHODIMP_(UINT) MetalDevice8::GetAvailableTextureMem() {
  return 512 * 1024 * 1024;
}
STDMETHODIMP MetalDevice8::ResourceManagerDiscardBytes(DWORD Bytes) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetAdapterIdentifier(UINT a, DWORD f,
                                                D3DADAPTER_IDENTIFIER8 *i) {
  if (i)
    memset(i, 0, sizeof(*i));
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetDeviceCaps(D3DCAPS8 *pCaps) {
  if (!pCaps)
    return E_POINTER;
  memset(pCaps, 0, sizeof(*pCaps));
  pCaps->DeviceType = D3DDEVTYPE_HAL;
  pCaps->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
  pCaps->MaxSimultaneousTextures = 8;
  pCaps->MaxTextureBlendStages = 8;
  pCaps->VertexShaderVersion = 0x0101;
  pCaps->PixelShaderVersion = 0x0101;
  pCaps->MaxPrimitiveCount = 0xFFFFFF;
  pCaps->MaxVertexIndex = 0xFFFFFF;
  pCaps->MaxStreams = 8;
  pCaps->MaxActiveLights = 4;
  pCaps->MaxTextureWidth = 4096;
  pCaps->MaxTextureHeight = 4096;
  pCaps->RasterCaps =
      D3DPRASTERCAPS_FOGRANGE | 0x00000100 | 0x00000200 | D3DPRASTERCAPS_ZBIAS;
  pCaps->TextureCaps = 0x00000001 | 0x00000002 | 0x00000004;
  pCaps->TextureOpCaps =
      D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 |
      D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_ADD |
      D3DTEXOPCAPS_BLENDDIFFUSEALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHA;
  pCaps->PrimitiveMiscCaps = D3DPMISCCAPS_COLORWRITEENABLE;
  pCaps->Caps2 = D3DCAPS2_FULLSCREENGAMMA;
  pCaps->SrcBlendCaps = 0x1FFF;
  pCaps->DestBlendCaps = 0x1FFF;
  pCaps->ZCmpCaps = 0xFF;
  pCaps->AlphaCmpCaps = 0xFF;
  pCaps->StencilCaps = 0xFF;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetDisplayMode(D3DDISPLAYMODE *pMode) {
  if (!pMode)
    return E_POINTER;
  pMode->Width = (UINT)m_ScreenWidth;
  pMode->Height = (UINT)m_ScreenHeight;
  pMode->RefreshRate = 60;
  pMode->Format = D3DFMT_A8R8G8B8;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Swap Chain / Present
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS *p,
                                                     IDirect3DSwapChain8 **s) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::Reset(D3DPRESENT_PARAMETERS *p) { return D3D_OK; }

STDMETHODIMP MetalDevice8::Present(const void *s, const void *d, HWND w,
                                   const void *r) {
  if (m_CurrentEncoder) {
    [MTL_ENCODER endEncoding];
    CLEAR_MTL(CurrentEncoder);
  }
  if (m_CurrentDrawable && m_CurrentCommandBuffer) {
    [MTL_CMD_BUF presentDrawable:MTL_DRAWABLE];
  }
  if (m_CurrentCommandBuffer) {
    [MTL_CMD_BUF commit];
    CLEAR_MTL(CurrentCommandBuffer);
  }
  CLEAR_MTL(CurrentDrawable);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetBackBuffer(UINT i, D3DBACKBUFFER_TYPE t,
                                         IDirect3DSurface8 **b) {
  if (b)
    *b = nullptr;
  return E_NOTIMPL;
}

// ─────────────────────────────────────────────────────
//  Gamma
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetGammaRamp(DWORD f, const D3DGAMMARAMP *p) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::GetGammaRamp(D3DGAMMARAMP *p) { return D3D_OK; }

// ─────────────────────────────────────────────────────
//  Resource Creation
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateTexture(UINT w, UINT h, UINT l, DWORD u,
                                         D3DFORMAT f, D3DPOOL p,
                                         IDirect3DTexture8 **t) {
  if (!t)
    return E_POINTER;
  ITexture *tex =
      GetRenderDevice()->CreateTexture(w, h, l, u, WW3D_FORMAT_A8R8G8B8);
  if (tex) {
    extern IDirect3DTexture8 *CreateMacOSD3DTextureWrapper(ITexture * tex);
    *t = CreateMacOSD3DTextureWrapper(tex);
    return D3D_OK;
  }
  return E_FAIL;
}

STDMETHODIMP MetalDevice8::CreateVolumeTexture(UINT w, UINT h, UINT d, UINT l,
                                               DWORD u, D3DFORMAT f, D3DPOOL p,
                                               IDirect3DVolumeTexture8 **t) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::CreateCubeTexture(UINT s, UINT l, DWORD u,
                                             D3DFORMAT f, D3DPOOL p,
                                             IDirect3DCubeTexture8 **t) {
  return E_NOTIMPL;
}

STDMETHODIMP MetalDevice8::CreateVertexBuffer(UINT Length, DWORD Usage,
                                              DWORD FVF, D3DPOOL Pool,
                                              IDirect3DVertexBuffer8 **ppVB) {
  if (!ppVB)
    return E_POINTER;
  UINT vertexSize = D3DXGetFVFVertexSize(FVF);
  if (vertexSize == 0)
    vertexSize = 32;
  UINT count = Length / vertexSize;
  *ppVB = new MacOSVertexBufferClass(FVF, (unsigned short)count, vertexSize);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::CreateIndexBuffer(UINT Length, DWORD Usage,
                                             D3DFORMAT Format, D3DPOOL Pool,
                                             IDirect3DIndexBuffer8 **ppIB) {
  if (!ppIB)
    return E_POINTER;
  bool is32bit = (Format == D3DFMT_INDEX32);
  UINT count = Length / (is32bit ? 4 : 2);
  *ppIB = new MacOSIndexBufferClass(count, is32bit);
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::CreateImageSurface(UINT w, UINT h, D3DFORMAT f,
                                              IDirect3DSurface8 **s) {
  if (!s)
    return E_POINTER;
  ITexture *tex =
      GetRenderDevice()->CreateTexture(w, h, 1, 0, WW3D_FORMAT_A8R8G8B8);
  if (tex) {
    extern IDirect3DSurface8 *CreateMacOSD3DSurfaceWrapper(ITexture * tex,
                                                           int level);
    *s = CreateMacOSD3DSurfaceWrapper(tex, 0);
    return D3D_OK;
  }
  return E_FAIL;
}

// ─────────────────────────────────────────────────────
//  Surface / Texture Operations
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CopyRects(IDirect3DSurface8 *src, const void *sr,
                                     UINT c, IDirect3DSurface8 *dst,
                                     const void *dp) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::UpdateTexture(IDirect3DBaseTexture8 *s,
                                         IDirect3DBaseTexture8 *d) {
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetFrontBuffer(IDirect3DSurface8 *d) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Render Target
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetRenderTarget(IDirect3DSurface8 *s,
                                           IDirect3DSurface8 *d) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::GetRenderTarget(IDirect3DSurface8 **s) {
  if (s)
    *s = nullptr;
  return E_NOTIMPL;
}
STDMETHODIMP MetalDevice8::GetDepthStencilSurface(IDirect3DSurface8 **s) {
  if (s)
    *s = nullptr;
  return E_NOTIMPL;
}
STDMETHODIMP MetalDevice8::SetDepthStencilSurface(IDirect3DSurface8 *s) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Scene
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::BeginScene() {
  if (m_InScene)
    return D3D_OK;
  m_InScene = true;

  id<MTLCommandBuffer> cmdBuf = [MTL_QUEUE commandBuffer];
  SET_MTL(CurrentCommandBuffer, cmdBuf);

  id<CAMetalDrawable> drawable = [MTL_LAYER nextDrawable];
  if (!drawable) {
    fprintf(stderr, "[MetalDevice8] WARNING: No drawable available\n");
    m_InScene = false;
    return E_FAIL;
  }
  SET_MTL(CurrentDrawable, drawable);

  return D3D_OK;
}

STDMETHODIMP MetalDevice8::EndScene() {
  if (!m_InScene)
    return D3D_OK;
  m_InScene = false;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::Clear(DWORD Count, const void *pRects, DWORD Flags,
                                 D3DCOLOR Color, float Z, DWORD Stencil) {
  if (!m_CurrentDrawable)
    return D3D_OK;

  if (m_CurrentEncoder) {
    [MTL_ENCODER endEncoding];
    CLEAR_MTL(CurrentEncoder);
  }

  MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor renderPassDescriptor];
  rpd.colorAttachments[0].texture = MTL_DRAWABLE.texture;

  if (Flags & D3DCLEAR_TARGET) {
    float a = ((Color >> 24) & 0xFF) / 255.0f;
    float r = ((Color >> 16) & 0xFF) / 255.0f;
    float g = ((Color >> 8) & 0xFF) / 255.0f;
    float b = ((Color >> 0) & 0xFF) / 255.0f;
    rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
    rpd.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, a);
  } else {
    rpd.colorAttachments[0].loadAction = MTLLoadActionLoad;
  }
  rpd.colorAttachments[0].storeAction = MTLStoreActionStore;

  id<MTLRenderCommandEncoder> encoder =
      [MTL_CMD_BUF renderCommandEncoderWithDescriptor:rpd];
  [encoder setLabel:@"MetalDevice8 RenderPass"];
  SET_MTL(CurrentEncoder, encoder);

  MTLViewport vp;
  vp.originX = m_Viewport.X;
  vp.originY = m_Viewport.Y;
  vp.width =
      m_Viewport.Width > 0 ? m_Viewport.Width : MTL_LAYER.drawableSize.width;
  vp.height =
      m_Viewport.Height > 0 ? m_Viewport.Height : MTL_LAYER.drawableSize.height;
  vp.znear = m_Viewport.MinZ;
  vp.zfar = m_Viewport.MaxZ > 0 ? m_Viewport.MaxZ : 1.0;
  [MTL_ENCODER setViewport:vp];

  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Transforms
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetTransform(D3DTRANSFORMSTATETYPE State,
                                        const D3DMATRIX *pMatrix) {
  if (!pMatrix)
    return E_POINTER;
  if ((int)State >= 0 && (int)State < 260) {
    m_Transforms[(int)State] = *pMatrix;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetTransform(D3DTRANSFORMSTATETYPE State,
                                        D3DMATRIX *pMatrix) {
  if (!pMatrix)
    return E_POINTER;
  if ((int)State >= 0 && (int)State < 260) {
    *pMatrix = m_Transforms[(int)State];
  }
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Viewport
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetViewport(const D3DVIEWPORT8 *pViewport) {
  if (!pViewport)
    return E_POINTER;
  m_Viewport = *pViewport;

  if (m_CurrentEncoder) {
    MTLViewport vp;
    vp.originX = pViewport->X;
    vp.originY = pViewport->Y;
    vp.width = pViewport->Width;
    vp.height = pViewport->Height;
    vp.znear = pViewport->MinZ;
    vp.zfar = pViewport->MaxZ;
    [MTL_ENCODER setViewport:vp];
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetViewport(D3DVIEWPORT8 *pViewport) {
  if (!pViewport)
    return E_POINTER;
  *pViewport = m_Viewport;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Material / Lighting
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetMaterial(const D3DMATERIAL8 *p) {
  if (!p)
    return E_POINTER;
  m_Material = *p;
  return D3D_OK;
}

HRESULT MetalDevice8::GetMaterial(D3DMATERIAL8 *p) {
  if (!p)
    return E_POINTER;
  *p = m_Material;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetLight(DWORD i, const D3DLIGHT8 *l) {
  if (i < MAX_LIGHTS && l)
    m_Lights[i] = *l;
  return D3D_OK;
}

HRESULT MetalDevice8::GetLight(DWORD i, D3DLIGHT8 *l) {
  if (i < MAX_LIGHTS && l)
    *l = m_Lights[i];
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::LightEnable(DWORD i, BOOL b) {
  if (i < MAX_LIGHTS)
    m_LightEnabled[i] = b;
  return D3D_OK;
}

HRESULT MetalDevice8::GetLightEnable(DWORD i, BOOL *b) {
  if (i < MAX_LIGHTS && b)
    *b = m_LightEnabled[i];
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Clip Planes
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetClipPlane(DWORD i, const float *p) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Render State
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetRenderState(D3DRENDERSTATETYPE State,
                                          DWORD Value) {
  if ((int)State < 256)
    m_RenderStates[(int)State] = Value;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::GetRenderState(D3DRENDERSTATETYPE State,
                                          DWORD *pValue) {
  if (!pValue)
    return E_POINTER;
  if ((int)State < 256)
    *pValue = m_RenderStates[(int)State];
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Textures / Texture Stage States
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetTexture(DWORD Stage,
                                      IDirect3DBaseTexture8 *pTexture) {
  if (Stage < MAX_TEXTURE_STAGES)
    m_Textures[Stage] = pTexture;
  return D3D_OK;
}

HRESULT MetalDevice8::GetTexture(DWORD Stage,
                                 IDirect3DBaseTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  if (Stage < MAX_TEXTURE_STAGES) {
    *ppTexture = m_Textures[Stage];
    if (*ppTexture)
      (*ppTexture)->AddRef();
  } else {
    *ppTexture = nullptr;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetTextureStageState(DWORD Stage,
                                                D3DTEXTURESTAGESTATETYPE Type,
                                                DWORD Value) {
  if (Stage < MAX_TEXTURE_STAGES && (int)Type < 32) {
    m_TextureStageStates[Stage][(int)Type] = Value;
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetTextureStageState(DWORD Stage,
                                           D3DTEXTURESTAGESTATETYPE Type,
                                           DWORD *pValue) {
  if (!pValue)
    return E_POINTER;
  if (Stage < MAX_TEXTURE_STAGES && (int)Type < 32) {
    *pValue = m_TextureStageStates[Stage][(int)Type];
  }
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Validate
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::ValidateDevice(DWORD *pNumPasses) {
  if (pNumPasses)
    *pNumPasses = 1;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Drawing — Stage 0 stubs
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::DrawPrimitive(DWORD pt, UINT sv, UINT pc) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::DrawIndexedPrimitive(DWORD pt, UINT mi, UINT nv,
                                                UINT si, UINT pc) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::DrawPrimitiveUP(DWORD pt, UINT pc, const void *data,
                                           UINT stride) {
  return D3D_OK;
}
STDMETHODIMP MetalDevice8::DrawIndexedPrimitiveUP(DWORD pt, UINT mvi, UINT nvi,
                                                  UINT pc, const void *idata,
                                                  D3DFORMAT ifmt,
                                                  const void *vdata,
                                                  UINT vstride) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Vertex Shaders
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreateVertexShader(const DWORD *decl,
                                              const DWORD *func, DWORD *handle,
                                              DWORD usage) {
  if (handle)
    *handle = 0;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetVertexShader(DWORD h) {
  m_VertexShader = h;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DeleteVertexShader(DWORD h) { return D3D_OK; }

STDMETHODIMP MetalDevice8::SetVertexShaderConstant(DWORD r, const void *d,
                                                   DWORD c) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Stream Source / Indices
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::SetStreamSource(UINT streamNum,
                                           IDirect3DVertexBuffer8 *vb,
                                           UINT stride) {
  if (streamNum == 0) {
    m_StreamSource = vb;
    m_StreamStride = stride;
  }
  return D3D_OK;
}

HRESULT MetalDevice8::GetStreamSource(UINT streamNum,
                                      IDirect3DVertexBuffer8 **vb,
                                      UINT *stride) {
  if (streamNum == 0) {
    if (vb)
      *vb = m_StreamSource;
    if (stride)
      *stride = m_StreamStride;
  }
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetIndices(IDirect3DIndexBuffer8 *ib, UINT base) {
  m_IndexBuffer = ib;
  m_BaseVertexIndex = base;
  return D3D_OK;
}

HRESULT MetalDevice8::GetIndices(IDirect3DIndexBuffer8 **ib, UINT *base) {
  if (ib)
    *ib = m_IndexBuffer;
  if (base)
    *base = m_BaseVertexIndex;
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Pixel Shaders
// ─────────────────────────────────────────────────────

STDMETHODIMP MetalDevice8::CreatePixelShader(const DWORD *func, DWORD *handle) {
  if (handle)
    *handle = 0;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::SetPixelShader(DWORD h) {
  m_PixelShader = h;
  return D3D_OK;
}

STDMETHODIMP MetalDevice8::DeletePixelShader(DWORD h) { return D3D_OK; }

STDMETHODIMP MetalDevice8::SetPixelShaderConstant(DWORD r, const void *d,
                                                  DWORD c) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Non-override helpers
// ─────────────────────────────────────────────────────

HRESULT MetalDevice8::GetDirect3D(IDirect3D8 **ppD3D8) {
  if (ppD3D8)
    *ppD3D8 = nullptr;
  return D3D_OK;
}

#endif // __APPLE__
