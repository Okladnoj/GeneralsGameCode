#include "MetalTexture8.h"
#include "MetalDevice8.h"
#include "MetalSurface8.h"
#include <map>
#include <cstdio>

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL E_FAIL
#endif

// Helper: Get Bytes Per Pixel or Block Size
UINT BytesPerPixelFromD3D(D3DFORMAT fmt) {
  switch (fmt) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8:
    return 4;
  case D3DFMT_R5G6B5:
  case D3DFMT_X1R5G5B5:
  case D3DFMT_A1R5G5B5:
  case D3DFMT_A4R4G4B4:
    return 2;
  case D3DFMT_R8G8B8:
    return 3;
  case D3DFMT_A8:
  case D3DFMT_L8:
  case D3DFMT_P8:
    return 1;
  case D3DFMT_DXT1:
    return 8; // Per 4x4 block (8 bytes)
  case D3DFMT_DXT2:
  case D3DFMT_DXT3:
  case D3DFMT_DXT4:
  case D3DFMT_DXT5:
    return 16; // Per 4x4 block (16 bytes)
  default:
    return 4; // Fallback
  }
}

// Helper: Metal Pixel Format
MTLPixelFormat MetalFormatFromD3D(D3DFORMAT fmt) {
  switch (fmt) {
  case D3DFMT_A8R8G8B8:
  case D3DFMT_X8R8G8B8: // Ignore alpha channel in shader if needed
    return MTLPixelFormatBGRA8Unorm;

  // macOS Metal supports BC compression
  case D3DFMT_DXT1:
    return MTLPixelFormatBC1_RGBA;
  case D3DFMT_DXT3: // DXT2 is premultiplied alpha DXT3, usually same storage
    return MTLPixelFormatBC2_RGBA;
  case D3DFMT_DXT5: // DXT4 is premultiplied alpha DXT5
    return MTLPixelFormatBC3_RGBA;

    // 16-bit formats often need conversion on macOS Desktop (no B5G6R5 native
    // support typically) Converting to BGRA8 on CPU is safer, but let's assume
    // keys are handled or return Invalid for now. Actually, we can return
    // BGRA8Unorm and rely on LockRect/UnlockRect to convert if we implement
    // conversion logic. For basic support, let's map to BGRA8Unorm and warn if
    // mismatch. But D3D will try to write 16-bit data. We'll stick to 32-bit
    // and Compressed for Stage 3.

  default:
    return MTLPixelFormatBGRA8Unorm;
  }
}

MetalTexture8::MetalTexture8(MetalDevice8 *device, UINT width, UINT height,
                             UINT levels, DWORD usage, D3DFORMAT format,
                             D3DPOOL pool)
    : m_RefCount(1), m_Device(device), m_Width(width), m_Height(height),
      m_Levels(levels), m_Usage(usage), m_Format(format), m_Pool(pool) {

  if (m_Device)
    m_Device->AddRef();

  if (m_Levels == 0)
    m_Levels = 1; // Actually 0 means "all levels", typically mipmapping. Metal
                  // needs specific count.
  // Typically we'd calculate log2(max(w, h)) + 1. But for now clamp to 1 if 0.
  // IDirect3DTexture8::GetLevelCount returns actual count.

  MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
  desc.pixelFormat = MetalFormatFromD3D(format);
  desc.width = width;
  desc.height = height;
  desc.mipmapLevelCount = m_Levels;
  desc.storageMode = MTLStorageModeShared; // Ensure CPU writes via replaceRegion are immediately GPU-visible
  desc.usage = MTLTextureUsageShaderRead;
  if (usage & D3DUSAGE_RENDERTARGET) {
    desc.usage |= MTLTextureUsageRenderTarget;
  }

  id<MTLDevice> mtlDev = (__bridge id<MTLDevice>)m_Device->GetMTLDevice();
  id<MTLTexture> tex = [mtlDev newTextureWithDescriptor:desc];
  m_Texture = (__bridge_retained void *)tex; // Retain manual ref
}

MetalTexture8::MetalTexture8(MetalDevice8 *device, void *mtlTexture,
                             D3DFORMAT format)
    : m_RefCount(1), m_Device(device), m_Width(0), m_Height(0), m_Levels(1),
      m_Usage(0), m_Format(format), m_Pool(D3DPOOL_DEFAULT) {

  if (m_Device)
    m_Device->AddRef();

  id<MTLTexture> tex = (__bridge id<MTLTexture>)mtlTexture;
  if (tex) {
    // We want to RETAIN it because we will Release it in destructor.
    // __bridge_retained transfers +1.
    // If tex is __bridge (no transfer), we need a retain.
    // If we cast `void*` -> `id`, it's __bridge.
    // `tex` is just a pointer usage.
    // We want to store it as `m_Texture` (void*) with +1 ref count.
    m_Texture = (__bridge_retained void *)tex;

    m_Width = (UINT)tex.width;
    m_Height = (UINT)tex.height;
    m_Levels = (UINT)tex.mipmapLevelCount;
  } else {
    m_Texture = nullptr;
  }
}

