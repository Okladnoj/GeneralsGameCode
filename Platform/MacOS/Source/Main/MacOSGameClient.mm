#import "MacOSGameClient.h"
#import "Common/GameEngine.h"
#import "GameClient/Display.h"
#import "GameClient/GameClient.h"
#import "GameClient/GameWindowManager.h"
#import "IRenderDevice.h"
#import "MacOSGameWindowManager.h"
#import "MacOSRenderDevice_Internal.h"
#import "StdKeyboard.h"
#import "StdMouse.h"
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

// Engine headers
#include "Common/ArchiveFileSystem.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameFont.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/VideoPlayer.h"
#include <set>
#include <string>

extern "C" void MacOS_PumpEvents();

class MacOSDisplay : public Display {
public:
  MacOSDisplay() {
    fprintf(stderr, "MacOSDisplay ctor %p\n", this);
    fflush(stderr);
  }
  virtual ~MacOSDisplay() {}
  virtual void init(void) override {
    fprintf(stderr, "MacOSDisplay::init %p\n", this);
    int w = 800, h = 600;
    GetRenderDevice()->GetDisplaySize(w, h);
    setWidth(w);
    setHeight(h);
    fprintf(stderr, "MacOSDisplay::init: set dimensions to %dx%d\n", w, h);
    fflush(stderr);
  }
  virtual void reset(void) override {}
  virtual void drawFillRect(Int x, Int y, Int w, Int h,
                            UnsignedInt color) override {
    static int fillCount = 0;
    if (fillCount++ % 100 == 0) {
      fprintf(stderr,
              "MacOSDisplay::drawFillRect: x=%d y=%d w=%d h=%d color=%08X\n", x,
              y, w, h, color);
      fflush(stderr);
    }
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
    AsciiString filename = image->getFilename();
    printf("MacOSDisplay::drawImage: %x, %x -> %x, %x  file: %s\n", (int)sx,
           (int)sy, (int)ex, (int)ey, filename.str());
    fflush(stdout);
    ITexture *tex = GetRenderDevice()->CreateTextureFromFile(filename.str());
    if (!tex) {
      static std::set<std::string> missing;
      if (missing.find(filename.str()) == missing.end()) {
        printf("MISSING TEXTURE: %s\n", filename.str());
        fflush(stdout);
        missing.insert(filename.str());
      }
      return;
    } else {
      GetRenderDevice()->SetTexture(0, tex);
      const Region2D *uv = image->getUV();
      GetRenderDevice()->DrawTexturedRect(sx, sy, ex - sx, ey - sy, uv->lo.x,
                                          uv->lo.y, uv->hi.x, uv->hi.y, color);
      GetRenderDevice()->SetTexture(0, nullptr);
    }
  }
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

  virtual void draw(void) override {
    static int drawCount = 0;
    if (drawCount++ % 100 == 0) {
      fprintf(stderr,
              "MacOSDisplay::draw: calling winRepaint, context=%p, count=%d\n",
              (void *)TheDisplay, drawCount);
      fflush(stderr);
    }
    GetRenderDevice()->BeginScene();
    if (TheWindowManager)
      TheWindowManager->winRepaint();
    GetRenderDevice()->EndScene();
  }
};

class MacOSDisplayString : public DisplayString {
public:
  MEMORY_POOL_GLUE_WITHOUT_GCMP(MacOSDisplayString)
  static MemoryPool *getClassMemoryPool() {
    static MemoryPool *pool =
        TheMemoryPoolFactory->findMemoryPool("DisplayString");
    return pool;
  }

  MacOSDisplayString() {}
  virtual void setWordWrap(Int wordWrap) override {}
  virtual void setWordWrapCentered(Bool isCentered) override {}
  virtual void draw(Int x, Int y, Color color, Color dropColor) override {
    Int len = m_textString.getLength();
    if (len > 0) {
      AsciiString ascii;
      ascii.translate(m_textString);
      printf("MacOSDisplayString::draw: '%s' at %d,%d, len=%d\n", ascii.str(),
             (int)x, (int)y, (int)len);
      fflush(stdout);
    }
    // Safety cap to prevent screen flooding
    if (len > 100)
      len = 100;
    for (Int i = 0; i < len; ++i) {
      TheDisplay->drawFillRect(x + i * 8, y, 6, 12,
                               0xFFFFFF00); // Yellow blocks
    }
  }
  virtual void draw(Int x, Int y, Color color, Color dropColor, Int xDrop,
                    Int yDrop) override {
    draw(x, y, color, dropColor);
  }
  virtual void getSize(Int *width, Int *height) override {
    if (width)
      *width = m_textString.getLength() * 8;
    if (height)
      *height = 14;
  }
  virtual Int getWidth(Int charPos = -1) override {
    if (charPos < 0)
      return m_textString.getLength() * 8;
    return charPos * 8;
  }
  virtual void setUseHotkey(Bool useHotkey, Color hotKeyColor) override {}
};
MacOSDisplayString::~MacOSDisplayString() {}

