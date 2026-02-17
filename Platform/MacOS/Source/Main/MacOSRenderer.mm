#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <algorithm>
#include <map>
#import <memory>
#include <string>
#import <vector>

#include "Common/ArchiveFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/GlobalData.h"
#include "Common/file.h"
#import "IRenderDevice.h"
#import "MacOSBuffers.h"
#import "MacOSRenderDevice_Internal.h"
#include "MacOSScreenshot.h"
#import "MacOSTexture.h"
#include "TARGA.h"
#include "ddsfile.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "matrix4.h"
#include "vector3.h"
#include "ww3dformat.h"

static NSString *const shaderSource =
    @""
     "#include <metal_stdlib>\n"
     "using namespace metal;\n"
     "\n"
     "struct VertexIn {\n"
     "    float3 position [[attribute(0)]];\n"
     "    float4 diffuse [[attribute(2)]];\n"
     "    float2 texCoord [[attribute(3)]];\n"
     "};\n"
     "\n"
     "struct VertexOut {\n"
     "    float4 position [[position]];\n"
     "    float4 color;\n"
     "    float2 texCoord;\n"
     "};\n"
     "\n"
     "struct Uniforms {\n"
     "    float4x4 world;\n"
     "    float4x4 view;\n"
     "    float4x4 projection;\n"
     "    float2 screenSize;\n"
     "    int useProjection;\n"
     "    uint shaderSettings;\n"
     "};\n"
     "\n"
     "vertex VertexOut vertex_main(VertexIn in [[stage_in]], constant Uniforms "
     "&u [[buffer(1)]]) {\n"
     "    VertexOut out;\n"
     "    if (u.useProjection == 1) {\n"
     "        out.position = u.projection * u.view * u.world * "
     "float4(in.position, 1.0);\n"
     "    } else {\n"
     "        out.position.x = (in.position.x / u.screenSize.x) * 2.0 - 1.0;\n"
     "        out.position.y = 1.0 - (in.position.y / u.screenSize.y) * 2.0;\n"
     "        out.position.z = in.position.z;\n"
     "        out.position.w = 1.0;\n"
     "    }\n"
     "    out.color = in.diffuse;\n"
     "    out.texCoord = in.texCoord;\n"
     "    return out;\n"
     "}\n"
     "\n"
     "fragment float4 fragment_main(VertexOut in [[stage_in]],\n"
     "                              texture2d<float> tex [[texture(0)]]) {\n"
     "    constexpr sampler s(address::clamp_to_edge, filter::linear);\n"
     "    float4 c = tex.sample(s, in.texCoord);\n"
     "    return c * in.color;\n"
     "}\n";

// TGAHeader structure removed to avoid clash with TARGA.h

id<MTLDevice> MacOSRenderDevice::m_StaticDevice = nil;
id<MTLDevice> MacOSRenderDevice::GetMTLDevice() { return m_StaticDevice; }

MacOSRenderDevice::MacOSRenderDevice()
    : m_Device(nil), m_CommandQueue(nil), m_CurrentEncoder(nil),
      m_CurrentCommandBuffer(nil), m_MetalLayer(nil), m_AlphaPS(nil),
      m_CurrentColor(0), m_CurrentDrawable(nil), m_WhiteTexture(nil),
      m_DepthTexture(nil), m_DepthStencilState(nil) {
  m_ScreenSize[0] = 800;
  m_ScreenSize[1] = 600;
  for (int i = 0; i < 8; ++i)
    m_Textures[i] = nullptr;
}

MacOSRenderDevice::~MacOSRenderDevice() { Shutdown(); }

