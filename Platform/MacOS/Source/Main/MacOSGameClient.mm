#import "MacOSGameClient.h"
#include "Common/AsciiString.h"
#include "Common/CommandLine.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#import "Common/GameType.h"
#include "Common/GlobalData.h"
#import "Common/Snapshot.h"
#include "Common/UnicodeString.h"
#import "GameClient/Display.h"
#import "GameClient/DisplayString.h"
#import "GameClient/DisplayStringManager.h"
#import "GameClient/Drawable.h"
#import "GameClient/GameFont.h"
#import "GameClient/GameWindowManager.h"
#import "GameClient/InGameUI.h"
#import "GameClient/Keyboard.h"
#import "GameClient/Mouse.h"
#import "GameClient/Snow.h"
#import "GameClient/TerrainVisual.h"
#import "GameClient/VideoPlayer.h"
#import "IRenderDevice.h"
#import "MacOSGameWindowManager.h"
#import "MacOSRenderer.h"
#include "PreRTS.h"

class MacOSView : public View {
public:
  MacOSView() {}
  virtual void init(void) override {}
  virtual void reset(void) override {}
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
  virtual const Coord3D &get3DCameraPosition() const override {
    static Coord3D pos = {0, 0, 0};
    return pos;
  }
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
  virtual void setGuardBandBias(const Coord2D *gb) override {}
};

class MacOSInGameUI : public InGameUI {
public:
  MacOSInGameUI() {}
  virtual void draw(void) override {}
  virtual View *createView(void) override { return new MacOSView(); }
};

extern "C" void *MacOS_CreateWindow(int width, int height, const char *title);

extern "C" void *ApplicationHWnd;

class MacOSDisplay : public Display {
public:
  MacOSDisplay() {
    fprintf(stderr, "MacOSDisplay constructor EXECUTING\n");
    fflush(stderr);
    m_MovieStartTime = 0;
  }
  virtual void init(void) override {
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr,
            "MacOSDisplay::init(void) EXECUTING (TheDisplay ptr addr=%p)\n",
            (void *)&TheDisplay);
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fflush(stderr);

    int width = m_width > 0 ? m_width : 800;
    int height = m_height > 0 ? m_height : 600;
    if (TheGlobalData) {
      width = TheGlobalData->m_xResolution;
      height = TheGlobalData->m_yResolution;
    }
    m_width = width;
    m_height = height;

    void *window = nullptr;
    fprintf(stderr, "MacOSDisplay::init: Creating window %dx%d\n", width,
            height);
    fflush(stderr);
    window = MacOS_CreateWindow(width, height, "C&C Generals (macOS)");
    ApplicationHWnd = window;
    fprintf(stderr,
            "MacOSDisplay::init: window created at %p, ApplicationHWnd set\n",
            window);
    fflush(stderr);

    if (GetRenderDevice()) {
      fprintf(stderr,
              "MacOSDisplay::init: Initializing RenderDevice with window %p\n",
              window);
      fflush(stderr);
      GetRenderDevice()->Initialize(window);
    }
  }
  virtual void reset(void) override {}
  virtual Bool setDisplayMode(UnsignedInt xres, UnsignedInt yres,
                              UnsignedInt bitdepth, Bool windowed) override {
    m_width = xres;
    m_height = yres;
    m_bitDepth = bitdepth;
    m_windowed = windowed;
    return TRUE;
  }
  virtual void doSmartAssetPurgeAndPreload(const char *usageFileName) override {
  }
#if defined(RTS_DEBUG)
  virtual void dumpAssetUsage(const char *mapname) override {}
