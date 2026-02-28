#import <AppKit/AppKit.h>
#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/UnicodeString.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameFont.h"
#include "Lib/BaseType.h"

// DX8 includes for rendering through the Metal pipeline
#include "WW3D2/dx8wrapper.h"
#include "d3d8.h"

#include "MacOSDebugLog.h"

class MacOSDisplayString : public DisplayString {
public:
  void *operator new(size_t size, const char *msg) {
    return getClassMemoryPool()->allocateBlock(msg);
  }
  void operator delete(void *p) { getClassMemoryPool()->freeBlock(p); }
  virtual MemoryPool *getObjectMemoryPool() override {
    return getClassMemoryPool();
  }
  static MemoryPool *getClassMemoryPool() {
    static MemoryPool *pool =
        TheMemoryPoolFactory->findMemoryPool("DisplayString");
    return pool;
  }

  MacOSDisplayString();
  virtual ~MacOSDisplayString();

  void invalidate() { m_Stale = true; }

  virtual void setText(UnicodeString text) override {
    m_textString = text;
    invalidate();
  }
  virtual void setFont(GameFont *font) override {
    m_font = font;
    invalidate();
  }
  virtual void setWordWrap(Int wordWrap) override { invalidate(); }
  virtual void setWordWrapCentered(Bool isCentered) override { invalidate(); }
  virtual void notifyTextChanged(void) override { invalidate(); }

  void updateTexture() {
    if (!m_Stale && m_D3DTexture)
      return;
    if (m_textString.getLength() == 0)
      return;

    AsciiString ascii;
    ascii.translate(m_textString);
    NSString *nsText = [NSString stringWithUTF8String:ascii.str()];
    if (!nsText)
      return;

    // TheSuperHackers @fix macOS: Use CoreText + CoreGraphics for text
    // rendering instead of AppKit (NSGraphicsContext/NSBitmapImageRep).
    // AppKit text APIs internally schedule performSelector:onMainThread:
    // callbacks on the RunLoop. CoreGraphics bitmap contexts are purely
    // CPU-side and never interact with the RunLoop.

    float fontSize = 16.0f;
    if (m_font && m_font->pointSize > 0)
      fontSize = (float)m_font->pointSize;

    // Create CoreText font and attributes
    CTFontRef ctFont = CTFontCreateWithName(CFSTR("Arial-BoldMT"), fontSize, NULL);
    if (!ctFont)
      ctFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, fontSize, NULL);

    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGFloat whiteComps[] = {1.0, 1.0, 1.0, 1.0};
    CGColorRef whiteColor = CGColorCreate(cs, whiteComps);

    NSDictionary *ctAttrs = @{
      (__bridge id)kCTFontAttributeName : (__bridge id)ctFont,
      (__bridge id)kCTForegroundColorAttributeName : (__bridge id)whiteColor,
    };
    NSAttributedString *attrStr = [[NSAttributedString alloc]
        initWithString:nsText
            attributes:ctAttrs];