bool MacOSRenderDevice::Initialize(void *hwnd) {
  m_TextureCache.clear();
  m_Device = MTLCreateSystemDefaultDevice();
  m_StaticDevice = m_Device;
  if (!m_Device)
    return false;
  m_CommandQueue = [m_Device newCommandQueue];
  NSWindow *window = (__bridge NSWindow *)hwnd;
  m_MetalLayer = [CAMetalLayer layer];
  m_MetalLayer.device = m_Device;
  m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
#if ENABLE_SCREENSHOTS
  m_MetalLayer.framebufferOnly = NO; // Allow getBytes: for screenshots
#endif
  if (window) {
    window.contentView.layer = m_MetalLayer;
    window.contentView.wantsLayer = YES;
    CGSize vs = window.contentView.bounds.size;
    m_MetalLayer.drawableSize =
        CGSizeMake(vs.width * window.backingScaleFactor,
                   vs.height * window.backingScaleFactor);
    m_ScreenSize[0] = (float)vs.width;
    m_ScreenSize[1] = (float)vs.height;

    // Create Depth Texture
    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                     width:m_MetalLayer.drawableSize.width
                                    height:m_MetalLayer.drawableSize.height
                                 mipmapped:NO];
    td.usage = MTLTextureUsageRenderTarget;
    td.storageMode = MTLStorageModePrivate;
    m_DepthTexture = [m_Device newTextureWithDescriptor:td];

    // Create Depth Stencil State
    MTLDepthStencilDescriptor *dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthCompareFunction = MTLCompareFunctionLessEqual;
    dsd.depthWriteEnabled = YES;
    m_DepthStencilState = [m_Device newDepthStencilStateWithDescriptor:dsd];

    printf("DEBUG: METAL INITIALIZED: ScreenSize=(%.1f, %.1f) "
           "DrawableSize=(%.1f, %.1f) Scale=%.1f\n",
           m_ScreenSize[0], m_ScreenSize[1], m_MetalLayer.drawableSize.width,
           m_MetalLayer.drawableSize.height, (float)window.backingScaleFactor);
    fflush(stdout);
  }
#if ENABLE_SCREENSHOTS
  MacOS_InitScreenshots();
#endif

  NSError *err = nil;
  id<MTLLibrary> lib = [m_Device newLibraryWithSource:shaderSource
                                              options:nil
                                                error:&err];
  if (!lib) {
    printf("METAL ERROR: Library creation failed: %s\n",
           [[err localizedDescription] UTF8String]);
    return false;
  }
  id<MTLFunction> vf = [lib newFunctionWithName:@"vertex_main"],
                  ff = [lib newFunctionWithName:@"fragment_main"];
  if (!vf || !ff) {
    printf("METAL ERROR: Function lookup failed: vf=%p ff=%p\n", vf, ff);
    return false;
  }
  MTLRenderPipelineDescriptor *psd = [[MTLRenderPipelineDescriptor alloc] init];
  psd.vertexFunction = vf;
  psd.fragmentFunction = ff;
  psd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
  psd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

  // --- 3D VERTEX DESCRIPTOR (44 bytes) ---
  MTLVertexDescriptor *vd3d = [[MTLVertexDescriptor alloc] init];
  vd3d.attributes[0].format = MTLVertexFormatFloat3; // Pos
  vd3d.attributes[0].offset = 0;
  vd3d.attributes[0].bufferIndex = 0;
  vd3d.attributes[1].format = MTLVertexFormatFloat3; // Normal
  vd3d.attributes[1].offset = 12;
  vd3d.attributes[1].bufferIndex = 0;
  vd3d.attributes[2].format = MTLVertexFormatUChar4Normalized; // Diffuse
  vd3d.attributes[2].offset = 24;
  vd3d.attributes[2].bufferIndex = 0;
  vd3d.attributes[3].format = MTLVertexFormatFloat2; // UV1
  vd3d.attributes[3].offset = 28;
  vd3d.attributes[3].bufferIndex = 0;
  vd3d.attributes[4].format = MTLVertexFormatFloat2; // UV2
  vd3d.attributes[4].offset = 36;
  vd3d.attributes[4].bufferIndex = 0;
  vd3d.layouts[0].stride = 44;

  // --- UI VERTEX DESCRIPTOR (24 bytes) ---
  MTLVertexDescriptor *vdui = [[MTLVertexDescriptor alloc] init];
  vdui.attributes[0].format = MTLVertexFormatFloat3; // Pos
  vdui.attributes[0].offset = 0;
  vdui.attributes[0].bufferIndex = 0;
  vdui.attributes[2].format =
      MTLVertexFormatUChar4Normalized; // Diffuse (at index 2 in shader)
  vdui.attributes[2].offset = 12;
  vdui.attributes[2].bufferIndex = 0;
  vdui.attributes[3].format = MTLVertexFormatFloat2; // UV
  vdui.attributes[3].offset = 16;
  vdui.attributes[3].bufferIndex = 0;
  vdui.layouts[0].stride = 24;

  // Opaque 3D
  psd.vertexDescriptor = vd3d;
  psd.colorAttachments[0].blendingEnabled = NO;
  m_OpaquePS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&err];

  // Alpha 3D
  psd.colorAttachments[0].blendingEnabled = YES;
  psd.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
  psd.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
  psd.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  psd.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
  psd.colorAttachments[0].destinationRGBBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  psd.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
  m_AlphaPS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&err];

  // UI (2D)
  psd.vertexDescriptor = vdui;
  m_UIPS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&err];

  if (!m_OpaquePS || !m_AlphaPS || !m_UIPS) {
    printf("METAL ERROR: Pipeline creation failed!\n");
    return false;
  }

  uint32_t white = 0xFFFFFFFF;
  m_WhiteTexture = [m_Device
      newTextureWithDescriptor:
          [MTLTextureDescriptor
              texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                           width:1
                                          height:1
                                       mipmapped:NO]];
  [m_WhiteTexture replaceRegion:MTLRegionMake2D(0, 0, 1, 1)
                    mipmapLevel:0
                      withBytes:&white
                    bytesPerRow:4];
  return true;
}

