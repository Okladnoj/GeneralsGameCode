#include "dx8vertexbuffer.h"
#include "always.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include <Utility/win_compat.h>

#ifdef __APPLE__
#include "MacOSBuffers.h"
#include <d3d8.h>
#endif

#define DEFAULT_VB_SIZE 5000

VertexBufferClass::VertexBufferClass(unsigned type_, unsigned FVF_,
                                     unsigned short VertexCount_,
                                     unsigned int vertex_size_)
    : type(type_), VertexCount(VertexCount_), engine_refs(0) {
  fvf_info = W3DNEW FVFInfoClass(FVF_, vertex_size_);
}

VertexBufferClass::~VertexBufferClass() { delete fvf_info; }

void VertexBufferClass::Add_Engine_Ref() const {
  const_cast<VertexBufferClass *>(this)->engine_refs++;
}

VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass *vb, int)
    : VertexBufferLockClass(vb) {
  Vertices = nullptr;
}
VertexBufferClass::WriteLockClass::~WriteLockClass() {}
VertexBufferClass::AppendLockClass::AppendLockClass(VertexBufferClass *vb,
                                                    unsigned, unsigned)
    : VertexBufferLockClass(vb) {
  Vertices = nullptr;
}
VertexBufferClass::AppendLockClass::~AppendLockClass() {}

void VertexBufferClass::Release_Engine_Ref() const {
  WWASSERT(engine_refs > 0);
  VertexBufferClass *_this = const_cast<VertexBufferClass *>(this);
  _this->engine_refs--;
  if (_this->engine_refs == 0) {
    delete _this;
  }
}

DX8VertexBufferClass::DX8VertexBufferClass(unsigned FVF,
                                           unsigned short VertexCount,
                                           UsageType usage,
                                           unsigned vertex_size)
    : VertexBufferClass(BUFFER_TYPE_DX8, FVF, VertexCount, vertex_size),
      VertexBuffer(0) {
  Create_Vertex_Buffer(usage);
}

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage) {
  DWORD dwUsage = D3DUSAGE_WRITEONLY;
  if (usage & USAGE_DYNAMIC) {
    dwUsage |= D3DUSAGE_DYNAMIC;
  }

#ifdef __APPLE__
  VertexBuffer = (IDirect3DVertexBuffer8 *)new MacOSVertexBufferClass(
      fvf_info->Get_FVF(), VertexCount, fvf_info->Get_FVF_Size());
#else
  D3DPOOL pool = D3DPOOL_MANAGED;
  if (dwUsage & D3DUSAGE_DYNAMIC) {
    pool = D3DPOOL_DEFAULT;
  }
  HRESULT ret = DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
      VertexCount * fvf_info->Get_FVF_Size(), dwUsage, fvf_info->Get_FVF(),
      pool, &VertexBuffer);
  DX8_ErrorCode(ret);
#endif
}

DX8VertexBufferClass::~DX8VertexBufferClass() {
  if (VertexBuffer) {
    VertexBuffer->Release();
  }
}

SortingVertexBufferClass::SortingVertexBufferClass(unsigned short count)
    : VertexBufferClass(BUFFER_TYPE_SORTING, dynamic_fvf_type, count) {
  VertexBuffer = new VertexFormatXYZNDUV2[count];
}

SortingVertexBufferClass::~SortingVertexBufferClass() { delete[] VertexBuffer; }

DynamicVBAccessClass::DynamicVBAccessClass(unsigned type_, unsigned fvf_,
                                           unsigned short count)
    : FVFInfo(*(new FVFInfoClass(fvf_))), Type(type_), VertexCount(count),
      VertexBufferOffset(0), VertexBuffer(0) {}

DynamicVBAccessClass::~DynamicVBAccessClass() {
  delete const_cast<FVFInfoClass *>(&FVFInfo);
}
void DynamicVBAccessClass::_Deinit() {}
void DynamicVBAccessClass::_Reset(bool) {}
unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count() {
  return DEFAULT_VB_SIZE;
}

DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass *va)
    : DynamicVBAccess(va), Vertices(0) {}
DynamicVBAccessClass::WriteLockClass::~WriteLockClass() {}
