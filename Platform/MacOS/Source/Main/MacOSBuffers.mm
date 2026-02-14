#import "MacOSBuffers.h"
#import "MacOSRenderDevice_Internal.h"
#include "dx8wrapper.h"

// VertexBufferClass implementations
MacOSVertexBufferClass::MacOSVertexBufferClass(unsigned FVF,
                                               unsigned short VertexCount,
                                               unsigned vertex_size)
    : VertexBufferClass(BUFFER_TYPE_METAL, FVF, VertexCount, vertex_size),
      m_Buffer(nil), m_LockedData(nullptr) {
  unsigned size = FVF_Info().Get_FVF_Size() * VertexCount;
  id<MTLDevice> device = MacOSRenderDevice::GetMTLDevice();
  m_Buffer = [device newBufferWithLength:size
                                 options:MTLResourceStorageModeShared];
}

MacOSVertexBufferClass::~MacOSVertexBufferClass() { m_Buffer = nil; }

void *MacOSVertexBufferClass::Lock(unsigned offset, unsigned size, int flags) {
  if (m_LockedData)
    return m_LockedData;
  m_LockedData = (uint8_t *)[m_Buffer contents] + offset;
  return m_LockedData;
}

void MacOSVertexBufferClass::Unlock() { m_LockedData = nullptr; }

// IndexBufferClass implementations
MacOSIndexBufferClass::MacOSIndexBufferClass(unsigned count, bool is32bit)
    : IndexBufferClass(BUFFER_TYPE_METAL, count, is32bit), m_Buffer(nil),
      m_LockedData(nullptr) {
  unsigned size = count * (is32bit ? 4 : 2);
  id<MTLDevice> device = MacOSRenderDevice::GetMTLDevice();
  m_Buffer = [device newBufferWithLength:size
                                 options:MTLResourceStorageModeShared];
}

MacOSIndexBufferClass::~MacOSIndexBufferClass() { m_Buffer = nil; }

void *MacOSIndexBufferClass::Lock(unsigned offset, unsigned size, int flags) {
  if (m_LockedData)
    return m_LockedData;
  m_LockedData = (uint8_t *)[m_Buffer contents] + offset;
  return m_LockedData;
}

void MacOSIndexBufferClass::Unlock() { m_LockedData = nullptr; }