void MacOSRenderDevice::Shutdown() {
  m_Device = nil;
  m_CommandQueue = nil;
  m_MetalLayer = nil;
}
void MacOSRenderDevice::Clear(unsigned int color) {
  printf("DEBUG: MacOSRenderDevice::Clear(0x%08X)\n", color);
  fflush(stdout);
  m_CurrentColor = color;
}
void MacOSRenderDevice::Present() {
  if (m_CurrentCommandBuffer) {
    if (m_CurrentDrawable) {
#if ENABLE_SCREENSHOTS
      // Capture screenshot every 300 frames (~5s at 60fps)
      MacOS_SaveScreenshot((__bridge void *)m_CurrentDrawable.texture, 300,
                           false);
#endif
      [m_CurrentCommandBuffer presentDrawable:m_CurrentDrawable];
    }
    [m_CurrentCommandBuffer commit];
    m_CurrentCommandBuffer = nil;
    m_CurrentDrawable = nil;
  }
}

void MacOSRenderDevice::BeginScene() {
  printf("DEBUG: MacOSRenderDevice::BeginScene()\n");
  fflush(stdout);
  if (m_CurrentCommandBuffer)
    return;
  m_CurrentCommandBuffer = [m_CommandQueue commandBuffer];
  m_CurrentDrawable = [m_MetalLayer nextDrawable];
  if (!m_CurrentDrawable)
    return;
  MTLRenderPassDescriptor *pass = [[MTLRenderPassDescriptor alloc] init];
  pass.colorAttachments[0].texture = m_CurrentDrawable.texture;
  pass.colorAttachments[0].loadAction = MTLLoadActionClear;
  float r = ((m_CurrentColor >> 16) & 0xFF) / 255.0f;
  float g = ((m_CurrentColor >> 8) & 0xFF) / 255.0f;
  float b = (m_CurrentColor & 0xFF) / 255.0f;

  // High-end deep navy background for a premium feel
  if (r == 0 && g == 0 && b == 0) {
    r = 0.02f;
    g = 0.05f;
    b = 0.1f;
  }

  pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, 1.0);
  pass.depthAttachment.texture = m_DepthTexture;
  pass.depthAttachment.loadAction = MTLLoadActionClear;
  pass.depthAttachment.clearDepth = 1.0;

  m_CurrentEncoder =
      [m_CurrentCommandBuffer renderCommandEncoderWithDescriptor:pass];
  [m_CurrentEncoder setDepthStencilState:m_DepthStencilState];

  // Dynamic screen size update
  m_ScreenSize[0] = m_MetalLayer.drawableSize.width /
                    [NSScreen mainScreen].backingScaleFactor;
  m_ScreenSize[1] = m_MetalLayer.drawableSize.height /
                    [NSScreen mainScreen].backingScaleFactor;

  [m_CurrentEncoder
      setViewport:{0, 0, (double)m_MetalLayer.drawableSize.width,
                   (double)m_MetalLayer.drawableSize.height, 0, 1}];
}

