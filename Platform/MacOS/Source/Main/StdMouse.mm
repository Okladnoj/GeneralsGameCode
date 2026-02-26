#include "StdMouse.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "always.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/Common/W3DConvert.h"
#include "WW3D2/render2d.h"
#include "WW3D2/texture.h"
#include "WW3D2/hanim.h"
#include "WW3D2/camera.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/rendobj.h"
#import <AppKit/AppKit.h>

StdMouse::StdMouse(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
  m_w3dCamera = nullptr;
  m_currentW3DCursor = NONE;
  m_w3dAssetsLoaded = false;
  for (int i = 0; i < NUM_MOUSE_CURSORS; i++) {
    m_cursorModels[i] = nullptr;
    m_cursorAnims[i] = nullptr;
  }
}

StdMouse::~StdMouse(void) {
  freeW3DAssets();
}

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

void StdMouse::initW3DAssets() {
  if (m_w3dAssetsLoaded || !W3DDisplay::m_assetManager) return;

  for (int i = 1; i < NUM_MOUSE_CURSORS; i++) {
    if (!m_cursorInfo[i].W3DModelName.isEmpty()) {
      if (m_orthoCamera) {
        m_cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale * m_orthoZoom, 0);
      } else {
        m_cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale, 0);
      }
      if (m_cursorModels[i]) {
        m_cursorModels[i]->Set_Position(Vector3(0.0f, 0.0f, -1.0f));
      }
    }
  }

  for (int i = 1; i < NUM_MOUSE_CURSORS; i++) {
    if (!m_cursorInfo[i].W3DAnimName.isEmpty()) {
      m_cursorAnims[i] = W3DDisplay::m_assetManager->Get_HAnim(m_cursorInfo[i].W3DAnimName.str());
      if (m_cursorAnims[i] && m_cursorModels[i]) {
        m_cursorModels[i]->Set_Animation(m_cursorAnims[i], 0, (m_cursorInfo[i].loop) ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE);
      }
    }
  }

  m_w3dCamera = new CameraClass();
  m_w3dCamera->Set_Position(Vector3(0, 1, 1));
  Vector2 min = Vector2(-1, -1);
  Vector2 max = Vector2(+1, +1);
  m_w3dCamera->Set_View_Plane(min, max);
  m_w3dCamera->Set_Clip_Planes(0.995f, 20.0f);
  if (m_orthoCamera) {
    m_w3dCamera->Set_Projection_Type(CameraClass::ORTHO);
  }

  m_w3dAssetsLoaded = true;
}

void StdMouse::freeW3DAssets() {
  for (int i = 0; i < NUM_MOUSE_CURSORS; i++) {
    if (W3DDisplay::m_3DInterfaceScene && m_cursorModels[i]) {
      W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(m_cursorModels[i]);
    }
    REF_PTR_RELEASE(m_cursorModels[i]);
    REF_PTR_RELEASE(m_cursorAnims[i]);
  }
  REF_PTR_RELEASE(m_w3dCamera);
  m_w3dAssetsLoaded = false;
  m_currentW3DCursor = NONE;
}

void StdMouse::setRedrawMode(RedrawMode mode) {
  MouseCursor cursor = getMouseCursor();
  setCursor(NONE);
  m_currentRedrawMode = mode;
  if (mode == RM_W3D) {
    initW3DAssets();
  } else {
    freeW3DAssets();
  }
  setCursor(cursor);
}

void StdMouse::setCursor(MouseCursor cursor) {
  Mouse::setCursor(cursor);

  if (m_currentCursor == cursor && m_currentW3DCursor == cursor) {
    return;
  }

  if (m_currentRedrawMode == RM_W3D) {
    if (cursor != m_currentW3DCursor) {
      if (!m_w3dAssetsLoaded) {
        initW3DAssets();
      }
      if (m_currentW3DCursor != NONE && m_cursorModels[m_currentW3DCursor] && W3DDisplay::m_3DInterfaceScene) {
        W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(m_cursorModels[m_currentW3DCursor]);
      }
      m_currentW3DCursor = cursor;
      if (m_currentW3DCursor != NONE && m_cursorModels[m_currentW3DCursor] && W3DDisplay::m_3DInterfaceScene) {
        W3DDisplay::m_3DInterfaceScene->Add_Render_Object(m_cursorModels[m_currentW3DCursor]);
        if (m_cursorInfo[m_currentW3DCursor].loop == FALSE && m_cursorAnims[m_currentW3DCursor]) {
          m_cursorModels[m_currentW3DCursor]->Set_Animation(m_cursorAnims[m_currentW3DCursor], 0, RenderObjClass::ANIM_MODE_ONCE);
        }
      }
    } else {
      m_currentW3DCursor = cursor;
    }
  }

  m_currentCursor = cursor;

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
#if defined(__APPLE__) && defined(__OBJC__)
  @autoreleasepool {
    if (visible) {
      [NSCursor unhide];
    } else {
      [NSCursor hide];
    }
  }
#endif
}

