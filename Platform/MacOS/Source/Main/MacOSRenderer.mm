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
#include "TARGA.h"
#include "ddsfile.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "matrix4.h"
#include "vector3.h"
#include "ww3dformat.h"

// Metal Shaders (Extremely simplified to debug hang)
static NSString *const shaderSource = @"\n\
#include <metal_stdlib>\n\
using "
                                      @"namespace metal;\n\
\n\
struct "
                                      @"VertexIn {\n\
    float3 position "
                                      @"[[attribute(0)]];\n\
    float4 "
                                      @"diffuse [[attribute(1)]];\n\
    "
                                      @"float2 texCoord "
                                      @"[[attribute(2)]];\n\
};\n\
\n\
struct "
                                      @"VertexOut {\n\
    float4 position "
                                      @"[[position]];\n\
    float4 color;\n\
 "
                                      @"   float2 "
                                      @"texCoord;\n\
};\n\
\n\
struct Uniforms "
                                      @"{\n\
    float4x4 world;\n\
    "
                                      @"float4x4 view;\n\
    float4x4 "
                                      @"projection;\n\
    float2 "
                                      @"screenSize;\n\
    int "
                                      @"useProjection;\n\
    uint "
                                      @"shaderSettings;\n\
};\n\
\n\
vertex "
                                      @"VertexOut vertex_main(VertexIn in "
                                      @"[[stage_in]],\n\
                      "
                                      @"     constant Uniforms &u "
                                      @"[[buffer(1)]]) {\n\
    VertexOut "
                                      @"out;\n\
    // Force 2D for "
                                      @"debugging\n\
    out.position.x = "
                                      @"(in.position.x / u.screenSize.x) * 2.0 "
                                      @"- 1.0;\n\
    out.position.y = 1.0 - "
                                      @"(in.position.y / u.screenSize.y) * "
                                      @"2.0;\n\
    out.position.z = "
                                      @"in.position.z;\n\
    out.position.w = "
                                      @"1.0;\n\
    out.color = "
                                      @"in.diffuse;\n\
    out.texCoord = "
                                      @"in.texCoord;\n\
    return "
                                      @"out;\n\
}\n\
\n\
fragment float4 "
                                      @"fragment_main(VertexOut in "
                                      @"[[stage_in]]) {\n\
    return "
                                      @"in.color;\n\
}\n\
";

class MacOSTexture : public ITexture {
public:
  MacOSTexture(id<MTLTexture> tex, int w, int h, int mips, WW3DFormat fmt)
      : m_MTLTexture(tex), m_Width(w), m_Height(h), m_MipCount(mips),
        m_Format(fmt), m_LockedLevel(-1) {}

  bool LockRect(unsigned int level, LockedRect *pLockedRect,
                const RectClass *pRect = nullptr,
                unsigned int flags = 0) override {
    if (level >= (unsigned int)m_MipCount || m_LockedLevel != -1)
      return false;

    m_LockedLevel = (int)level;
    int levelWidth = std::max(1, m_Width >> level);
    int levelHeight = std::max(1, m_Height >> level);

    size_t size = 0;
    if (m_Format >= WW3D_FORMAT_DXT1 && m_Format <= WW3D_FORMAT_DXT5) {
      int blocksW = (levelWidth + 3) / 4;
      int blocksH = (levelHeight + 3) / 4;
      int bytesPerBlock = (m_Format == WW3D_FORMAT_DXT1) ? 8 : 16;
      size = (size_t)blocksW * blocksH * bytesPerBlock;
      pLockedRect->Pitch = (int)(blocksW * bytesPerBlock);
    } else {
      pLockedRect->Pitch = (int)(levelWidth * 4);
      size = (size_t)levelHeight * pLockedRect->Pitch;
    }

    m_LockedData.resize(size);
    pLockedRect->pBits = m_LockedData.data();
    return true;
  }