void MacOSRenderDevice::EndScene() {
  if (m_CurrentEncoder) {
    [m_CurrentEncoder endEncoding];
    m_CurrentEncoder = nil;
  }
  Present();
}
void MacOSRenderDevice::SetViewport(const Viewport *vp) {
  printf("DEBUG: MacOSRenderDevice::SetViewport(%p)\n", vp);
  fflush(stdout);
  if (vp) {
    printf("DEBUG:   x=%d, y=%d, w=%d, h=%d\n", vp->x, vp->y, vp->w, vp->h);
    fflush(stdout);
    [m_CurrentEncoder
        setViewport:{(double)vp->x, (double)vp->y, (double)vp->w, (double)vp->h,
                     (double)vp->min_depth, (double)vp->max_depth}];
  }
}
void MacOSRenderDevice::SetTransform(TransformType t, const float *m) {
  printf("DEBUG: SetTransform type=%d\n", (int)t);
  fflush(stdout);
  matrix_float4x4 mat;
  memcpy(&mat, m, sizeof(mat));
  if (t == 0) // TRANSFORM_TYPE_WORLD
    m_Uniforms.world = mat;
  else if (t == 1) // TRANSFORM_TYPE_VIEW
    m_Uniforms.view = mat;
  else if (t == 2) // TRANSFORM_TYPE_PROJECTION
    m_Uniforms.projection = mat;
}
ITexture *MacOSRenderDevice::CreateTexture(unsigned int w, unsigned int h,
                                           unsigned int mips,
                                           unsigned int usage, WW3DFormat fmt) {
  bool mipmapped = (mips == 0 || mips > 1);
  id<MTLTexture> tex = [m_Device
      newTextureWithDescriptor:
          [MTLTextureDescriptor
              texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                           width:w
                                          height:h
                                       mipmapped:mipmapped]];

  int actual_mips = mips;
  if (mips == 0) {
    actual_mips = 1 + floor(log2(std::max(w, h)));
  }
  return new MacOSTexture(w, h, actual_mips, usage, fmt, tex);
}

void MacOSRenderDevice::SetTexture(unsigned int stage, ITexture *tex) {
  m_Textures[stage] = tex;
  if (stage == 0) {
    printf("DEBUG: SetTexture(0, %p)\n", tex);
    fflush(stdout);
  }
}
void MacOSRenderDevice::SetRenderState(RenderState s, unsigned int v) {}
void MacOSRenderDevice::SetMeshShader(unsigned int b) {}

void MacOSRenderDevice::DrawIndexed(PrimitiveType type, int vc, int ic,
                                    const Vertex *vd,
                                    const unsigned short *id) {
  printf("DEBUG: DrawIndexed type=%d vc=%d ic=%d\n", type, vc, ic);
  fflush(stdout);
  if (!m_CurrentEncoder)
    return;
  [m_CurrentEncoder setRenderPipelineState:m_UIPS];

  if (m_Textures[0]) {
    [m_CurrentEncoder
        setFragmentTexture:((MacOSTexture *)m_Textures[0])->GetMTLTexture()
                   atIndex:0];
  } else {
    [m_CurrentEncoder setFragmentTexture:m_WhiteTexture atIndex:0];
  }

  m_Uniforms.useProjection =
      0; // Inline shader: else branch does 2D screen-space conversion
  m_Uniforms.screenSize[0] = m_ScreenSize[0];
  m_Uniforms.screenSize[1] = m_ScreenSize[1];
  [m_CurrentEncoder setVertexBytes:vd length:24 * vc atIndex:0];
  [m_CurrentEncoder setVertexBytes:&m_Uniforms
                            length:sizeof(m_Uniforms)
                           atIndex:1];
  [m_CurrentEncoder
      drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                 indexCount:ic
                  indexType:MTLIndexTypeUInt16
                indexBuffer:[m_Device
                                newBufferWithBytes:id
                                            length:2 * ic
                                           options:MTLResourceStorageModeShared]
          indexBufferOffset:0];
}

void MacOSRenderDevice::DrawIndexed(PrimitiveType t, int ic,
                                    IndexBufferClass *ib,
                                    VertexBufferClass *vb) {
  printf("DEBUG: DrawIndexed (3D) ic=%d vb=%p\n", ic, vb);
  fflush(stdout);
  if (!m_CurrentEncoder || !ib || !vb)
    return;

  [m_CurrentEncoder setRenderPipelineState:m_OpaquePS];

  if (m_Textures[0]) {
    [m_CurrentEncoder
        setFragmentTexture:((MacOSTexture *)m_Textures[0])->GetMTLTexture()
                   atIndex:0];
  } else {
    [m_CurrentEncoder setFragmentTexture:m_WhiteTexture atIndex:0];
  }

  m_Uniforms.useProjection = 1;
  m_Uniforms.screenSize[0] = m_ScreenSize[0];
  m_Uniforms.screenSize[1] = m_ScreenSize[1];
  [m_CurrentEncoder setVertexBytes:&m_Uniforms
                            length:sizeof(m_Uniforms)
                           atIndex:1];

  // vb is VertexBufferClass* (actually DX8VertexBufferClass*)
  // ib is IndexBufferClass* (actually DX8IndexBufferClass*)
  // The MacOS buffer objects are INSIDE these DX8 wrappers
  DX8VertexBufferClass *dx8vb = static_cast<DX8VertexBufferClass *>(vb);
  DX8IndexBufferClass *dx8ib = static_cast<DX8IndexBufferClass *>(ib);

  MacOSVertexBufferClass *mvb =
      (MacOSVertexBufferClass *)dx8vb->Get_DX8_Vertex_Buffer();
  MacOSIndexBufferClass *mib =
      (MacOSIndexBufferClass *)dx8ib->Get_DX8_Index_Buffer();

  if (!mvb || !mib)
    return;

  void *metalVB = mvb->Get_Metal_Buffer();
  void *metalIB = mib->Get_Metal_Buffer();
  if (!metalVB || !metalIB)
    return;

  [m_CurrentEncoder setVertexBuffer:(__bridge id<MTLBuffer>)metalVB
                             offset:0
                            atIndex:0];

  MTLIndexType indexType =
      mib->Is_32Bit() ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;

  [m_CurrentEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:ic
                                indexType:indexType
                              indexBuffer:(__bridge id<MTLBuffer>)metalIB
                        indexBufferOffset:0];
}
void MacOSRenderDevice::DrawDebugText(const char *t, float x, float y,
                                      unsigned int c) {}