MetalTexture8::~MetalTexture8() {
  if (m_Texture) {
    CFRelease(m_Texture); // Specific matching retain/release
    m_Texture = nullptr;
  }
  if (m_Device)
    m_Device->Release();
}

STDMETHODIMP MetalTexture8::QueryInterface(REFIID riid, void **ppvObj) {
  if (!ppvObj)
    return E_POINTER;
  *ppvObj = nullptr;
  // Basic IUnknown check (omitting UUID check for brevity/uuid lib missing)
  *ppvObj = this;
  AddRef();
  return D3D_OK;
}

STDMETHODIMP_(ULONG) MetalTexture8::AddRef() { return ++m_RefCount; }

STDMETHODIMP_(ULONG) MetalTexture8::Release() {
  if (--m_RefCount == 0) {
    delete this;
    return 0;
  }
  return m_RefCount;
}

// IDirect3DResource8
STDMETHODIMP MetalTexture8::GetDevice(IDirect3DDevice8 **ppDevice) {
  if (ppDevice) {
    *ppDevice = m_Device;
    m_Device->AddRef();
    return D3D_OK;
  }
  return D3DERR_INVALIDCALL;
}

STDMETHODIMP MetalTexture8::SetPrivateData(REFGUID refguid, CONST void *pData,
                                           DWORD SizeOfData, DWORD Flags) {
  return D3D_OK;
}
STDMETHODIMP MetalTexture8::GetPrivateData(REFGUID refguid, void *pData,
                                           DWORD *pSizeOfData) {
  return D3DERR_NOTFOUND;
}
STDMETHODIMP MetalTexture8::FreePrivateData(REFGUID refguid) { return D3D_OK; }
STDMETHODIMP_(DWORD) MetalTexture8::SetPriority(DWORD PriorityNew) { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetPriority() { return 0; }
STDMETHODIMP_(void) MetalTexture8::PreLoad() {}
STDMETHODIMP_(D3DRESOURCETYPE) MetalTexture8::GetType() {
  return D3DRTYPE_TEXTURE;
}

// IDirect3DBaseTexture8
STDMETHODIMP_(DWORD) MetalTexture8::SetLOD(DWORD LODNew) { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetLOD() { return 0; }
STDMETHODIMP_(DWORD) MetalTexture8::GetLevelCount() { return m_Levels; }

// IDirect3DTexture8
STDMETHODIMP MetalTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
  if (!pDesc)
    return D3DERR_INVALIDCALL;
  if (Level >= m_Levels)
    return D3DERR_INVALIDCALL;

  pDesc->Format = m_Format;
  pDesc->Type = D3DRTYPE_SURFACE;
  pDesc->Usage = m_Usage;
  pDesc->Pool = m_Pool;
  pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
  pDesc->Width = std::max(1u, m_Width >> Level);
  pDesc->Height = std::max(1u, m_Height >> Level);
  pDesc->Size = 0; // Not used often
  return D3D_OK;
}

STDMETHODIMP
MetalTexture8::GetSurfaceLevel(UINT Level, IDirect3DSurface8 **ppSurfaceLevel) {
  if (!ppSurfaceLevel)
    return E_POINTER;
  if (Level >= m_Levels) {
    *ppSurfaceLevel = nullptr;
    return D3DERR_INVALIDCALL;
  }

  UINT w = std::max(1u, m_Width >> Level);
  UINT h = std::max(1u, m_Height >> Level);

  static int gslLog = 0;
  if (gslLog < 20) {
    fprintf(stderr, "GetSurfaceLevel[%d] tex=%p level=%u %ux%u fmt=%d mtlTex=%p\n",
            gslLog++, (void*)this, Level, w, h, (int)m_Format, m_Texture);
    fflush(stderr);
  }

  // Create a surface wrapper linked to this texture's mip level.
  // When the surface is unlocked, it will upload data to our Metal texture.
  auto *surface =
      new MetalSurface8(m_Device, MetalSurface8::kColor, w, h, m_Format,
                        this, Level);
  *ppSurfaceLevel = surface;
  return D3D_OK;
}

STDMETHODIMP MetalTexture8::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect,
                                     CONST RECT *pRect, DWORD Flags) {
  if (Level >= m_Levels || !pLockedRect)
    return D3DERR_INVALIDCALL;

  // Check if checks already locked
  if (m_LockedLevels.count(Level))
    return D3DERR_INVALIDCALL; // Already locked

  // Allocate staging memory
  UINT width = std::max(1u, m_Width >> Level);
  UINT height = std::max(1u, m_Height >> Level);
  UINT bpp = BytesPerPixelFromD3D(m_Format);

  UINT pitch = 0;
  UINT dataSize = 0;

  bool isCompressed = (m_Format == D3DFMT_DXT1 || m_Format == D3DFMT_DXT2 ||
                       m_Format == D3DFMT_DXT3 || m_Format == D3DFMT_DXT4 ||
                       m_Format == D3DFMT_DXT5);

  if (isCompressed) {
    // Blocks are 4x4
    UINT blocksWide = std::max(1u, (width + 3) / 4);
    UINT blocksHigh = std::max(1u, (height + 3) / 4);
    pitch = blocksWide * bpp; // bpp is bytes per block (8 or 16)
    dataSize = pitch * blocksHigh;
  } else {
    pitch = width * bpp;
    dataSize = pitch * height;
  }

  void *data = malloc(dataSize);
  if (!data)
    return D3DERR_OUTOFVIDEOMEMORY;

  // Retrieve existing texture data if it's already uploaded.
  // Skip for compressed textures — getBytes on uninitialized BC textures can corrupt heap.
  // Also skip if D3DLOCK_DISCARD is set (caller will overwrite all data).
  if (m_Texture && !(Flags & D3DLOCK_DISCARD) && !isCompressed && m_HasBeenWritten) {
    id<MTLTexture> mtlTex = (__bridge id<MTLTexture>)m_Texture;
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [mtlTex getBytes:data bytesPerRow:pitch fromRegion:region mipmapLevel:Level];
  } else {
    // Zero-fill for fresh textures or compressed formats
    memset(data, 0, dataSize);
  }

  uint8_t *pBits = (uint8_t *)data;
  if (pRect) {
    printf("TEX_LOCK_PRECT fmt=%d level=%u prect=%d,%d,%d,%d\n", m_Format, Level, pRect->left, pRect->top, pRect->right, pRect->bottom);
    fflush(stdout);
    if (isCompressed) {
      pBits += (pRect->top / 4) * pitch + (pRect->left / 4) * bpp;
    } else {
      pBits += pRect->top * pitch + pRect->left * bpp;
    }
  }

  pLockedRect->pBits = pBits;
  pLockedRect->Pitch = pitch;

  LockedLevel lvl;
  lvl.ptr = data;
  lvl.pitch = pitch;
  lvl.bytesPerPixel = bpp;
  if (pRect) {
    lvl.hasRect = true;
    lvl.rect = *pRect;
  } else {
    lvl.hasRect = false;
  }

  m_LockedLevels[Level] = lvl;

  return D3D_OK;
}

