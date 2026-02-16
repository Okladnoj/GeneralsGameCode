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
     "    float4 diffuse [[attribute(1)]];\n"
     "    float2 texCoord [[attribute(2)]];\n"
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
     "    out.position.x = (in.position.x / u.screenSize.x) * 2.0 - 1.0;\n"
     "    out.position.y = 1.0 - (in.position.y / u.screenSize.y) * 2.0;\n"
     "    out.position.z = in.position.z / 1000.0;\n"
     "    out.position.w = 1.0;\n"
     "    out.color = in.diffuse;\n"
     "    out.texCoord = in.texCoord;\n"
     "    return out;\n"
     "}\n"
     "\n"
     "fragment float4 fragment_main(VertexOut in [[stage_in]], "
     "texture2d<float> tex [[texture(0)]]) {\n"
     "    constexpr sampler s(address::clamp_to_edge, filter::linear);\n"
     "    return tex.sample(s, in.texCoord) * in.color;\n"
     "}\n";

#pragma pack(push, 1)
struct TGAHeaderPacked {
  unsigned char idlength;
  unsigned char colourmaptype;
  unsigned char datatypecode;
  unsigned short colourmaporigin;
  unsigned short colourmaplength;
  unsigned char colourmapdepth;
  unsigned short x_origin;
  unsigned short y_origin;
  unsigned short width;
  unsigned short height;
  unsigned char bitsperpixel;
  unsigned char imagedescriptor;
};
#pragma pack(pop)

id<MTLDevice> MacOSRenderDevice::m_StaticDevice = nil;
id<MTLDevice> MacOSRenderDevice::GetMTLDevice() { return m_StaticDevice; }

MacOSRenderDevice::MacOSRenderDevice()
    : m_Device(nil), m_CommandQueue(nil), m_CurrentEncoder(nil),
      m_CurrentCommandBuffer(nil), m_MetalLayer(nil), m_AlphaPS(nil),
      m_CurrentColor(0), m_CurrentDrawable(nil), m_WhiteTexture(nil) {
  m_ScreenSize[0] = 800;
  m_ScreenSize[1] = 600;
  for (int i = 0; i < 8; ++i)
    m_Textures[i] = nullptr;
}

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
  if (window) {
    window.contentView.layer = m_MetalLayer;
    window.contentView.wantsLayer = YES;
    CGSize vs = window.contentView.bounds.size;
    m_MetalLayer.drawableSize =
        CGSizeMake(vs.width * window.backingScaleFactor,
                   vs.height * window.backingScaleFactor);
    m_ScreenSize[0] = (float)vs.width;
    m_ScreenSize[1] = (float)vs.height;
  }

  NSError *err = nil;
  id<MTLLibrary> lib = [m_Device newLibraryWithSource:shaderSource
                                              options:nil
                                                error:&err];
  id<MTLFunction> vf = [lib newFunctionWithName:@"vertex_main"],
                  ff = [lib newFunctionWithName:@"fragment_main"];
  MTLRenderPipelineDescriptor *psd = [[MTLRenderPipelineDescriptor alloc] init];
  psd.vertexFunction = vf;
  psd.fragmentFunction = ff;
  psd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
  psd.colorAttachments[0].blendingEnabled = YES;
  psd.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
  psd.colorAttachments[0].destinationRGBBlendFactor =
      MTLBlendFactorOneMinusSourceAlpha;

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
  m_AlphaPS = [m_Device newRenderPipelineStateWithDescriptor:psd error:&err];

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
void MacOSRenderDevice::Clear(unsigned int color) { m_CurrentColor = color; }
void MacOSRenderDevice::Present() {
  if (m_CurrentCommandBuffer) {
    if (m_CurrentDrawable)
      [m_CurrentCommandBuffer presentDrawable:m_CurrentDrawable];
    [m_CurrentCommandBuffer commit];
    m_CurrentCommandBuffer = nil;
    m_CurrentDrawable = nil;
  }
}

void MacOSRenderDevice::BeginScene() {
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
  pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, 1.0);
  m_CurrentEncoder =
      [m_CurrentCommandBuffer renderCommandEncoderWithDescriptor:pass];
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
void MacOSRenderDevice::SetViewport(const Viewport *vp) {}
void MacOSRenderDevice::SetTransform(TransformType t, const float *m) {}
ITexture *MacOSRenderDevice::CreateTexture(unsigned int w, unsigned int h,
                                           unsigned int mips,
                                           unsigned int usage, WW3DFormat fmt) {
  id<MTLTexture> tex = [m_Device
      newTextureWithDescriptor:
          [MTLTextureDescriptor
              texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                           width:w
                                          height:h
                                       mipmapped:(mips > 1)]];
  return new MacOSTexture(w, h, mips, usage, fmt, tex);
}