void MacOSRenderDevice::DrawFillRect(float x, float y, float w, float h,
                                     unsigned int c) {
  DrawTexturedRect(x, y, w, h, 0, 0, 1, 1, c);
}

struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};
struct DDS_HEADER {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwHeight;
  uint32_t dwWidth;
  uint32_t dwPitchOrLinearSize;
  uint32_t dwDepth;
  uint32_t dwMipMapCount;
  uint32_t dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t dwCaps;
  uint32_t dwCaps2;
  uint32_t dwCaps3;
  uint32_t dwCaps4;
  uint32_t dwReserved2;
};

struct uint8_t4 {
  uint8_t r, g, b, a;
};
static void DecompressDXT1(int w, int h, const uint8_t *src, uint8_t *dst) {
  for (int y = 0; y < h; y += 4) {
    for (int x = 0; x < w; x += 4) {
      uint16_t c0 = *(uint16_t *)src;
      src += 2;
      uint16_t c1 = *(uint16_t *)src;
      src += 2;
      uint32_t bits = *(uint32_t *)src;
      src += 4;
      auto decode = [](uint16_t c) {
        return uint8_t4{(uint8_t)((c >> 11) << 3),
                        (uint8_t)(((c >> 5) & 0x3F) << 2),
                        (uint8_t)((c & 0x1F) << 3), 255};
      };
      uint8_t4 palette[4];
      palette[0] = decode(c0);
      palette[1] = decode(c1);
      if (c0 > c1) {
        palette[2] = {(uint8_t)((2 * palette[0].r + palette[1].r) / 3),
                      (uint8_t)((2 * palette[0].g + palette[1].g) / 3),
                      (uint8_t)((2 * palette[0].b + palette[1].b) / 3), 255};
        palette[3] = {(uint8_t)((palette[0].r + 2 * palette[1].r) / 3),
                      (uint8_t)((palette[0].g + 2 * palette[1].g) / 3),
                      (uint8_t)((palette[0].b + 2 * palette[1].b) / 3), 255};
      } else {
        palette[2] = {(uint8_t)((palette[0].r + palette[1].r) / 2),
                      (uint8_t)((palette[0].g + palette[1].g) / 2),
                      (uint8_t)((palette[0].b + palette[1].b) / 2), 255};
        palette[3] = {0, 0, 0, 255};
      }
      for (int iy = 0; iy < 4; iy++) {
        for (int ix = 0; ix < 4; ix++) {
          if (x + ix < w && y + iy < h) {
            int idx = (bits >> (2 * (iy * 4 + ix))) & 3;
            uint8_t *p = &dst[((y + iy) * w + (x + ix)) * 4];
            p[0] = palette[idx].b;
            p[1] = palette[idx].g;
            p[2] = palette[idx].r;
            p[3] = 255;
          }
        }
      }
    }
  }
}