#endif
  virtual VideoBuffer *createVideoBuffer(void) override { return nullptr; }
  virtual void setClipRegion(IRegion2D *region) override {}
  virtual Bool isClippingEnabled(void) override { return FALSE; }
  virtual void enableClipping(Bool onoff) override {}

  virtual void setTimeOfDay(TimeOfDay tod) override {}
  virtual void createLightPulse(const Coord3D *pos, const RGBColor *color,
                                Real innerRadius, Real attenuationWidth,
                                UnsignedInt increaseFrameTime,
                                UnsignedInt decayFrameTime) override {}

  virtual void drawLine(Int startX, Int startY, Int endX, Int endY,
                        Real lineWidth, UnsignedInt lineColor) override {}
  virtual void drawLine(Int startX, Int startY, Int endX, Int endY,
                        Real lineWidth, UnsignedInt lineColor1,
                        UnsignedInt lineColor2) override {}
  virtual void drawOpenRect(Int startX, Int startY, Int width, Int height,
                            Real lineWidth, UnsignedInt lineColor) override {}

  virtual void drawFillRect(Int startX, Int startY, Int width, Int height,
                            UnsignedInt color) override {
    GetRenderDevice()->DrawFillRect((float)startX, (float)startY, (float)width,
                                    (float)height, color);
  }

  virtual void drawRectClock(Int startX, Int startY, Int width, Int height,
                             Int percent, UnsignedInt color) override {}
  virtual void drawRemainingRectClock(Int startX, Int startY, Int width,
                                      Int height, Int percent,
                                      UnsignedInt color) override {}

  virtual void drawImage(const Image *image, Int startX, Int startY, Int endX,
                         Int endY, Color color = 0xFFFFFFFF,
                         DrawImageMode mode = DRAW_IMAGE_ALPHA) override {}
  virtual void drawScaledVideoBuffer(VideoBuffer *buffer,
                                     VideoStreamInterface *stream) override {}
  virtual void drawVideoBuffer(VideoBuffer *buffer, Int startX, Int startY,
                               Int endX, Int endY) override {}

  virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting) override {
  }
  virtual void clearShroud() override {}
  virtual void setBorderShroudLevel(UnsignedByte level) override {}

#if defined(RTS_DEBUG)
  virtual void dumpModelAssets(const char *path) override {}
#endif
  virtual void preloadModelAssets(AsciiString model) override {}
  virtual void preloadTextureAssets(AsciiString texture) override {}

  virtual void takeScreenShot(void) override {}
  virtual void toggleMovieCapture(void) override {}
  virtual void toggleLetterBox(void) override {}
  virtual void enableLetterBox(Bool enable) override {}

  virtual void playLogoMovie(AsciiString movieName, Int minMovieLength,
                             Int minCopyrightLength) override {
    fprintf(stderr, "MacOSDisplay::playLogoMovie: %s\n", movieName.str());
    m_MovieStartTime = [NSDate timeIntervalSinceReferenceDate];
  }

  virtual Bool isMoviePlaying(void) override {
    if (m_MovieStartTime > 0) {
      double elapsed =
          [NSDate timeIntervalSinceReferenceDate] - m_MovieStartTime;
      if (elapsed < 3.0) { // 3 seconds warm-up
        return TRUE;
      }
      m_MovieStartTime = 0;
    }
    return FALSE;
  }

  virtual Real getAverageFPS(void) override { return 60.0f; }
  virtual Real getCurrentFPS(void) override { return 60.0f; }
  virtual Int getLastFrameDrawCalls(void) override { return 0; }

  virtual void draw(void) override {
    static int drawCount = 0;
    if (drawCount++ % 1 == 0) {
      fprintf(stderr, "MacOSDisplay::draw: frame %d\n", drawCount);
      fflush(stderr);
    }
    GetRenderDevice()->BeginScene();
    fprintf(stderr, "MacOSDisplay::draw: calling base Display::draw()...\n");
    fflush(stderr);
    Display::draw(); // Draw views
    fprintf(stderr, "MacOSDisplay::draw: base Display::draw() returned.\n");
    fflush(stderr);
    if (TheWindowManager) {
      TheWindowManager->winRepaint();
    }
    // Draw a test rectangle to verify DrawFillRect
    drawFillRect(10, 10, 100, 100, 0xFFFF0000); // Red rect
    GetRenderDevice()->EndScene();
  }
  NSTimeInterval m_MovieStartTime;
};

class MacOSKeyboard : public Keyboard {
public:
  virtual Bool getCapsState(void) override { return FALSE; }

protected:
  virtual void getKey(KeyboardIO *key) override { key->key = KEY_NONE; }
};

class MacOSFontLibrary : public FontLibrary {
public:
  virtual Bool loadFontData(GameFont *font) override { return TRUE; }
};

