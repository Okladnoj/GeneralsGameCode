#ifndef IRENDERDEVICE_H
#define IRENDERDEVICE_H

#include <memory>
#include <vector>

// Forward declarations of W3D classes
class IndexBufferClass;
class VertexBufferClass;
class RectClass;
class Vector3;

#include "ww3dformat.h"

/** Locked rectangle data structure */
struct LockedRect {
  int Pitch;
  void *pBits;
};

/** Information about a certain Mip level of a texture. */
struct LevelInfo {
  WW3DFormat Format;
  unsigned int Width;
  unsigned int Height;
  unsigned int Depth;
};

enum ResourceLocation {
  RESOURCE_LOCATION_VIDEOMEMORY = 0,
  RESOURCE_LOCATION_SYSTEMMEMORY = 1
};

enum TransformType {
  TRANSFORM_TYPE_WORLD = 1,
  TRANSFORM_TYPE_VIEW = 2,
  TRANSFORM_TYPE_PROJECTION = 3
};

enum PrimitiveType {
  PRIMITIVE_TYPE_TRIANGLELIST = 4,
  PRIMITIVE_TYPE_TRIANGLESTRIP = 5,
  PRIMITIVE_TYPE_TRIANGLEFAN = 6,
  PRIMITIVE_TYPE_LINELIST = 2
};

enum RenderState {
  RENDER_STATE_ZENABLE = 7,
  RENDER_STATE_FILLMODE = 8,
  RENDER_STATE_SHADEMODE = 9,
  RENDER_STATE_ZWRITEENABLE = 10,
  RENDER_STATE_ALPHATESTENABLE = 15,
  RENDER_STATE_SRCBLEND = 19,
  RENDER_STATE_DESTBLEND = 20,
  RENDER_STATE_CULLMODE = 22,
  RENDER_STATE_ZFUNC = 23,
  RENDER_STATE_ALPHAREF = 24,
  RENDER_STATE_ALPHAFUNC = 25,
  RENDER_STATE_ALPHABLENDENABLE = 27,
  RENDER_STATE_FOGENABLE = 28,
  RENDER_STATE_FOGCOLOR = 34,
  RENDER_STATE_FOGTABLEMODE = 35,
  RENDER_STATE_FOGSTART = 36,
  RENDER_STATE_FOGEND = 37,
  RENDER_STATE_FOGDENSITY = 38
};

struct Viewport {
  unsigned int x, y, w, h;
  float min_depth, max_depth;
};

struct Vertex {
  float x, y, z;
  unsigned int diffuse;
  float u, v;
};

class ITexture {
public:
  virtual ~ITexture() {}
  virtual bool LockRect(unsigned int level, LockedRect *pLockedRect,
                        const RectClass *pRect = nullptr,
                        unsigned int flags = 0) = 0;
  virtual bool UnlockRect(unsigned int level) = 0;
  virtual bool GetLevelDesc(unsigned int level, LevelInfo *outInfo) = 0;
};

class IRenderDevice {
public:
  virtual ~IRenderDevice() {}

  virtual bool Initialize(void *windowHandle) = 0;
  virtual void Shutdown() = 0;

  virtual void Clear(unsigned int color) = 0;
  virtual void BeginScene() = 0;
  virtual void EndScene() = 0;
  virtual void Present() = 0;

  virtual void SetViewport(const Viewport *pViewport) = 0;
  virtual void SetTransform(TransformType type, const float *matrix) = 0;

  virtual ITexture *CreateTexture(unsigned int width, unsigned int height,
                                  unsigned int mips, unsigned int usage,
                                  WW3DFormat format) = 0;

  virtual void SetTexture(unsigned int stage, ITexture *pTexture) = 0;
  virtual void SetRenderState(RenderState state, unsigned int value) = 0;

  virtual void SetMeshShader(unsigned int shaderBits) = 0;

  virtual void DrawIndexed(PrimitiveType type, int vertexCount, int indexCount,
                           const Vertex *vertexData,
                           const unsigned short *indexData) = 0;

  virtual void DrawIndexed(PrimitiveType type, int indexCount,
                           IndexBufferClass *ib, VertexBufferClass *vb) = 0;

  virtual void DrawDebugText(const char *text, float x, float y,
                             unsigned int color) = 0;
  virtual void DrawFillRect(float x, float y, float w, float h,
                            unsigned int color) = 0;
};

extern "C" IRenderDevice *GetRenderDevice();

#endif