static void DecompressDXT5(int w, int h, const uint8_t *src, uint8_t *dst) {
  for (int y = 0; y < h; y += 4) {
    for (int x = 0; x < w; x += 4) {
      uint8_t a0 = *src++;
      uint8_t a1 = *src++;
      uint64_t abits = 0;
      memcpy(&abits, src, 6);
      src += 6;
      uint16_t c0 = *(uint16_t *)src;
      src += 2;
      uint16_t c1 = *(uint16_t *)src;
      src += 2;
      uint32_t cbits = *(uint32_t *)src;
      src += 4;
      uint8_t alphas[8];
      alphas[0] = a0;
      alphas[1] = a1;
      if (a0 > a1) {
        for (int i = 1; i < 7; i++)
          alphas[i + 1] = ((7 - i) * a0 + i * a1) / 7;
      } else {
        for (int i = 1; i < 5; i++)
          alphas[i + 1] = ((5 - i) * a0 + i * a1) / 5;
        alphas[6] = 0;
        alphas[7] = 255;
      }
      auto decode = [](uint16_t c) {
        return uint8_t4{(uint8_t)((c >> 11) << 3),
                        (uint8_t)(((c >> 5) & 0x3F) << 2),
                        (uint8_t)((c & 0x1F) << 3), 255};
      };
      uint8_t4 palette[4];
      palette[0] = decode(c0);
      palette[1] = decode(c1);
      palette[2] = {(uint8_t)((2 * palette[0].r + palette[1].r) / 3),
                    (uint8_t)((2 * palette[0].g + palette[1].g) / 3),
                    (uint8_t)((2 * palette[0].b + palette[1].b) / 3), 255};
      palette[3] = {(uint8_t)((palette[0].r + 2 * palette[1].r) / 3),
                    (uint8_t)((palette[0].g + 2 * palette[1].g) / 3),
                    (uint8_t)((palette[0].b + 2 * palette[1].b) / 3), 255};
      for (int iy = 0; iy < 4; iy++) {
        for (int ix = 0; ix < 4; ix++) {
          if (x + ix < w && y + iy < h) {
            int cidx = (cbits >> (2 * (iy * 4 + ix))) & 3;
            int aidx = (abits >> (3 * (iy * 4 + ix))) & 7;
            uint8_t *p = &dst[((y + iy) * w + (x + ix)) * 4];
            p[0] = palette[cidx].b;
            p[1] = palette[cidx].g;
            p[2] = palette[cidx].r;
            p[3] = alphas[aidx];
          }
        }
      }
    }
  }
}