class MacOSDisplayStringManager : public DisplayStringManager {
public:
  virtual void init(void) override {
    fprintf(stderr, "MacOSDisplayStringManager::init\n");
    fflush(stderr);
    TheMemoryPoolFactory->createMemoryPool(
        "DisplayString", sizeof(MacOSDisplayString), 100, 100);
    DisplayStringManager::init(); // Call base class init
  }
  virtual DisplayString *newDisplayString(void) override {
    return newInstance(MacOSDisplayString);
  }
  virtual void freeDisplayString(DisplayString *string) override {
    deleteInstance(string);
  }
  virtual DisplayString *getGroupNumeralString(Int numeral) override {
    return nullptr;
  }
  virtual DisplayString *getFormationLetterString(void) override {
    return nullptr;
  }
};

class MacOSFontLibrary : public FontLibrary {
public:
  virtual Bool loadFontData(GameFont *font) override { return TRUE; }
};

class MacOSView : public View {
public:
  virtual void init(void) override { View::init(); }
  virtual Drawable *pickDrawable(const ICoord2D *screen, Bool forceAttack,
                                 PickType pickType) override {
    return nullptr;
  }
  virtual Int iterateDrawablesInRegion(IRegion2D *screenRegion,
                                       Bool (*callback)(Drawable *draw,
                                                        void *userData),
                                       void *userData) override {
    return 0;
  }
  virtual void forceRedraw() override {}
  virtual WorldToScreenReturn worldToScreenTriReturn(const Coord3D *w,
                                                     ICoord2D *s) override {
    return WTS_INVALID;
  }
  virtual void screenToTerrain(const ICoord2D *screen,
                               Coord3D *world) override {}
  virtual void screenToWorldAtZ(const ICoord2D *s, Coord3D *w,
                                Real z) override {}
  virtual void drawView(void) override {}
  virtual void updateView(void) override {}
  virtual void stepView() override {}
  virtual const Coord3D &get3DCameraPosition() const override {
    static Coord3D dummy;
    return dummy;
  }
  virtual void setGuardBandBias(const Coord2D *gb) override {}
};

class MacOSInGameUI : public InGameUI {
public:
  virtual void init(void) override {
    fprintf(stderr, "MacOSInGameUI::init %p\n", this);
    fflush(stderr);
    InGameUI::init(); // Call base class init
  }
  virtual void reset(void) override {}
  virtual void update(void) override {}
  virtual void draw(void) override {}
  virtual View *createView(void) override { return new MacOSView(); }
};

class MacOSSnowManager : public SnowManager {
public:
  virtual void init(void) override {
    fprintf(stderr, "MacOSSnowManager::init %p\n", this);
    fflush(stderr);
    SnowManager::init(); // Call base class init
  }
  virtual void reset(void) override {}
  virtual void update(void) override {}
};

