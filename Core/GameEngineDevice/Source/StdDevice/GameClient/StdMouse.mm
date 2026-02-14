#include "always.h"
#include "StdDevice/GameClient/StdMouse.h"
#include "Common/GlobalData.h"


StdMouse::StdMouse(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

StdMouse::~StdMouse(void) {}

void StdMouse::init(void) { Mouse::init(); }

void StdMouse::reset(void) {
  m_nextFreeIndex = 0;
  m_nextGetIndex = 0;
}

void StdMouse::update(void) {
  // Event polling is now handled in Win32GameEngine::serviceWindowsOS
  // which calls addEvent. We just need to process our buffer here if needed.
}

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
    return 0;
  }

  MacOSMouseEvent &ev = m_eventBuffer[m_nextGetIndex];
  m_nextGetIndex = (m_nextGetIndex + 1) % MAX_EVENTS;

  memset(result, 0, sizeof(MouseIO));
  result->pos.x = ev.x;
  result->pos.y = ev.y;
  result->time = ev.time;
  result->wheelPos = ev.wheelDelta;

  result->leftState = MBS_None;
  result->rightState = MBS_None;
  result->middleState = MBS_None;

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
