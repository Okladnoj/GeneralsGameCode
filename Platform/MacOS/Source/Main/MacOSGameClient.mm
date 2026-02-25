#import "MacOSGameClient.h"
#import "Common/GameEngine.h"
#import "GameClient/Display.h"
#import "GameClient/GameClient.h"
#import "GameClient/GameWindowManager.h"
#import "MacOSGameWindowManager.h"
#import "MacOSWindowManager.h"
#include "PreRTS.h"
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
#include "GameClient/View.h"
#include "GameClient/Shell.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/W3DInGameUI.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"  // TheTerrainRenderObject

extern "C" {
Display *MacOS_CreateDisplay(void);
DisplayStringManager *MacOS_CreateDisplayStringManager(void);
}

// ─────────────────────────────────────────────────────
//  MacOS Font Library — uses CoreText for font metrics
// ─────────────────────────────────────────────────────
class MacOSFontLibrary : public FontLibrary {
public:
  virtual Bool loadFontData(GameFont *font) override {
    if (!font) return FALSE;
    @autoreleasepool {
      NSString *fontName = [NSString stringWithUTF8String:font->nameString.str()];
      CGFloat pointSize = (CGFloat)font->pointSize;
      NSFont *nsFont = [NSFont fontWithName:fontName size:pointSize];
      if (!nsFont && [fontName isEqualToString:@"Generals"]) {
        nsFont = [NSFont fontWithName:@"Arial-BoldMT" size:pointSize];
      }
      if (!nsFont) {
        nsFont = font->bold
          ? [NSFont boldSystemFontOfSize:pointSize]
          : [NSFont systemFontOfSize:pointSize];
      }
      int pixelHeight = (int)ceil([nsFont ascender] - [nsFont descender] + [nsFont leading]);
      if (pixelHeight < 1) pixelHeight = (int)ceil(pointSize * 96.0 / 72.0);
      font->height = pixelHeight;
      font->fontData = nullptr;
    }
    return TRUE;
  }
};

// ─────────────────────────────────────────────────────
//  Snow Manager — delegates to W3DSnowManager
// ─────────────────────────────────────────────────────
#include "W3DDevice/GameClient/W3DSnow.h"

// ─────────────────────────────────────────────────────
//  Video Player — simple wrapper
// ─────────────────────────────────────────────────────
class MacOSVideoPlayer : public VideoPlayer {
public:
  virtual void init(void) override { VideoPlayer::init(); }
  virtual void reset(void) override { VideoPlayer::reset(); }
  virtual void update(void) override { VideoPlayer::update(); }
  virtual void deinit(void) override { VideoPlayer::deinit(); }
};

// ─────────────────────────────────────────────────────
//  MacOSGameClient
// ─────────────────────────────────────────────────────

MacOSGameClient::MacOSGameClient() {
  printf("[MacOSGameClient] constructor\n"); fflush(stdout);
}
MacOSGameClient::~MacOSGameClient() {}

void MacOSGameClient::init() {
  printf("[MacOSGameClient] init\n"); fflush(stdout);
  GameClient::init();
}

void MacOSGameClient::update() {
  static int callCount = 0;
  if (callCount < 3) {
    printf("[MacOSGameClient] update() #%d\n", callCount);
    fflush(stdout);
  }
  MacOS_PumpEvents();

  if (callCount == 0) {
    TheWritableGlobalData->m_playIntro = FALSE;
    TheWritableGlobalData->m_afterIntro = FALSE;
    TheWritableGlobalData->m_allowExitOutOfMovies = TRUE;
  }

  GameClient::update();

  if (callCount == 0) {
    printf("[MacOSGameClient] Forcing showShellMap + showShell\n"); fflush(stdout);
    TheShell->showShellMap(TRUE);
    TheShell->showShell();
  }
  callCount++;
}

// ─────────────────────────────────────────────────────
//  Factory methods — macOS-specific
// ─────────────────────────────────────────────────────
Display *MacOSGameClient::createGameDisplay(void) { return MacOS_CreateDisplay(); }
DisplayStringManager *MacOSGameClient::createDisplayStringManager(void) { return MacOS_CreateDisplayStringManager(); }
FontLibrary *MacOSGameClient::createFontLibrary(void) { return new MacOSFontLibrary(); }
InGameUI *MacOSGameClient::createInGameUI(void) { return new W3DInGameUI(); }
VideoPlayerInterface *MacOSGameClient::createVideoPlayer(void) { return new MacOSVideoPlayer(); }
GameWindowManager *MacOSGameClient::createWindowManager(void) { return new MacOSGameWindowManager(); }
Keyboard *MacOSGameClient::createKeyboard(void) { return new StdKeyboard(); }
Mouse *MacOSGameClient::createMouse(void) { return new StdMouse(); }

#include "W3DDevice/GameClient/W3DTerrainVisual.h"
TerrainVisual *MacOSGameClient::createTerrainVisual(void) { return new W3DTerrainVisual(); }

void MacOSGameClient::setFrameRate(Real msecsPerFrame) {
  // W3DGameClient stores this in TheW3DFrameLengthInMsec
  // For now, no-op — frame rate governed by display vsync
}

// ─────────────────────────────────────────────────────
//  Game methods — delegate to W3D subsystems
// ─────────────────────────────────────────────────────
#import "GameClient/Drawable.h"

Drawable *MacOSGameClient::friend_createDrawable(const ThingTemplate *thing,
                                                 DrawableStatusBits statusBits) {
  return newInstance(Drawable)(thing, statusBits);
}

void MacOSGameClient::addScorch(const Coord3D *pos, Real radius, Scorches type) {
  if (TheTerrainRenderObject && pos) {
    Vector3 loc(pos->x, pos->y, pos->z);
    TheTerrainRenderObject->addScorch(loc, radius, type);
  }
}

void MacOSGameClient::createRayEffectByTemplate(const Coord3D *start,
                                                 const Coord3D *end,
                                                 const ThingTemplate *tmpl) {
  // W3DGameClient creates W3D line objects here
  // For now log — ray effects (lasers, tracers) not critical for shell map
  static int count = 0;
  if (count++ < 5) {
    printf("[MacOSGameClient] createRayEffect called (stub)\n"); fflush(stdout);
  }
}

void MacOSGameClient::setTeamColor(Int red, Int green, Int blue) {
  // W3DGameClient calls W3DDisplay::setTeamColor
  // Delegate if display supports it
  static bool logged = false;
  if (!logged) {
    printf("[MacOSGameClient] setTeamColor(%d,%d,%d)\n", red, green, blue);
    fflush(stdout);
    logged = true;
  }
}

void MacOSGameClient::setTextureLOD(Int level) {
  // W3DGameClient calls TextureLoader::setMinTextureFilters()
  // No-op for now — Metal handles texture quality automatically
}

void MacOSGameClient::releaseShadows(void) {
  GameClient::releaseShadows();
}

void MacOSGameClient::allocateShadows(void) {
  GameClient::allocateShadows();
}

#if RTS_ZEROHOUR
void MacOSGameClient::notifyTerrainObjectMoved(Object *obj) {
  // W3DGameClient updates terrain LOD when objects move
  // BaseHeightMapRenderObjClass doesn't expose this — safe no-op
}

SnowManager *MacOSGameClient::createSnowManager(void) {
  return new W3DSnowManager();
}
#endif