STDMETHODIMP MetalTexture8::UnlockRect(UINT Level) {
  static int unlockEntryCount = 0;
  auto it = m_LockedLevels.find(Level);
  if (it == m_LockedLevels.end()) {
    if (unlockEntryCount < 50) {
      printf("TEX_UNLOCK_MISS[%d] level=%u lockedCount=%zu this=%p\n",
             unlockEntryCount++, Level, m_LockedLevels.size(), this);
      fflush(stdout);
    }
    return D3DERR_INVALIDCALL;
  }

  LockedLevel &lvl = it->second;

  // Upload to Metal Texture
  id<MTLTexture> tex = (__bridge id<MTLTexture>)m_Texture;

  // On Apple Silicon, textures use Shared memory. Updating a texture via replaceRegion
  // while the GPU might be reading from it causes tearing / flickering.
  // For single-level textures (which are typical for dynamic UI/video), we can simply
  // allocate a new texture, resolving the synchronization problem.
  // Only recreate if we are updating the entire texture (!lvl.hasRect).
  if (m_Levels == 1 && !lvl.hasRect) {
    MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = tex.pixelFormat;
    desc.width = tex.width;
    desc.height = tex.height;
    desc.mipmapLevelCount = 1;
    desc.storageMode = MTLStorageModeShared;
    desc.usage = tex.usage;
    
    id<MTLTexture> newTex = [tex.device newTextureWithDescriptor:desc];
    CFRelease(m_Texture);
    m_Texture = (__bridge_retained void *)newTex;
    tex = newTex;
  }

  UINT width = std::max(1u, m_Width >> Level);
  UINT height = std::max(1u, m_Height >> Level);

  bool isCompressed = (m_Format == D3DFMT_DXT1 || m_Format == D3DFMT_DXT2 ||
                       m_Format == D3DFMT_DXT3 || m_Format == D3DFMT_DXT4 ||
                       m_Format == D3DFMT_DXT5);

  static int texUploadLogCount = 0;

  MTLRegion region = MTLRegionMake2D(0, 0, width, height);
  uint8_t *uploadPtr = (uint8_t *)lvl.ptr;

  if (lvl.hasRect) {
    UINT rW = lvl.rect.right - lvl.rect.left;
    UINT rH = lvl.rect.bottom - lvl.rect.top;
    region = MTLRegionMake2D(lvl.rect.left, lvl.rect.top, rW, rH);
    if (isCompressed) {
      UINT pitch = std::max(1u, (width + 3) / 4) * lvl.bytesPerPixel;
      uploadPtr += (lvl.rect.top / 4) * pitch + (lvl.rect.left / 4) * lvl.bytesPerPixel;
    } else {
      uploadPtr += lvl.rect.top * lvl.pitch + lvl.rect.left * lvl.bytesPerPixel;
    }
  }

  if (isCompressed) {
    // For BC compressed formats, bytesPerRow = blocksWide * bytesPerBlock
    UINT bytesPerBlock = lvl.bytesPerPixel; // 8 for DXT1, 16 for DXT3/5
    UINT blocksWide = std::max(1u, (width + 3) / 4);
    UINT bytesPerRow = blocksWide * bytesPerBlock;
    
    // bytesPerImage is not strictly required for 2D textures, but good for validation
    UINT heightInBlocks = std::max(1u, (height + 3) / 4);
    if (lvl.hasRect) {
       heightInBlocks = std::max(1u, ((UINT)(lvl.rect.bottom - lvl.rect.top) + 3) / 4);
    }
    UINT bytesPerImage = bytesPerRow * heightInBlocks;

    if (texUploadLogCount < 200) {
      uint8_t *bytes = uploadPtr;
      printf("TEX_UPLOAD[%d] COMPRESSED fmt=%d level=%u r=%d,%d,%dx%d bpr=%u bpi=%u first4bytes=[%02x %02x %02x %02x]\n",
              texUploadLogCount++, (int)m_Format, Level, 
              (int)region.origin.x, (int)region.origin.y, (int)region.size.width, (int)region.size.height,
              bytesPerRow, bytesPerImage,
              bytes[0], bytes[1], bytes[2], bytes[3]);
      fflush(stdout);
    }

    [tex replaceRegion:region
           mipmapLevel:Level
                 slice:0
             withBytes:uploadPtr
           bytesPerRow:bytesPerRow
         bytesPerImage:bytesPerImage];
  } else {
    if (texUploadLogCount < 200) {
      uint8_t *bytes = (uint8_t*)lvl.ptr;
      printf("TEX_UPLOAD[%d] UNCOMPRESSED fmt=%d level=%u %ux%u pitch=%u bpp=%u first4bytes=[%02x %02x %02x %02x]\n",
              texUploadLogCount++, (int)m_Format, Level, width, height,
              lvl.pitch, lvl.bytesPerPixel,
              bytes[0], bytes[1], bytes[2], bytes[3]);
      fflush(stdout);
    }

    if (lvl.bytesPerPixel == 2 && (m_Format == D3DFMT_A1R5G5B5 || m_Format == D3DFMT_X1R5G5B5 || m_Format == D3DFMT_R5G6B5 || m_Format == D3DFMT_A4R4G4B4)) {
      UINT pixelCount = width * height;
      uint8_t *converted = (uint8_t *)malloc(pixelCount * 4);
      uint16_t *src = (uint16_t *)lvl.ptr;
      for (UINT i = 0; i < pixelCount; i++) {
        uint16_t px = src[i];
        uint8_t a = 255, r = 0, g = 0, b = 0;
        
        switch (m_Format) {
          case D3DFMT_A1R5G5B5:
            a = (px & 0x8000) ? 255 : 0;
            r = ((px >> 10) & 0x1F) * 255 / 31;
            g = ((px >> 5) & 0x1F) * 255 / 31;
            b = (px & 0x1F) * 255 / 31;
            break;
          case D3DFMT_X1R5G5B5:
            a = 255;
            r = ((px >> 10) & 0x1F) * 255 / 31;
            g = ((px >> 5) & 0x1F) * 255 / 31;
            b = (px & 0x1F) * 255 / 31;
            break;
          case D3DFMT_R5G6B5:
            a = 255;
            r = ((px >> 11) & 0x1F) * 255 / 31;
            g = ((px >> 5) & 0x3F) * 255 / 63;
            b = (px & 0x1F) * 255 / 31;
            break;
          case D3DFMT_A4R4G4B4:
            a = ((px >> 12) & 0xF) * 255 / 15;
            r = ((px >> 8) & 0xF) * 255 / 15;
            g = ((px >> 4) & 0xF) * 255 / 15;
            b = (px & 0xF) * 255 / 15;
            break;
          default:
            break;
        }

        converted[i * 4 + 0] = b; // B
        converted[i * 4 + 1] = g; // G
        converted[i * 4 + 2] = r; // R
        converted[i * 4 + 3] = a; // A
      }
      
      [tex replaceRegion:region
             mipmapLevel:Level
               withBytes:converted
             bytesPerRow:width * 4];
      free(converted);
    } else {
      [tex replaceRegion:region
             mipmapLevel:Level
               withBytes:lvl.ptr
             bytesPerRow:lvl.pitch];
    }
  }

  free(lvl.ptr);
  m_LockedLevels.erase(it);
  m_HasBeenWritten = true;

  return D3D_OK;
}

