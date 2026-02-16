#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "GameClient/Mouse.h"
#include "IRenderDevice.h"
#include "Lib/BaseType.h"
#import "MacOSBuffers.h"
#import "MacOSGameWindowManager.h"
#import "MacOSRenderDevice_Internal.h"
#import "MacOSTexture.h"
#include "PreRTS.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "WW3D2/ww3d.h"

class MacOSDisplay : public Display {
public:
  MacOSDisplay() {
    TheDisplay = this;
    printf("DEBUG: MacOSDisplay created and assigned to TheDisplay at %p\n",
           this);
    fflush(stdout);
  }
  virtual ~MacOSDisplay() {
    if (TheDisplay == this)
      TheDisplay = nullptr;
  }

  virtual void init(void) override {
    printf("DEBUG: MacOSDisplay::init called\n");
    fflush(stdout);

    if (W3DDisplay::m_3DScene == nullptr) {
      printf("DEBUG: MacOSDisplay::init: Creating RTS3DScene...\n");
      fflush(stdout);
      W3DDisplay::m_3DScene = new RTS3DScene();
      printf("DEBUG: MacOSDisplay::init: Created RTS3DScene=%p\n",
             (void *)W3DDisplay::m_3DScene);
      fflush(stdout);
    }
    if (W3DDisplay::m_2DScene == nullptr) {
      printf("DEBUG: MacOSDisplay::init: Creating RTS2DScene...\n");
      fflush(stdout);
      W3DDisplay::m_2DScene = new RTS2DScene();
      printf("DEBUG: MacOSDisplay::init: Created RTS2DScene=%p\n",
             (void *)W3DDisplay::m_2DScene);
      fflush(stdout);
    }
    if (W3DDisplay::m_3DInterfaceScene == nullptr) {
      printf("DEBUG: MacOSDisplay::init: Creating RTS3DInterfaceScene...\n");
      fflush(stdout);
      W3DDisplay::m_3DInterfaceScene = new RTS3DInterfaceScene();
      printf("DEBUG: MacOSDisplay::init: Created RTS3DInterfaceScene=%p\n",
             (void *)W3DDisplay::m_3DInterfaceScene);
      fflush(stdout);
    }
    if (W3DDisplay::m_assetManager == nullptr) {
      printf("DEBUG: MacOSDisplay::init: Creating W3DAssetManager...\n");
      fflush(stdout);
      W3DDisplay::m_assetManager = new W3DAssetManager();
      printf("DEBUG: MacOSDisplay::init: Created W3DAssetManager=%p\n",
             (void *)W3DDisplay::m_assetManager);
      fflush(stdout);
    }

    printf("DEBUG: MacOSDisplay::init: Calling W3DShaderManager::init()...\n");
    fflush(stdout);
    W3DShaderManager::init();
    printf("DEBUG: MacOSDisplay::init finished\n");
    fflush(stdout);

    int w = 800, h = 600;
    GetRenderDevice()->GetDisplaySize(w, h);
    setWidth(w);
    setHeight(h);
  }

  virtual void reset(void) override { Display::reset(); }

  virtual void drawFillRect(Int x, Int y, Int w, Int h,
                            UnsignedInt color) override {
    GetRenderDevice()->SetTexture(0, nullptr);
    GetRenderDevice()->DrawFillRect((float)x, (float)y, (float)w, (float)h,
                                    color);
  }

  virtual void drawLine(Int x1, Int y1, Int x2, Int y2, Real width,
                        UnsignedInt color) override {
    GetRenderDevice()->SetTexture(0, nullptr);
    GetRenderDevice()->DrawLine((float)x1, (float)y1, (float)x2, (float)y2,
                                (float)width, color);
  }

  virtual void drawLine(Int x1, Int y1, Int x2, Int y2, Real width,
                        UnsignedInt color1, UnsignedInt color2) override {}
  virtual void drawOpenRect(Int x, Int y, Int w, Int h, Real width,
                            UnsignedInt color) override {}
  virtual void drawRectClock(Int x, Int y, Int w, Int h, Int percent,
                             UnsignedInt color) override {}
  virtual void drawRemainingRectClock(Int x, Int y, Int w, Int h, Int percent,
                                      UnsignedInt color) override {}

