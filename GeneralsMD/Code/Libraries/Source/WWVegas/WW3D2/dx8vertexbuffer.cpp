#include "dx8vertexbuffer.h"
#include "always.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include <Utility/win_compat.h>

#ifdef __APPLE__
#include "MetalVertexBuffer8.h"
#include <d3d8.h>
#endif

#define DEFAULT_VB_SIZE 5000

static bool _DynamicSortingVertexArrayInUse = false;
static SortingVertexBufferClass *_DynamicSortingVertexArray = nullptr;
static unsigned short _DynamicSortingVertexArraySize = 0;
static unsigned short _DynamicSortingVertexArrayOffset = 0;

static bool _DynamicDX8VertexBufferInUse = false;
static DX8VertexBufferClass *_DynamicDX8VertexBuffer = nullptr;
static unsigned short _DynamicDX8VertexBufferSize = DEFAULT_VB_SIZE;
static unsigned short _DynamicDX8VertexBufferOffset = 0;

static const FVFInfoClass _DynamicFVFInfo(dynamic_fvf_type);

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

// --- WriteLockClass: Lock the underlying DX8 VB to get writable vertex pointer
// ---
VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass *vb,
                                                  int flags)
    : VertexBufferLockClass(vb) {
  WWASSERT(VertexBuffer);
  VertexBuffer->Add_Ref();
  switch (VertexBuffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8VertexBufferClass *>(VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Lock(0, 0, (unsigned char **)&Vertices, flags);
    break;
  case BUFFER_TYPE_SORTING:
    Vertices =
        static_cast<SortingVertexBufferClass *>(VertexBuffer)->VertexBuffer;
    break;
  default:
    WWASSERT(0);
    Vertices = nullptr;
    break;
  }
}

VertexBufferClass::WriteLockClass::~WriteLockClass() {
  switch (VertexBuffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8VertexBufferClass *>(VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
  VertexBuffer->Release_Ref();
}

// --- AppendLockClass: Lock at a specific offset  ---
VertexBufferClass::AppendLockClass::AppendLockClass(VertexBufferClass *vb,
                                                    unsigned start_index,
                                                    unsigned index_range)
    : VertexBufferLockClass(vb) {
  WWASSERT(VertexBuffer);
  WWASSERT(start_index + index_range <= VertexBuffer->Get_Vertex_Count());
  VertexBuffer->Add_Ref();
  switch (VertexBuffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8VertexBufferClass *>(VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Lock(start_index * VertexBuffer->FVF_Info().Get_FVF_Size(),
               index_range * VertexBuffer->FVF_Info().Get_FVF_Size(),
               (unsigned char **)&Vertices, 0);
    break;
  case BUFFER_TYPE_SORTING:
    Vertices =
        static_cast<SortingVertexBufferClass *>(VertexBuffer)->VertexBuffer +
        start_index;
    break;
  default:
    WWASSERT(0);
    Vertices = nullptr;
    break;
  }
}

VertexBufferClass::AppendLockClass::~AppendLockClass() {
  switch (VertexBuffer->Type()) {
  case BUFFER_TYPE_DX8:
    static_cast<DX8VertexBufferClass *>(VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
  VertexBuffer->Release_Ref();
}

void VertexBufferClass::Release_Engine_Ref() const {
  WWASSERT(engine_refs > 0);
  VertexBufferClass *_this = const_cast<VertexBufferClass *>(this);
  _this->engine_refs--;
#ifndef RA_MACOS
  if (_this->engine_refs == 0) {
    delete _this;
  }
#endif
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
  VertexBuffer = (IDirect3DVertexBuffer8 *)new MetalVertexBuffer8(
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
      VertexBufferOffset(0), VertexBuffer(0) {
  WWASSERT(fvf_ == dynamic_fvf_type);
  WWASSERT(Type == BUFFER_TYPE_DYNAMIC_DX8 ||
           Type == BUFFER_TYPE_DYNAMIC_SORTING);

  if (Type == BUFFER_TYPE_DYNAMIC_DX8) {
    Allocate_DX8_Dynamic_Buffer();
  } else {
    Allocate_Sorting_Dynamic_Buffer();
  }
}

DynamicVBAccessClass::~DynamicVBAccessClass() {
  if (Type == BUFFER_TYPE_DYNAMIC_DX8) {
    _DynamicDX8VertexBufferInUse = false;
    _DynamicDX8VertexBufferOffset += (unsigned)VertexCount;
  } else {
    _DynamicSortingVertexArrayInUse = false;
    _DynamicSortingVertexArrayOffset += VertexCount;
  }
  REF_PTR_RELEASE(VertexBuffer);
  delete const_cast<FVFInfoClass *>(&FVFInfo);
}

void DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer() {
  WWASSERT(!_DynamicDX8VertexBufferInUse);
  _DynamicDX8VertexBufferInUse = true;

  if (VertexCount > _DynamicDX8VertexBufferSize) {
    REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
    _DynamicDX8VertexBufferSize = VertexCount;
    if (_DynamicDX8VertexBufferSize < DEFAULT_VB_SIZE)
      _DynamicDX8VertexBufferSize = DEFAULT_VB_SIZE;
  }

  if (!_DynamicDX8VertexBuffer) {
    unsigned usage = DX8VertexBufferClass::USAGE_DYNAMIC;
    _DynamicDX8VertexBuffer = NEW_REF(
        DX8VertexBufferClass, (dynamic_fvf_type, _DynamicDX8VertexBufferSize,
                               (DX8VertexBufferClass::UsageType)usage));
    _DynamicDX8VertexBufferOffset = 0;
  }

  if (((unsigned)VertexCount + _DynamicDX8VertexBufferOffset) >
      _DynamicDX8VertexBufferSize) {
    _DynamicDX8VertexBufferOffset = 0;
  }

  REF_PTR_SET(VertexBuffer, _DynamicDX8VertexBuffer);
  VertexBufferOffset = _DynamicDX8VertexBufferOffset;
}

void DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer() {
  WWASSERT(!_DynamicSortingVertexArrayInUse);
  _DynamicSortingVertexArrayInUse = true;

  unsigned new_vertex_count = _DynamicSortingVertexArrayOffset + VertexCount;
  WWASSERT(new_vertex_count < 65536);
  if (new_vertex_count > _DynamicSortingVertexArraySize) {
    REF_PTR_RELEASE(_DynamicSortingVertexArray);
    _DynamicSortingVertexArraySize = new_vertex_count;
    if (_DynamicSortingVertexArraySize < DEFAULT_VB_SIZE)
      _DynamicSortingVertexArraySize = DEFAULT_VB_SIZE;
  }

  if (!_DynamicSortingVertexArray) {
    _DynamicSortingVertexArray =
        NEW_REF(SortingVertexBufferClass, (_DynamicSortingVertexArraySize));
    _DynamicSortingVertexArrayOffset = 0;
  }

  REF_PTR_SET(VertexBuffer, _DynamicSortingVertexArray);
  VertexBufferOffset = _DynamicSortingVertexArrayOffset;
}

void DynamicVBAccessClass::_Deinit() {
  WWASSERT((_DynamicDX8VertexBuffer == nullptr) ||
           (_DynamicDX8VertexBuffer->Num_Refs() == 1));
  REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
  _DynamicDX8VertexBufferInUse = false;
  _DynamicDX8VertexBufferSize = DEFAULT_VB_SIZE;
  _DynamicDX8VertexBufferOffset = 0;

  WWASSERT((_DynamicSortingVertexArray == nullptr) ||
           (_DynamicSortingVertexArray->Num_Refs() == 1));
  REF_PTR_RELEASE(_DynamicSortingVertexArray);
  WWASSERT(!_DynamicSortingVertexArrayInUse);
  _DynamicSortingVertexArrayInUse = false;
  _DynamicSortingVertexArraySize = 0;
  _DynamicSortingVertexArrayOffset = 0;
}

void DynamicVBAccessClass::_Reset(bool frame_changed) {
  _DynamicSortingVertexArrayOffset = 0;
  if (frame_changed)
    _DynamicDX8VertexBufferOffset = 0;
}

unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count() {
  return _DynamicDX8VertexBufferSize;
}

// --- DynamicVBAccessClass::WriteLockClass ---
DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass *va)
    : DynamicVBAccess(va), Vertices(0) {
  switch (DynamicVBAccess->Get_Type()) {
  case BUFFER_TYPE_DYNAMIC_DX8:
    WWASSERT(_DynamicDX8VertexBuffer);
    static_cast<DX8VertexBufferClass *>(DynamicVBAccess->VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Lock(DynamicVBAccess->VertexBufferOffset *
                   _DynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size(),
               DynamicVBAccess->Get_Vertex_Count() *
                   DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Size(),
               (unsigned char **)&Vertices,
               D3DLOCK_NOSYSLOCK | (!DynamicVBAccess->VertexBufferOffset
                                        ? D3DLOCK_DISCARD
                                        : D3DLOCK_NOOVERWRITE));
    break;
  case BUFFER_TYPE_DYNAMIC_SORTING:
    Vertices =
        static_cast<SortingVertexBufferClass *>(DynamicVBAccess->VertexBuffer)
            ->VertexBuffer;
    Vertices += DynamicVBAccess->VertexBufferOffset;
    break;
  default:
    WWASSERT(0);
    break;
  }
}

DynamicVBAccessClass::WriteLockClass::~WriteLockClass() {
  switch (DynamicVBAccess->Get_Type()) {
  case BUFFER_TYPE_DYNAMIC_DX8:
    static_cast<DX8VertexBufferClass *>(DynamicVBAccess->VertexBuffer)
        ->Get_DX8_Vertex_Buffer()
        ->Unlock();
    break;
  case BUFFER_TYPE_DYNAMIC_SORTING:
    break;
  default:
    WWASSERT(0);
    break;
  }
}

// --- Additional DX8VertexBufferClass constructors ---

DX8VertexBufferClass::DX8VertexBufferClass(const Vector3 *vertices,
                                           const Vector3 *normals,
                                           const Vector2 *tex_coords,
                                           unsigned short VertexCount,
                                           UsageType usage)
    : VertexBufferClass(BUFFER_TYPE_DX8,
                        D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_NORMAL, VertexCount),
      VertexBuffer(nullptr) {
  WWASSERT(vertices);
  WWASSERT(normals);
  WWASSERT(tex_coords);
  Create_Vertex_Buffer(usage);
  Copy(vertices, normals, tex_coords, 0, VertexCount);
}

DX8VertexBufferClass::DX8VertexBufferClass(
    const Vector3 *vertices, const Vector3 *normals, const Vector4 *diffuse,
    const Vector2 *tex_coords, unsigned short VertexCount, UsageType usage)
    : VertexBufferClass(BUFFER_TYPE_DX8,
                        D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_NORMAL |
                            D3DFVF_DIFFUSE,
                        VertexCount),
      VertexBuffer(nullptr) {
  WWASSERT(vertices);
  WWASSERT(normals);
  WWASSERT(tex_coords);
  WWASSERT(diffuse);
  Create_Vertex_Buffer(usage);
  Copy(vertices, normals, tex_coords, diffuse, 0, VertexCount);
}

DX8VertexBufferClass::DX8VertexBufferClass(const Vector3 *vertices,
                                           const Vector4 *diffuse,
                                           const Vector2 *tex_coords,
                                           unsigned short VertexCount,
                                           UsageType usage)
    : VertexBufferClass(BUFFER_TYPE_DX8,
                        D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE, VertexCount),
      VertexBuffer(nullptr) {
  WWASSERT(vertices);
  WWASSERT(tex_coords);
  WWASSERT(diffuse);
  Create_Vertex_Buffer(usage);
  Copy(vertices, tex_coords, diffuse, 0, VertexCount);
}

DX8VertexBufferClass::DX8VertexBufferClass(const Vector3 *vertices,
                                           const Vector2 *tex_coords,
                                           unsigned short VertexCount,
                                           UsageType usage)
    : VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ | D3DFVF_TEX1, VertexCount),
      VertexBuffer(nullptr) {
  WWASSERT(vertices);
  WWASSERT(tex_coords);
  Create_Vertex_Buffer(usage);
  Copy(vertices, tex_coords, 0, VertexCount);
}

// --- DX8VertexBufferClass::Copy methods ---

void DX8VertexBufferClass::Copy(const Vector3 *loc, unsigned first_vertex,
                                unsigned count) {
  WWASSERT(loc);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZ *verts = (VertexFormatXYZ *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZ *verts = (VertexFormatXYZ *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
    }
  }
}

void DX8VertexBufferClass::Copy(const Vector3 *loc, const Vector2 *uv,
                                unsigned first_vertex, unsigned count) {
  WWASSERT(loc);
  WWASSERT(uv);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZUV1 *verts = (VertexFormatXYZUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZUV1 *verts = (VertexFormatXYZUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
    }
  }
}

void DX8VertexBufferClass::Copy(const Vector3 *loc, const Vector3 *norm,
                                unsigned first_vertex, unsigned count) {
  WWASSERT(loc);
  WWASSERT(norm);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZN *verts = (VertexFormatXYZN *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZN *verts = (VertexFormatXYZN *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
    }
  }
}

void DX8VertexBufferClass::Copy(const Vector3 *loc, const Vector3 *norm,
                                const Vector2 *uv, unsigned first_vertex,
                                unsigned count) {
  WWASSERT(loc);
  WWASSERT(norm);
  WWASSERT(uv);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZNUV1 *verts = (VertexFormatXYZNUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZNUV1 *verts = (VertexFormatXYZNUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
    }
  }
}

void DX8VertexBufferClass::Copy(const Vector3 *loc, const Vector3 *norm,
                                const Vector2 *uv, const Vector4 *diffuse,
                                unsigned first_vertex, unsigned count) {
  WWASSERT(loc);
  WWASSERT(norm);
  WWASSERT(uv);
  WWASSERT(diffuse);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZNDUV1 *verts = (VertexFormatXYZNDUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
      verts[v].diffuse = DX8Wrapper::Convert_Color(diffuse[v]);
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZNDUV1 *verts = (VertexFormatXYZNDUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].nx = (*norm)[0];
      verts[v].ny = (*norm)[1];
      verts[v].nz = (*norm++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
      verts[v].diffuse = DX8Wrapper::Convert_Color(diffuse[v]);
    }
  }
}

void DX8VertexBufferClass::Copy(const Vector3 *loc, const Vector2 *uv,
                                const Vector4 *diffuse, unsigned first_vertex,
                                unsigned count) {
  WWASSERT(loc);
  WWASSERT(uv);
  WWASSERT(diffuse);
  WWASSERT(count <= VertexCount);
  if (first_vertex) {
    VertexBufferClass::AppendLockClass l(this, first_vertex, count);
    VertexFormatXYZDUV1 *verts = (VertexFormatXYZDUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
      verts[v].diffuse = DX8Wrapper::Convert_Color(diffuse[v]);
    }
  } else {
    VertexBufferClass::WriteLockClass l(this);
    VertexFormatXYZDUV1 *verts = (VertexFormatXYZDUV1 *)l.Get_Vertex_Array();
    for (unsigned v = 0; v < count; ++v) {
      verts[v].x = (*loc)[0];
      verts[v].y = (*loc)[1];
      verts[v].z = (*loc++)[2];
      verts[v].u1 = (*uv)[0];
      verts[v].v1 = (*uv++)[1];
      verts[v].diffuse = DX8Wrapper::Convert_Color(diffuse[v]);
    }
  }
}
