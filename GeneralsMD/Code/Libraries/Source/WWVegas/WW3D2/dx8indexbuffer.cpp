#include "dx8indexbuffer.h"
#include "always.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include <Utility/win_compat.h>

#ifdef __APPLE__
#include "MetalIndexBuffer8.h"
#include <d3d8.h>
#endif

#define DEFAULT_IB_SIZE 5000

static bool _DynamicSortingIndexArrayInUse = false;
static SortingIndexBufferClass *_DynamicSortingIndexArray = nullptr;
static unsigned short _DynamicSortingIndexArraySize = 0;
static unsigned short _DynamicSortingIndexArrayOffset = 0;

static bool _DynamicDX8IndexBufferInUse = false;
static DX8IndexBufferClass *_DynamicDX8IndexBuffer = nullptr;
static unsigned short _DynamicDX8IndexBufferSize = DEFAULT_IB_SIZE;
static unsigned short _DynamicDX8IndexBufferOffset = 0;

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
#ifndef RA_MACOS
  if (_this->engine_refs == 0) {
    delete _this;
  }
#endif
}

// --- IndexBufferClass::Copy methods ---
void IndexBufferClass::Copy(unsigned int *indices, unsigned first_index,
                            unsigned count) {
  WWASSERT(indices);
  if (first_index) {
    IndexBufferClass::AppendLockClass l(this, first_index, count);
    unsigned short *inds = l.Get_Index_Array();
    for (unsigned v = 0; v < count; ++v) {
      *inds++ = (unsigned short)(*indices++);
    }
  } else {
    IndexBufferClass::WriteLockClass l(this);
    unsigned short *inds = l.Get_Index_Array();
    for (unsigned v = 0; v < count; ++v) {
      *inds++ = (unsigned short)(*indices++);
    }
  }
}

void IndexBufferClass::Copy(unsigned short *indices, unsigned first_index,
                            unsigned count) {
  WWASSERT(indices);
  if (first_index) {
    IndexBufferClass::AppendLockClass l(this, first_index, count);
    unsigned short *inds = l.Get_Index_Array();
    for (unsigned v = 0; v < count; ++v) {
      *inds++ = *indices++;
    }
  } else {
    IndexBufferClass::WriteLockClass l(this);
    unsigned short *inds = l.Get_Index_Array();
    for (unsigned v = 0; v < count; ++v) {
      *inds++ = *indices++;
    }
  }
}

// --- WriteLockClass: Lock the underlying DX8 IB to get writable index pointer
// ---
IndexBufferClass::WriteLockClass::WriteLockClass(IndexBufferClass *ib,
                                                 int flags)
    : index_buffer(ib) {
  WWASSERT(index_buffer);
  index_buffer->Add_Ref();
  switch (index_buffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8IndexBufferClass *>(index_buffer)
        ->Get_DX8_Index_Buffer()
        ->Lock(0, index_buffer->Get_Index_Count() * sizeof(unsigned short),
               (unsigned char **)&indices, flags);
    break;
  case BUFFER_TYPE_SORTING:
    indices =
        static_cast<SortingIndexBufferClass *>(index_buffer)->index_buffer;
    break;
  default:
    WWASSERT(0);
    indices = 0;
    break;
  }
}