    // Measure text size using CTFramesetter
    CTFramesetterRef framesetter =
        CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef)attrStr);
    CGSize suggestedSize = CTFramesetterSuggestFrameSizeWithConstraints(
        framesetter, CFRangeMake(0, 0), NULL,
        CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX), NULL);

    if (suggestedSize.width <= 0 || suggestedSize.height <= 0) {
      CFRelease(framesetter);
      CGColorRelease(whiteColor);
      CGColorSpaceRelease(cs);
      CFRelease(ctFont);
      return;
    }

    m_Width = (int)ceil(suggestedSize.width) + 4;
    m_Height = (int)ceil(suggestedSize.height) + 4;

    // Release old texture if size changed
    if (m_D3DTexture) {
      D3DSURFACE_DESC desc;
      m_D3DTexture->GetLevelDesc(0, &desc);
      if (desc.Width != (unsigned int)m_Width ||
          desc.Height != (unsigned int)m_Height) {
        m_D3DTexture->Release();
        m_D3DTexture = nullptr;
      }
    }

    // Create new texture via DX8 pipeline (MetalDevice8)
    if (!m_D3DTexture) {
      IDirect3DDevice8 *dev = DX8Wrapper::_Get_D3D_Device8();
      if (!dev) {
        CFRelease(framesetter);
        CGColorRelease(whiteColor);
        CGColorSpaceRelease(cs);
        CFRelease(ctFont);
        return;
      }
      HRESULT hr = dev->CreateTexture(m_Width, m_Height, 1, 0, D3DFMT_A8R8G8B8,
                                      D3DPOOL_MANAGED, &m_D3DTexture);
      if (FAILED(hr) || !m_D3DTexture) {
        CFRelease(framesetter);
        CGColorRelease(whiteColor);
        CGColorSpaceRelease(cs);
        CFRelease(ctFont);
        return;
      }
    }

    // Render text to bitmap and upload to texture
    D3DLOCKED_RECT lr;
    if (m_D3DTexture->LockRect(0, &lr, nullptr, 0) == D3D_OK) {
      // Create BGRA bitmap context — matches Metal's BGRA8Unorm directly
      CGContextRef ctx = CGBitmapContextCreate(
          NULL, m_Width, m_Height, 8, m_Width * 4, cs,
          kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);

      if (ctx) {
        CGContextClearRect(ctx, CGRectMake(0, 0, m_Width, m_Height));

        // Flip to top-down coordinates (origin at top-left, like AppKit)
        CGContextTranslateCTM(ctx, 0, m_Height);
        CGContextScaleCTM(ctx, 1.0, -1.0);

        // CRITICAL: Reset text matrix to identity for CoreText.
        // Without this, the flipped CTM causes each glyph to be mirrored.
        CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);

        // Create a frame for the text in the full rect
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddRect(path, NULL, CGRectMake(2, 0, m_Width - 4, m_Height));
        CTFrameRef frame = CTFramesetterCreateFrame(
            framesetter, CFRangeMake(0, 0), path, NULL);
        CTFrameDraw(frame, ctx);
        CFRelease(frame);
        CGPathRelease(path);

        // Copy pixels to texture — BGRA matches Metal directly
        // CG data buffer is bottom-up in memory, we need top-down
        unsigned char *src = (unsigned char *)CGBitmapContextGetData(ctx);
        unsigned char *dst = (unsigned char *)lr.pBits;

        if (src && dst) {
          int srcPitch = (int)CGBitmapContextGetBytesPerRow(ctx);
          int dstPitch = lr.Pitch;

          for (int y = 0; y < m_Height; y++) {
            unsigned char *srcRow = src + (m_Height - 1 - y) * srcPitch;
            unsigned char *dstRow = dst + y * dstPitch;

            for (int x = 0; x < m_Width; x++) {
              unsigned char b = srcRow[x * 4 + 0];
              unsigned char g = srcRow[x * 4 + 1];
              unsigned char r = srcRow[x * 4 + 2];
              unsigned char a = srcRow[x * 4 + 3];

              // Unpremultiply alpha
              if (a > 0 && a < 255) {
                dstRow[x * 4 + 0] = (unsigned char)((int)b * 255 / a);
                dstRow[x * 4 + 1] = (unsigned char)((int)g * 255 / a);
                dstRow[x * 4 + 2] = (unsigned char)((int)r * 255 / a);
              } else {
                dstRow[x * 4 + 0] = b;
                dstRow[x * 4 + 1] = g;
                dstRow[x * 4 + 2] = r;
              }
              dstRow[x * 4 + 3] = a;
            }
          }
        }

        CGContextRelease(ctx);
      }
      m_D3DTexture->UnlockRect(0);
    }

    CFRelease(framesetter);
    CGColorRelease(whiteColor);
    CGColorSpaceRelease(cs);
    CFRelease(ctFont);
    m_Stale = false;
  }

  virtual void draw(Int x, Int y, Color color, Color dropColor) override {
    if (m_textString.getLength() == 0)
      return;

    AsciiString ascii;
    ascii.translate(m_textString);
    DLOG_RFLOW(8, "MacOSDisplayString::draw x=%d y=%d color=0x%08X text='%s' w=%d h=%d",
      x, y, (unsigned)color, ascii.str(), m_Width, m_Height);

    updateTexture();



    if (m_D3DTexture) {
      // Draw through the DX8/Metal pipeline using TheDisplay
      // For now, use DX8Wrapper directly to set texture and draw a quad
      float fx = (float)x - 2.0f;
      float fy = (float)y;

      // Use the W3D pipeline: DX8Wrapper::Set_Texture + Render2DClass
      DX8Wrapper::Set_Texture(0, nullptr);
      IDirect3DDevice8 *dev = DX8Wrapper::_Get_D3D_Device8();
      if (dev) {
        dev->SetTexture(0, m_D3DTexture);
      }

      // Extract color components
      float a = ((color >> 24) & 0xFF) / 255.0f;
      float r = ((color >> 16) & 0xFF) / 255.0f;
      float g = ((color >> 8) & 0xFF) / 255.0f;
      float b = (color & 0xFF) / 255.0f;

      // Draw a textured quad using DX8Wrapper
      struct TexturedVertex {
        float x, y, z, rhw;
        DWORD diffuse;
        float u, v;
      };

      DWORD d3dColor = color; // Already in D3DCOLOR format (ARGB)

      TexturedVertex verts[4] = {
          {fx, fy, 0.0f, 1.0f, d3dColor, 0.0f, 0.0f},
          {fx + m_Width, fy, 0.0f, 1.0f, d3dColor, 1.0f, 0.0f},
          {fx, fy + m_Height, 0.0f, 1.0f, d3dColor, 0.0f, 1.0f},
          {fx + m_Width, fy + m_Height, 0.0f, 1.0f, d3dColor, 1.0f, 1.0f},
      };

      // Backup states
      DWORD oldAlphaBlend, oldSrcBlend, oldDstBlend;
      DWORD oldColorOp, oldColorArg1, oldColorArg2;
      DWORD oldAlphaOp, oldAlphaArg1, oldAlphaArg2;
      dev->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlphaBlend);
      dev->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
      dev->GetRenderState(D3DRS_DESTBLEND, &oldDstBlend);
      dev->GetTextureStageState(0, D3DTSS_COLOROP, &oldColorOp);
      dev->GetTextureStageState(0, D3DTSS_COLORARG1, &oldColorArg1);
      dev->GetTextureStageState(0, D3DTSS_COLORARG2, &oldColorArg2);
      dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &oldAlphaOp);
      dev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &oldAlphaArg1);
      dev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &oldAlphaArg2);

      // Set correct states for text rendering
      dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
      dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
      
      dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
      
      dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
      dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

      dev->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);



      dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts,
                           sizeof(TexturedVertex));
                           
      // Restore states
      dev->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlphaBlend);
      dev->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
      dev->SetRenderState(D3DRS_DESTBLEND, oldDstBlend);
      dev->SetTextureStageState(0, D3DTSS_COLOROP, oldColorOp);
      dev->SetTextureStageState(0, D3DTSS_COLORARG1, oldColorArg1);
      dev->SetTextureStageState(0, D3DTSS_COLORARG2, oldColorArg2);
      dev->SetTextureStageState(0, D3DTSS_ALPHAOP, oldAlphaOp);
      dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, oldAlphaArg1);
      dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, oldAlphaArg2);
      
      dev->SetTexture(0, nullptr);
    }
  }

  virtual void draw(Int x, Int y, Color color, Color dropColor, Int xDrop,
                    Int yDrop) override {
    draw(x, y, color, dropColor);
  }

  virtual void getSize(Int *width, Int *height) override {
    updateTexture();
    AsciiString ascii;
    ascii.translate(m_textString);
    DLOG_RFLOW(8, "MacOSDisplayString::getSize w=%d h=%d text='%s' stale=%d tex=%p",
      m_Width, m_Height, ascii.str(), (int)m_Stale, (void*)m_D3DTexture);
    if (width)
      *width = m_Width;
    if (height)
      *height = m_Height;
  }

  virtual Int getWidth(Int charPos = -1) override {
    updateTexture();
    return m_Width;
  }

  virtual void setUseHotkey(Bool useHotkey, Color hotKeyColor) override {}