class MacOSTerrainVisual : public TerrainVisual {
public:
  virtual void init(void) override {
    fprintf(stderr, "MacOSTerrainVisual::init %p\n", this);
    fflush(stderr);
    TerrainVisual::init(); // Call base class init
  }
  virtual void setRawMapHeight(const ICoord2D *gridPos, Int height) override {}
  virtual Int getRawMapHeight(const ICoord2D *gridPos) override { return 0; }
  virtual void replaceSkyboxTextures(
      const AsciiString *oldTexName[NumSkyboxTextures],
      const AsciiString *newTexName[NumSkyboxTextures]) override {}
  virtual void getTerrainColorAt(Real x, Real y, RGBColor *pColor) override {}
  virtual TerrainType *getTerrainTile(Real x, Real y) override {
    return nullptr;
  }
  virtual void enableWaterGrid(Bool enable) override {}
  virtual void setWaterGridHeightClamps(const WaterHandle *waterTable,
                                        Real minZ, Real maxZ) override {}
  virtual void setWaterAttenuationFactors(const WaterHandle *waterTable, Real a,
                                          Real b, Real c, Real range) override {
  }
  virtual void setWaterTransform(const WaterHandle *waterTable, Real angle,
                                 Real x, Real y, Real z) override {}
  virtual void setWaterTransform(const Matrix3D *transform) override {}
  virtual void getWaterTransform(const WaterHandle *waterTable,
                                 Matrix3D *transform) override {}
  virtual void setWaterGridResolution(const WaterHandle *waterTable,
                                      Real gridCellsX, Real gridCellsY,
                                      Real cellSize) override {}
  virtual void getWaterGridResolution(const WaterHandle *waterTable,
                                      Real *gridCellsX, Real *gridCellsY,
                                      Real *cellSize) override {}
  virtual void changeWaterHeight(Real x, Real y, Real delta) override {}
  virtual void addWaterVelocity(Real worldX, Real worldY, Real velocity,
                                Real preferredHeight) override {}
  virtual Bool getWaterGridHeight(Real worldX, Real worldY,
                                  Real *height) override {
    return FALSE;
  }
  virtual void setTerrainTracksDetail(void) override {}
  virtual void setShoreLineDetail(void) override {}
  virtual void addFactionBib(Object *factionBuilding, Bool highlight,
                             Real extra = 0) override {}
  virtual void removeFactionBib(Object *factionBuilding) override {}
  virtual void addFactionBibDrawable(Drawable *factionBuilding, Bool highlight,
                                     Real extra = 0) override {}
  virtual void removeFactionBibDrawable(Drawable *factionBuilding) override {}
  virtual void removeAllBibs(void) override {}
  virtual void removeBibHighlighting(void) override {}
  virtual void removeTreesAndPropsForConstruction(const Coord3D *pos,
                                                  const GeometryInfo &geom,
                                                  Real angle) override {}
  virtual void addProp(const ThingTemplate *tt, const Coord3D *pos,
                       Real angle) override {}
};

class MacOSVideoPlayer : public VideoPlayer {
public:
  virtual void init(void) override {
    fprintf(stderr, "MacOSVideoPlayer::init\n");
    fflush(stderr);
    VideoPlayer::init();
  }
  virtual void reset(void) override { VideoPlayer::reset(); }
  virtual void update(void) override { VideoPlayer::update(); }
  virtual void deinit(void) override { VideoPlayer::deinit(); }
};

MacOSGameClient::MacOSGameClient() {}

MacOSGameClient::~MacOSGameClient() {}

void MacOSGameClient::update() {
  MacOS_PumpEvents();
  GameClient::update();
}

Drawable *
MacOSGameClient::friend_createDrawable(const ThingTemplate *thing,
                                       DrawableStatusBits statusBits) {
  return nullptr;
}
void MacOSGameClient::addScorch(const Coord3D *pos, Real radius,
                                Scorches type) {}
void MacOSGameClient::createRayEffectByTemplate(const Coord3D *start,
                                                const Coord3D *end,
                                                const ThingTemplate *tmpl) {}
void MacOSGameClient::setTeamColor(Int red, Int green, Int blue) {}
void MacOSGameClient::setTextureLOD(Int level) {}
void MacOSGameClient::notifyTerrainObjectMoved(Object *obj) {}
void MacOSGameClient::releaseShadows(void) {}
void MacOSGameClient::allocateShadows(void) {}

Display *MacOSGameClient::createGameDisplay(void) { return new MacOSDisplay(); }
DisplayStringManager *MacOSGameClient::createDisplayStringManager(void) {
  return new MacOSDisplayStringManager();
}
FontLibrary *MacOSGameClient::createFontLibrary(void) {
  fprintf(stderr, "MacOSGameClient::createFontLibrary called\n");
  return new MacOSFontLibrary();
}
InGameUI *MacOSGameClient::createInGameUI(void) { return new MacOSInGameUI(); }
SnowManager *MacOSGameClient::createSnowManager(void) {
  return new MacOSSnowManager();
}
TerrainVisual *MacOSGameClient::createTerrainVisual(void) {
  return new MacOSTerrainVisual();
}
VideoPlayerInterface *MacOSGameClient::createVideoPlayer(void) {
  return new MacOSVideoPlayer();
}

GameWindowManager *MacOSGameClient::createWindowManager(void) {
  return new MacOSGameWindowManager();
}

Keyboard *MacOSGameClient::createKeyboard(void) { return new StdKeyboard(); }

Mouse *MacOSGameClient::createMouse(void) { return new StdMouse(); }

void MacOSGameClient::setFrameRate(Real msecsPerFrame) {}