IndexBufferClass::WriteLockClass::~WriteLockClass() {
  switch (index_buffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8IndexBufferClass *>(index_buffer)
        ->Get_DX8_Index_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
  index_buffer->Release_Ref();
}

// --- AppendLockClass: Lock at a specific offset ---
IndexBufferClass::AppendLockClass::AppendLockClass(IndexBufferClass *ib,
                                                   unsigned start_index,
                                                   unsigned index_range)
    : index_buffer(ib) {
  WWASSERT(index_buffer);
  WWASSERT(start_index + index_range <= index_buffer->Get_Index_Count());
  index_buffer->Add_Ref();
  switch (index_buffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8IndexBufferClass *>(index_buffer)
        ->Get_DX8_Index_Buffer()
        ->Lock(start_index * sizeof(unsigned short),
               index_range * sizeof(unsigned short), (unsigned char **)&indices,
               0);
    break;
  case BUFFER_TYPE_SORTING:
    indices =
        static_cast<SortingIndexBufferClass *>(index_buffer)->index_buffer +
        start_index;
    break;
  default:
    WWASSERT(0);
    indices = 0;
    break;
  }
}

IndexBufferClass::AppendLockClass::~AppendLockClass() {
  switch (index_buffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8IndexBufferClass *>(index_buffer)
        ->Get_DX8_Index_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
  index_buffer->Release_Ref();
}

DX8IndexBufferClass::DX8IndexBufferClass(unsigned short index_count,
                                         UsageType usage)
    : IndexBufferClass(BUFFER_TYPE_DX8, index_count, false), index_buffer(0) {
  DWORD dwUsage = D3DUSAGE_WRITEONLY;
  if (usage & USAGE_DYNAMIC) {
    dwUsage |= D3DUSAGE_DYNAMIC;
  }

#ifdef __APPLE__
  index_buffer =
      (IDirect3DIndexBuffer8 *)new MetalIndexBuffer8(index_count, false);
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

// --- DynamicIBAccessClass ---
DynamicIBAccessClass::DynamicIBAccessClass(unsigned short type_,
                                           unsigned short count)
    : Type(type_), IndexCount(count), IndexBufferOffset(0), IndexBuffer(0) {
  WWASSERT(Type == BUFFER_TYPE_DYNAMIC_DX8 ||
           Type == BUFFER_TYPE_DYNAMIC_SORTING);
  if (Type == BUFFER_TYPE_DYNAMIC_DX8) {
    Allocate_DX8_Dynamic_Buffer();
  } else {
    Allocate_Sorting_Dynamic_Buffer();
  }
}

DynamicIBAccessClass::~DynamicIBAccessClass() {
  REF_PTR_RELEASE(IndexBuffer);
  if (Type == BUFFER_TYPE_DYNAMIC_DX8) {
    _DynamicDX8IndexBufferInUse = false;
    _DynamicDX8IndexBufferOffset += IndexCount;
  } else {
    _DynamicSortingIndexArrayInUse = false;
    _DynamicSortingIndexArrayOffset += IndexCount;
  }
}

void DynamicIBAccessClass::Allocate_DX8_Dynamic_Buffer() {
  WWASSERT(!_DynamicDX8IndexBufferInUse);
  _DynamicDX8IndexBufferInUse = true;

  if (IndexCount > _DynamicDX8IndexBufferSize) {
    REF_PTR_RELEASE(_DynamicDX8IndexBuffer);
    _DynamicDX8IndexBufferSize = IndexCount;
    if (_DynamicDX8IndexBufferSize < DEFAULT_IB_SIZE)
      _DynamicDX8IndexBufferSize = DEFAULT_IB_SIZE;
  }

  if (!_DynamicDX8IndexBuffer) {
    unsigned usage = DX8IndexBufferClass::USAGE_DYNAMIC;
    _DynamicDX8IndexBuffer =
        NEW_REF(DX8IndexBufferClass, (_DynamicDX8IndexBufferSize,
                                      (DX8IndexBufferClass::UsageType)usage));
    _DynamicDX8IndexBufferOffset = 0;
  }

  if (((unsigned)IndexCount + _DynamicDX8IndexBufferOffset) >
      _DynamicDX8IndexBufferSize) {
    _DynamicDX8IndexBufferOffset = 0;
  }

  REF_PTR_SET(IndexBuffer, _DynamicDX8IndexBuffer);
  IndexBufferOffset = _DynamicDX8IndexBufferOffset;
}

void DynamicIBAccessClass::Allocate_Sorting_Dynamic_Buffer() {
  WWASSERT(!_DynamicSortingIndexArrayInUse);
  _DynamicSortingIndexArrayInUse = true;

  unsigned new_index_count = _DynamicSortingIndexArrayOffset + IndexCount;
  WWASSERT(new_index_count < 65536);
  if (new_index_count > _DynamicSortingIndexArraySize) {
    REF_PTR_RELEASE(_DynamicSortingIndexArray);
    _DynamicSortingIndexArraySize = new_index_count;
    if (_DynamicSortingIndexArraySize < DEFAULT_IB_SIZE)
      _DynamicSortingIndexArraySize = DEFAULT_IB_SIZE;
  }

  if (!_DynamicSortingIndexArray) {
    _DynamicSortingIndexArray =
        NEW_REF(SortingIndexBufferClass, (_DynamicSortingIndexArraySize));
    _DynamicSortingIndexArrayOffset = 0;
  }

  REF_PTR_SET(IndexBuffer, _DynamicSortingIndexArray);
  IndexBufferOffset = _DynamicSortingIndexArrayOffset;
}

void DynamicIBAccessClass::_Deinit() {
  WWASSERT((_DynamicDX8IndexBuffer == nullptr) ||
           (_DynamicDX8IndexBuffer->Num_Refs() == 1));
  REF_PTR_RELEASE(_DynamicDX8IndexBuffer);
  _DynamicDX8IndexBufferInUse = false;
  _DynamicDX8IndexBufferSize = DEFAULT_IB_SIZE;
  _DynamicDX8IndexBufferOffset = 0;

  WWASSERT((_DynamicSortingIndexArray == nullptr) ||
           (_DynamicSortingIndexArray->Num_Refs() == 1));
  REF_PTR_RELEASE(_DynamicSortingIndexArray);
  _DynamicSortingIndexArrayInUse = false;
  _DynamicSortingIndexArraySize = 0;
  _DynamicSortingIndexArrayOffset = 0;
}

void DynamicIBAccessClass::_Reset(bool frame_changed) {
  _DynamicSortingIndexArrayOffset = 0;
  if (frame_changed)
    _DynamicDX8IndexBufferOffset = 0;
}

unsigned short DynamicIBAccessClass::Get_Default_Index_Count() {
  return _DynamicDX8IndexBufferSize;
}

// --- DynamicIBAccessClass::WriteLockClass ---
DynamicIBAccessClass::WriteLockClass::WriteLockClass(DynamicIBAccessClass *ia)
    : DynamicIBAccess(ia), Indices(0) {
  DynamicIBAccess->IndexBuffer->Add_Ref();
  switch (DynamicIBAccess->Get_Type()) {
  case BUFFER_TYPE_DYNAMIC_DX8:
    WWASSERT(DynamicIBAccess);
    static_cast<DX8IndexBufferClass *>(DynamicIBAccess->IndexBuffer)
        ->Get_DX8_Index_Buffer()
        ->Lock(DynamicIBAccess->IndexBufferOffset * sizeof(unsigned short),
               DynamicIBAccess->Get_Index_Count() * sizeof(unsigned short),
               (unsigned char **)&Indices,
               !DynamicIBAccess->IndexBufferOffset ? D3DLOCK_DISCARD
                                                   : D3DLOCK_NOOVERWRITE);
    break;
  case BUFFER_TYPE_DYNAMIC_SORTING:
    Indices =
        static_cast<SortingIndexBufferClass *>(DynamicIBAccess->IndexBuffer)
            ->index_buffer;
    Indices += DynamicIBAccess->IndexBufferOffset;
    break;
  default:
    WWASSERT(0);
    break;
  }
}

DynamicIBAccessClass::WriteLockClass::~WriteLockClass() {
  switch (DynamicIBAccess->Get_Type()) {
  case BUFFER_TYPE_DYNAMIC_DX8:
    static_cast<DX8IndexBufferClass *>(DynamicIBAccess->IndexBuffer)
        ->Get_DX8_Index_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_DYNAMIC_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
  DynamicIBAccess->IndexBuffer->Release_Ref();
}