private:
  IDirect3DTexture8 *m_D3DTexture;
  int m_Width, m_Height;
  bool m_Stale;
};

MacOSDisplayString::MacOSDisplayString()
    : m_D3DTexture(nullptr), m_Width(0), m_Height(0), m_Stale(true) {}

MacOSDisplayString::~MacOSDisplayString() {
  if (m_D3DTexture) {
    m_D3DTexture->Release();
    m_D3DTexture = nullptr;
  }
}

class MacOSDisplayStringManager : public DisplayStringManager {
public:
  MacOSDisplayStringManager() { TheDisplayStringManager = this; }
  virtual void init(void) override {
    TheMemoryPoolFactory->createMemoryPool(
        "DisplayString", sizeof(MacOSDisplayString), 100, 100);
    DisplayStringManager::init();
  }
  virtual DisplayString *newDisplayString(void) override {
    MacOSDisplayString *s = new ("DisplayString") MacOSDisplayString();
    link(s);
    return s;
  }
  virtual void freeDisplayString(DisplayString *string) override {
    if (!string) return;
    unLink(string);
    deleteInstance(string);
  }
  virtual DisplayString *getGroupNumeralString(Int numeral) override {
    return nullptr;
  }
  virtual DisplayString *getFormationLetterString(void) override {
    return nullptr;
  }
};

extern "C" DisplayStringManager *MacOS_CreateDisplayStringManager(void) {
  return (DisplayStringManager *)new MacOSDisplayStringManager();
}