ITexture *MacOSRenderDevice::CreateTextureFromFile(const char *filename) {
  if (m_TextureCache.count(filename))
    return m_TextureCache[filename];

  if (!filename || !filename[0])
    return nullptr;

  printf("DEBUG: CreateTextureFromFile: request '%s' (TheFileSystem=%s)\n",
         filename, TheFileSystem ? "OK" : "NULL");
  fflush(stdout);

  std::string fileStr = filename;
  std::vector<unsigned char> data;
  const char *prefs[] = {"",
                         "Art/",
                         "Art/Textures/",
                         "Data/",
                         "Data/English/Art/Textures/",
                         "Data/Window/",
                         "Window/",
                         "assets/"};
  bool found = false;
  for (int i = 0; i < 8; i++) {
    char p[1024];
    snprintf(p, 1024, "%s%s", prefs[i], filename);
    for (char *c = p; *c; c++)
      if (*c == '\\')
        *c = '/';

    if (TheFileSystem) {
      File *gf = TheFileSystem->openFile(p);
      if (gf) {
        size_t s = gf->size();
        data.resize(s);
        size_t readLen = gf->read(data.data(), s);
        gf->close();
        printf("DEBUG: CreateTextureFromFile: FOUND '%s', size=%zu read=%zu. "
               "First bytes: %02X %02X %02X %02X\n",
               p, s, readLen, data[0], data[1], data[2], data[3]);
        fflush(stdout);
        found = true;
      }
    }

    if (found)
      break;
  }
  if (!found)
    return nullptr;

  TGAHeader *h = (TGAHeader *)data.data();
  int w = h->Width, height = h->Height, bpp = h->PixelDepth,
      type = h->ImageType;

  if (data.size() >= 4 && data[0] == 'D' && data[1] == 'D' && data[2] == 'S' &&
      data[3] == ' ') {
    DDS_HEADER *dh = (DDS_HEADER *)(data.data() + 4);
    int dw = dh->dwWidth, dhh = dh->dwHeight;
    uint32_t fourcc = dh->ddspf.dwFourCC;
    const uint32_t FOURCC_DXT1 = 0x31545844; // 'DXT1'
    const uint32_t FOURCC_DXT5 = 0x35545844; // 'DXT5'

    printf("DEBUG: DDS detected: %dx%d, FourCC: %c%c%c%c (0x%08X)\n", dw, dhh,
           (fourcc & 0xFF), ((fourcc >> 8) & 0xFF), ((fourcc >> 16) & 0xFF),
           ((fourcc >> 24) & 0xFF), fourcc);
    std::vector<uint8_t> rgba(dw * dhh * 4);
    const uint8_t *src = (const uint8_t *)(dh + 1);
    if (fourcc == FOURCC_DXT1)
      DecompressDXT1(dw, dhh, src, rgba.data());
    else if (fourcc == FOURCC_DXT5)
      DecompressDXT5(dw, dhh, src, rgba.data());
    else {
      printf("DEBUG: Unsupported DDS FourCC\n");
      return nullptr;
    }
    ITexture *it = CreateTexture(dw, dhh, 1, 0, WW3D_FORMAT_A8R8G8B8);
    id<MTLTexture> mtl = ((MacOSTexture *)it)->GetMTLTexture();
    [mtl replaceRegion:MTLRegionMake2D(0, 0, dw, dhh)
           mipmapLevel:0
             withBytes:rgba.data()
           bytesPerRow:dw * 4];
    m_TextureCache[filename] = it;
    return it;
  }

  printf("DEBUG: CreateTextureFromFile: %s w=%d h=%d bpp=%d type=%d "
         "(palette=%d)\n",
         filename, w, height, bpp, type, h->ColorMapType);
  fflush(stdout);

  if (w <= 0 || height <= 0 || (bpp != 8 && bpp != 24 && bpp != 32)) {
    printf("DEBUG: CreateTextureFromFile: INVALID HEADER w=%d h=%d bpp=%d\n", w,
           height, bpp);
    fflush(stdout);
    return nullptr;
  }

  unsigned char *src_ptr = (unsigned char *)(h + 1) + h->IDLength;

  // Support 8-bit palettized TGA (used in some UI parts like logos/icons)
  struct RGBCol {
    uint8_t b, g, r, a;
  };
  std::vector<RGBCol> palette(256);
  if (h->ColorMapType == 1) {
    int len = h->CMapLength;
    int depth = h->CMapDepth; // 24 or 32
    int db = depth / 8;
    unsigned char *p_ptr = src_ptr;
    src_ptr += len * db;
    for (int i = 0; i < len; i++) {
      palette[i].b = p_ptr[i * db + 0];
      palette[i].g = p_ptr[i * db + 1];
      palette[i].r = p_ptr[i * db + 2];
      palette[i].a = (db == 4) ? p_ptr[i * db + 3] : 255;
    }
  }

  std::vector<unsigned char> decomp;
  int bv = bpp / 8;
  // Type 1 = Uncompressed Color-mapped, 9 = RLE Color-mapped, 2/3 =
  // Uncompressed RGB/Gray, 10/11 = RLE RGB/Gray
  if (type == 10 || type == 11 || type == 9) { // RLE
    decomp.resize(w * height * bv);
    unsigned char *dst = decomp.data();
    int pixels_count = 0, total_pixels = w * height;
    while (pixels_count < total_pixels) {
      unsigned char ch = *src_ptr++;
      if (ch & 0x80) {
        int n = (ch & 0x7F) + 1;
        for (int j = 0; j < n && pixels_count < total_pixels; j++) {
          memcpy(dst + pixels_count * bv, src_ptr, bv);
          pixels_count++;
        }
        src_ptr += bv;
      } else {
        int n = ch + 1;
        for (int j = 0; j < n && pixels_count < total_pixels; j++) {
          memcpy(dst + pixels_count * bv, src_ptr, bv);
          src_ptr += bv;
          pixels_count++;
        }
      }
    }
    src_ptr = decomp.data();
  }

  std::vector<unsigned char> rgba(w * height * 4);
  bool flipY = !(h->ImageDescriptor & 0x20);
  for (int y = 0; y < height; y++) {
    int sy = flipY ? (height - 1 - y) : y;
    const unsigned char *sline = src_ptr + (sy * w * bv);
    unsigned char *dline = rgba.data() + (y * w * 4);
    for (int x = 0; x < w; x++) {
      if (bv == 1 && h->ColorMapType == 1) { // 8-bit palettized
        uint8_t idx = sline[x];
        dline[x * 4 + 0] = palette[idx].b;
        dline[x * 4 + 1] = palette[idx].g;
        dline[x * 4 + 2] = palette[idx].r;
        dline[x * 4 + 3] = palette[idx].a;
      } else {
        dline[x * 4 + 0] = sline[x * bv + 0]; // B
        dline[x * 4 + 1] = sline[x * bv + 1]; // G
        dline[x * 4 + 2] = sline[x * bv + 2]; // R
        dline[x * 4 + 3] = (bv == 4) ? sline[x * bv + 3] : 255;
      }
    }
  }

  ITexture *it = CreateTexture(w, height, 1, 0, WW3D_FORMAT_A8R8G8B8);
  id<MTLTexture> mtl = ((MacOSTexture *)it)->GetMTLTexture();
  [mtl replaceRegion:MTLRegionMake2D(0, 0, w, height)
         mipmapLevel:0
           withBytes:rgba.data()
         bytesPerRow:w * 4];
  m_TextureCache[filename] = it;
  return it;
}

