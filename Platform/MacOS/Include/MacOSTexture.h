#ifndef MACOSTEXTURE_H
#define MACOSTEXTURE_H

#import "IRenderDevice.h"
#import <Metal/Metal.h>

class MacOSTexture : public ITexture {
public:
  MacOSTexture(int w, int h, int mips, unsigned int usage, WW3DFormat format,
               id<MTLTexture> tex);
  virtual ~MacOSTexture();

  bool LockRect(unsigned int level, LockedRect *pLockedRect,
                const RectClass *pRect = nullptr,
                unsigned int flags = 0) override;
  bool UnlockRect(unsigned int level) override;
  bool GetLevelDesc(unsigned int level, LevelInfo *outInfo) override;

  id<MTLTexture> GetMTLTexture();

private:
  id<MTLTexture> m_Texture;
  int m_Width;
  int m_Height;
  int m_MipLevels;
  unsigned int m_Usage;
  WW3DFormat m_Format;
  void *m_LockedData;
  int m_LockedLevel;
};

#endif