  bool UnlockRect(unsigned int level) override {
    if (m_LockedLevel == -1 || (unsigned int)m_LockedLevel != level)
      return false;

    int levelWidth = std::max(1, m_Width >> level);
    int levelHeight = std::max(1, m_Height >> level);

    MTLRegion region = MTLRegionMake2D(0, 0, levelWidth, levelHeight);

    if (m_Format >= WW3D_FORMAT_DXT1 && m_Format <= WW3D_FORMAT_DXT5) {
      int bytesPerBlock = (m_Format == WW3D_FORMAT_DXT1) ? 8 : 16;
      int blocksW = (levelWidth + 3) / 4;
      [m_MTLTexture replaceRegion:region
                      mipmapLevel:level
                        withBytes:m_LockedData.data()
                      bytesPerRow:blocksW * bytesPerBlock];
    } else {
      [m_MTLTexture replaceRegion:region
                      mipmapLevel:level
                        withBytes:m_LockedData.data()
                      bytesPerRow:levelWidth * 4];
    }

    m_LockedLevel = -1;
    m_LockedData.clear();
    return true;
  }

  bool GetLevelDesc(unsigned int level, LevelInfo *outInfo) override {
    if (level >= (unsigned int)m_MipCount)
      return false;
    outInfo->Width = std::max(1, m_Width >> level);
    outInfo->Height = std::max(1, m_Height >> level);
    outInfo->Depth = 1;
    outInfo->Format = m_Format;
    return true;
  }

  id<MTLTexture> GetMTLTexture() const { return m_MTLTexture; }

private:
  id<MTLTexture> m_MTLTexture;
  int m_Width, m_Height, m_MipCount;
  WW3DFormat m_Format;
  std::vector<uint8_t> m_LockedData;
  int m_LockedLevel;
};

// Implementations for MacOSRenderDevice methods
id<MTLDevice> MacOSRenderDevice::m_StaticDevice = nil;
id<MTLDevice> MacOSRenderDevice::GetMTLDevice() { return m_StaticDevice; }

MacOSRenderDevice::MacOSRenderDevice()
    : m_Device(nil), m_CommandQueue(nil), m_CurrentEncoder(nil),
      m_CurrentCommandBuffer(nil), m_MetalLayer(nil), m_OpaquePS(nil),
      m_AlphaPS(nil), m_LinearSampler(nil), m_PointSampler(nil),
      m_Is2DMode(false), m_CurrentColor(0), m_CurrentDrawable(nil) {
  m_ScreenSize[0] = 800;
  m_ScreenSize[1] = 600;
  for (int i = 0; i < 8; ++i)
    m_Textures[i] = nullptr;
}