void MacOSRenderDevice::GetDisplaySize(int &w, int &h) {
  w = (int)m_ScreenSize[0];
  h = (int)m_ScreenSize[1];
}

void MacOSRenderDevice::SetScissorRect(int x, int y, int w, int h) {
  if (m_CurrentEncoder) {
    MTLScissorRect rect = {(NSUInteger)x, (NSUInteger)y, (NSUInteger)w,
                           (NSUInteger)h};
    // Ensure the rect is within the drawable bounds
    if (rect.x + rect.width > (NSUInteger)m_MetalLayer.drawableSize.width)
      rect.width = (NSUInteger)m_MetalLayer.drawableSize.width - rect.x;
    if (rect.y + rect.height > (NSUInteger)m_MetalLayer.drawableSize.height)
      rect.height = (NSUInteger)m_MetalLayer.drawableSize.height - rect.y;

    // Scissor rect must be at least 1x1 in Metal to be valid
    if (rect.width > 0 && rect.height > 0) {
      [m_CurrentEncoder setScissorRect:rect];
    }
  }
}

void MacOSRenderDevice::EnableScissor(bool enable) {
  if (!enable && m_CurrentEncoder) {
    // Reset to full screen
    MTLScissorRect rect = {0, 0, (NSUInteger)m_MetalLayer.drawableSize.width,
                           (NSUInteger)m_MetalLayer.drawableSize.height};
    [m_CurrentEncoder setScissorRect:rect];
  }
}
void MacOSRenderDevice::DrawTexturedRect(float x, float y, float w, float h,
                                         float u1, float v1, float u2, float v2,
                                         unsigned int c) {
  printf("DEBUG: DrawTexturedRect (%.1f, %.1f) size(%.1f, %.1f) uv1(%.3f, "
         "%.3f) uv2(%.3f, %.3f) color=%08X\n",
         x, y, w, h, u1, v1, u2, v2, c);
  fflush(stdout);
  Vertex vs[4] = {{x, y, 0, c, u1, v1},
                  {x + w, y, 0, c, u2, v1},
                  {x, y + h, 0, c, u1, v2},
                  {x + w, y + h, 0, c, u2, v2}};
  static unsigned short ids[6] = {0, 1, 2, 1, 3, 2};
  [m_CurrentEncoder setRenderPipelineState:m_UIPS];
  if (m_Textures[0]) {
    printf("DEBUG: DrawTexturedRect with texture %p\n", m_Textures[0]);
    [m_CurrentEncoder
        setFragmentTexture:((MacOSTexture *)m_Textures[0])->GetMTLTexture()
                   atIndex:0];
  } else {
    printf("DEBUG: DrawTexturedRect with WHITE texture\n");
    [m_CurrentEncoder setFragmentTexture:m_WhiteTexture atIndex:0];
  }
  m_Uniforms.useProjection =
      0; // Inline shader: else branch does 2D screen-space conversion
  m_Uniforms.screenSize[0] = m_ScreenSize[0];
  m_Uniforms.screenSize[1] = m_ScreenSize[1];
  [m_CurrentEncoder setVertexBytes:vs length:sizeof(Vertex) * 4 atIndex:0];
  [m_CurrentEncoder setVertexBytes:&m_Uniforms
                            length:sizeof(m_Uniforms)
                           atIndex:1];
  [m_CurrentEncoder
      drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                 indexCount:6
                  indexType:MTLIndexTypeUInt16
                indexBuffer:[m_Device
                                newBufferWithBytes:ids
                                            length:12
                                           options:MTLResourceStorageModeShared]
          indexBufferOffset:0];
}

void MacOSRenderDevice::DrawLine(float x1, float y1, float x2, float y2,
                                 float w, unsigned int c) {}
static MacOSRenderDevice g_RD;
extern "C" IRenderDevice *GetRenderDevice() { return &g_RD; }
extern "C" void MacOS_InitRenderer(void *h) {
  GetRenderDevice()->Initialize(h);
}
extern "C" void MacOS_Render() {}
