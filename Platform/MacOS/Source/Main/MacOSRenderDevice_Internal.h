#import "../../Include/IRenderDevice.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <vector>

// Forward declaration
class MacOSTexture;

struct Uniforms {
  float world[16];
  float view[16];
  float projection[16];
  float screenSize[2];
  int useProjection;
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
};