bool MacOSRenderDevice::Initialize(void *hwnd) {
  printf("MacOSRenderDevice::Initialize: hwnd = %p\n", hwnd);
  fflush(stdout);
  m_Device = MTLCreateSystemDefaultDevice();
  m_StaticDevice = m_Device;
  if (!m_Device) {
    printf("MacOSRenderDevice::Initialize: Failed to create Metal Device!\n");
    return false;
  }
  m_CommandQueue = [m_Device newCommandQueue];

  NSWindow *window = (__bridge NSWindow *)hwnd;
  if (window && window.contentView == nil) {
    printf(
        "MacOSRenderDevice::Initialize: FAILED - window.contentView is nil!\n");
    fflush(stdout);
    return false;
  }

  m_MetalLayer = [CAMetalLayer layer];
  m_MetalLayer.device = m_Device;
  m_MetalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  m_MetalLayer.framebufferOnly = NO;

  if (window) {
    printf(
        "MacOSRenderDevice::Initialize: Attaching Metal Layer to window...\n");
    fflush(stdout);
    window.contentView.layer = m_MetalLayer;
    window.contentView.wantsLayer = YES;

    CGSize viewSize = window.contentView.bounds.size;
    CGFloat scale = window.backingScaleFactor;
    m_MetalLayer.drawableSize =
        CGSizeMake(viewSize.width * scale, viewSize.height * scale);
    m_MetalLayer.contentsScale = scale;

    printf("MacOSRenderDevice::Initialize: Metal Layer attached, size=%.1f x "
           "%.1f, scale=%.1f\n",
           viewSize.width, viewSize.height, scale);
    fflush(stdout);

    m_ScreenSize[0] = (float)window.contentView.frame.size.width;
    m_ScreenSize[1] = (float)window.contentView.frame.size.height;
  }

  // Load Shaders
  NSError *error = nil;
  id<MTLLibrary> library = [m_Device newLibraryWithSource:shaderSource
                                                  options:nil
                                                    error:&error];
  if (!library) {
    printf("Failed to compile shaders: %s\n",
           [[error localizedDescription] UTF8String]);
    return false;
  }

  id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
  id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];

  MTLRenderPipelineDescriptor *psd = [[MTLRenderPipelineDescriptor alloc] init];
  psd.vertexFunction = vertexFunc;
  psd.fragmentFunction = fragmentFunc;
  psd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

  MTLVertexDescriptor *vd = [[MTLVertexDescriptor alloc] init];
  vd.attributes[0].format = MTLVertexFormatFloat3;
  vd.attributes[0].offset = 0;
  vd.attributes[0].bufferIndex = 0;
  vd.attributes[1].format = MTLVertexFormatUChar4Normalized;
  vd.attributes[1].offset = 12;
  vd.attributes[1].bufferIndex = 0;
  vd.attributes[2].format = MTLVertexFormatFloat2;
  vd.attributes[2].offset = 16;
  vd.attributes[2].bufferIndex = 0;
  vd.layouts[0].stride = 24;
  psd.vertexDescriptor = vd;

  // Create Opaque PS
  psd.colorAttachments[0].blendingEnabled = NO;
  m_OpaquePS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&error];

  // Create Alpha PS
  psd.colorAttachments[0].blendingEnabled = YES;
  psd.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  psd.colorAttachments[0].destinationRGBBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  psd.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
  psd.colorAttachments[0].destinationAlphaBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;
  m_AlphaPS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&error];

  if (!m_AlphaPS) {
    printf("Failed to create pipeline state: %s\n",
           [[error localizedDescription] UTF8String]);
    fflush(stdout);
    return false;
  }

  printf("MacOSRenderDevice::Initialize: SUCCESS\n");
  fflush(stdout);
  return true;
}

void MacOSRenderDevice::Shutdown() {
  m_Device = nil;
  m_CommandQueue = nil;
  m_MetalLayer = nil;
  m_OpaquePS = nil;
  m_AlphaPS = nil;
  m_LinearSampler = nil;
  m_PointSampler = nil;
}

void MacOSRenderDevice::Clear(unsigned int color) { m_CurrentColor = color; }

void MacOSRenderDevice::Present() {
  if (m_CurrentCommandBuffer) {
    if (m_CurrentDrawable) {
      [m_CurrentCommandBuffer presentDrawable:m_CurrentDrawable];
      m_CurrentDrawable = nil;
    }
    [m_CurrentCommandBuffer commit];
    m_CurrentCommandBuffer = nil;
  }
}

