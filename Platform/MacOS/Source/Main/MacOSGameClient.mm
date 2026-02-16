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

#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameFont.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Snow.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/VideoPlayer.h"

extern "C" {
Display *MacOS_CreateDisplay(void);
DisplayStringManager *MacOS_CreateDisplayStringManager(void);
}

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
  virtual void init(void) override { InGameUI::init(); }
  virtual void reset(void) override {}
  virtual void update(void) override {}
  virtual void draw(void) override {}
  virtual View *createView(void) override { return new MacOSView(); }
};

class MacOSSnowManager : public SnowManager {
public:
  virtual void init(void) override { SnowManager::init(); }
  virtual void reset(void) override {}
  virtual void update(void) override {}
};

class MacOSTerrainVisual : public TerrainVisual {
public:
  virtual void init(void) override { TerrainVisual::init(); }
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
  virtual void init(void) override { VideoPlayer::init(); }
  virtual void reset(void) override { VideoPlayer::reset(); }
  virtual void update(void) override { VideoPlayer::update(); }
  virtual void deinit(void) override { VideoPlayer::deinit(); }
};

MacOSGameClient::MacOSGameClient() {
  printf("DEBUG: MacOSGameClient constructor called!\n");
}
MacOSGameClient::~MacOSGameClient() {}

void MacOSGameClient::init() {
  printf("DEBUG: MacOSGameClient::init called!\n");
  GameClient::init();
}

void MacOSGameClient::update() {
  MacOS_PumpEvents();
  GameClient::update();
}

Display *MacOSGameClient::createGameDisplay(void) {
  printf("DEBUG: MacOSGameClient::createGameDisplay called!\n");
  return MacOS_CreateDisplay();
}

DisplayStringManager *MacOSGameClient::createDisplayStringManager(void) {
  printf("DEBUG: MacOSGameClient::createDisplayStringManager called!\n");
  return MacOS_CreateDisplayStringManager();
}

FontLibrary *MacOSGameClient::createFontLibrary(void) {
  printf("DEBUG: MacOSGameClient::createFontLibrary called!\n");
  return new MacOSFontLibrary();
}

InGameUI *MacOSGameClient::createInGameUI(void) {
  printf("DEBUG: MacOSGameClient::createInGameUI called!\n");
  return new MacOSInGameUI();
}

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
  printf("DEBUG: MacOSGameClient::createWindowManager called!\n");
  return new MacOSGameWindowManager();
}

Keyboard *MacOSGameClient::createKeyboard(void) { return new StdKeyboard(); }
Mouse *MacOSGameClient::createMouse(void) { return new StdMouse(); }
void MacOSGameClient::setFrameRate(Real msecsPerFrame) {}

// Stubs for friend functions
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
