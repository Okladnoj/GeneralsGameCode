/**
 * D3DXStubs.mm — D3DX8 helper function implementations for macOS
 *
 * All texture loading goes through MetalDevice8 (DX8-compatible adapter).
 * The old MacOSRenderDevice pipeline is no longer used.
 */
#include "d3d8.h"
#include "d3dx8.h"
#include <windows.h>  // macOS Win32 type shim
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#import <Metal/Metal.h>

#include "MetalDevice8.h"
#include "MetalTexture8.h"

// Forward declarations
extern "C" IDirect3D8 *CreateMetalInterface8();
extern "C" IDirect3DDevice8 *CreateMetalDevice8();

// File system for loading from .big archives
#include "Common/File.h"
#include "Common/FileSystem.h"

// ═══════════════════════════════════════════════════════════════
//  TGA/DDS loading helpers (moved from MacOSRenderer.mm)
// ═══════════════════════════════════════════════════════════════

#pragma pack(push, 1)
struct TGAHeader {
  uint8_t IDLength;
  uint8_t ColorMapType;
  uint8_t ImageType;
  uint16_t CMapStart;
  uint16_t CMapLength;
  uint8_t CMapDepth;
  uint16_t XOffset;
  uint16_t YOffset;
  uint16_t Width;
  uint16_t Height;
  uint8_t PixelDepth;
  uint8_t ImageDescriptor;
};

struct DDS_PIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwABitMask;
};

struct DDS_HEADER {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwHeight;
  uint32_t dwWidth;
  uint32_t dwPitchOrLinearSize;
  uint32_t dwDepth;
  uint32_t dwMipMapCount;
  uint32_t dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  uint32_t dwCaps;
  uint32_t dwCaps2;
  uint32_t dwCaps3;
  uint32_t dwCaps4;
  uint32_t dwReserved2;
};
#pragma pack(pop)