void StdMouse::draw(void) {
  // NOTE: do NOT check m_visible here — that flag controls the OS cursor.
  // W3DMouse::draw() also doesn't check m_visible for DX8/POLYGON modes.

  setCursor(m_currentCursor);

  if (m_currentRedrawMode == RM_W3D) {
    if (W3DDisplay::m_3DInterfaceScene && m_w3dCamera && m_visible) {
      if (m_currentW3DCursor != NONE && m_cursorModels[m_currentW3DCursor]) {
        Real xPercent = (1.0f - (TheDisplay->getWidth() - m_currMouse.pos.x) / (Real)TheDisplay->getWidth());
        Real yPercent = ((TheDisplay->getHeight() - m_currMouse.pos.y) / (Real)TheDisplay->getHeight());

        Real x, y, z = -1.0f;

        if (m_orthoCamera) {
          x = xPercent * 2 - 1;
          y = yPercent * 2;
        } else {
          Real logX, logY;
          PixelScreenToW3DLogicalScreen(m_currMouse.pos.x, m_currMouse.pos.y, &logX, &logY, TheDisplay->getWidth(), TheDisplay->getHeight());

          Vector3 rayStart;
          Vector3 rayEnd;
          rayStart = m_w3dCamera->Get_Position();
          m_w3dCamera->Un_Project(rayEnd, Vector2(logX, logY));
          rayEnd -= rayStart;
          rayEnd.Normalize();
          rayEnd *= m_w3dCamera->Get_Depth();
          rayEnd += rayStart;

          x = Vector3::Find_X_At_Z(z, rayStart, rayEnd);
          y = Vector3::Find_Y_At_Z(z, rayStart, rayEnd);
        }

        Matrix3D tm(1);
        tm.Set_Translation(Vector3(x, y, z));
        Coord2D offset = {0, 0};
        if (TheInGameUI && TheInGameUI->isScrolling()) {
          offset = TheInGameUI->getScrollAmount();
          offset.normalize();
          Real theta = atan2(-offset.y, offset.x);
          theta -= (Real)M_PI / 2;
          tm.Rotate_Z(theta);
        }
        m_cursorModels[m_currentW3DCursor]->Set_Transform(tm);

        WW3D::Render(W3DDisplay::m_3DInterfaceScene, m_w3dCamera);
      }
    }
    
    drawCursorText();
    if (m_visible) {
      drawTooltip();
    }
    return;
  }

  // --- Full W3DMouse-equivalent cursor rendering (2D texture mode) ---
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
  // NOTE: CGAssociateMouseAndMouseCursorPosition(false) breaks input because
  // StdMouse uses absolute NSEvent coordinates, not mouse deltas.
  // To implement properly, need to switch to delta-based input first.
  // For now, just notify the base class.
  onCursorCaptured(TRUE);
}

void StdMouse::releaseCapture(void) {
  onCursorCaptured(FALSE);
}

void StdMouse::regainFocus() {
  Mouse::regainFocus();
}

void StdMouse::loseFocus() {
  Mouse::loseFocus();
}

UnsignedByte StdMouse::getMouseEvent(MouseIO *result, Bool flush) {
  // Match Win32Mouse::getMouseEvent behavior exactly:
  // When buffer is empty, return MOUSE_NONE without touching result.
  // This is critical because updateMouseData() increments index after this call,
  // and if we write zeroed button states into result, the engine processes them
  // as MBS_Up events which immediately kill any ongoing drag.

  if (m_nextGetIndex == m_nextFreeIndex) {
    return MOUSE_NONE;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextGetIndex];
  m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;

  // Zero everything first, same as Win32Mouse::translateEvent
  result->leftState = result->middleState = result->rightState = MBS_None;
  result->pos.x = result->pos.y = result->wheelPos = 0;
  result->time = ev.time;

  // Set position for all events
  result->pos.x = ev.x;
  result->pos.y = ev.y;

  switch (ev.type) {
  case MACOS_MOUSE_LBUTTON_DOWN:
    result->leftState = MBS_Down;
    break;
  case MACOS_MOUSE_LBUTTON_UP:
    result->leftState = MBS_Up;
    break;
  case MACOS_MOUSE_LBUTTON_DBLCLK:
    result->leftState = MBS_DoubleClick;
    break;
  case MACOS_MOUSE_RBUTTON_DOWN:
    result->rightState = MBS_Down;
    break;
  case MACOS_MOUSE_RBUTTON_UP:
    result->rightState = MBS_Up;
    break;
  case MACOS_MOUSE_RBUTTON_DBLCLK:
    result->rightState = MBS_DoubleClick;
    break;
  case MACOS_MOUSE_MBUTTON_DOWN:
    result->middleState = MBS_Down;
    break;
  case MACOS_MOUSE_MBUTTON_UP:
    result->middleState = MBS_Up;
    break;
  case MACOS_MOUSE_WHEEL:
    result->wheelPos = ev.wheelDelta;
    break;
  default:
    // MACOS_MOUSE_MOVE: just position, all button states stay MBS_None
    break;
  }

  return MOUSE_OK;
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