void MacOSRenderDevice::SetTexture(unsigned int stage, ITexture *tex) {
  m_Textures[stage] = tex;
}
void MacOSRenderDevice::SetRenderState(RenderState s, unsigned int v) {}
void MacOSRenderDevice::SetMeshShader(unsigned int b) {}

void MacOSRenderDevice::DrawIndexed(PrimitiveType type, int vc, int ic,
                                    const Vertex *vd,
                                    const unsigned short *id) {
  if (!m_CurrentEncoder)
    return;
  [m_CurrentEncoder setRenderPipelineState:m_AlphaPS];
  Uniforms u;
  memset(&u, 0, sizeof(u));
  u.world = u.view = u.projection = matrix_identity_float4x4;
  u.screenSize[0] = m_ScreenSize[0];
  u.screenSize[1] = m_ScreenSize[1];
  [m_CurrentEncoder setVertexBytes:vd length:sizeof(Vertex) * vc atIndex:0];
  [m_CurrentEncoder setVertexBytes:&u length:sizeof(u) atIndex:1];
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
                                    VertexBufferClass *vb) {}
void MacOSRenderDevice::DrawDebugText(const char *t, float x, float y,
                                      unsigned int c) {}
void MacOSRenderDevice::DrawFillRect(float x, float y, float w, float h,
                                     unsigned int c) {
  DrawTexturedRect(x, y, w, h, 0, 0, 1, 1, c);
}

ITexture *MacOSRenderDevice::CreateTextureFromFile(const char *filename) {
  if (!filename || !filename[0])
    return nullptr;
  if (m_TextureCache.count(filename))
    return m_TextureCache[filename];

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
    FILE *f = fopen(p, "rb");
    if (f) {
      fseek(f, 0, SEEK_END);
      size_t s = ftell(f);
      fseek(f, 0, SEEK_SET);
      data.resize(s);
      fread(data.data(), 1, s, f);
      fclose(f);
      found = true;
    } else if (TheFileSystem) {
      for (char *c = p; *c; c++)
        if (*c == '/')
          *c = '\\';
      File *gf = TheFileSystem->openFile(p);
      if (gf) {
        data.resize(gf->size());
        gf->read(data.data(), gf->size());
        gf->close();
        found = true;
      }
    }
    if (found)
      break;
  }
  if (!found)
    return nullptr;

  TGAHeaderPacked *h = (TGAHeaderPacked *)data.data();
  int w = h->width, height = h->height, bpp = h->bitsperpixel,
      type = h->datatypecode;
  if (w <= 0 || height <= 0 || (bpp != 24 && bpp != 32))
    return nullptr;

  unsigned char *src_ptr = (unsigned char *)(h + 1) + h->idlength;
  std::vector<unsigned char> decomp;
  int bv = bpp / 8;
  if (type == 10 || type == 11) { // RLE
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
  bool flipY = !(h->imagedescriptor & 0x20);
  for (int y = 0; y < height; y++) {
    int sy = flipY ? (height - 1 - y) : y;
    unsigned char *srow = src_ptr + (sy * w * bv);
    unsigned char *drow = rgba.data() + (y * w * 4);
    for (int x = 0; x < w; x++) {
      drow[x * 4 + 0] = srow[0];
      drow[x * 4 + 1] = srow[1];
      drow[x * 4 + 2] = srow[2];
      drow[x * 4 + 3] = (bv == 4) ? srow[3] : 255;
      srow += bv;
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
void MacOSRenderDevice::DrawTexturedRect(float x, float y, float w, float h,
                                         float u1, float v1, float u2, float v2,
                                         unsigned int c) {
  static int drawCount = 0;
  if (drawCount++ % 1000 == 0) {
    printf("DEBUG: DrawTexturedRect pulse (count=%d)\n", drawCount);
    fflush(stdout);
  }
  Vertex vs[4] = {{x, y, 0, c, u1, v1},
                  {x + w, y, 0, c, u2, v1},
                  {x, y + h, 0, c, u1, v2},
                  {x + w, y + h, 0, c, u2, v2}};
  static unsigned short ids[6] = {0, 1, 2, 1, 3, 2};
  [m_CurrentEncoder setRenderPipelineState:m_AlphaPS];
  if (m_Textures[0])
    [m_CurrentEncoder
        setFragmentTexture:((MacOSTexture *)m_Textures[0])->GetMTLTexture()
                   atIndex:0];
  else
    [m_CurrentEncoder setFragmentTexture:m_WhiteTexture atIndex:0];
  Uniforms u;
  memset(&u, 0, sizeof(u));
  u.world = u.view = u.projection = matrix_identity_float4x4;
  u.screenSize[0] = m_ScreenSize[0];
  u.screenSize[1] = m_ScreenSize[1];
  [m_CurrentEncoder setVertexBytes:vs length:24 * 4 atIndex:0];
  [m_CurrentEncoder setVertexBytes:&u length:sizeof(u) atIndex:1];
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
