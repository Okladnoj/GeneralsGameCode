#import "MacOSTexture.h"
#include <cstdlib>

MacOSTexture::MacOSTexture(int w, int h, int mips, unsigned int usage,
                           WW3DFormat format, id<MTLTexture> tex)
    : m_Width(w), m_Height(h), m_MipLevels(mips), m_Usage(usage),
      m_Format(format), m_Texture(tex), m_LockedData(nullptr),
      m_LockedLevel(-1) {}

MacOSTexture::~MacOSTexture() { m_Texture = nil; }

bool MacOSTexture::LockRect(unsigned int level, LockedRect *lr,
                            const RectClass *rect, unsigned int flags) {
  if (level >= (unsigned int)m_MipLevels || !lr)
    return false;

  // Track the current locked level and memory
  m_LockedLevel = level;
  int bpp = (m_Format == WW3D_FORMAT_A8R8G8B8) ? 4 : 4;
  int w = std::max(1, m_Width >> level);
  int h = std::max(1, m_Height >> level);
  int rowPitch = w * bpp;
  m_LockedData = malloc(rowPitch * h);
  lr->pBits = m_LockedData;
  lr->Pitch = rowPitch;
  return true;
}

bool MacOSTexture::UnlockRect(unsigned int level) {
  if (level != (unsigned int)m_LockedLevel || !m_LockedData)
    return false;

  int w = std::max(1, m_Width >> level);
  int h = std::max(1, m_Height >> level);

  [m_Texture replaceRegion:MTLRegionMake2D(0, 0, w, h)
               mipmapLevel:level
                 withBytes:m_LockedData
               bytesPerRow:w * 4];

  free(m_LockedData);
  m_LockedData = nullptr;
  return true;
}

bool MacOSTexture::GetLevelDesc(unsigned int level, LevelInfo *outInfo) {
  if (level >= (unsigned int)m_MipLevels || !outInfo)
    return false;
  outInfo->Width = std::max(1, m_Width >> level);
  outInfo->Height = std::max(1, m_Height >> level);
  outInfo->Format = m_Format;
  outInfo->Depth = 1;
  return true;
}

id<MTLTexture> MacOSTexture::GetMTLTexture() { return m_Texture; }
