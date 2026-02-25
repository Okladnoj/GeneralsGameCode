#include "StdMouse.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "always.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "WW3D2/render2d.h"
#include "WW3D2/texture.h"
#import <AppKit/AppKit.h>

StdMouse::StdMouse(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

StdMouse::~StdMouse(void) {}

void StdMouse::init(void) {
  Mouse::init();
  m_inputMovesAbsolute = TRUE;
  setVisibility(TRUE);
}

void StdMouse::reset(void) {
  Mouse::reset();
  m_inputMovesAbsolute = TRUE;
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

void StdMouse::update(void) { Mouse::update(); }

void StdMouse::initCursorResources(void) {
  // macOS system cursors are usually fine, but we could load custom ones here
}

void StdMouse::setCursor(MouseCursor cursor) {
  // Map engine cursors to macOS cursors
#if defined(__APPLE__) && defined(__OBJC__)
  @autoreleasepool {
    switch (cursor) {
    case ARROW:
    case NORMAL:
      [[NSCursor arrowCursor] set];
      break;
    case CROSS:
      [[NSCursor crosshairCursor] set];
      break;
    case SCROLL:
      [[NSCursor openHandCursor] set];
      break;
    default:
      [[NSCursor arrowCursor] set];
      break;
    }
  }
#endif
}

void StdMouse::setVisibility(Bool visible) {
  m_visible = visible;
  printf("DEBUG: StdMouse::setVisibility(%s) -> m_visible is now %d\n",
         visible ? "TRUE" : "FALSE", (int)m_visible);
  fflush(stdout);
#if defined(__APPLE__) && defined(__OBJC__)
  @autoreleasepool {
    if (visible) {
      [NSCursor unhide];
    } else {
      // Force unhide for now to help debugging
      [NSCursor unhide];
    }
  }
#endif
}

void StdMouse::draw(void) {
  // NOTE: do NOT check m_visible here — that flag controls the OS cursor.
  // W3DMouse::draw() also doesn't check m_visible for DX8/POLYGON modes.

  // --- Full W3DMouse-equivalent cursor rendering (2D texture mode) ---
  // TODO: RM_W3D mode not implemented — 3D model cursors (green targeting
  //   crosshairs, red attack circles, move arrows) are rendered as 2D fallback.
  //   Needs: W3D model loading, ortho camera, WW3D::Render() per-frame.
  // Load multi-frame TGA textures via WW3DAssetManager, animate per FPS,
  // handle directional cursors (8 scroll directions).

  static TextureClass *cursorTextures[NUM_MOUSE_CURSORS][MAX_2D_CURSOR_ANIM_FRAMES] = {};
  static int cursorFrameCount[NUM_MOUSE_CURSORS] = {};
  static Render2DClass *cursorRenderer = nullptr;
  static bool assetsLoaded = false;
  static float animFrame = 0.0f;
  static unsigned int lastAnimTime = 0;

  WW3DAssetManager *am = WW3DAssetManager::Get_Instance();
  if (!assetsLoaded && am) {
    int totalLoaded = 0;
    for (int i = 0; i < NUM_MOUSE_CURSORS; i++) {
      if (m_cursorInfo[i].textureName.isEmpty()) continue;

      const char *baseName = m_cursorInfo[i].textureName.str();
      int numFrames = m_cursorInfo[i].numFrames;
      int numDirs = m_cursorInfo[i].numDirections;
      if (numFrames < 1) numFrames = 1;
      if (numFrames > MAX_2D_CURSOR_ANIM_FRAMES) numFrames = MAX_2D_CURSOR_ANIM_FRAMES;

      // Directional cursors: frameName = "SCCScroll0000.tga" to "SCCScroll0007.tga"
      // Non-directional single: "SCCPointer.tga"
      // Non-directional multi:  "SCCAttack0000.tga", "SCCAttack0001.tga", ...
      int loaded = 0;
      if (numFrames == 1 && numDirs <= 1) {
        char tgaName[128];
        snprintf(tgaName, sizeof(tgaName), "%s.tga", baseName);
        cursorTextures[i][0] = am->Get_Texture(tgaName);
        if (cursorTextures[i][0]) loaded = 1;
      } else {
        int totalFrames = (numDirs > 1) ? numDirs : numFrames;
        if (totalFrames > MAX_2D_CURSOR_ANIM_FRAMES) totalFrames = MAX_2D_CURSOR_ANIM_FRAMES;
        for (int f = 0; f < totalFrames; f++) {
          char tgaName[128];
          snprintf(tgaName, sizeof(tgaName), "%s%04d.tga", baseName, f);
          cursorTextures[i][f] = am->Get_Texture(tgaName);
          if (cursorTextures[i][f]) loaded++;
        }
      }
      cursorFrameCount[i] = loaded;
      totalLoaded += loaded;
    }
    cursorRenderer = new Render2DClass();
    assetsLoaded = true;
    lastAnimTime = timeGetTime();
    printf("[StdMouse] Loaded %d cursor texture frames total\n", totalLoaded);
    fflush(stdout);
  }

  if (!cursorRenderer) return;

  CursorInfo *info = &m_cursorInfo[m_currentCursor];
  int frameCount = cursorFrameCount[m_currentCursor];
  if (frameCount < 1) {
    // No texture for this cursor — fallback green crosshair
    if (TheDisplay) {
      int cx = m_currMouse.pos.x;
      int cy = m_currMouse.pos.y;
      TheDisplay->drawFillRect(cx - 8, cy - 1, 16, 2, GameMakeColor(0, 255, 0, 200));
      TheDisplay->drawFillRect(cx - 1, cy - 8, 2, 16, GameMakeColor(0, 255, 0, 200));
    }
    drawCursorText();
    drawTooltip();
    return;
  }

  // Determine which frame to display
  int frameIdx = 0;
  if (info->numDirections > 1) {
    // Directional cursor (scroll): pick direction frame
    // m_directionFrame is not accessible (W3DMouse member), compute inline
    if (TheInGameUI && TheInGameUI->isScrolling()) {
      Coord2D offset = TheInGameUI->getScrollAmount();
      if (offset.x != 0 || offset.y != 0) {
        Real len = sqrtf(offset.x * offset.x + offset.y * offset.y);
        offset.x /= len; offset.y /= len;
        Real theta = atan2f(offset.y, offset.x);
        theta = fmodf(theta + (float)M_PI * 2.0f, (float)M_PI * 2.0f);
        int numDirs = info->numDirections;
        frameIdx = (int)(theta / (2.0f * (float)M_PI / (float)numDirs) + 0.5f);
        if (frameIdx >= numDirs) frameIdx = 0;
      }
    }
    if (frameIdx >= frameCount) frameIdx = 0;
  } else if (frameCount > 1) {
    // Animated cursor: advance frame by elapsed time and FPS
    unsigned int now = timeGetTime();
    float elapsed = (float)(now - lastAnimTime);
    lastAnimTime = now;
    float fpsMs = info->fps; // already in frames-per-ms from Mouse.ini
    if (fpsMs > 0) {
      animFrame += elapsed * fpsMs;
      animFrame = fmodf(animFrame, (float)frameCount);
    }
    frameIdx = (int)animFrame;
    if (frameIdx >= frameCount) frameIdx = 0;
  }

  TextureClass *tex = cursorTextures[m_currentCursor][frameIdx];
  if (!tex) tex = cursorTextures[m_currentCursor][0]; // fallback to frame 0

  if (tex) {
    int texW = tex->Get_Width();
    int texH = tex->Get_Height();
    if (texW < 1) texW = 32;
    if (texH < 1) texH = 32;
    int x = m_currMouse.pos.x - info->hotSpotPosition.x;
    int y = m_currMouse.pos.y - info->hotSpotPosition.y;

    cursorRenderer->Set_Coordinate_Range(RectClass(0, 0, TheDisplay->getWidth(), TheDisplay->getHeight()));
    cursorRenderer->Reset();
    cursorRenderer->Enable_Texturing(TRUE);
    cursorRenderer->Enable_Alpha(TRUE);
    cursorRenderer->Set_Texture(tex);
    cursorRenderer->Add_Quad(
      RectClass(x, y, x + texW, y + texH),
      RectClass(0.0f, 0.0f, 1.0f, 1.0f)
    );
    cursorRenderer->Render();
  }

  drawCursorText();
  drawTooltip();
}

void StdMouse::capture(void) {
  // Confine cursor to game window — equivalent to Win32 SetCapture + ClipCursor
  CGAssociateMouseAndMouseCursorPosition(false);
  onCursorCaptured(TRUE);
}

void StdMouse::releaseCapture(void) {
  CGAssociateMouseAndMouseCursorPosition(true);
  onCursorCaptured(FALSE);
}

void StdMouse::regainFocus() {
  Mouse::regainFocus();
}

void StdMouse::loseFocus() {
  Mouse::loseFocus();
}

UnsignedByte StdMouse::getMouseEvent(MouseIO *result, Bool flush) {
  static int pollCount = 0;
  if (pollCount++ % 500 == 0) {
    // printf("DEBUG: StdMouse::getMouseEvent polled %d times, buffer
    // size=%d\n",
    //        pollCount, (m_nextFreeIndex - m_nextGetIndex + MAX_EVENTS) %
    //        MAX_EVENTS);
    // fflush(stdout);
  }

  if (m_nextGetIndex == m_nextFreeIndex) {
    if (result) {
      memset(result, 0, sizeof(MouseIO));
      result->pos = m_currMouse.pos;
      result->leftState = m_currMouse.leftState;
      result->rightState = m_currMouse.rightState;
      result->middleState = m_currMouse.middleState;
    }
    return 0;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextGetIndex];
  m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;

  memset(result, 0, sizeof(MouseIO));
  result->pos.x = ev.x;
  result->pos.y = ev.y;
  result->time = ev.time;
  result->wheelPos = ev.wheelDelta;

  switch (ev.type) {
  case MACOS_MOUSE_LBUTTON_DOWN:
    result->leftState = MBS_Down;
    break;
  case MACOS_MOUSE_LBUTTON_UP:
    result->leftState = MBS_Up;
    break;
  case MACOS_MOUSE_RBUTTON_DOWN:
    result->rightState = MBS_Down;
    break;
  case MACOS_MOUSE_RBUTTON_UP:
    result->rightState = MBS_Up;
    break;
  case MACOS_MOUSE_MBUTTON_DOWN:
    result->middleState = MBS_Down;
    break;
  case MACOS_MOUSE_MBUTTON_UP:
    result->middleState = MBS_Up;
    break;
  default:
    break;
  }

  if (ev.type != MACOS_MOUSE_MOVE) {
    printf("INPUT: Mouse Event type=%d pos=(%d,%d) L-State=%d\n", ev.type, ev.x,
           ev.y, (int)result->leftState);
    fflush(stdout);
  }

  return 1;
}

void StdMouse::addEvent(int type, int x, int y, int button, int wheelDelta,
                        unsigned int time) {
  // OPTIMIZATION: If same as last move event, skip to avoid flooding
  static int lastX = -1, lastY = -1;
  if (type == MACOS_MOUSE_MOVE && x == lastX && y == lastY) {
    return;
  }
  if (type == MACOS_MOUSE_MOVE) {
    lastX = x;
    lastY = y;
  }

  // Update position immediately so 'draw' stays in sync
  m_currMouse.pos.x = x;
  m_currMouse.pos.y = y;

  // LOG ALL BUTTON EVENTS
  if (type != MACOS_MOUSE_MOVE) {
    printf("QUEUE: addEvent type=%d at (%d, %d)\n", type, x, y);
    fflush(stdout);
  }

  unsigned int nextIndex = (m_nextFreeIndex + 1) % MAX_EVENTS;
  if (nextIndex == m_nextGetIndex) {
    // Buffer overflow, drop oldest event
    m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextFreeIndex];
  ev.type = type;
  ev.x = x;
  ev.y = y;
  ev.button = button;
  ev.wheelDelta = wheelDelta;
  ev.time = time;

  m_nextFreeIndex = nextIndex;
}
