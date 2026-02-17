#import "MacOSBuffers.h"
#include "MacOSRenderDevice_Internal.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include <Utility/win_compat.h>
#include <cstdio>

// MacOSVertexBufferClass implementation

MacOSVertexBufferClass::MacOSVertexBufferClass(unsigned int fvf,
                                               unsigned short count,
                                               unsigned int size)
    : VertexBufferClass(BUFFER_TYPE_DX8, fvf, count, size), m_fvf(fvf),
      m_count(count), m_vertexSize(size), m_refCount(1),
      m_MetalBuffer(nullptr) {
  m_data = new uint8_t[count * size];
}

MacOSVertexBufferClass::~MacOSVertexBufferClass() {
  delete[] m_data;
  if (m_MetalBuffer) {
    id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)m_MetalBuffer;
    buf = nil;
  }
}

STDMETHODIMP MacOSVertexBufferClass::QueryInterface(REFIID riid,
                                                    void **ppvObj) {
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MacOSVertexBufferClass::AddRef() { return ++m_refCount; }

STDMETHODIMP_(ULONG) MacOSVertexBufferClass::Release() {
  ULONG res = --m_refCount;
  if (res == 0)
    delete this;
  return res;
}

STDMETHODIMP MacOSVertexBufferClass::GetDevice(IDirect3DDevice8 **ppDevice) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSVertexBufferClass::SetPrivateData(REFGUID guid,
                                                    const void *pData,
                                                    DWORD SizeOfData,
                                                    DWORD Flags) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSVertexBufferClass::GetPrivateData(REFGUID guid, void *pData,
                                                    DWORD *pSizeOfData) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSVertexBufferClass::FreePrivateData(REFGUID guid) {
  return E_NOTIMPL;
}

STDMETHODIMP_(DWORD) MacOSVertexBufferClass::SetPriority(DWORD PriorityNew) {
  return 0;
}

STDMETHODIMP_(DWORD) MacOSVertexBufferClass::GetPriority() { return 0; }

STDMETHODIMP_(void) MacOSVertexBufferClass::PreLoad() {}

STDMETHODIMP_(D3DRESOURCETYPE) MacOSVertexBufferClass::GetType() {
  return D3DRTYPE_VERTEXBUFFER;
}

STDMETHODIMP MacOSVertexBufferClass::Lock(UINT OffsetToLock, UINT SizeToLock,
                                          BYTE **ppbData, DWORD Flags) {
  if (ppbData) {
    *ppbData = m_data + OffsetToLock;
    return D3D_OK;
  }
  return E_POINTER;
}

STDMETHODIMP MacOSVertexBufferClass::Unlock() {
  if (m_MetalBuffer) {
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MetalBuffer;
    memcpy([buf contents], m_data, m_count * m_vertexSize);
  }
  return D3D_OK;
}

void *MacOSVertexBufferClass::Get_Metal_Buffer() {
  if (!m_MetalBuffer) {
    id<MTLDevice> device = MacOSRenderDevice::GetMTLDevice();
    if (device) {
      id<MTLBuffer> buf =
          [device newBufferWithBytes:m_data
                              length:m_count * m_vertexSize
                             options:MTLResourceStorageModeShared];
      m_MetalBuffer = (__bridge_retained void *)buf;
    }
  }
  return m_MetalBuffer;
}

STDMETHODIMP MacOSVertexBufferClass::GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
  if (pDesc) {
    pDesc->Format = D3DFMT_UNKNOWN;
    pDesc->Type = D3DRTYPE_VERTEXBUFFER;
    pDesc->Usage = 0;
    pDesc->Pool = D3DPOOL_MANAGED;
    pDesc->Size = m_count * m_vertexSize;
    pDesc->FVF = m_fvf;
    return D3D_OK;
  }
  return E_POINTER;
}

// MacOSIndexBufferClass implementation

MacOSIndexBufferClass::MacOSIndexBufferClass(unsigned int count, bool is32bit)
    : IndexBufferClass(BUFFER_TYPE_DX8, count, is32bit), m_count(count),
      m_is32bit(is32bit), m_refCount(1), m_MetalBuffer(nullptr) {
  m_data = new uint8_t[count * (is32bit ? 4 : 2)];
}

MacOSIndexBufferClass::~MacOSIndexBufferClass() {
  delete[] m_data;
  if (m_MetalBuffer) {
    id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)m_MetalBuffer;
    buf = nil;
  }
}

STDMETHODIMP MacOSIndexBufferClass::QueryInterface(REFIID riid, void **ppvObj) {
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MacOSIndexBufferClass::AddRef() { return ++m_refCount; }

STDMETHODIMP_(ULONG) MacOSIndexBufferClass::Release() {
  ULONG res = --m_refCount;
  if (res == 0)
    delete this;
  return res;
}

STDMETHODIMP MacOSIndexBufferClass::GetDevice(IDirect3DDevice8 **ppDevice) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSIndexBufferClass::SetPrivateData(REFGUID guid,
                                                   const void *pData,
                                                   DWORD SizeOfData,
                                                   DWORD Flags) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSIndexBufferClass::GetPrivateData(REFGUID guid, void *pData,
                                                   DWORD *pSizeOfData) {
  return E_NOTIMPL;
}

STDMETHODIMP MacOSIndexBufferClass::FreePrivateData(REFGUID guid) {
  return E_NOTIMPL;
}

STDMETHODIMP_(DWORD) MacOSIndexBufferClass::SetPriority(DWORD PriorityNew) {
  return 0;
}

STDMETHODIMP_(DWORD) MacOSIndexBufferClass::GetPriority() { return 0; }

STDMETHODIMP_(void) MacOSIndexBufferClass::PreLoad() {}

STDMETHODIMP_(D3DRESOURCETYPE) MacOSIndexBufferClass::GetType() {
  return D3DRTYPE_INDEXBUFFER;
}

STDMETHODIMP MacOSIndexBufferClass::Lock(UINT OffsetToLock, UINT SizeToLock,
                                         BYTE **ppbData, DWORD Flags) {
  if (ppbData) {
    *ppbData = m_data + OffsetToLock;
    return D3D_OK;
  }
  return E_POINTER;
}

STDMETHODIMP MacOSIndexBufferClass::Unlock() {
  if (m_MetalBuffer) {
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_MetalBuffer;
    memcpy([buf contents], m_data, m_count * (m_is32bit ? 4 : 2));
  }
  return D3D_OK;
}

void *MacOSIndexBufferClass::Get_Metal_Buffer() {
  if (!m_MetalBuffer) {
    id<MTLDevice> device = MacOSRenderDevice::GetMTLDevice();
    if (device) {
      id<MTLBuffer> buf =
          [device newBufferWithBytes:m_data
                              length:m_count * (m_is32bit ? 4 : 2)options
                                    :MTLResourceStorageModeShared];
      m_MetalBuffer = (__bridge_retained void *)buf;
    }
  }
  return m_MetalBuffer;
}

STDMETHODIMP MacOSIndexBufferClass::GetDesc(D3DINDEXBUFFER_DESC *pDesc) {
  if (pDesc) {
    pDesc->Format = m_is32bit ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    pDesc->Type = D3DRTYPE_INDEXBUFFER;
    pDesc->Usage = 0;
    pDesc->Pool = D3DPOOL_MANAGED;
    pDesc->Size = m_count * (m_is32bit ? 4 : 2);
    return D3D_OK;
  }
  return E_POINTER;
}