static void DecompressDXT1(int w, int h, const uint8_t *src, uint8_t *dst) {
  int bw = (w + 3) / 4, bh = (h + 3) / 4;
  for (int by = 0; by < bh; by++) {
    for (int bx = 0; bx < bw; bx++) {
      uint16_t c0 = src[0] | (src[1] << 8);
      uint16_t c1 = src[2] | (src[3] << 8);
      uint32_t bits = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
      src += 8;
      uint8_t colors[4][4];
      colors[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
      colors[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
      colors[0][2] = (c0 & 0x1F) * 255 / 31;
      colors[0][3] = 255;
      colors[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
      colors[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
      colors[1][2] = (c1 & 0x1F) * 255 / 31;
      colors[1][3] = 255;
      if (c0 > c1) {
        for (int i = 0; i < 3; i++) {
          colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
          colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
        }
        colors[2][3] = colors[3][3] = 255;
      } else {
        for (int i = 0; i < 3; i++)
          colors[2][i] = (colors[0][i] + colors[1][i]) / 2;
        colors[2][3] = 255;
        colors[3][0] = colors[3][1] = colors[3][2] = 0;
        colors[3][3] = 0;
      }
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          int x = bx * 4 + px, y = by * 4 + py;
          if (x >= w || y >= h) {
            bits >>= 2;
            continue;
          }
          int idx = bits & 3;
          bits >>= 2;
          int off = (y * w + x) * 4;
          // Output BGRA (for MTLPixelFormatBGRA8Unorm)
          dst[off + 0] = colors[idx][2]; // B
          dst[off + 1] = colors[idx][1]; // G
          dst[off + 2] = colors[idx][0]; // R
          dst[off + 3] = colors[idx][3]; // A
        }
      }
    }
  }
}

static void DecompressDXT3(int w, int h, const uint8_t *src, uint8_t *dst) {
  int bw = (w + 3) / 4, bh = (h + 3) / 4;
  for (int by = 0; by < bh; by++) {
    for (int bx = 0; bx < bw; bx++) {
      // DXT3 Alpha Block: 64 bits (4 bits per pixel * 16 pixels)
      uint16_t alphaRow[4];
      for (int i = 0; i < 4; i++) {
        alphaRow[i] = src[i*2] | (src[i*2 + 1] << 8);
      }
      src += 8;
      
      // DXT1/3/5 Color Block: 64 bits
      uint16_t c0 = src[0] | (src[1] << 8);
      uint16_t c1 = src[2] | (src[3] << 8);
      uint32_t bits = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
      src += 8;
      
      uint8_t colors[4][3];
      colors[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
      colors[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
      colors[0][2] = (c0 & 0x1F) * 255 / 31;
      colors[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
      colors[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
      colors[1][2] = (c1 & 0x1F) * 255 / 31;
      for (int i = 0; i < 3; i++) {
        colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
        colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
      }
      
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          int x = bx * 4 + px, y = by * 4 + py;
          if (x >= w || y >= h) {
            bits >>= 2;
            continue;
          }
          int ci = bits & 3;
          bits >>= 2;
          
          uint8_t a = (alphaRow[py] >> (px * 4)) & 0xF;
          a = a | (a << 4); // scale to 255
          
          int off = (y * w + x) * 4;
          dst[off + 0] = colors[ci][2]; // B
          dst[off + 1] = colors[ci][1]; // G
          dst[off + 2] = colors[ci][0]; // R
          dst[off + 3] = a;             // A
        }
      }
    }
  }
}

static void DecompressDXT5(int w, int h, const uint8_t *src, uint8_t *dst) {
  int bw = (w + 3) / 4, bh = (h + 3) / 4;
  for (int by = 0; by < bh; by++) {
    for (int bx = 0; bx < bw; bx++) {
      uint8_t a0 = src[0], a1 = src[1];
      uint64_t abits = 0;
      for (int i = 0; i < 6; i++)
        abits |= (uint64_t)src[2 + i] << (8 * i);
      src += 8;
      uint8_t alphas[8];
      alphas[0] = a0;
      alphas[1] = a1;
      if (a0 > a1) {
        for (int i = 1; i <= 6; i++)
          alphas[1 + i] = ((7 - i) * a0 + i * a1) / 7;
      } else {
        for (int i = 1; i <= 4; i++)
          alphas[1 + i] = ((5 - i) * a0 + i * a1) / 5;
        alphas[6] = 0;
        alphas[7] = 255;
      }
      uint16_t c0 = src[0] | (src[1] << 8);
      uint16_t c1 = src[2] | (src[3] << 8);
      uint32_t bits = src[4] | (src[5] << 8) | (src[6] << 16) | (src[7] << 24);
      src += 8;
      uint8_t colors[4][3];
      colors[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
      colors[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
      colors[0][2] = (c0 & 0x1F) * 255 / 31;
      colors[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
      colors[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
      colors[1][2] = (c1 & 0x1F) * 255 / 31;
      for (int i = 0; i < 3; i++) {
        colors[2][i] = (2 * colors[0][i] + colors[1][i]) / 3;
        colors[3][i] = (colors[0][i] + 2 * colors[1][i]) / 3;
      }
      for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
          int x = bx * 4 + px, y = by * 4 + py;
          if (x >= w || y >= h) {
            bits >>= 2;
            abits >>= 3;
            continue;
          }
          int ci = bits & 3;
          bits >>= 2;
          int ai = abits & 7;
          abits >>= 3;
          int off = (y * w + x) * 4;
          dst[off + 0] = colors[ci][2]; // B
          dst[off + 1] = colors[ci][1]; // G
          dst[off + 2] = colors[ci][0]; // R
          dst[off + 3] = alphas[ai];    // A
        }
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  File loading — reads from .big archives via TheFileSystem
// ═══════════════════════════════════════════════════════════════
static bool LoadFileData(const char *filename,
                         std::vector<unsigned char> &data) {
  const char *prefixes[] = {"",
                            "Art/",
                            "Art/Textures/",
                            "Data/",
                            "Data/English/Art/Textures/",
                            "Data/Window/",
                            "Window/",
                            "assets/"};
  for (int i = 0; i < 8; i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s%s", prefixes[i], filename);
    for (char *c = path; *c; c++)
      if (*c == '\\')
        *c = '/';
    if (TheFileSystem) {
      File *gf = TheFileSystem->openFile(path);
      if (gf) {
        size_t s = gf->size();
        data.resize(s);
        gf->read(data.data(), s);
        gf->close();
        return true;
      }
    }
  }
  return false;
}

// ═══════════════════════════════════════════════════════════════
//  Texture cache (avoids reloading same file)
// ═══════════════════════════════════════════════════════════════
static std::map<std::string, IDirect3DTexture8 *> s_TextureCache;

// ═══════════════════════════════════════════════════════════════
//  D3DX Functions
// ═══════════════════════════════════════════════════════════════

extern "C" {

HRESULT WINAPI D3DXCreateTexture(IDirect3DDevice8 *pDevice, UINT Width,
                                 UINT Height, UINT MipLevels, DWORD Usage,
                                 D3DFORMAT Format, D3DPOOL Pool,
                                 IDirect3DTexture8 **ppTexture) {
  if (!ppTexture || !pDevice)
    return E_POINTER;
  
  static int s_createCount = 0;
  s_createCount++;
  if (true) {
    fprintf(stderr, "[D3DXCreateTexture] #%d: %ux%u fmt=%u mips=%u usage=0x%x pool=%u dev=%p\n",
            s_createCount, Width, Height, (unsigned)Format, MipLevels, 
            (unsigned)Usage, (unsigned)Pool, (void*)pDevice);
  }
  
  HRESULT hr = pDevice->CreateTexture(Width, Height, MipLevels, Usage, Format, Pool, ppTexture);
  
  if (true) {
    fprintf(stderr, "[D3DXCreateTexture] #%d: result=0x%x tex=%p\n",
            s_createCount, (unsigned)hr, ppTexture ? (void*)*ppTexture : nullptr);
  }
  return hr;
}

HRESULT WINAPI D3DXCreateTextureFromFileExA(
    IDirect3DDevice8 *pDevice, const char *pSrcFile, UINT Width, UINT Height,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter,
    DWORD MipFilter, D3DCOLOR ColorKey, void *pSrcInfo, void *pPalette,
    IDirect3DTexture8 **ppTexture) {
  if (!ppTexture)
    return E_POINTER;
  *ppTexture = nullptr;

  static int s_fromFileCount = 0;
  s_fromFileCount++;
  if (s_fromFileCount <= 30) {
    fprintf(stderr, "[D3DXCreateTextureFromFileExA] #%d: '%s'\n",
            s_fromFileCount, pSrcFile ? pSrcFile : "(null)");
  }

  if (!pSrcFile || !pSrcFile[0])
    return E_FAIL;

  // Check cache first
  auto it = s_TextureCache.find(pSrcFile);
  if (it != s_TextureCache.end()) {
    it->second->AddRef();
    *ppTexture = it->second;
    return D3D_OK;
  }

  // Load file data from .big archives or filesystem
  std::vector<unsigned char> fileData;
  if (!LoadFileData(pSrcFile, fileData)) {
    return E_FAIL;
  }

  // Detect format and load
  // DDS?
  if (fileData.size() >= 4 && fileData[0] == 'D' && fileData[1] == 'D' &&
      fileData[2] == 'S' && fileData[3] == ' ') {
    DDS_HEADER *dh = (DDS_HEADER *)(fileData.data() + 4);
    int dw = dh->dwWidth, dh_h = dh->dwHeight;
    uint32_t fourcc = dh->ddspf.dwFourCC;
    const uint32_t FOURCC_DXT1 = 0x31545844;
    const uint32_t FOURCC_DXT3 = 0x33545844;
    const uint32_t FOURCC_DXT5 = 0x35545844;

    if (fourcc == FOURCC_DXT1 || fourcc == FOURCC_DXT3 ||
        fourcc == FOURCC_DXT5) {
      // Decompress to BGRA8 and upload
      std::vector<uint8_t> rgba(dw * dh_h * 4);
      const uint8_t *src = (const uint8_t *)(dh + 1);
      if (fourcc == FOURCC_DXT1)
        DecompressDXT1(dw, dh_h, src, rgba.data());
      else if (fourcc == FOURCC_DXT3)
        DecompressDXT3(dw, dh_h, src, rgba.data());
      else if (fourcc == FOURCC_DXT5)
        DecompressDXT5(dw, dh_h, src, rgba.data());

      IDirect3DTexture8 *tex = nullptr;
      HRESULT hr = pDevice->CreateTexture(dw, dh_h, 1, 0, D3DFMT_A8R8G8B8,
                                          D3DPOOL_MANAGED, &tex);
      if (FAILED(hr) || !tex)
        return E_FAIL;

      D3DLOCKED_RECT lr;
      if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
        for (int y = 0; y < dh_h; y++) {
          memcpy((uint8_t *)lr.pBits + y * lr.Pitch, rgba.data() + y * dw * 4,
                 dw * 4);
        }
        tex->UnlockRect(0);
      }
      s_TextureCache[pSrcFile] = tex;
      tex->AddRef(); // one for cache, one for caller
      *ppTexture = tex;
      return D3D_OK;
    }

    // Uncompressed DDS - check if it has RGB data
    if (dh->ddspf.dwFlags & 0x40) { // DDPF_RGB
      int dw2 = dh->dwWidth, dh2 = dh->dwHeight;
      int bpp = dh->ddspf.dwRGBBitCount / 8;
      const uint8_t *src = (const uint8_t *)(dh + 1);

      IDirect3DTexture8 *tex = nullptr;
      HRESULT hr = pDevice->CreateTexture(dw2, dh2, 1, 0, D3DFMT_A8R8G8B8,
                                          D3DPOOL_MANAGED, &tex);
      if (FAILED(hr) || !tex)
        return E_FAIL;

      D3DLOCKED_RECT lr;
      if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
        for (int y = 0; y < dh2; y++) {
          const uint8_t *sline = src + y * dw2 * bpp;
          uint8_t *dline = (uint8_t *)lr.pBits + y * lr.Pitch;
          for (int x = 0; x < dw2; x++) {
            uint8_t b = sline[x * bpp + 0];
            uint8_t g = sline[x * bpp + 1];
            uint8_t r = sline[x * bpp + 2];
            uint8_t a = (bpp >= 4) ? sline[x * bpp + 3] : 255;
            if (ColorKey != 0) {
              uint32_t px_rgb = (r << 16) | (g << 8) | b;
              uint32_t ck_rgb = ColorKey & 0xFFFFFF;
              if (px_rgb == ck_rgb) {
                r = g = b = a = 0;
              }
            }
            dline[x * 4 + 0] = b;
            dline[x * 4 + 1] = g;
            dline[x * 4 + 2] = r;
            dline[x * 4 + 3] = a;
          }
        }
        tex->UnlockRect(0);
      }
      s_TextureCache[pSrcFile] = tex;
      tex->AddRef();
      *ppTexture = tex;
      return D3D_OK;
    }

    return E_FAIL;
  }

  // TGA
  if (fileData.size() < sizeof(TGAHeader))
    return E_FAIL;

  TGAHeader *hdr = (TGAHeader *)fileData.data();
  int w = hdr->Width, h = hdr->Height, bpp = hdr->PixelDepth;
  int type = hdr->ImageType;

  if (w <= 0 || h <= 0 || (bpp != 8 && bpp != 24 && bpp != 32))
    return E_FAIL;

  unsigned char *src_ptr = (unsigned char *)(hdr + 1) + hdr->IDLength;

  // Palette for 8-bit
  struct RGBCol {
    uint8_t b, g, r, a;
  };
  std::vector<RGBCol> palette(256);
  if (hdr->ColorMapType == 1) {
    int len = hdr->CMapLength;
    int depth = hdr->CMapDepth;
    int db = depth / 8;
    unsigned char *p_ptr = src_ptr;
    src_ptr += len * db;
    for (int i = 0; i < len; i++) {
      palette[i].b = p_ptr[i * db + 0];
      palette[i].g = p_ptr[i * db + 1];
      palette[i].r = p_ptr[i * db + 2];
      palette[i].a = (db == 4) ? p_ptr[i * db + 3] : 255;
    }
  }

  int bv = bpp / 8;
  std::vector<unsigned char> decomp;
  if (type == 9 || type == 10 || type == 11) { // RLE
    decomp.resize(w * h * bv);
    unsigned char *dst = decomp.data();
    int pixels = 0, total = w * h;
    while (pixels < total) {
      unsigned char ch = *src_ptr++;
      if (ch & 0x80) {
        int n = (ch & 0x7F) + 1;
        for (int j = 0; j < n && pixels < total; j++) {
          memcpy(dst + pixels * bv, src_ptr, bv);
          pixels++;
        }
        src_ptr += bv;
      } else {
        int n = ch + 1;
        for (int j = 0; j < n && pixels < total; j++) {
          memcpy(dst + pixels * bv, src_ptr, bv);
          src_ptr += bv;
          pixels++;
        }
      }
    }
    src_ptr = decomp.data();
  }

  // Convert to BGRA
  std::vector<unsigned char> rgba(w * h * 4);
  bool flipY = !(hdr->ImageDescriptor & 0x20);
  for (int y = 0; y < h; y++) {
    int sy = flipY ? (h - 1 - y) : y;
    const unsigned char *sline = src_ptr + (sy * w * bv);
    unsigned char *dline = rgba.data() + (y * w * 4);
    for (int x = 0; x < w; x++) {
      if (bv == 1 && hdr->ColorMapType == 1) {
        uint8_t idx = sline[x];
        dline[x * 4 + 0] = palette[idx].b;
        dline[x * 4 + 1] = palette[idx].g;
        dline[x * 4 + 2] = palette[idx].r;
        dline[x * 4 + 3] = palette[idx].a;
      } else {
        uint8_t b = sline[x * bv + 0];
        uint8_t g = sline[x * bv + 1];
        uint8_t r = sline[x * bv + 2];
        uint8_t a = (bv == 4) ? sline[x * bv + 3] : 255;
        if (ColorKey != 0) {
          uint32_t px_rgb = (r << 16) | (g << 8) | b;
          uint32_t ck_rgb = ColorKey & 0xFFFFFF;
          if (px_rgb == ck_rgb) {
            r = g = b = a = 0;
          }
        }
        dline[x * 4 + 0] = b; // B
        dline[x * 4 + 1] = g; // G
        dline[x * 4 + 2] = r; // R
        dline[x * 4 + 3] = a;
      }
    }
  }

  IDirect3DTexture8 *tex = nullptr;
  HRESULT hr = pDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8,
                                      D3DPOOL_MANAGED, &tex);
  if (FAILED(hr) || !tex)
    return E_FAIL;

  D3DLOCKED_RECT lr;
  if (tex->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
    for (int y = 0; y < h; y++) {
      memcpy((uint8_t *)lr.pBits + y * lr.Pitch, rgba.data() + y * w * 4,
             w * 4);
    }
    tex->UnlockRect(0);
  }

  s_TextureCache[pSrcFile] = tex;
  tex->AddRef();
  *ppTexture = tex;
  return D3D_OK;
}

} // extern "C"

// Legacy wrappers — no longer needed
IDirect3DTexture8 *CreateMacOSD3DTextureWrapper(void *tex) { return nullptr; }

IDirect3DSurface8 *CreateMacOSD3DSurfaceWrapper(void *tex, int level) {
  return nullptr;
}

// ═══════════════════════════════════════════════════════════════
//  More D3DX Functions
// ═══════════════════════════════════════════════════════════════

extern "C" {


HRESULT WINAPI D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *pDestSurface,
                                          const void *pDestPalette,
                                          const RECT *pDestRect,
                                          IDirect3DSurface8 *pSrcSurface,
                                          const void *pSrcPalette,
                                          const RECT *pSrcRect, DWORD Filter,
                                          DWORD ColorKey) {
  D3DLOCKED_RECT srcLR, destLR;
  if (pSrcSurface->LockRect(&srcLR, pSrcRect, 0) == 0) {
    if (pDestSurface->LockRect(&destLR, pDestRect, 0) == 0) {
      D3DSURFACE_DESC desc;
      pDestSurface->GetDesc(&desc);
      for (unsigned int y = 0; y < desc.Height; ++y) {
        memcpy((char *)destLR.pBits + y * destLR.Pitch,
               (char *)srcLR.pBits + y * srcLR.Pitch, desc.Width * 4);
      }
      pDestSurface->UnlockRect();
    }
    pSrcSurface->UnlockRect();
  }
  return D3D_OK;
}

HRESULT WINAPI D3DXCreateCubeTexture(IDirect3DDevice8 *pDevice, UINT Size,
                                     UINT MipLevels, DWORD Usage,
                                     D3DFORMAT Format, D3DPOOL Pool,
                                     IDirect3DCubeTexture8 **ppCubeTexture) {
  return E_NOTIMPL;
}

HRESULT WINAPI D3DXCreateVolumeTexture(
    IDirect3DDevice8 *pDevice, UINT Width, UINT Height, UINT Depth,
    UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool,
    IDirect3DVolumeTexture8 **ppVolumeTexture) {
  return E_NOTIMPL;
}


class MockD3DXBuffer : public ID3DXBuffer {
  ULONG m_ref = 1;
  void *m_data;
  DWORD m_size;

public:
  MockD3DXBuffer(const void *data, DWORD size) : m_size(size) {
    m_data = calloc(1, size > 0 ? size : 1);
    if (data && size > 0)
      memcpy(m_data, data, size);
  }
  virtual ~MockD3DXBuffer() { free(m_data); }

  STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj) override {
    if (!ppvObj)
      return E_POINTER;
    *ppvObj = this; // Minimal mock
    AddRef();
    return S_OK;
  }
  STDMETHODIMP_(ULONG) AddRef() override { return ++m_ref; }
  STDMETHODIMP_(ULONG) Release() override {
    ULONG r = --m_ref;
    if (r == 0)
      delete this;
    return r;
  }
  STDMETHODIMP_(void *) GetBufferPointer() override { return m_data; }
  STDMETHODIMP_(DWORD) GetBufferSize() override { return m_size; }
};

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors) {
  if (ppConstants) {
    *ppConstants = nullptr;
  }
  if (ppCompilationErrors) {
    *ppCompilationErrors = nullptr;
  }
  if (!ppCompiledShader) {
    return S_OK;
  }

  // The real D3DXAssembleShader compiles PS assembly text to bytecode.
  // On macOS we don't need real D3D bytecode, but CreatePixelShader
  // needs to parse the output to classify the shader type.
  //
  // We analyze the ASCII source to determine which water shader this is,
  // then emit minimal valid PS 1.1 bytecode with the right opcodes for
  // CreatePixelShader's heuristic classifier:
  //   - texbem present → PS_WATER_BUMP (env-mapped reflection water)
  //   - 4x tex + mad → PS_WATER_TRAPEZOID (standing water with sparkles)
  //   - 4x tex + no mad → PS_WATER_RIVER (river water with sparkles)
  //   - Other patterns: emit generic bytecode that matches existing terrain PS types

  std::string src;
  if (pSrcData && SrcDataLen > 0) {
    src = std::string((const char *)pSrcData, SrcDataLen);
  }

  // Count opcodes in the ASCII source
  int texCount = 0;
  bool hasTexbem = false;
  bool hasMad = false;
  bool hasMul = false;
  bool hasAdd = false;
  bool hasLrp = false;
  bool hasDp3 = false;

  // Simple line-by-line analysis 
  size_t pos = 0;
  while (pos < src.size()) {
    size_t eol = src.find('\n', pos);
    if (eol == std::string::npos) eol = src.size();
    std::string line = src.substr(pos, eol - pos);
    pos = eol + 1;

    // Strip comments (;) and leading whitespace
    size_t semi = line.find(';');
    if (semi != std::string::npos) line = line.substr(0, semi);
    size_t first = line.find_first_not_of(" \t\r");
    if (first == std::string::npos) continue;
    line = line.substr(first);

    // Skip version directive and empty lines
    if (line.empty() || line[0] == '\0') continue;
    if (line.substr(0, 2) == "ps") continue; // ps.1.1 version line

    // Check for opcodes
    if (line.substr(0, 6) == "texbem") { hasTexbem = true; texCount++; }
    else if (line.substr(0, 3) == "tex") { texCount++; }
    else if (line.substr(0, 3) == "mad") { hasMad = true; }
    else if (line.substr(0, 3) == "mul") { hasMul = true; }
    else if (line.substr(0, 3) == "add") { hasAdd = true; }
    else if (line.substr(0, 3) == "lrp") { hasLrp = true; }
    else if (line.substr(0, 3) == "dp3") { hasDp3 = true; }
  }

  // Build minimal PS 1.1 bytecode with correct opcodes for classification.
  // PS 1.1 bytecode format:
  //   DWORD version = 0xFFFF0101 (PS 1.1)
  //   DWORD instruction tokens...
  //   DWORD end = 0x0000FFFF
  //
  // tex t0:     opcode 0x40, dest reg (t0)
  // texbem t2,t1: opcode 0x41, dest reg, src reg
  // mul r0,r1:  opcode 0x05, dest, src, src  (skip=3)
  // mad r0,r1,r2,r3: opcode 0x05, dest, src, src, src (skip=4 — but same opcode 0x05)
  //   Actually: mul = 0x05, mad = 0x04? No — in PS1.1 bytecode:
  //   mul = 0x05, add = 0x02, mad = 0x04, dp3 = 0x09, lrp = 0x12
  //
  // We'll emit enough tokens for the classifier to count correctly.

  std::vector<DWORD> bytecode;
  bytecode.push_back(0xFFFF0101); // PS 1.1 version

  // Emit tex instructions
  for (int i = 0; i < texCount; i++) {
    if (hasTexbem && i == texCount - 1) {
      // texbem: opcode 0x41 with dest + src (skip=2)
      bytecode.push_back(0x00000041); // texbem opcode
      bytecode.push_back(0x800F0800 + (uint32_t)i); // dest: t[i]
      bytecode.push_back(0x80E40800 + (uint32_t)(i > 0 ? i - 1 : 0)); // src: t[i-1]
    } else {
      // tex: opcode 0x40 with dest only (skip=1)
      bytecode.push_back(0x00000040); // tex opcode
      bytecode.push_back(0x800F0800 + (uint32_t)i); // dest: t[i]
    }
  }

  // Emit arithmetic instructions
  if (hasMul) {
    bytecode.push_back(0x00000005); // mul
    bytecode.push_back(0x800F0800); // dest r0
    bytecode.push_back(0x80E40000); // src v0
    bytecode.push_back(0x80E40800); // src t0
  }
  if (hasMad) {
    bytecode.push_back(0x00000004); // mad (opcode 4 = mad in PS1.x)
    bytecode.push_back(0x800F0800); // dest r0
    bytecode.push_back(0x80E40801); // src t1
    bytecode.push_back(0x80E40802); // src t2
    bytecode.push_back(0x80E40800); // src r0
  }
  if (hasAdd) {
    bytecode.push_back(0x00000002); // add
    bytecode.push_back(0x800F0800); // dest
    bytecode.push_back(0x80E40800); // src
  }
  if (hasLrp) {
    bytecode.push_back(0x00000012); // lrp
    bytecode.push_back(0x800F0800);
    bytecode.push_back(0x80E40000);
    bytecode.push_back(0x80E40800);
    bytecode.push_back(0x80E40801);
  }
  if (hasDp3) {
    bytecode.push_back(0x00000009); // dp3
    bytecode.push_back(0x800F0800);
    bytecode.push_back(0x80E40800);
    bytecode.push_back(0x80E40000);
  }

  bytecode.push_back(0x0000FFFF); // end token

  DWORD byteSize = (DWORD)(bytecode.size() * sizeof(DWORD));
  *ppCompiledShader = new MockD3DXBuffer(bytecode.data(), byteSize);

  return S_OK;
}

} // extern "C"

// D3DXFilterTexture is declared with C++ linkage in d3dx8core.h
HRESULT WINAPI D3DXFilterTexture(IDirect3DBaseTexture8 *pTexture,
                                  const void *pPalette, DWORD SrcLevel,
                                  DWORD Filter) {
  if (!pTexture) return E_POINTER;
  
  // All our textures are MetalTexture8 — safe to static_cast
  MetalTexture8 *mtlTex = static_cast<MetalTexture8 *>(
      static_cast<IDirect3DTexture8 *>(pTexture));
  if (!mtlTex) return D3D_OK;
  
  id<MTLTexture> tex = mtlTex->GetMTLTexture();
  if (!tex || tex.mipmapLevelCount <= 1) return D3D_OK;
  
  // Use Metal blit encoder to generate mipmaps from level 0
  id<MTLDevice> device = tex.device;
  id<MTLCommandQueue> queue = [device newCommandQueue];
  if (!queue) return E_FAIL;
  
  id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
  if (!cmdBuf) return E_FAIL;
  
  id<MTLBlitCommandEncoder> blit = [cmdBuf blitCommandEncoder];
  [blit generateMipmapsForTexture:tex];
  [blit endEncoding];
  
  [cmdBuf commit];
  [cmdBuf waitUntilCompleted];
  
  static int s_filterCount = 0;
  if (s_filterCount < 20) {
    printf("[D3DXFilterTexture] Generated mipmaps for %lux%lu tex=%p levels=%lu\n",
           (unsigned long)tex.width, (unsigned long)tex.height,
           (__bridge void*)tex, (unsigned long)tex.mipmapLevelCount);
    fflush(stdout);
    s_filterCount++;
  }
  
  return D3D_OK;
}

// ═══════════════════════════════════════════════════════════════
//  Entry Points — called from dx8wrapper.cpp
// ═══════════════════════════════════════════════════════════════

extern "C" IDirect3D8 *CreateMacOSD3D8() { return CreateMetalInterface8(); }

extern "C" IDirect3DDevice8 *CreateMacOSD3DDevice8() {
  return CreateMetalDevice8();
}

// ═══════════════════════════════════════════════════════════════
//  Stubs for removed MacOSRenderDevice pipeline
// ═══════════════════════════════════════════════════════════════

// Previously defined in MacOSRenderer.mm — no longer needed since
// all rendering goes through MetalDevice8 (DX8 pipeline).
extern "C" void MacOS_InitRenderer(void *windowHandle) {
  // No-op: old MacOSRenderDevice initialization removed
}

extern "C" void MacOS_Render() {
  // No-op: old MacOSRenderDevice render removed
}