class MacOSDisplayString : public DisplayString {
public:
  MacOSDisplayString() {}
  virtual ~MacOSDisplayString() {}
  virtual void setWordWrap(Int wordWrap) override {}
  virtual void setWordWrapCentered(Bool isCentered) override {}
  virtual void draw(Int x, Int y, Color color, Color dropColor) override {
    AsciiString ascii;
    ascii.translate(m_textString);
    GetRenderDevice()->DrawDebugText(ascii.str(), (float)x, (float)y, color);
  }
  virtual void draw(Int x, Int y, Color color, Color dropColor, Int xDrop,
                    Int yDrop) override {
    AsciiString ascii;
    ascii.translate(m_textString);
    // Draw shadow first
    GetRenderDevice()->DrawDebugText(ascii.str(), (float)(x + xDrop),
                                     (float)(y + yDrop), dropColor);
    // Then main text
    GetRenderDevice()->DrawDebugText(ascii.str(), (float)x, (float)y, color);
  }
  virtual void getSize(Int *width, Int *height) override {
    if (width)
      *width = m_textString.getLength() * 10;
    if (height)
      *height = 18;
  }
  virtual Int getWidth(Int charPos = -1) override {
    if (charPos < 0)
      return m_textString.getLength() * 10;
    return charPos * 10;
  }
  virtual void setUseHotkey(Bool useHotkey, Color hotKeyColor) override {}
};

class MacOSDisplayStringManager : public DisplayStringManager {
public:
  virtual DisplayString *newDisplayString(void) override {
    return new MacOSDisplayString();
  }
  virtual void freeDisplayString(DisplayString *string) override {
    deleteInstance(string);
  }
  virtual DisplayString *getGroupNumeralString(Int numeral) override {
    return new MacOSDisplayString();
  }
  virtual DisplayString *getFormationLetterString(void) override {
    return new MacOSDisplayString();
  }
};

class MacOSVideoPlayer : public VideoPlayer {
public:
  virtual void init(void) override {}
};

class MacOSTerrainVisual : public TerrainVisual {
public:
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
  virtual void setRawMapHeight(const ICoord2D *gridPos, Int height) override {}
  virtual Int getRawMapHeight(const ICoord2D *gridPos) override { return 0; }
  virtual void replaceSkyboxTextures(
      const AsciiString *oldTexName[NumSkyboxTextures],
      const AsciiString *newTexName[NumSkyboxTextures]) override {}
};

class MacOSSnowManager : public SnowManager {
public:
  virtual void update(void) override {}
};

MacOSGameClient::MacOSGameClient() {}
MacOSGameClient::~MacOSGameClient() {}

void MacOSGameClient::update(void) {
  static int updateCount = 0;
  if (updateCount++ % 100 == 0) {
    fprintf(stderr,
            "MacOSGameClient::update: frame %d, TheDisplay=%p, intro=%d, "
            "afterIntro=%d, noDraw=%d\n",
            updateCount, (void *)TheDisplay, (int)TheGlobalData->m_playIntro,
            (int)TheGlobalData->m_afterIntro, (int)TheGlobalData->m_noDraw);
    fflush(stderr);
  }
  GameClient::update();
}

Drawable *
MacOSGameClient::friend_createDrawable(const ThingTemplate *thing,
                                       DrawableStatusBits statusBits) {
  return nullptr; // Will implement later
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

void Global_InitMacOSDisplay() {
  if (TheDisplay) {
    TheDisplay->init();
  }
}
InGameUI *MacOSGameClient::createInGameUI(void) { return new MacOSInGameUI(); }
GameWindowManager *MacOSGameClient::createWindowManager(void) {
  return new MacOSGameWindowManager();
}
FontLibrary *MacOSGameClient::createFontLibrary(void) {
  return new MacOSFontLibrary();
}
DisplayStringManager *MacOSGameClient::createDisplayStringManager(void) {
  return new MacOSDisplayStringManager();
}
VideoPlayerInterface *MacOSGameClient::createVideoPlayer(void) {
  return new MacOSVideoPlayer();
}
TerrainVisual *MacOSGameClient::createTerrainVisual(void) {
  return new MacOSTerrainVisual();
}
Keyboard *MacOSGameClient::createKeyboard(void) { return new MacOSKeyboard(); }
Mouse *MacOSGameClient::createMouse(void) { return new MouseDummy(); }
SnowManager *MacOSGameClient::createSnowManager(void) {
  return new MacOSSnowManager();
}
void MacOSGameClient::setFrameRate(Real msecsPerFrame) {}