STDMETHODIMP MetalTexture8::AddDirtyRect(CONST RECT *pDirtyRect) {
  return D3D_OK;
}

// ─────────────────────────────────────────────────────
//  Live Surface Sync: re-upload surface data before GPU reads
// ─────────────────────────────────────────────────────

id<MTLTexture> MetalTexture8::GetMTLTexture() {
  if (!m_Texture)
    return nil;
  
  id<MTLTexture> tex = (__bridge id<MTLTexture>)m_Texture;
  
  // Re-upload from any live surface buffers (games write after UnlockRect)
  for (auto &pair : m_LiveSurfaces) {
    UINT level = pair.first;
    LiveSurface &ls = pair.second;
    if (!ls.dataPtr) continue;
    
    MTLPixelFormat mtlFmt = tex.pixelFormat;
    bool needsConvert = (ls.format == D3DFMT_A1R5G5B5 || ls.format == D3DFMT_X1R5G5B5 ||
                         ls.format == D3DFMT_R5G6B5 || ls.format == D3DFMT_A4R4G4B4);
    
    if (needsConvert) {
      // 16-bit → BGRA8 conversion
      UINT pixelCount = ls.width * ls.height;
      uint8_t *converted = (uint8_t *)malloc(pixelCount * 4);
      uint16_t *src = (uint16_t *)ls.dataPtr;
      for (UINT i = 0; i < pixelCount; i++) {
        uint16_t px = src[i];
        uint8_t a = 255, r = 0, g = 0, b = 0;
        switch (ls.format) {
          case D3DFMT_A1R5G5B5:
            a = (px & 0x8000) ? 255 : 0;
            r = ((px >> 10) & 0x1F) * 255 / 31;
            g = ((px >> 5) & 0x1F) * 255 / 31;
            b = (px & 0x1F) * 255 / 31;
            break;
          case D3DFMT_R5G6B5:
            a = 255;
            r = ((px >> 11) & 0x1F) * 255 / 31;
            g = ((px >> 5) & 0x3F) * 255 / 63;
            b = (px & 0x1F) * 255 / 31;
            break;
          default:
            a = 255; r = g = b = 128;
            break;
        }
        converted[i * 4 + 0] = b;
        converted[i * 4 + 1] = g;
        converted[i * 4 + 2] = r;
        converted[i * 4 + 3] = a;
      }
      MTLRegion region = MTLRegionMake2D(0, 0, ls.width, ls.height);
      [tex replaceRegion:region mipmapLevel:level withBytes:converted bytesPerRow:ls.width * 4];
      free(converted);
    } else {
      // Direct upload (BGRA8 → BGRA8)
      UINT bpp = 4;
      MTLRegion region = MTLRegionMake2D(0, 0, ls.width, ls.height);
      [tex replaceRegion:region mipmapLevel:level withBytes:ls.dataPtr bytesPerRow:ls.width * bpp];
    }
  }
  
  return tex;
}

void MetalTexture8::RegisterLiveSurface(UINT level, void *dataPtr, UINT width,
                                         UINT height, UINT pitch, D3DFORMAT surfFmt) {
  LiveSurface ls;
  ls.dataPtr = dataPtr;
  ls.width = width;
  ls.height = height;
  ls.pitch = pitch;
  ls.format = surfFmt;
  m_LiveSurfaces[level] = ls;
}

void MetalTexture8::UnregisterLiveSurface(UINT level) {
  m_LiveSurfaces.erase(level);
}