void MacOSRenderDevice::BeginScene() {
  fprintf(stderr, "BeginScene [1]: entering\n");
  fflush(stderr);
  if (m_CurrentCommandBuffer)
    return;
  if (!m_CommandQueue)
    return;

  fprintf(stderr, "BeginScene [2]: getting command buffer\n");
  fflush(stderr);
  m_CurrentCommandBuffer = [m_CommandQueue commandBuffer];

  fprintf(stderr, "BeginScene [3]: getting next drawable\n");
  fflush(stderr);
  m_CurrentDrawable = [m_MetalLayer nextDrawable];

  if (!m_CurrentDrawable) {
    fprintf(stderr, "BeginScene [F]: nextDrawable is NIL\n");
    fflush(stderr);
    return;
  }

  fprintf(stderr, "BeginScene [4]: creating pass descriptor\n");
  fflush(stderr);
  MTLRenderPassDescriptor *pass = [[MTLRenderPassDescriptor alloc] init];
  if (!pass) {
    fprintf(stderr, "BeginScene [F]: pass descriptor is NIL\n");
    fflush(stderr);
    return;
  }

  fprintf(stderr, "BeginScene [5]: setting color attachment texture\n");
  fflush(stderr);
  pass.colorAttachments[0].texture = m_CurrentDrawable.texture;
  pass.colorAttachments[0].loadAction = MTLLoadActionClear;
  pass.colorAttachments[0].storeAction = MTLStoreActionStore;

  float r = ((m_CurrentColor >> 16) & 0xFF) / 255.0f;
  float g = ((m_CurrentColor >> 8) & 0xFF) / 255.0f;
  float b = (m_CurrentColor & 0xFF) / 255.0f;
  float a = ((m_CurrentColor >> 24) & 0xFF) / 255.0f;
  pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, a);

  fprintf(stderr, "BeginScene [6]: creating render encoder...\n");
  fflush(stderr);
  @try {
    m_CurrentEncoder =
        [m_CurrentCommandBuffer renderCommandEncoderWithDescriptor:pass];
    fprintf(stderr, "BeginScene [7]: encoder created: %p\n",
            (__bridge void *)m_CurrentEncoder);
    fflush(stderr);
  } @catch (NSException *exception) {
    fprintf(stderr, "BeginScene [X]: EXCEPTION: %s\n",
            [exception.reason UTF8String]);
    fflush(stderr);
  }

  fprintf(stderr, "BeginScene [8]: exiting\n");
  fflush(stderr);
}

void MacOSRenderDevice::EndScene() {
  if (m_CurrentEncoder) {
    [m_CurrentEncoder endEncoding];
    m_CurrentEncoder = nil;
  }
  Present();
}

void MacOSRenderDevice::SetViewport(const Viewport *pViewport) {
  if (m_CurrentEncoder) {
    MTLViewport mvp = {
        (double)pViewport->x,         (double)pViewport->y,
        (double)pViewport->w,         (double)pViewport->h,
        (double)pViewport->min_depth, (double)pViewport->max_depth};
    [m_CurrentEncoder setViewport:mvp];
  }
  m_ScreenSize[0] = (float)pViewport->w;
  m_ScreenSize[1] = (float)pViewport->h;
}

void MacOSRenderDevice::SetTransform(TransformType type, const float *matrix) {}

ITexture *MacOSRenderDevice::CreateTexture(unsigned int width,
                                           unsigned int height,
                                           unsigned int mips,
                                           unsigned int usage,
                                           WW3DFormat format) {
  MTLPixelFormat mtlFormat = MTLPixelFormatBGRA8Unorm;
  MTLTextureDescriptor *td =
      [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mtlFormat
                                                         width:width
                                                        height:height
                                                     mipmapped:(mips > 1)];
  id<MTLTexture> tex = [m_Device newTextureWithDescriptor:td];
  return new MacOSTexture(tex, width, height, mips, format);
}

void MacOSRenderDevice::SetTexture(unsigned int stage, ITexture *pTexture) {
  m_Textures[stage] = pTexture;
}

void MacOSRenderDevice::SetRenderState(RenderState state, unsigned int value) {}
void MacOSRenderDevice::SetMeshShader(unsigned int shaderBits) {}

