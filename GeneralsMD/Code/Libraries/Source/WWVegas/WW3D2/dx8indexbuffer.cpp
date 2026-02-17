#include "dx8indexbuffer.h"
#include "always.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include <Utility/win_compat.h>

#ifdef __APPLE__
#include "MacOSBuffers.h"
#include <d3d8.h>
#endif

IndexBufferClass::IndexBufferClass(unsigned type_, unsigned short index_count_,
                                   bool is_32bit)
    : type(type_), index_count(index_count_), m_Is32Bit(is_32bit),
      engine_refs(0) {}

IndexBufferClass::~IndexBufferClass() {}

void IndexBufferClass::Add_Engine_Ref() const {
  const_cast<IndexBufferClass *>(this)->engine_refs++;
}

void IndexBufferClass::Release_Engine_Ref() const {
  WWASSERT(engine_refs > 0);
  IndexBufferClass *_this = const_cast<IndexBufferClass *>(this);
  _this->engine_refs--;
  if (_this->engine_refs == 0) {
    delete _this;
  }
}

IndexBufferClass::WriteLockClass::WriteLockClass(IndexBufferClass *ib, int)
    : index_buffer(ib) {
  indices = 0;
}
IndexBufferClass::WriteLockClass::~WriteLockClass() {}
IndexBufferClass::AppendLockClass::AppendLockClass(IndexBufferClass *ib,
                                                   unsigned, unsigned)
    : index_buffer(ib) {
  indices = 0;
}
IndexBufferClass::AppendLockClass::~AppendLockClass() {}

DX8IndexBufferClass::DX8IndexBufferClass(unsigned short index_count,
                                         UsageType usage)
    : IndexBufferClass(BUFFER_TYPE_DX8, index_count, false), index_buffer(0) {
  DWORD dwUsage = D3DUSAGE_WRITEONLY;
  if (usage & USAGE_DYNAMIC) {
    dwUsage |= D3DUSAGE_DYNAMIC;
  }

#ifdef __APPLE__
  index_buffer =
      (IDirect3DIndexBuffer8 *)new MacOSIndexBufferClass(index_count, false);
#else
  D3DPOOL pool =
      (dwUsage & D3DUSAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
  HRESULT ret = DX8Wrapper::_Get_D3D_Device8()->CreateIndexBuffer(
      index_count * 2, dwUsage, D3DFMT_INDEX16, pool, &index_buffer);
  DX8_ErrorCode(ret);
#endif
}

DX8IndexBufferClass::~DX8IndexBufferClass() {
  if (index_buffer) {
    index_buffer->Release();
  }
}

SortingIndexBufferClass::SortingIndexBufferClass(unsigned short count)
    : IndexBufferClass(BUFFER_TYPE_SORTING, count) {
  index_buffer = new unsigned short[count];
}

SortingIndexBufferClass::~SortingIndexBufferClass() { delete[] index_buffer; }

DynamicIBAccessClass::DynamicIBAccessClass(unsigned short type_,
                                           unsigned short count)
    : Type(type_), IndexCount(count), IndexBufferOffset(0), IndexBuffer(0) {}
DynamicIBAccessClass::~DynamicIBAccessClass() {}
void DynamicIBAccessClass::_Deinit() {}
void DynamicIBAccessClass::_Reset(bool) {}
unsigned short DynamicIBAccessClass::Get_Default_Index_Count() { return 5000; }

DynamicIBAccessClass::WriteLockClass::WriteLockClass(DynamicIBAccessClass *ia)
    : DynamicIBAccess(ia), Indices(0) {}
DynamicIBAccessClass::WriteLockClass::~WriteLockClass() {}