  virtual void drawImage(const Image *image, Int sx, Int sy, Int ex, Int ey,
                         Color color, DrawImageMode mode) override {
    if (!image)
      return;
    const char *filename = image->getFilename().str();

    static int imgCount = 0;
    if (imgCount++ % 100 == 0) {
      printf("DEBUG: MacOSDisplay::drawImage '%s' at (%d,%d)\n", filename,
             (int)sx, (int)sy);
      fflush(stdout);
    }

    ITexture *tex = GetRenderDevice()->CreateTextureFromFile(filename);
    if (tex) {
      GetRenderDevice()->SetTexture(0, tex);
      auto uv = image->getUV();
      if (uv) {
        GetRenderDevice()->DrawTexturedRect(
            (float)sx, (float)sy, (float)(ex - sx), (float)(ey - sy), uv->lo.x,
            uv->lo.y, uv->hi.x, uv->hi.y, color);
      }
      GetRenderDevice()->SetTexture(0, nullptr);
    }
  }

  virtual void draw(void) override {
    if (++m_FrameCount % 1000 == 1) {
      printf("DEBUG: MacOSDisplay::draw heartbeat frame %d\n", m_FrameCount);
      fflush(stdout);
    }

    GetRenderDevice()->BeginScene();

    // Call base class draw to render views (tactical map, etc)
    Display::draw();

    // Repaint windows (GUI)
    if (TheWindowManager) {
      TheWindowManager->winRepaint();
    }

    if (TheMouse) {
      TheMouse->draw();
    }

    GetRenderDevice()->EndScene();
  }

  static int m_FrameCount;

  // Stubs for required virtuals from Display.h
  virtual void drawScaledVideoBuffer(VideoBuffer *buffer,
                                     VideoStreamInterface *stream) override {}
  virtual void drawVideoBuffer(VideoBuffer *buffer, Int sx, Int sy, Int ex,
                               Int ey) override {}
  virtual void setClipRegion(IRegion2D *region) override {}
  virtual Bool isClippingEnabled(void) override { return FALSE; }
  virtual void enableClipping(Bool onoff) override {}
  virtual void setTimeOfDay(TimeOfDay tod) override {}
  virtual void createLightPulse(const Coord3D *pos, const RGBColor *color,
                                Real innerRadius, Real attenuationWidth,
                                UnsignedInt increaseFrameTime,
                                UnsignedInt decayFrameTime) override {}
  virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting) override {
  }
  virtual void clearShroud() override {}
  virtual void setBorderShroudLevel(UnsignedByte level) override {}
  virtual void preloadModelAssets(AsciiString model) override {}
  virtual void preloadTextureAssets(AsciiString texture) override {}
  virtual void takeScreenShot(void) override {}
  virtual void toggleMovieCapture(void) override {}
  virtual void toggleLetterBox(void) override {}
  virtual void enableLetterBox(Bool enable) override {}
  virtual Real getAverageFPS(void) override { return 60.0f; }
  virtual Real getCurrentFPS(void) override { return 60.0f; }
  virtual Int getLastFrameDrawCalls(void) override { return 0; }
  virtual void doSmartAssetPurgeAndPreload(const char *usageFileName) override {
  }
  virtual VideoBuffer *createVideoBuffer(void) override { return nullptr; }
  virtual void dumpAssetUsage(const char *mapname) override {}
  virtual void dumpModelAssets(const char *path) override {}
  virtual void update(void) override {}
};

extern "C" Display *MacOS_CreateDisplay(void) {
  Display *d = new MacOSDisplay();
  printf("DEBUG: MacOS_CreateDisplay CALL returning %p\n", d);
  fflush(stdout);
  return d;
}

int MacOSDisplay::m_FrameCount = 0;