void MacOSRenderDevice::DrawIndexed(PrimitiveType type, int vertexCount,
                                    int indexCount, const Vertex *vertexData,
                                    const unsigned short *indexData) {
  if (!m_CurrentEncoder)
    return;

  [m_CurrentEncoder setRenderPipelineState:m_AlphaPS];

  Uniforms uniforms;
  memset(&uniforms, 0, sizeof(Uniforms));
  // Identity matrices
  uniforms.world[0] = uniforms.view[5] = uniforms.projection[10] =
      uniforms.world[15] = uniforms.view[15] = uniforms.projection[15] = 1.0f;
  uniforms.world[5] = uniforms.world[10] = uniforms.view[0] =
      uniforms.view[10] = uniforms.projection[0] = uniforms.projection[5] =
          1.0f;

  uniforms.screenSize[0] = m_ScreenSize[0];
  uniforms.screenSize[1] = m_ScreenSize[1];
  uniforms.useProjection = 0; // 2D mode

  [m_CurrentEncoder setVertexBytes:vertexData
                            length:sizeof(Vertex) * vertexCount
                           atIndex:0];
  [m_CurrentEncoder setVertexBytes:&uniforms length:sizeof(Uniforms) atIndex:1];

  MTLPrimitiveType mtlType = MTLPrimitiveTypeTriangle;
  if (type == PRIMITIVE_TYPE_LINELIST)
    mtlType = MTLPrimitiveTypeLine;

  [m_CurrentEncoder
      drawIndexedPrimitives:mtlType
                 indexCount:indexCount
                  indexType:MTLIndexTypeUInt16
                indexBuffer:[m_Device
                                newBufferWithBytes:indexData
                                            length:sizeof(unsigned short) *
                                                   indexCount
                                           options:MTLResourceStorageModeShared]
          indexBufferOffset:0];
}

void MacOSRenderDevice::DrawIndexed(PrimitiveType type, int indexCount,
                                    IndexBufferClass *ib,
                                    VertexBufferClass *vb) {}

void MacOSRenderDevice::DrawDebugText(const char *text, float x, float y,
                                      unsigned int color) {}

void MacOSRenderDevice::DrawFillRect(float x, float y, float w, float h,
                                     unsigned int color) {
  if (!m_CurrentEncoder)
    return;

  Vertex vertices[4];
  vertices[0] = {x, y, 0.0f, color, 0.0f, 0.0f};
  vertices[1] = {x + w, y, 0.0f, color, 1.0f, 0.0f};
  vertices[2] = {x, y + h, 0.0f, color, 0.0f, 1.0f};
  vertices[3] = {x + w, y + h, 0.0f, color, 1.0f, 1.0f};

  static unsigned short indices[6] = {0, 1, 2, 1, 3, 2};
  static id<MTLBuffer> s_indexBuffer = nil;
  if (!s_indexBuffer) {
    s_indexBuffer = [m_Device newBufferWithBytes:indices
                                          length:sizeof(indices)
                                         options:MTLResourceStorageModeShared];
  }

  [m_CurrentEncoder setRenderPipelineState:m_AlphaPS];

  Uniforms uniforms;
  memset(&uniforms, 0, sizeof(Uniforms));
  // Identity matrices
  uniforms.world[0] = uniforms.world[5] = uniforms.world[10] =
      uniforms.world[15] = 1.0f;
  uniforms.view[0] = uniforms.view[5] = uniforms.view[10] = uniforms.view[15] =
      1.0f;
  uniforms.projection[0] = uniforms.projection[5] = uniforms.projection[10] =
      uniforms.projection[15] = 1.0f;

  uniforms.screenSize[0] = m_ScreenSize[0];
  uniforms.screenSize[1] = m_ScreenSize[1];
  uniforms.useProjection = 0; // 2D mode

  [m_CurrentEncoder setVertexBytes:vertices
                            length:sizeof(Vertex) * 4
                           atIndex:0];
  [m_CurrentEncoder setVertexBytes:&uniforms length:sizeof(Uniforms) atIndex:1];

  [m_CurrentEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:6
                                indexType:MTLIndexTypeUInt16
                              indexBuffer:s_indexBuffer
                        indexBufferOffset:0];
}

static MacOSRenderDevice g_MacOSRenderDevice;
extern "C" IRenderDevice *GetRenderDevice() { return &g_MacOSRenderDevice; }

extern "C" void MacOS_InitRenderer(void *windowHandle) {
  GetRenderDevice()->Initialize(windowHandle);
}

extern "C" void MacOS_Render() {}
