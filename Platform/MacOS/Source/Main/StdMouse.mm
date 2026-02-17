#include "StdMouse.h"
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "always.h"

StdMouse::StdMouse(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

StdMouse::~StdMouse(void) {}

void StdMouse::init(void) { setVisibility(TRUE); }

void StdMouse::reset(void) {
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
  if (!m_visible)
    return;

  CursorInfo *info = &m_cursorInfo[m_currentCursor];
  if (info->imageName.isNotEmpty()) {
    const Image *img =
        TheMappedImageCollection->findImageByName(info->imageName);
    if (img) {
      Int w = img->getImageWidth();
      Int h = img->getImageHeight();
      Int x = m_currMouse.pos.x - info->hotSpotPosition.x;
      Int y = m_currMouse.pos.y - info->hotSpotPosition.y;

      TheDisplay->drawImage(img, x, y, x + w, y + h,
                            GameMakeColor(255, 255, 255, 255),
                            Display::DRAW_IMAGE_ALPHA);
    }
  }

  drawCursorText();
  drawTooltip();
}

void StdMouse::capture(void) {
  // macOS doesn't have a direct "capture" like Win32 SetCapture
  // but we can use CGAssociateMouseAndMouseCursorPosition or similar if needed
}

void StdMouse::releaseCapture(void) {
  // Reverse of capture
}

void StdMouse::regainFocus() {
  // macOS specific focus regain logic
}

void StdMouse::loseFocus() {
  // macOS specific focus loss logic
}

UnsignedByte StdMouse::getMouseEvent(MouseIO *result, Bool flush) {
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

  m_currMouse.pos.x = ev.x;
  m_currMouse.pos.y = ev.y;

  switch (ev.type) {
  case MACOS_MOUSE_LBUTTON_DOWN:
    result->leftState = MBS_Down;
    m_currMouse.leftState = MBS_Down;
    break;
  case MACOS_MOUSE_LBUTTON_UP:
    result->leftState = MBS_Up;
    m_currMouse.leftState = MBS_Up;
    break;
  case MACOS_MOUSE_RBUTTON_DOWN:
    result->rightState = MBS_Down;
    m_currMouse.rightState = MBS_Down;
    break;
  case MACOS_MOUSE_RBUTTON_UP:
    result->rightState = MBS_Up;
    m_currMouse.rightState = MBS_Up;
    break;
  case MACOS_MOUSE_MBUTTON_DOWN:
    result->middleState = MBS_Down;
    m_currMouse.middleState = MBS_Down;
    break;
  case MACOS_MOUSE_MBUTTON_UP:
    result->middleState = MBS_Up;
    m_currMouse.middleState = MBS_Up;
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
