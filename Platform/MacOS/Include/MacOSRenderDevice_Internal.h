#import <map>
#import <string>

#import "IRenderDevice.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>
#import <vector>

// Forward declaration
class MacOSTexture;

// Shader Settings Bit Positions (from shader.h)
enum ShaderSettings {
  SETTINGS_DEPTHCOMPARE = 0,
  SETTINGS_DEPTHMASK = 3,
  SETTINGS_COLORMASK = 4,
  SETTINGS_DSTBLEND = 5,
  SETTINGS_FOG = 8,
  SETTINGS_PRIGRADIENT = 10,
  SETTINGS_SECGRADIENT = 13,
  SETTINGS_SRCBLEND = 14,
  SETTINGS_TEXTURING = 16,
  SETTINGS_ALPHATEST = 18,
  SETTINGS_POSTDETAILCOLOR = 20,
  SETTINGS_POSTDETAILALPHA = 24,
};

struct Uniforms {
  matrix_float4x4 world;
  matrix_float4x4 view;
  matrix_float4x4 projection;
  float screenSize[2];
  int useProjection; // 0=NDC, 1=3D, 2=2D (Screen)
  unsigned int shaderSettings;
};

class MacOSRenderDevice : public IRenderDevice {
public:
  MacOSRenderDevice();
  static id<MTLDevice> GetMTLDevice();

  bool Initialize(void *windowHandle) override;
  void Shutdown() override;
  void Clear(unsigned int color) override;
  void BeginScene() override;
  void EndScene() override;
  void Present() override;
  void SetViewport(const Viewport *pViewport) override;
  void SetTransform(TransformType type, const float *matrix) override;
  ITexture *CreateTexture(unsigned int width, unsigned int height,
                          unsigned int mips, unsigned int usage,
                          WW3DFormat format) override;
  ITexture *CreateTextureFromFile(const char *filename) override;
  void SetTexture(unsigned int stage, ITexture *pTexture) override;
  void SetRenderState(RenderState state, unsigned int value) override;
  void SetMeshShader(unsigned int shaderBits) override;
  void DrawIndexed(PrimitiveType type, int vertexCount, int indexCount,
                   const Vertex *vertexData,
                   const unsigned short *indexData) override;
  void DrawIndexed(PrimitiveType type, int indexCount, IndexBufferClass *ib,
                   VertexBufferClass *vb) override;
  void DrawDebugText(const char *text, float x, float y,
                     unsigned int color) override;
  void DrawFillRect(float x, float y, float w, float h,
                    unsigned int color) override;
  void DrawTexturedRect(float x, float y, float w, float h, float u1, float v1,
                        float u2, float v2, unsigned int color) override;
  void DrawLine(float x1, float y1, float x2, float y2, float width,
                unsigned int color) override;
  void GetDisplaySize(int &w, int &h) override;

private:
  static id<MTLDevice> m_StaticDevice;
  id<MTLDevice> m_Device;
  id<MTLCommandQueue> m_CommandQueue;
  id<MTLRenderCommandEncoder> m_CurrentEncoder;
  id<MTLCommandBuffer> m_CurrentCommandBuffer;
  CAMetalLayer *m_MetalLayer;

  id<MTLRenderPipelineState> m_OpaquePS;
  id<MTLRenderPipelineState> m_AlphaPS;
  id<MTLSamplerState> m_LinearSampler;
  id<MTLSamplerState> m_PointSampler;

  bool m_Is2DMode;
  float m_ScreenSize[2];
  ITexture *m_Textures[8];
  unsigned int m_CurrentColor;
  id<CAMetalDrawable> m_CurrentDrawable;
  void *m_HWND;

  Uniforms m_Uniforms;
  void updateUniforms();

  id<MTLTexture> m_WhiteTexture;
  std::map<std::string, ITexture *> m_TextureCache;
};
