#pragma once

#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#ifdef __OBJC__
#include <Metal/Metal.h>
typedef id<MTLBuffer> MTLBufferPtr;
#else
typedef void *MTLBufferPtr;
#endif

/**
 * Metal implementation of a Vertex Buffer.
 */
class MacOSVertexBufferClass : public VertexBufferClass {
  W3DMPO_GLUE(MacOSVertexBufferClass)
public:
  MacOSVertexBufferClass(unsigned FVF, unsigned short VertexCount,
                         unsigned vertex_size = 0);
  virtual ~MacOSVertexBufferClass();

  MTLBufferPtr Get_Metal_Buffer() { return m_Buffer; }

protected:
  MTLBufferPtr m_Buffer;
  void *m_LockedData;

  // Internal lock/unlock for WriteLockClass and AppendLockClass
  void *Lock(unsigned offset, unsigned size, int flags);
  void Unlock();

  friend class VertexBufferClass::WriteLockClass;
  friend class VertexBufferClass::AppendLockClass;
};

/**
 * Metal implementation of an Index Buffer.
 */
class MacOSIndexBufferClass : public IndexBufferClass {
  W3DMPO_GLUE(MacOSIndexBufferClass)
public:
  MacOSIndexBufferClass(unsigned count, bool is32bit = false);
  virtual ~MacOSIndexBufferClass();

  MTLBufferPtr Get_Metal_Buffer() { return m_Buffer; }

public:
  void *Lock(unsigned offset, unsigned size, int flags);
  void Unlock();

protected:
  MTLBufferPtr m_Buffer;

  void *m_LockedData;

  friend class IndexBufferClass::WriteLockClass;
};
