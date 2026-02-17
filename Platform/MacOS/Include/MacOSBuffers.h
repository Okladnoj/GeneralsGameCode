#pragma once

#include "always.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include <Utility/win_compat.h>
#include <d3d8.h>

/**
 * Metal implementation of a Vertex Buffer.
 * Inherits from IDirect3DVertexBuffer8 to be binary compatible with DX8Wrapper
 * calls.
 */
class MacOSVertexBufferClass : public VertexBufferClass,
                               public IDirect3DVertexBuffer8 {
  W3DMPO_GLUE(MacOSVertexBufferClass)
public:
  MacOSVertexBufferClass(unsigned FVF, unsigned short VertexCount,
                         unsigned vertex_size = 0);
  virtual ~MacOSVertexBufferClass();

  // IUnknown methods
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override;
  STDMETHOD_(ULONG, AddRef)(void) override;
  STDMETHOD_(ULONG, Release)(void) override;

  // IDirect3DResource8 methods
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) override;
  STDMETHOD(SetPrivateData)
  (REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) override;
  STDMETHOD(GetPrivateData)
  (REFGUID refguid, void *pData, DWORD *pSizeOfData) override;
  STDMETHOD(FreePrivateData)(REFGUID refguid) override;
  STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew) override;
  STDMETHOD_(DWORD, GetPriority)(void) override;
  STDMETHOD_(void, PreLoad)(void) override;
  STDMETHOD_(D3DRESOURCETYPE, GetType)(void) override;

  // IDirect3DVertexBuffer8 methods
  STDMETHOD(Lock)
  (THIS_ UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags)
      override;
  STDMETHOD(Unlock)(THIS) override;
  STDMETHOD(GetDesc)(D3DVERTEXBUFFER_DESC *pDesc) override;
  virtual void *Get_Metal_Buffer();

protected:
  uint8_t *m_data;
  unsigned int m_fvf;
  unsigned int m_count;
  unsigned int m_vertexSize;
  int m_refCount;
  void *m_MetalBuffer;
};

/**
 * Metal implementation of an Index Buffer.
 */
class MacOSIndexBufferClass : public IndexBufferClass,
                              public IDirect3DIndexBuffer8 {
  W3DMPO_GLUE(MacOSIndexBufferClass)
public:
  MacOSIndexBufferClass(unsigned count, bool is32bit = false);
  virtual ~MacOSIndexBufferClass();

  // IUnknown methods
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override;
  STDMETHOD_(ULONG, AddRef)(void) override;
  STDMETHOD_(ULONG, Release)(void) override;

  // IDirect3DResource8 methods
  STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice) override;
  STDMETHOD(SetPrivateData)
  (REFGUID refguid, CONST void *pData, DWORD SizeOfData, DWORD Flags) override;
  STDMETHOD(GetPrivateData)
  (REFGUID refguid, void *pData, DWORD *pSizeOfData) override;
  STDMETHOD(FreePrivateData)(REFGUID refguid) override;
  STDMETHOD_(DWORD, SetPriority)(DWORD PriorityNew) override;
  STDMETHOD_(DWORD, GetPriority)(void) override;
  STDMETHOD_(void, PreLoad)(void) override;
  STDMETHOD_(D3DRESOURCETYPE, GetType)(void) override;

  // IDirect3DIndexBuffer8 methods
  STDMETHOD(Lock)
  (THIS_ UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags)
      override;
  STDMETHOD(Unlock)(THIS) override;
  STDMETHOD(GetDesc)(D3DINDEXBUFFER_DESC *pDesc) override;
  virtual void *Get_Metal_Buffer();

protected:
  uint8_t *m_data;
  unsigned int m_count;
  bool m_is32bit;
  int m_refCount;
  void *m_MetalBuffer;
};
